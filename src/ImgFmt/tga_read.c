/**
 *  @file   tga_read.c
 *  @brief  tga画像入力.
 *  @author Masashi Kitamura
 * @note
 *  ※  入出力で色数や画像サイズの違う圧縮データの展開に malloc&freeを使用
 *  2001-01-30  tgaの16ビット色のαの扱いがポカしてたのを(0x80でなく0xFFに)修正.
 */


#include "tga_read.h"
#include <string.h>
#include <assert.h>



/*---------------------------------------------------------------------------*/
/* コンパイル環境の辻褄あわせ. */


#if (defined _MSC_VER) || (defined __BORLANDC__ && __BORLANDC__ <= 0x0551)
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned       uint32_t;
#else
#include <stdint.h>
#endif


#if !defined(inline) && !(defined __GNUC__) && (__STDC_VERSION__ < 199901L)
#define inline          __inline
#endif


// このファイル内のみでの malloc/free の調整.
#ifdef _WINDOWS         // winアプリ(iftga.spi)で、なるべく Cライブラリをリンクしたくない場合用.
#include <windows.h>
#define MALLOC(sz)          LocalAlloc(LMEM_FIXED, (sz))
#define FREE(p)             LocalFree(p)
#define MEM_CPY(d,s,sz)     CopyMemory((d),(s),(sz))
#define MEM_CLR(m,sz)       ZeroMemory((m), (sz))
#else
#include <stdlib.h>     /* NULL, malloc, free を使用 */
#define MALLOC(sz)          malloc(sz)
#define FREE(p)             free(p)
#define MEM_CPY(d,s,sz)     memcpy((d),(s),(sz))
#define MEM_CLR(m,sz)       memset((m),0,(sz))
#endif




/*---------------------------------------------------------------------------*/

#define BB(a,b)         ((((uint8_t)(a))<<8)+(uint8_t)(b))
#define BBBB(a,b,c,d)   ((((uint8_t)(a))<<24)+(((uint8_t)(b))<<16)+(((uint8_t)(c))<<8)+((uint8_t)(d)))

/* バイト単位のアドレスから値を取り出すメモリアクセス用マクロ */
#define PEEKB(a)        (*(const unsigned char  *)(a))

#if defined _M_IX86 || defined _X86_ || defined _M_AMD64 || defined __amd64__   // X86 は、アライメントを気にする必要がないので直接アクセス.
#define PEEKiW(a)       (*(const unsigned short *)(a))
#define PEEKiD(a)       (*(const unsigned long  *)(a))
#else
#define PEEKiW(a)       ( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define PEEKiD(a)       ( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#endif

#define PEEKmW(a)       ( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define PEEKmD(a)       ( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )

#ifdef __SC__           // DM-C v8.41での-j0のバグ対策で, マクロ内にマクロを書かないように修正.
#define PEEKW(a)        (*(const unsigned short *)(a))
#define PEEKD(a)        (*(const unsigned long  *)(a))
#elif defined(BIG_ENDIAN)
#define PEEKW(a)        PEEKmW(a)
#define PEEKD(a)        PEEKmD(a)
#else /* LITTLE_ENDIAN */
#define PEEKW(a)        PEEKiW(a)
#define PEEKD(a)        PEEKiD(a)
#endif

#define BPP2BYT(bpp)    (((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)
#define WID2BYT(w,bpp)  (((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define WID2BYT4(w,bpp) ((WID2BYT(w,bpp) + 3) & ~3)
#define BYT2WID(w,bpp)  (((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))



/*---------------------------------------------------------------------------*/

