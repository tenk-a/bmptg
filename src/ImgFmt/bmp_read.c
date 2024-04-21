/**
 *  @file   bmp_read.c
 *  @brief  BMP 画像データ入力
 *  @author Masashi Kitamura
 *  @date   199?
 *  @note
 *  2001-04     bmp-rle関係追加
 */


#include "bmp_read.h"
#include <stdlib.h>     /* NULL,malloc,free のためにinclude */
#include <assert.h>


/*---------------------------------------------------------------------------*/
/* コンパイル環境の辻褄あわせ. */

#if (defined _MSC_VER) || (defined __BORLANDC__ && __BORLANDC__ <= 0x0551)
typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned        uint32_t;
#else
#include <stdint.h>
#endif


#if !defined(inline) && !(defined __GNUC__) && (__STDC_VERSION__ < 199901L)
#define inline          __inline
#endif



/* ------------------------------------------------------------------------ */

#define BB(a,b)         ((((uint8_t)(a))<<8)+(uint8_t)(b))
#define BBBB(a,b,c,d)   ((((uint8_t)(a))<<24)+(((uint8_t)(b))<<16)+(((uint8_t)(c))<<8)+((uint8_t)(d)))
#define PEEKB(a)        (*(const uint8_t *)(a))

#if defined _M_IX86 || defined _X86_ || defined _M_AMD64 || defined __amd64__   // X86 は、アライメントを気にする必要がないので直接アクセス
#define PEEKiW(a)       (*(const uint16_t *)(a))
#define PEEKiD(a)       (*(const uint32_t *)(a))
#else
#define PEEKiW(a)       ( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define PEEKiD(a)       ( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#endif

#define PEEKmW(a)       ( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )

#define ABS(a)          ((a) < 0 ? -(a) : (a))      /* 絶対値 */

#define MEMSET4(d, s, sz)   do {int  *d__ = (int *)(d); int c__ = (unsigned)(sz)>>2; do { *d__++ = (int) (s); } while(--c__); } while (0)
#define MEMCPY4(d, s, sz)   do {int  *d__ = (int *)(d); const int *s__ = (const int *)(s); int c__ = (unsigned)(sz)>>2; do { *d__++ = *s__++; } while (--c__); } while (0)

#define WID2BYT(w,bpp)  (((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define WID2BYT4(w,bpp) ((WID2BYT(w,bpp) + 3) & ~3)
#define BYT2WID(w,bpp)  (((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

#undef  DBG_F
#define DBG_F(x)




/*---------------------------------------------------------------------------*/

/// bmpヘッダ情報
typedef struct bmp_hdr_t {
    uint16_t    id;
    uint32_t    fsize;
    uint32_t    rsv1;
    uint32_t    pdatOfs;

    uint32_t    hdrSz;
    int32_t     w;
    int32_t     h;
    uint16_t    pln;
    uint16_t    bpp;
    uint32_t    cmplMode;
    uint32_t    cmplDatSz;
    uint32_t    xresol;
    uint32_t    yresol;
    uint32_t    clutNum/*palCnt*/;
    uint32_t    impColCnt;
    uint32_t    mskR,mskG,mskB,mskA;

    /*- ヘッダでない情報 */
    int         lenR,lenG,lenB,lenA;
    int         shlR,shlG,shlB,shlA;
    int         shrR,shrG,shrB,shrA;
    uint32_t    *clut;
    uint8_t     *pix;
} bmp_hdr_t;
                                    /* 一応 bmp_hdr_t はヘッダ定義そのままを構造体化してるが  */
                                    /* (char=1byte, short=2byte, int=4byte)                   */
                                    /* 構造体パティングの問題があるので、この構造体でbmp デー */
                                    /* タの先頭をキャストしないこと                           */

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3
//#define BI_JPEG       4
//#define BI_PNG        5


/*---------------------------------------------------------------------------*/
static int bmp_getPixs(uint8_t *dst, int dst_wb, int dstBpp, const uint8_t *src, int w, int h, int srcBpp, uint32_t *clut, int ofs_dir);
static int bmp_decodeRle4(uint8_t *dst, const uint8_t *src, int w, int h);
static int bmp_decodeRle8(uint8_t *dst, const uint8_t *src, int w, int h);

static bmp_hdr_t *bmp_hdr;


