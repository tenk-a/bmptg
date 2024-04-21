/**
 *  @file   bmp_read.h
 *  @brief  メモリ上のbmp画像を展開する.
 *  @author Masashi Kitamura
 * @note
 *  ※  入出力で色数や画像サイズの違う圧縮データの展開に malloc&freeを使用
 */
#ifndef BMP_READ_H
#define BMP_READ_H


#if defined __cplusplus
extern "C" {
#endif


/// メモリ上のbmpデータから、横幅、縦幅、ビット/ピクセル、clutの色数を取得する.
int  bmp_getHdr(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, int *clutSize_p);

/// メモリ上のbmpデータから、横幅、縦幅、ビット/ピクセル、clutの色数、画像の比率、を取得する.
int bmp_getHdrEx(const void *bmp_data, int *wp, int *hp, int *bppp, int *clutNump, int *resolXp, int *resolYp);


/// メモリ上のbmpデータから、ピクセルデータ、clutを取得する. (サイズは予めbmp_getHdrで取得して用意のこと)
int  bmp_read(const void *bmp_data, void *dst, int widthByte, int h, int bpp, void *clut, int dir);

/// メモリ上のbmpデータから、clut のみをclutSize個まで取得する. 1色はA8R8G8B8.
int  bmp_getClut(const void *bmp_data, void *clut, int clutSize);


/// bmp_getHdrで取得した横幅,bppより横幅バイト数を計算する.
int  bmp_width2byte(int w, int bpp, int algnByte);


#if defined __cplusplus
};
#endif




// ===========================================================================
// (上記関数の使用例)

// inline が指定できる場合.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)
#include <assert.h>

// 予め stdlib.h をincludeしているときのみ利用可能.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) || (defined _STDLIB_H_INCLUDED/*watcom*/)
#include <stdlib.h>     // calloc,freeのため.

#if defined __cplusplus
static inline void* bmp_readMalloc(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=0, int dir=0);
#endif

/** bmp_dataより画像を展開、mallocしたメモリに入れて返す.
 *  成功したら *w_p,*h_p,*bpp_p,clutに横幅,縦幅,bpp,clut(4*256)が入る. ※clutはclut付画像の時のみ.
 *  dstBpp は生成される画像のbppを設定. 0なら画像自身のbpp.            ※色数が減る設定は単純処理で対処.
 *  algn は横幅バイト数のアライメント. 1,2,4,8,16を想定. 通常は1, win-bitmap用なら4を設定のこと.
 *  dir は 0なら上ラインから、1なら下ラインから取得.  win-bitmap用なら1を設定のこと.
 */
static inline void* bmp_readMalloc(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
    int     w, h, bpp, csz, wb;
    void    *m = NULL;

    assert(bmp_data && dstBpp >= 0);
    assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);

    if (bmp_getHdr(bmp_data,&w,&h,&bpp,&csz) == 0)
        return NULL;

    if (dstBpp <= 0)
        dstBpp = bpp;

    wb = bmp_width2byte(w, dstBpp, algn);
    m  = calloc(1, wb*h);

    if (bmp_read(bmp_data, m, wb, h, dstBpp, clut, dir) == 0) {
        free(m);
        return NULL;
    }

    if (w_p)   *w_p   = w;
    if (h_p)   *h_p   = h;
    if (bpp_p) *bpp_p = bpp;
    return m;
}
#endif


// 予め stdio.h をincludeしているときのみ利用可能.
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_) || (defined _STDIO_H_INCLUDED/*watcom*/)
#include <stdio.h>

#if defined __cplusplus
static void* bmp_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=1, int dir=0);
#endif

static inline void* bmp_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
    void*   pix = NULL;
    FILE*   fp  = fopen(fname, "rb");
    if (fp) {
        size_t l;
        fseek(fp, 0, SEEK_END);
        l = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (l) {
            void* dat = malloc(l);
            if (dat) {
                size_t sz = fread(dat, 1, l, fp);
                if (sz == l)
                    pix  = bmp_readMalloc(dat, w_p, h_p, bpp_p, clut, dstBpp, algn, dir);
                free(dat);
            }
        }
        fclose(fp);
    }
    return pix;

}
#endif

#endif  // inlineが使える場合.




#endif  // BMP_READ_H