/** tgaヘッダ情報 */
typedef struct tga_hdr_t {
  #if 1 //
    unsigned char   exHdrSz;        /**< ヘッダ後のピクセルデータまでのバイト数? */
    unsigned char   colorMapFlag;   /**< カラーマップの有無?(0=なし,1=あり)      */
  #else
    unsigned short  exHdrSz;        /**< ヘッダ後のピクセルデータまでのバイト数? */
  #endif
    char            id;             /**< フォーマットの種類. 0,1,2,3,9,10,11 */
    unsigned short  clutNo;         /**< 色パレットの開始番号 */
    unsigned short  clutNum;        /**< 色パレットの個数 */
    char            clutBit;        /**< 色パレットのビット数 15,16,24,32 */
    short           x0;             /**< 始点x */
    short           y0;             /**< 始点y 画面左下が原点(0,0)なので注意！ */
    short           w;              /**< 横幅 */
    short           h;              /**< 縦幅 */
    char            bpp;            /**< ピクセルのビット数  8,16,24,32 */
    char            mode;           /**< bit 3..0(4bits) : 属性. 0:αなし. 1:未定義のため無視可能. 2:未定義だが保存すべき<br>
                                     *   3:通常のα 4:予め乗算されたα. 8:詳細不明. <br>
                                     *   (bpp=32のとき:0x08, 24のとき0x00, 16のとき0x01 8のとき0x08,だった)<br>
                                     *   bit 5,4         : ピクセルの格納順は  0:左下から 1:左上 2:右下  3:右上
                                     */
} tga_hdr_t;
                                    /* 一応 tga_hdr_t はヘッダ定義そのままを構造体化してるが  */
                                    /* (char=1byte, short=2byte, int=4byte)                   */
                                    /* 構造体パティングの問題があるので、この構造体でtga デー */
                                    /* タの先頭をキャストしないこと                           */



/*---------------------------------------------------------------------------*/
static int  tga_getRawHdr(tga_hdr_t *t, const void *tga_data);
static int  tga_getPixs(uint8_t *dst, int dst_wb, int dstBpp, const uint8_t *src, int w, int h, int srcBpp, uint32_t *clut, int dir);
static void tga_read_decode(uint8_t *dst, const uint8_t *s, int w, int h, int bpp);
static void tga_getPixs_fast_8(uint8_t *dst, const uint8_t *s, int w, int h, int id, int dir, int ofs);
static void tga_getPixs_fast(uint8_t *dst, const uint8_t *s, int w, int h, int bpp, int id, int dir);


/** 生のtgaのヘッダ情報を取得
 */
static int  tga_getRawHdr(tga_hdr_t *t, const void *tga_data)
{
    uint8_t *buf = (uint8_t*)tga_data;

  #if 1
    t->exHdrSz      = PEEKB(buf);
    t->colorMapFlag = PEEKB(buf+1);
  #else
    t->exHdrSz  = PEEKiW(buf);
  #endif
    t->id       = PEEKB( buf+2);
    t->clutNo   = PEEKiW(buf+3);
    t->clutNum  = PEEKiW(buf+5);
    t->clutBit  = PEEKB( buf+7);
    t->x0       = PEEKiW(buf + 0x08);
    t->y0       = PEEKiW(buf + 0x0a);
    t->w        = PEEKiW(buf + 0x0c);
    t->h        = PEEKiW(buf + 0x0e);
    t->bpp      = PEEKB( buf + 0x10);
    t->mode     = PEEKB( buf + 0x11);
  #ifdef dbgExLog_getSw
    if (dbgExLog_getSw()) printf("tga clut(%d,%d,%d) %d,%d %d*%d %dbpp mode=%d\n",
        t->clutNo, t->clutNum, t->clutBit, t->x0, t->y0, t->w, t->h, t->bpp, t->mode);
  #endif
    return 1;
}



/** ヘッダより画像サイズと色数を得る
 */
int tga_getHdr(const void *tga_data, int *wp, int *hp, int *bppp, int *clutNump)
{
    return tga_getHdrEx(tga_data,wp,hp,bppp,clutNump,NULL);
}



/** ヘッダより画像サイズと色数を得る. αの有無も返す.
 */