static void genShVal(int msk, int *len, int *shl, int *shr)
{
    int r,l;

    if (msk == 0) {
        *shl = *shr = 0;
        return;
    }
    for (r = 0; r < 32; r++) {
        if (msk & (1<<r)) {
            break;
        }
    }
    for (l = r; l < 32; l++) {
        if ((msk & (1<<l)) == 0) {
            break;
        }
    }
    l -= r;
    if (l > 8) {
        r = r + (l-8);
        l = 8;
    }
    *len = l;
    *shr = r;
    *shl = 8-l;
}



static int  bmp_getRawHdr(bmp_hdr_t *t, const void *bmp_data)
{
    uint8_t *b = (uint8_t*)bmp_data;
    uint8_t *b0;
    int     rc =  0;

    MEMSET4(t, 0, sizeof(*t));
    t->id       = PEEKmW(b);
    if (t->id == BB('B','M')) {             /* BMPファイルの場合。リソースの場合はこの2文字がない模様 */
        b += 2;
        rc = 1;
    }
    b0 = b;
    t->fsize    = PEEKiD(b); b += 4;        /* ファイルサイズ */
    t->rsv1     = PEEKiD(b); b += 4;        /* 予約 */
    t->pdatOfs  = PEEKiD(b); b += 4;        /* ピクセル・データへのオフセット */
    t->hdrSz    = PEEKiD(b); b += 4;        /* ヘッダサイズ */
    if (t->hdrSz == 12) {                   /* OS/2ヘッダ   */
        t->w    = PEEKiW(b); b += 2;
        t->h    = PEEKiW(b); b += 2;
        t->pln  = PEEKiW(b); b += 2;        /* プレーン数. must be 1 */
        t->bpp  = PEEKiW(b); /*b += 2;*/    /* bits per pixel */
        t->cmplMode = 0;
    } else {                                /* ms-win */
        t->w    = PEEKiD(b); b += 4;
        t->h    = PEEKiD(b); b += 4;
        t->pln  = PEEKiW(b); b += 2;        /* プレーン数. must be 1 */
        t->bpp  = PEEKiW(b); b += 2;        /* bits per pixel */
        t->cmplMode  = PEEKiD(b); b += 4;   /* 圧縮モード */
        t->cmplDatSz = PEEKiD(b); b += 4;   /* 圧縮データサイズ */
        t->xresol    = PEEKiD(b); b += 4;   /* x resolution */
        t->yresol    = PEEKiD(b); b += 4;   /* y resolution */
        t->clutNum   = PEEKiD(b); b += 4;   /* clutの数. 2,4,8ビット色ではこの値に関係なく1<<n色ある. 16,24,32ビット色のときは気をつけよ */
        t->impColCnt = PEEKiD(b); b += 4;   /* 重要な色の数……普通無視するデータ */
        if (t->hdrSz >= 40 + 4 * 4) {
            t->mskR  = PEEKiD(b); b += 4;
            t->mskG  = PEEKiD(b); b += 4;
            t->mskB  = PEEKiD(b); b += 4;
            t->mskA  = PEEKiD(b); /*b += 4;*/
        } else {
            if (t->bpp == 16) {
                t->mskA = 0x8000, t->mskR = 0x7C00, t->mskG = 0x03E0, t->mskB = 0x001F;
            } else {
                t->mskA = 0xFF000000, t->mskR = 0xFF000, t->mskG = 0x00FF00, t->mskB = 0x0000FF;
            }
        }
        genShVal(t->mskR, &t->lenR, &t->shlR, &t->shrR);
        genShVal(t->mskG, &t->lenG, &t->shlG, &t->shrG);
        genShVal(t->mskB, &t->lenB, &t->shlB, &t->shrB);
        genShVal(t->mskA, &t->lenA, &t->shlA, &t->shrA);
    }
    if (t->bpp <= 8) {
        t->clutNum = 1 << t->bpp;
    }
    t->clut = (uint32_t*)((b0-2) + 2 + 12 + t->hdrSz);
    t->pix  = (uint8_t*)bmp_data + t->pdatOfs;
    //DBG_F(("bpp=%d, w,h=%d,%d, cm=%x, rsl=%d,%d cnum=%d\n", t->bpp, t->w, t->h, t->cmplMode, t->xresol, t->yresol,t->clutNum));
    return rc;
}



/** bmp_getHdrで取得した横幅,bppより横幅バイト数を計算する.
 */
int bmp_width2byte(int w, int bpp, int algn)
{
    int wb = WID2BYT(w, bpp);
    assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);
    wb = (wb + algn - 1) & ~algn;
    return wb;
}



/** メモリ上のbmpデータから、横幅、縦幅、ビット/ピクセル、clutの色数を取得する.
 */
