/**
 *  @file	tga_read.h
 *  @brief	メモリ上のtga画像を展開する.
 *	@author	Masashi Kitamura
 * @note
 *  ※  入出力で色数や画像サイズの違う圧縮データの展開に malloc&freeを使用
 */
#ifndef TGA_READ_H
#define TGA_READ_H


#if defined __cplusplus
extern "C" {
#endif


/// メモリ上のtgaデータから、横幅、縦幅、ビット/ピクセル、clutの色数を取得する.
int	 tga_getHdr(const void *tga_data, int *w_p, int *h_p, int *bpp_p, int *clutSize_p);
int	tga_getHdrEx(const void *tga_data, int *wp, int *hp, int *bppp, int *clutNump, int* alpModep);

/// メモリ上のtgaデータから、ピクセルデータ、clutを取得する. (サイズは予めtga_getHdrで取得して用意のこと)
int  tga_read(const void *tga_data, void *dst, int widthByte, int h, int bpp, void *clut, int dir);

/// メモリ上のtgaデータから、clut のみをclutSize個まで取得する. 1色はA8R8G8B8.
int  tga_getClut(const void *tga_data, void *clut, int clutSize);


#if defined __cplusplus
/// tga_getHdrで取得した横幅,bppより横幅バイト数を計算する.
inline int  tga_width2byte(int w, int bpp, int algnByte) { return (w * ((bpp+7)>>3) + algnByte-1) & ~algnByte; }
#else
#define tga_width2byte(w,bpp,algnByte)		((((w) * (((bpp)+7)>>3)) + (algnByte)-1) & ~(algnByte))
#endif


#if defined __cplusplus
}
#endif




// ===========================================================================
// (上記関数の使用例)

// inline が指定できる場合.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)
#include <assert.h>

// 予め stdlib.h をincludeしているときのみ利用可能.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) \
	|| (defined _STDLIB_H_INCLUDED/*watcom*/) || (defined _MSL_STDLIB_H/*CW*/)
#include <stdlib.h>		// calloc,freeのため.

#if defined __cplusplus
static inline void* tga_readMalloc(const void *d, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=0, int dir=0);
#endif

/** tga_dataより画像を展開、mallocしたメモリに入れて返す.
 *  成功したら *w_p,*h_p,*bpp_p,clutに横幅,縦幅,bpp,clut(4*256)が入る. ※clutはclut付画像の時のみ.
 *  dstBpp は生成される画像のbppを設定. 0なら画像自身のbpp. 		   ※色数が減る設定は単純処理で対処.
 *  algn は横幅バイト数のアライメント. 1,2,4,8,16を想定. 通常は1, win-bitmap用なら4を設定のこと.
 *  dir は 0なら上ラインから、1なら下ラインから取得.  win-bitmap用なら1を設定のこと.
 */
static inline void* tga_readMalloc(const void *tga_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
	int		w, h, bpp, csz, wb;
	void	*m = NULL;

	assert(tga_data && dstBpp >= 0);
	assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);

	if (tga_getHdr(tga_data,&w,&h,&bpp,&csz) == 0)
		return NULL;

	if (dstBpp <= 0)
		dstBpp = bpp;

	wb = tga_width2byte(w, dstBpp, algn);
	m  = calloc(1, wb*h);

	if (tga_read(tga_data, m, wb, h, dstBpp, clut, dir) == 0) {
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
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_/*GCC*/) \
	|| (defined _STDIO_H_INCLUDED/*watcom*/) || (defined _MSL_STDIO_H/*CW*/)
#include <stdio.h>

#if defined __cplusplus
static void* tga_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=1, int dir=0);
#endif

static inline void* tga_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
	void*	pix = NULL;
	FILE*	fp  = fopen(fname, "rb");
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
					pix  = tga_readMalloc(dat, w_p, h_p, bpp_p, clut, dstBpp, algn, dir);
				free(dat);
			}
		}
		fclose(fp);
	}
	return pix;

}
#endif

#endif	// inlineが使える場合.



#endif	// TGA_READ_H