int tga_getHdrEx(const void *tga_data, int *wp, int *hp, int *bppp, int *clutNump, int* alpModep)
{
    tga_hdr_t       tg;
    tga_hdr_t*      t = &tg;
    int             alpMode = 0;

    if (wp)         *wp         = 0;
    if (hp)         *hp         = 0;
    if (clutNump)   *clutNump   = 0;
    if (bppp)       *bppp       = -1;
    if (alpModep)   *alpModep   = 0;

    /* tga のヘッダ生情報を取得 */
    tga_getRawHdr(t, tga_data);

    /* id チェック */
    switch (t->id) {
    case 0:             /* イメージ無し */
    case 1:             /* clutあり. 無圧縮 */
    case 8+1:           /* clutあり. 圧縮 */
        switch (t->clutBit) {
        case 16:
        case 32:
            alpMode = 1;
            break;
        case 15:
        case 24:
            break;
        default:        /* 知らない色数 */
            return 0;
        }
        break;

    case 2:             /* 多色. 無圧縮 */
    case 3:             /* 白黒. 無圧縮 */
    case 8+2:           /* 多色. 圧縮   */
    case 8+3:           /* 白黒. 圧縮   */
        if (t->clutBit || t->clutNum)
            return 0;
        break;

    default:            /* しらないID */
        return 0;
    }

    /* ピクセルサイズチェック */
    switch (t->bpp) {
    case  8:
    /*case 15:*/
    case 24:
        break;

    case 16:
    case 32:
        alpMode = 1;
        break;

    default:
        return 0;
    }

    /* 画像サイズチェック。ひょっとしたら、定義上は unsigned かもだが実質の都合を考えれば */
    if (t->w <= 0 || t->h <= 0) {
        if (t->id == 0 && t->w == 0 && t->h == 0) {
            /* 画像無しデータの場合はエラーにしない */
        } else {
            return 0;
        }
    }

    if (wp)         *wp       = t->w;
    if (hp)         *hp       = t->h;
    if (clutNump)   *clutNump = ((t->id&3) == 1) ? t->clutNum : 0;
    if (bppp)       *bppp     = t->bpp;
    if (alpModep)   *alpModep = alpMode;

    return 1;
}



/** clut の取得. tga_data中にclutがnum個無かったときのために予め初期化しておくこと.
 *  @return  0:引数が不正  1:取得.  2:取得(αを持つパレットだった)
 */
int tga_getClut(
        const void  *tga_data,  ///< clutこみの tga データ.
        void        *clut0,     ///< CLUTを格納するアドレス. unsigned は32bitで num個分のメモリがあること.
        int         num)        ///< 今回取得留守パレットの個数 最大 256.
{
    tga_hdr_t   tg;
    tga_hdr_t   *t      = &tg;
    uint32_t    *clut   = (uint32_t*)clut0;
    uint8_t     *s;
    int         i;
    int         c;
    int         top;
    int         r;
    int         g;
    int         b;
    int         a       = 0xff;
    int         alpChk  = -1;
    int         haveAlp = 0;

    if (clut == NULL || tga_data == NULL || num == 0)
        return 0;

    tga_getRawHdr(t, tga_data);
  #if 1 //
    s       = (uint8_t*)tga_data + 0x12 + t->exHdrSz;
  #endif

    if (t->id == 1 || t->id == 8+1) {   /* パレット付きのとき */
        /* この関数は256色までだが、フォーマット的には 256色以上のパレットも可能みたいなのでガード */
        top = t->clutNo;
        num = (t->clutNum < num || num < 0) ? t->clutNum : num;
        if (top + num > 256) {
            if (num >= 256) {
                top = 0;
                num = 256;
            } else {
                top = 256 - num;
            }
        }
        switch (t->clutBit) {
        case 15:
            for (i = top; i < num; i++) {
                c = PEEKiW(s);
                b = ((c      ) & 0x1f) << 3;
                g = ((c >>  5) & 0x1f) << 3;
                r = ((c >> 10) & 0x1f) << 3;
                b |= b >> 5;
                g |= g >> 5;
                r |= r >> 5;
                clut[i] = BBBB(a, r,g,b);
                s += 2;
            }
            break;

        case 16:
            for (i = top; i < num; i++) {
                c = PEEKiW(s);
                b = ((c      ) & 0x1f) << 3;
                g = ((c >>  5) & 0x1f) << 3;
                r = ((c >> 10) & 0x1f) << 3;
                a = (c & 0x8000) ? 0xFF : 0;
                if (alpChk < 0)
                    alpChk = a;
                else if (alpChk != a)
                    haveAlp = 1;
                b |= b >> 5;
                g |= g >> 5;
                r |= r >> 5;
                clut[i] = BBBB(a, r, g, b);
                s += 2;
            }
            break;

        case 24:
            for (i = top; i < num; i++) {
                clut[i] = BBBB(a, s[2], s[1], s[0]);
                s += 3;
            }
            break;

        case 32:
            for (i = top; i < num; i++) {
                a = s[3];
                clut[i] = BBBB(a, s[2], s[1], s[0]);
                if (alpChk < 0)
                    alpChk = a;
                else if (alpChk != a)
                    haveAlp = 1;
                s += 4;
            }
            break;
        }
        return 1 + haveAlp;

    } else if (t->id == 3 || t->id == 8+3) {    /* モノクロのとき */
        for (i = 0; i < 256; i++)
            clut[i] = BBBB(a, i, i, i);
        return 1;

    } else {                                    /* パレットなしのとき .. 使わないはずだが適当に設定しておく. */
        i = 0;
        for (g = 0; g < 8; ++g) {
            for (r = 0; r < 8; ++r) {
                for (b = 0; b < 4; ++b) {
                    int gg = (g << 5) | (g << 2) | (g >> 1);
                    int rr = (r << 5) | (r << 2) | (r >> 1);
                    int bb = (b << 6) | (b << 4) | (b << 2) | b;
                    clut[i++] = BBBB(a, rr,gg,bb);
                }
            }
        }
        return 1;
    }
}