int bmp_getHdr(const void *bmp_data, int *wp, int *hp, int *bppp, int *clutNump)
{
    return bmp_getHdrEx(bmp_data, wp,hp,bppp,clutNump,NULL,NULL);
}




/** メモリ上のbmpデータから、横幅、縦幅、ビット/ピクセル、clutの色数を取得する.
 */
int bmp_getHdrEx(const void *bmp_data, int *wp, int *hp, int *bppp, int *clutNump, int *resolXp, int *resolYp)
{
    bmp_hdr_t   tg;
    bmp_hdr_t*  t = &tg;

    if (wp)         *wp         = 0;
    if (hp)         *hp         = 0;
    if (clutNump)   *clutNump   = 0;
    if (bppp)       *bppp       = -1;
    if (resolXp)    *resolXp    = 0;
    if (resolYp)    *resolYp    = 0;

    // tga のヘッダ生情報を取得
    bmp_getRawHdr(t, bmp_data);

  #if 1
    if (t->id != BB('B','M'))
        return 0;
  #endif
    if (t->pln != 1)
        return 0;

    // ピクセルサイズチェック
    switch (t->bpp) {
    case  1:
    case  2:            // ce ?
    case  4:
    case  8:
    //case 12: case 15: // かって解釈
    case 16:
    case 24:
    case 32:
        break;
    default:
        return 0;
    }

    // 画像サイズチェック。ひょっとしたら、定義上は unsigned かもだが実質の都合を考えれば
    if (t->w <= 0 || t->h == 0) {
        if (t->id == 0 && t->w == 0 && t->h == 0) {
            // 画像無しデータの場合はエラーにしない
        } else {
            return 0;
        }
    }

    if (wp)         *wp = t->w;
    if (hp)         *hp = t->h;
    if (clutNump)   *clutNump = (t->bpp <= 8) ? (1<<t->bpp) : 0;
    if (bppp)       *bppp = t->bpp;
    if (resolXp)    *resolXp = t->xresol;
    if (resolYp)    *resolYp = t->yresol;

    return 1;
}



/** clut 取得 */
int bmp_getClut(
    const void  *bmp_data,  ///< clutこみの tga データ
    void        *clut0,     ///< CLUTを格納するアドレス. unsigned は32bitで num個分のメモリがあること
    int         num)        ///< clutの個数
{
    bmp_hdr_t   tg;
    bmp_hdr_t*  t = &tg;
    uint32_t    *clut = (uint32_t*)clut0;
    uint8_t     *s;
    int         i;
    int         n;

    if (clut == NULL || bmp_data == NULL || num == 0)
        return 0;

    bmp_getRawHdr(t, bmp_data);

    if (t->bpp <= 8) {          // パレット付きのとき
        n = 1 << t->bpp;
        n = (n < num || num < 0) ? n : num;
        s = (uint8_t*)t->clut;
        for (i = 0; i < n; i++) {
            clut[i] = BBBB(s[3], s[2], s[1], s[0]);
            s += 4;
        }
        return 1;
    }
    return 0;
}


/** デフォルトパレットを作成 */
static void bmp_getDfltClut(int bpp, uint32_t *clut)
{
    int     i;
    int     c;
    int     r,g,b;

    if (clut == NULL)
        return;
    if (bpp == 1) {         // 黒白
        clut[0] = 0;
        clut[1] = 0xFFFFFF;
    } else if (bpp == 4) {  // 単色濃淡
        for (i = 0; i < 16; i++) {
            c = (i << 4) | i;
            clut[i] = BBBB(0, c,c,c);
        }
    } else {                // TOWNSや88VA系の G3R3B2
        i = 0;
        for (g = 0; g < 8; ++g) {
            for (r = 0; r < 8; ++r) {
                for (b = 0; b < 4; ++b) {
                    int gg = (g << 5) | (g << 2) | (g >> 1);
                    int rr = (r << 5) | (r << 2) | (r >> 1);
                    int bb = (b << 6) | (b << 4) | (b << 2) | b;
                    clut[i++] = BBBB(0, rr,gg,bb);
                }
            }
        }
    }
}


/** bmp_dataから画像をbppビット色で dst にサイズ wb*h でコピー。 bpp=8ならばclutも取得.
 * ofs_dir  bit0: 0=ピクセルは左上から  1=ピクセルは左下から.
 *          bit4: 0=データのclutを取得する  1=取得しない(引数のclutを用いる)
 *          bit15..8: 256色のときの、ピクセル値オフセット
 */