/** tga_dataから画像をbppビット色で dst にサイズ wb*h でコピー。
 *  bpp=8ならばclutも取得.※bpp8未満は未対応. clutは必ず256分あること.
 * @param ofs_dir bit0: 0=ピクセルは左上から  1=左下から.
 *                bit15..8: 256色のときの、ピクセル値オフセット
 */
int  tga_read(const void *tga_data, void *dst, int wb, int h, int bpp, void *clut, int ofs_dir)
{
    uint32_t    clut0[256];
    tga_hdr_t   tg;
    tga_hdr_t   *t = &tg;
    uint8_t     *s;
    uint8_t     *m = NULL;
    int         n;
    int         w4;
    int         dir = (uint8_t)ofs_dir;
    int         ofs = (uint8_t)(ofs_dir>>8);

    assert(tga_data && wb > 0 && h > 0 && bpp >= 0);

    if (tga_data == NULL || wb <= 0 || h <= 0 || bpp < 0)
        return 0;

    /* ヘッダ情報取得 */
    tga_getRawHdr(t, tga_data);
    if (bpp == 0)
        bpp = (t->bpp <= 8) ? 8 : (t->bpp <= 16) ? 32 : (t->bpp <= 24) ? 24 : 32;

    if (clut == 0) {
        clut = clut0;
    }
    // 256色画なら、clut取得しとく.
    if (t->bpp <= 8) {  /* 256色ならばclutを取得... */
        for (n = 0; n < 256; ++n)
            ((uint32_t*)clut)[n] = 0x00000000;  //x 0xFF000000;
        tga_getClut(tga_data, clut, -1);
    }

    if (t->w == 0 || t->h == 0)
        return 1;

    /* ピクセル位置取得 */
    n   = (((t->clutBit) > 24) ? 4 : ((t->clutBit) > 16) ? 3 : ((t->clutBit) > 8) ? 2 : ((t->clutBit) > 0) ? 1 : 0);    /*n = BPP2BYT(t->clutBit); */
  #if 1 //
    s   = (uint8_t *)tga_data + 0x12 + t->exHdrSz + t->clutNum*n;
  #else
    s   = (uint8_t *)tga_data + 0x12 + t->clutNum*n;
  #endif

    // 2007-05 Issiki氏の修正版から逆輸入. どうも bit配置を間違っていた模様.
    //x n   = (t->mode >> 4) ^ 1;
    // TGA 規格では、 ((t->mode)>>4)&0x3 が,
    //      0:左下始点 1:右下始点 2:左上始点 3:右上始点.
    // で、bit0が左右反転, bit2が上下反転、とxyの配置を勘違いしていた模様.
    n   = t->mode >> 4;
    n   = ((n&1)<<1) | ((n&2)>>1);      // 上下と左右のビットの並びを交換.
    n   ^= 1;                           // tga_read関数仕様のため、上下反転.
    // n= 0:左上始点 1:左下始点 2:右上始点 3:右下始点 //
    // これで、read_tga() の dir の定義に一致 //


    dir = (dir ^ n) & 3;

    h   = (h) ? h : t->h;
    w4  = WID2BYT4(t->w, bpp);
    wb  = (wb) ? wb : w4;

  #if 1
    /* 高速化のための専用ルーチン。ここはなくても動く */
    if (WID2BYT(t->w,bpp) == wb && t->h == h && (dir & 2) == 0) {
        if (bpp == 8 && t->bpp == 8) {  /* 入出力が256色かつ画像サイズが同一で横反転がないときの専用のコンバート */
            tga_getPixs_fast_8((uint8_t*)dst, s, t->w, h, t->id&8, dir, ofs);
            return 1;
        } else if (bpp == t->bpp) {     /* 入出力が 同一色かつ画像サイズが同一で横反転がないときの専用のコンバート */
            tga_getPixs_fast((uint8_t*)dst, s, t->w, h, bpp, t->id&8, dir);
            return 1;
        }
    }
  #endif
    h   = (h >= t->h) ? h : t->h;
    if (t->id & 0x8) {  /* 圧縮されてた */
        unsigned l;
        n = WID2BYT4(t->w, t->bpp);
        l = n * t->h + 10;
        m = (uint8_t*)MALLOC(l);            /*展開用バッファを malloc で確保 */
        if (m == NULL)
            return 0;
        MEM_CLR(m, l);
        tga_read_decode(m, s, t->w, t->h, t->bpp);
        s = m;
    }
    tga_getPixs((uint8_t*)dst, wb, bpp, s, t->w, h, t->bpp, (uint32_t*)clut, BB(ofs,dir));
    if (m)
        FREE(m);
    return 1;
}



/** 入出力が 同一色かつ画像サイズが同一のとき専用のコンバート */
static void tga_getPixs_fast(uint8_t *dst, const uint8_t *s, int w, int h, int bpp, int id, int dir)
{
    uint8_t *d = (uint8_t*)dst;
    uint8_t *e;
    int     i;
    int     l;
    int     c;
    int     wb;

    wb = WID2BYT(w, bpp);
    if (id == 0) {          /* 無圧縮 */
        if (dir == 0) {     /* 正方向 */
            i= wb * h;
            MEM_CPY(d, s, i);
        } else {            /* 逆方向 */
            d += wb * (h - 1);
            for (i = h; --i >= 0;) {
                MEM_CPY(d, s, wb);
                s += wb;
                d -= wb;
            };
        }
    } else {                /* 圧縮データ */
        tga_read_decode(d, s, w, h, bpp);
        if (dir) {
            d = (uint8_t*)dst;
            e = (uint8_t*)dst + wb * (h - 1);
            for (i = 0; i < h/2; i++) {
                for (l = 0; l < wb; l++) {
                    c = *d;
                    *d++ = *e;
                    *e++ = c;
                }
                e -= wb * 2;
            }
        }
    }
}



/** 入出力が 256色かつ画像サイズが同一のとき専用のコンバート */
static void tga_getPixs_fast_8(uint8_t *dst, const uint8_t *s, int w, int h, int id, int dir, int ofs)
{
    uint8_t *d = (uint8_t*)dst;
    uint8_t *e = (uint8_t*)dst + w*h;
    int     i;
    int     l;
    int     c;

    if (id == 0) {          /* 無圧縮 */
        if (dir == 0) {     /* 正方向 */
            if (ofs == 0) {
                i = w * h;
                MEM_CPY(d, s, i);
            } else {
                while (d < e) {
                    *d++ = *s++ + ofs;
                }
            }
        } else {            /* 逆方向 */
            d += w * (h - 1);
            if (ofs == 0) {
                for (i = h; --i >= 0;) {
                    MEM_CPY(d, s, w);
                    s += w;
                    d -= w;
                }
            } else {
                for (i = h; --i >= 0;) {
                    for (l = 0; l < w; l++) {
                        *d++ = *s++ + ofs;
                    }
                    d -= w*2;
                }
            }
        }
    } else {                /* 圧縮 */
        while (d < e) {
            l = *s++;
            if (l & 0x80) {
                l = (l & 0x7f) + 1;
                c = *s++ + ofs;
                do {
                    *d++ = c;
                } while (--l);
            } else {
                l = (l + 1) * 1;
                if (d + l > e)
                    l = (int)(e - d);
                while (l--) {
                    *d++ = *s++ + ofs;
                }
            }
        }
        if (dir) {          /* 逆方向 */
            d = (uint8_t*)dst;
            e = (uint8_t*)dst + w * (h - 1);
            for (i = 0; i < h/2; i++) {
                for (l = 0; l < w; l++) {
                    c = *d;
                    *d++ = *e;
                    *e++ = c;
                }
                e -= w * 2;
            }
        }
    }
}