int  bmp_read(const void *bmp_data, void *dst, int wb, int h, int bpp, void *clut, int ofs_dir)
{
    bmp_hdr_t   tg;
    bmp_hdr_t*  t   = &tg;
    uint8_t*    s;
    uint8_t*    m   = NULL;
    uint32_t    clutBuf[256];
    int         i;
    int         n;
    int         dir;
    int         ofs;
    int         th;

    bmp_hdr = t;

    dir     = (uint8_t)ofs_dir;
    ofs     = (uint8_t)(ofs_dir>>8);

    if (bmp_data == NULL)
        return 0;

    // ヘッダ情報取得
    bmp_getRawHdr(t, bmp_data);

    if (bpp == 0)
        bpp = t->bpp;

    if (clut == NULL || (dir & 0x10) || bpp > 8) {  //多色出力やclutの取得をしないときの、ダミーclut設定
        clut = clutBuf;
        MEMSET4(clut, 0, 256*4);
        bmp_getDfltClut(bpp, (uint32_t*)clut);
    }

    if (t->bpp <= 8) {              // 入力が 256色以下ならば、パレットを取得
        if (bpp < t->bpp) {         // 出力のbppが入力より小さければ 出力分のパレットを取得
            n = 1 << bpp;
            s = (uint8_t*)t->clut+(ofs*sizeof(uint32_t));
        } else {
            n = 1 << t->bpp;
            s = (uint8_t*)t->clut;
        }
        for (i = 0; i < n; i++) {
            ((uint32_t*)clut)[i] = BBBB(s[3], s[2], s[1], s[0]);
            //DBG_F(("\t-%3d %06x\n", i, ((uint32_t*)clut)[i]));
            s += 4;
        }
    } else if (bpp <= 8) {          // 出力が 256色以下ならば、デフォルトclutを取得
        bmp_getDfltClut(bpp, (uint32_t*)clut);
    }

    if (t->w == 0 || t->h == 0)
        return 0;

    // ピクセル位置取得
    s   = t->pix;

    th = t->h;
    if (th >= 0) {
        dir ^= 1;
    } else {
        th = -th;
    }

    dir &= 1;
    wb  = (wb) ? wb : WID2BYT4(t->w, bpp);
    h   = (h) ? h : th;
    if (h < th) {
        th = h;
    }

    if (t->cmplMode == 0) {
        if (t->bpp == 16)
            t->bpp = 15;    // A1R5G5B5 として処理

    } else {
        // 圧縮されていた. RLE4,RLE8 のみ対応
        if (t->cmplMode <= BI_RLE4 && t->bpp == 4) {    // RLE4 16色画圧縮
            m = (uint8_t*)malloc((t->w+4) * th);
            if (m == NULL)
                return 0;
            bmp_decodeRle4((uint8_t*)m, s, t->w, th);
            t->bpp = 8;
            s = m;
        } else if (t->cmplMode <= BI_RLE4 && t->bpp == 8) { // RLE8 256色画圧縮
            m = (uint8_t*)malloc((t->w+4) * th);
            if (m == NULL)
                return 0;
            bmp_decodeRle8((uint8_t*)m, s, t->w, th);
            s = m;
        } else if (t->bpp > 8 && t->cmplMode == BI_BITFIELDS) {
            if (t->bpp == 24 && t->mskR == 0xFF0000 && t->mskG == 0x00FF00 && t->mskB == 0x0000FF)
                ;
            else if (t->bpp == 32 && t->mskR == 0xFF0000 && t->mskG == 0x00FF00 && t->mskB == 0x0000FF)
                ;
            else if (t->bpp == 16 && t->mskA == 0 && t->mskR == 0x7C00 && t->mskG == 0x03E0 && t->mskB == 0x001F)
                t->bpp = 14;    // A0R5G5B5 を便宜上 14ビット色として処理
            else if (t->bpp == 16 && t->mskA == 0x8000 && t->mskR == 0x7C00 && t->mskG == 0x03E0 && t->mskB == 0x001F)
                t->bpp = 15;    // A1R5G5B5 を便宜上 15ビット色として処理
            else if (t->bpp == 16 && t->mskR == 0xF800 && t->mskG == 0x07E0 && t->mskB == 0x001F)
                t->bpp = 16;    // R5G6B5 を16ビット色として処理
            else if (t->bpp == 16 && t->mskA == 0xF000 && t->mskR == 0x0F00 && t->mskG == 0x00F0 && t->mskB == 0x000F)
                t->bpp = 12;
            else if (t->bpp <= 16)
                t->bpp = 11;    // マスクデータを用いて展開
            else if (t->bpp <= 24)
                t->bpp = 23;    // マスクデータを用いて展開
            else
                t->bpp = 31;    // マスクデータを用いて展開
        } else {
            return 0;
        }
    }

    bmp_getPixs((uint8_t*)dst, wb, bpp, s, t->w, th, t->bpp, (uint32_t*)clut, BB(ofs,dir));

    if (m)
        free(m);
    return 1;
}