static inline int tga_getPix32(const uint8_t *s, int x, int bpp, uint32_t *clut, int ofs)
{
    int     c;
    int     r,g,b,a;

    if (bpp <= 8) {
        c = clut[s[x] + ofs];
    } else if (bpp <= 16) {
        s += x * 2;
        c  = (s[1]<<8) | s[0];
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x1f) << 3;
        r  = ((c >> 10) & 0x1f) << 3;
        a  = (c & 0x8000) ? 0xFF : 0;
        c  = BBBB(a, r,g,b);
    } else if (bpp <= 24) {
        s += x * 3;
        c = BBBB(0xff, s[2], s[1], s[0]);
    } else {
        s += x * 4;
        c = s[3];
        c = BBBB(c, s[2], s[1], s[0]);
    }
    return c;
}



/* 多色の 256色化は G3R3B2 形式への簡易変換.
 */
static inline int tga_getPix8(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
    int     c;
    int     r,g,b;

    dmy_clut;

    if (bpp <= 8) {
        return s[x] + ofs;
    } else if (bpp <= 16) {
        s += x * 2;
        c  = s[1]<<8 | s[0];
        b  = ((c      ) & 0x1f) << 3;
        g  = ((c >>  5) & 0x1f) << 3;
        r  = ((c >> 10) & 0x1f) << 3;
        return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);
    } else if (bpp <= 24) {
        s += x * 3;
        return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
    } else {
        s += x * 4;
        return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
    }
}



static inline uint8_t *tga_putPix32(uint8_t *d, int c, int bpp)
{
    int     r,g,b,a;

    if (bpp <= 8) {
        *d  = c;
        return d + 1;
   #if 0
    } else if (bpp <= 15) {
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
        *(uint16_t*)d = c;
        return d + 2;
  #endif
    } else if (bpp <= 16) {
        a = ((uint32_t)c >> 24) ? 0x8000 : 0;
        r = (uint8_t)(c >> 16);
        g = (uint8_t)(c >>  8);
        b = (uint8_t)(c >>  0);
        c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
        *(uint16_t*)d = c;
        return d + 2;
    } else if (bpp <= 24) {
      #ifdef BIG_ENDIAN
        d[0] = (uint8_t)(c>>16);
        d[1] = (uint8_t)(c>>8);
        d[2] = (uint8_t)c;
      #else
        d[0] = (uint8_t)c;
        d[1] = (uint8_t)(c>>8);
        d[2] = (uint8_t)(c>>16);
      #endif
        return d + 3;
    } else {
        *(uint32_t*)d = c;
        return d + 4;
    }
}



/** srcBpp色 w*hドットのsrc 画像を、dstBpp色 w*h の dst画像に変換する.
 * 出力はbmp向けに 0パティングする.
 */