static inline uint8_t *bmp_putPix32(uint8_t *d, int c, int bpp)
{
    int     r,g,b,a;

    if (bpp <= 12) {
        a = (uint8_t)(c >> 24);
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = ((a >> 4) << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
        *(uint16_t*)d = c;
        return d + 2;

    } else if (bpp <= 14) {
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
        *(uint16_t*)d = c;
        return d + 2;

    } else if (bpp == 15) {
        a = (c >> 24) ? 0x8000 : 0;
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
        *(uint16_t*)d = c;
        return d + 2;

    } else if (bpp == 16) {
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
        *(uint16_t*)d = c;
        return d + 2;

    } else if (bpp <= 24) {
        d[0] = (uint8_t)c;
        d[1] = (uint8_t)(c>>8);
        d[2] = (uint8_t)(c>>16);
        return d + 3;

    } else {
        *(uint32_t*)d = c;
        return d + 4;
    }
}



static inline int bmp_getPix32(const uint8_t *s, int x, int bpp, uint32_t *clut, int ofs)
{
    int     c;
    int     r,g,b,a;

    if (bpp <= 1) {
        int n = x & 7;
        c = (s[x>>3] & (0x80 >> n)) >> (7-n);       //c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
        return clut[c + ofs];

    } else if (bpp <= 2) {
        int n = (x & 3)<<1;
        c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
        return clut[c];

    } else if (bpp <= 4) {
        c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;   //c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
        return clut[c + ofs];

    } else if (bpp <= 8) {
        return clut[s[x] + ofs];

    } else if (bpp <= 11) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        a  = ((c & t->mskA) >> t->shrA) << t->shlA;
        return  BBBB(a,r,g,b);

    } else if (bpp <= 12) { // a4r4g4b4
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c     ) & 0xf); b |= b << 4;
        g  = ((c >> 4) & 0xf); g |= g << 4;
        r  = ((c >> 8) & 0xf); r |= r << 4;
        a  = ((c >>12) & 0xf); a |= a << 4;
        return  BBBB(a,r,g,b);

    } else if (bpp <= 14) { // r5g5b5
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c      ) & 0x1f) << 3; b |= b >> 5;
        g  = ((c >>  5) & 0x1f) << 3; g |= g >> 5;
        r  = ((c >> 10) & 0x1f) << 3; r |= r >> 5;
        return  BBBB(0, r,g,b);

    } else if (bpp <= 15) { // a1r5g5b5
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c      ) & 0x1f) << 3; b |= b >> 5;
        g  = ((c >>  5) & 0x1f) << 3; g |= g >> 5;
        r  = ((c >> 10) & 0x1f) << 3; r |= r >> 5;
        a  = (c & 0x8000) ? 0xFF : 0;
        return  BBBB(a, r,g,b);

    } else if (bpp <= 16) { // r5g6b5
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c      ) & 0x1f) << 3; b |= b >> 5;
        g  = ((c >>  5) & 0x3f) << 2; g |= g >> 6;
        r  = ((c >> 11) & 0x1f) << 3; r |= r >> 5;
        return  BBBB(0, r,g,b);

    } else if (bpp <= 23) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 3;
        c  = BBBB(0, s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        a  = ((c & t->mskA) >> t->shrA) << t->shlA;
        return  BBBB(a,r,g,b);

    } else if (bpp <= 24) {
        s += x * 3;
        return BBBB(0, s[2], s[1], s[0]);

    } else if (bpp <= 31) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 4;
        c  = BBBB(s[3], s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        a  = ((c & t->mskA) >> t->shrA) << t->shlA;
        return  BBBB(a,r,g,b);

    } else {
        s += x * 4;
        return BBBB(s[3], s[2], s[1], s[0]);
    }
}



/** 多色の 256色化は G3R3B2 形式への簡易変換. */
static inline int bmp_getPix8(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
    int     c;
    int     r,g,b;

    dmy_clut;

    if (bpp <= 1) {
        int n = x & 7;
        c = (s[x>>3] & (0x80 >> n)) >> (7-n);       //c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
        return (uint8_t)(c + ofs);
    } else if (bpp <= 2) {
        int n = (x & 3)<<1;
        c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
        return (uint8_t)(c + ofs);
    } else if (bpp <= 4) {
        c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;   //c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
        return (uint8_t)(c + ofs);
    } else if (bpp <= 8) {
        return (uint8_t)(s[x] + ofs);

    } else if (bpp <= 11) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else if (bpp <= 12) {
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c      ) & 0xf) << 4;
        g  = ((c >>  4) & 0xf) << 4;
        r  = ((c >>  8) & 0xf) << 4;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else if (bpp <= 15) {
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x1f) << 3;
        r  = ((c >> 10) & 0x1f) << 3;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else if (bpp <= 16) {
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x3f) << 2;
        r  = ((c >> 11) & 0x1f) << 3;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else if (bpp <= 23) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 3;
        c  = BBBB(0, s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else if (bpp <= 24) {
        s += x * 3;
        return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);

    } else if (bpp <= 31) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 4;
        c  = BBBB(s[3], s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);

    } else {
        s += x * 4;
        return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
    }
}