static int tga_getPixs(
        uint8_t*        dst,
        int             dst_wb,
        int             dstBpp,
        const uint8_t*  src,
        int             w,
        int             h,
        int             srcBpp,
        uint32_t*       clut,
        int             ofs_dir)    ///< bit0:ピクセル順は0=上から 1=下から   bit1:0=左から 1=右から.
{
    uint8_t*        d = dst;
    const uint8_t*  s = src;
    int             dir = (uint8_t)ofs_dir;
    int             ofs = (uint8_t)(ofs_dir>>8);
    int             c;
    int             dpat;
    int             spat;
    int             sw;
    int             dw;
    int             x,y;
    int             x0,y0;
    int             x1,y1;
    int             xd,yd;

    if (dst == NULL || src == NULL || w == 0 || h == 0)
        return 0;

    sw   = WID2BYT(w, srcBpp);
    dw   = WID2BYT(w, dstBpp);
    spat = sw;
    dpat = dst_wb - dw;
    if (dpat < 0) {
        w    = BYT2WID(dst_wb, dstBpp);
        dpat = dst_wb - WID2BYT(w, dstBpp);
    }

    /* e = s + sw * h; */
    //DBG_F(("@>%d[%d] %d*%d[%d] dir=%d ofs=%d\n", dst_wb, dstBpp, w, h, srcBpp, dir, ofs));
    //if (dbgExLog_getSw()) for (int i = 0; i < 256; i++) printf("clut[%d]=%#x\n", i, ((uint32_t*)clut)[i]);

    /* 画像の向きの調整 */
    if (dir & 1) {
        y0   = h-1, y1 = -1, yd = -1;
        d    = d + y0 * dst_wb;
        dpat = dpat - dst_wb * 2;
    } else {
        y0   = 0 , y1 = h , yd = +1;
    }
    if (dir & 2) {
        x0  = w-1, x1 = -1, xd = -1;
    } else {
        x0  = 0  , x1 = w , xd = +1;
    }
    /*DBG_F(("[%d,%d,%d,%d,%d,%d, %d %d]\n", x0,y0,x1,y1,xd,yd,dpat,spat)); */

    /* 画像コピー */
    if (dstBpp > 8) {   /* 出力が多色のとき */
        for (y = y0; y != y1; y += yd) {
            for (x = x0; x != x1; x += xd) {
                c = tga_getPix32(s, x, srcBpp, clut,ofs);
                d = tga_putPix32(d, c, dstBpp);
            }
            d += dpat;
            s += spat;
        }
    } else {            /* 出力が256色のとき */
        for (y = y0; y != y1; y += yd) {
            for (x = x0; x != x1; x += xd) {
                c = tga_getPix8(s, x, srcBpp, clut,ofs);
                *d++ = c;
            }
            d += dpat;
            s += spat;
        }
    }
    return 1;
}



/** ランレングス圧縮されたデータの展開. dst側は必ずアライメントされていること.
 * ピクセル値については後で処理するので, ARGBかBGRAかは気にしなくてよい
 */
static void tga_read_decode(uint8_t *dst, const uint8_t *s, int w, int h, int bpp)
{
    int         c;
    int         l;
    int         n = BPP2BYT(bpp);
    uint8_t     *d = dst;
    uint8_t     *e = d + w * h * n;

    switch (n) {
    case 1:
        while (d < e) {
            l = *s++;
            if (l & 0x80) {
                l = (l & 0x7f) + 1;
                c = *s++;
                do {
                    *d++ = c;
                } while (--l);
            } else {
                l = (l + 1);
                /*if (d + l > e) */
                /*  l = (int)(e - d); */
                do {
                    *d++ = *s++;
                } while (--l);
            }
        }
        break;
    case 2:
        while (d < e) {
            l = *s++;
            if (l & 0x80) {
                l = (l & 0x7f) + 1;
                c = PEEKW(s);
                s += 2;
                do {
                    *(uint16_t*)d = c;  /*POKEW(d,c); */
                    d+=2;
                } while (--l);
            } else {
                l = (l + 1);
                do {
                    c = PEEKW(s);
                    s+=2;
                    *(uint16_t*)d = c;  /*POKEW(d,c); */
                    d+=2;
                } while (--l);
            }
        }
        break;
    case 3:
        while (d < e) {
            l = *s++;
            if (l & 0x80) {
                l = (l & 0x7f) + 1;
                do {
                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d += 3;
                } while (--l);
                s += 3;
            } else {
                l = (l + 1);
                do {
                    d[0] = s[0];
                    d[1] = s[1];
                    d[2] = s[2];
                    d += 3;
                    s += 3;
                } while (--l);
            }
        }
        break;
    default:
        while (d < e) {
            l = *s++;
            if (l & 0x80) {
                l = (l & 0x7f) + 1;
                c = PEEKD(s);
                s += 4;
                do {
                    *(uint32_t*)d = c;
                    d+=4;
                } while (--l);
            } else {
                l = (l + 1);
                do {
                    c = PEEKD(s);
                    s+=4;
                    *(uint32_t*)d = c;
                    d+=4;
                } while (--l);
            }
        }
        break;
    }
}