static inline int bmp_getPix42(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
    // 多色の 16色化は モノクロへの簡易変換.
    int     c;
    int     r,g,b;
    int     m = (1<<bpp)-1;
    int     j = 12-bpp;

    dmy_clut;

    if (bpp <= 1) {
        //c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
        int n = x & 7;
        c = (s[x>>3] & (0x80 >> n)) >> (7-n);
        return (c + ofs) & m;
    } else if (bpp <= 2) {
        int n = (x & 3)<<1;
        c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
        return (c + ofs) & m;
    } else if (bpp <= 4) {
        c = (s[x>>1] >> ((((x&1)^1)<<2)));
        return (c + ofs) & m;
    } else if (bpp <= 8) {
        return (s[x] + ofs) & m;
    } else if (bpp <= 11) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else if (bpp <= 12) {
        s += x * 2;
        c  = PEEKiW(s);
        b  = ((c     ) & 0xf) << 4;
        g  = ((c >> 4) & 0xf) << 4;
        r  = ((c >> 8) & 0xf) << 4;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else if (bpp <= 15) {
        s += x * 2;
        c  = PEEKiW(s);
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x1f) << 3;
        r  = ((c >> 10) & 0x1f) << 3;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else if (bpp <= 16) {
        s += x * 2;
        c  = PEEKiW(s);
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x3f) << 2;
        r  = ((c >> 11) & 0x1f) << 3;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else if (bpp <= 23) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 3;
        c  = BBBB(0, s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else if (bpp <= 24) {
        s += x * 3;
        return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> j;
    } else if (bpp <= 31) { // マスク情報を用いる
        bmp_hdr_t *t = bmp_hdr;
        s += x * 4;
        c  = BBBB(s[3], s[2], s[1], s[0]);
        b  = ((c & t->mskB) >> t->shrB) << t->shlB;
        g  = ((c & t->mskG) >> t->shrG) << t->shlG;
        r  = ((c & t->mskR) >> t->shrR) << t->shlR;
        return (g * 9 + r * 5 + b * 2) >> j;
    } else {
        s += x * 4;
        return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> j;
    }
}



static int bmp_getPix1(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
    // 多色の 256色化は G3R3B2 形式への簡易変換.
    int c, n;

    dmy_clut;

    if (bpp <= 1) {
        n = x & 7;
        c = (s[x>>3] & (0x80 >> n)) >> (7-n);
        //c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
        return (c+ofs)&1;
    } else if (bpp <= 2) {
        int n = (x & 3)<<1;
        c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
        return (c + ofs)&1;
    } else if (bpp <= 4) {
        c = s[x>>1] >> ((((x&1)^1)<<2));
        return (c + ofs)&1;
    } else if (bpp <= 8) {
        return (s[x] + ofs) & 1;
    } else if (bpp <= 16) {
        s += x * 2;
        return (PEEKiW(s) > 0);
    } else if (bpp <= 24) {
        s += x * 3;
        return BBBB(0,s[2],s[1],s[0]) > 0;
    } else {
        s += x * 4;
        return (PEEKiD(s) > 0);
    }
}



/** srcBpp色 w*hドットのsrc 画像を、dstBpp色 w*h の dst画像に変換する.
 * 出力はbmp向けに 0パティングする
 * dir : bit0:ピクセル順は0=上から 1=下から   bit1:0=左から 1=右から
 */
static int bmp_getPixs(uint8_t *dst, int dst_wb, int dstBpp, const uint8_t *src, int w, int h, int srcBpp, uint32_t *clut, int ofs_dir)
{
    const uint8_t*  s;
    uint8_t*        d;
    int             dir = (uint8_t)ofs_dir;
    int             ofs = (uint8_t)(ofs_dir>>8);
    int             c;
    int             dpat;
    int             spat;
    int             x;
    int             y, y0, y1, yd;

    if (dst == NULL || src == NULL || w == 0 || h == 0 || dst_wb == 0)
        return 0;

    //if (h2 > h)   h2 -= h;
    //else      h  = h2, h2 = 0;
    spat = WID2BYT4(w, srcBpp);
    dpat = dst_wb - WID2BYT(w, dstBpp);
    if (dpat < 0) {
        w    = BYT2WID(dst_wb, dstBpp);
        dpat = dst_wb - WID2BYT(w, dstBpp);
    }
    DBG_F(("bmp_read spat=%d, dpat=%d, w=%d, dst_wb=%d, srcBpp=%d, dstBpp=%d\n", spat, dpat, w, dst_wb, srcBpp,dstBpp));

    // 画像の向きの調整
    d   = (uint8_t *)dst;
    s   = (const uint8_t *)src;
    y0  = 0, y1 = h, yd = +1;
    if (dir & 1) {
        y0  = h-1, y1 = -1, yd = -1;
      #if 0
        //s    = s + y0 * spat;
        //spat = -spat;
      #else
        d   = d + y0 * dst_wb;
        dpat = dpat - dst_wb * 2;
      #endif
    }
    DBG_F(("[%d, %d, %d, %d]\n", y0,y1,yd,dpat));

  #ifndef BIG_ENDIAN
    // 入出力が同じbppで変換等なく横幅4バイト単位であれば、素早くコピー
    if (dstBpp == srcBpp && ofs == 0 && (dst_wb & 3) == 0 && (ABS(spat)&3) == 0 && (ABS(dpat)&3) == 0) {
        for (y = y0; y != y1; y += yd) {
            MEMCPY4(d, s, spat);
            d += spat + dpat;
            s += spat;
        }
        return 1;
    }
  #endif

    // 画像コピー
    if (dstBpp > 8) {   // 出力が多色のとき
        for (y = y0; y != y1; y += yd) {
            for (x = 0; x < w; x ++) {
                c = bmp_getPix32(s, x, srcBpp, clut,ofs);
                d = bmp_putPix32(d, c, dstBpp);
            }
            d += dpat;
            s += spat;
        }
    } else if (dstBpp > 4) {    // 出力が256色のとき
        for (y = y0; y != y1; y += yd) {
            for (x = 0; x < w; x ++) {
                c = bmp_getPix8(s, x, srcBpp, clut,ofs);
                *d++ = c;
            }
            d += dpat;
            s += spat;
        }
    } else if (dstBpp > 2) {    // 出力が 16色のとき
        for (y = y0; y != y1; y += yd) {
            for (x = 0; x < w; x++) {
                c = (uint8_t)(bmp_getPix42(s, x, srcBpp, clut,ofs) << 4);
                if (++x < w)
                    c |= bmp_getPix42(s, x, srcBpp, clut,ofs) & 0x0F;
                *d++ = c;
            }
            d += dpat;
            s += spat;
        }
    } else if (dstBpp > 1) {    // 出力が 4色のとき
        for (y = y0; y != y1; y += yd) {
            for (x = 0; x < w; x++) {
                c = bmp_getPix42(s, x, srcBpp, clut,ofs) << 6;
                if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs) << 4;
                if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs) << 2;
                if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs);
                *d++ = c;
            }
            d += dpat;
            s += spat;
        }
    } else {    // 出力が 2色のとき
        for (y = y0; y != y1; y += yd) {
            for (x = 0; x < w; x ++) {
                c = bmp_getPix1(s, x, srcBpp, clut,ofs) << 7;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 6;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 5;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 4;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 3;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 2;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 1;
                if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs);
                *d++ = c;
            }
            d += dpat;
            s += spat;
        }
    }
    return 1;
}



/** rle8 で圧縮されたデータを解凍 */
static int bmp_decodeRle8(uint8_t *dst, const uint8_t *src, int w, int h)
{
    const uint8_t*  s;
    uint8_t*        d;
    uint8_t*        e;
    int             wb;
    int             l;
    int             c;
    int             x, y;
    int             u, v;

    if (dst == NULL || src == NULL || w == 0 || h == 0)
        return 0;

    wb = WID2BYT4(w, 8);        // dstの横幅は4バイト単位
    s = (const uint8_t*)src;
    d = (uint8_t *)dst;
    e = dst + h * wb;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w;) {
            l = *s++;
            if (l == 0x00) {
                c = *s++;
                if (c == 0x00) {        // 行末までスキップ
                    goto LINE_END;
                } else if (c == 0x01) { // 画像の終端だった
                    while (d < e)
                        *d++ = 0;
                    goto EXIT;
                } else if (c == 0x02) { // (+u,+v)分スキップ
                    u = *s++;
                    v = *s++;
                    l = v * wb + u;
                    do {
                        if (x >= wb) {
                            if (++y >= h)
                                goto EXIT;
                            x = 0;
                        }
                        *d++ = 0;
                        x++;
                    } while (--l);
                } else {
                    l = c;
                    do {
                        if (x >= w) {
                            while (x++ < wb)
                                *d++ = 0;
                            if (++y >= h)
                                goto EXIT;
                            x = 0;
                        }
                        *d++ = *s++;
                        x++;
                    } while (--l);
                    if (c & 1)  // パティングをスキップ
                        s++;
                }
            } else {        // c をl bytes書き込む
                c = *s++;
                do {
                    if (x >= w) {
                        while (x++ < wb)
                            *d++ = 0;
                        if (++y >= h)
                            goto EXIT;
                        x = 0;
                    }
                    *d++ = c;
                    x++;
                } while (--l);
            }
        }
        if (s[0] == 0 && s[1] == 0) // どうやら絶対に行末符号は入っている?
            s += 2;
      LINE_END:
        while (x++ < wb)
            *d++ = 0;
    }
  EXIT:
    return 1;
}



/** rle4 で圧縮されたデータを解凍. 8ビット色にして返す */
static int bmp_decodeRle4(uint8_t *dst, const uint8_t *src, int w, int h)
{
    const uint8_t*  s;
    uint8_t*        d;
    uint8_t*        e;
    int             wb;
    int             l;
    int             c;
    int             x,y;
    int             u,v;

    if (dst == NULL || src == NULL || w == 0 || h == 0)
        return 0;

    wb   = WID2BYT4(w, 8);      // dstの横幅は4バイト単位
    s = (const uint8_t*)src;
    d = (uint8_t *)dst;
    e = dst + h * wb;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w;) {
            l = *s++;
            if (l == 0x00) {
                c = *s++;
                if (c == 0x00) {        // 行末までスキップ
                    goto LINE_END;
                } else if (c == 0x01) { // 画像の終端だった
                    while (d < e)
                        *d++ = 0;
                    goto EXIT;
                } else if (c == 0x02) { // (+u,+v)分スキップ
                    u = *s++;
                    v = *s++;
                    l = v * wb + u;
                    do {
                        if (x >= wb) {
                            if (++y >= h)
                                goto EXIT;
                            x = 0;
                        }
                        *d++ = 0;
                        x++;
                    } while (--l);
                } else {
                    l = v = c;
                    do {
                        c    = *s++;
                        if (x >= w) {
                            while (x++ < wb)
                                *d++ = 0;
                            if (++y >= h)
                                goto EXIT;
                            x = 0;
                        }
                        *d++ = c >> 4;
                        x++;
                        if (--l == 0)
                            break;
                        if (x >= w) {
                            while (x++ < wb)
                                *d++ = 0;
                            if (++y >= h)
                                goto EXIT;
                            x = 0;
                        }
                        *d++ = c & 0xf;
                        x++;
                    } while (--l);
                    v =  (-v) & 3;
                    if (v & 2)  // パティングをスキップ
                        s++;
                }
            } else {        // c をl bytes書き込む
                c = *s++;
                u = c & 0x0f;
                c = c >> 4;
                do {
                    if (x >= w) {
                        while (x++ < wb)
                            *d++ = 0;
                        x = 0;
                        if (++y >= h)
                            goto EXIT;
                    }
                    *d++ = c;
                    x++;
                    if (--l == 0)
                        break;
                    if (x >= w) {
                        while (x++ < wb)
                            *d++ = 0;
                        if (++y >= h)
                            goto EXIT;
                        x = 0;
                    }
                    *d++ = u;
                    x++;
                } while (--l);
            }
        }
        if (s[0] == 0 && s[1] == 0)
            s += 2;
      LINE_END:
        while (x++ < wb)
            *d++ = 0;
    }
  EXIT:
    return 1;
}

