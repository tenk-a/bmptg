/**
 *	@file	bmp_wrt.h
 *	@brief	BMP 画像データ出力
 *	@author	Masashi Kitamura
 *	@date	199?
 */
#ifndef BMP_WRT_H
#define BMP_WRT_H

#ifdef __cplusplus
extern "C" {
#endif

/// bmp画像イメージの生成.
int  bmp_write( void *bmp_data, 							// 生成されたbmpイメージを格納するバッファ.
				int w, int h, int bpp,						// 出力の横幅,縦幅, bpp
				const void *src, int srcWb, int srcBpp,		// 入力のピクセル,横幅バイト数,bpp
				const void *clut, int flags_dir);			// 入力の clut, 各種フラグ指定.

/// bmp画像イメージの生成.
int  bmp_writeEx(void *bmp_data,							// 生成されたbmpイメージを格納するバッファ.
				int w, int h, int dstBpp,					// 出力の横幅,縦幅, bpp
				const void* src, int srcWb, int srcBpp,		// 入力のピクセル,横幅バイト数,縦幅,bpp
				const void *clut, int clutSize,				// 入力の clut と、clutの色数,
				int   flags,								// 各種フラグ指定.
				int   resolX, int resolY);					// 出力のヘッダに入れる ピクセル/メートル の 横方向、縦方向.

				// フラグ指定.
				//	bit


/// 指定bppを、それを満たす実際に bmp でサポートされる bpp に変換.
int  bmp_chkDstBpp(int bpp);

/// w,h,bppからbmpイメージに必要なバイト数を返す(大目に返す).
int  bmp_encodeWorkSize(int w, int h, int bpp);


#ifdef __cplusplus
}
#endif




// ===========================================================================
// (上記関数の使用例)

// inline が指定できる場合.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)

// 予め stdlib.h をincludeしているときのみ利用可能.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) || (defined _STDLIB_H_INCLUDED/*watcom*/)
#include <stdlib.h>		// calloc,freeのため.

/** 指定した画像を、mallocしたメモリにbmp画像にして返す. 失敗するとNULL.
 */
static inline void* bmp_writeMalloc(
		int w, int h, int dstBpp,					// 出力の横幅,縦幅, bpp
		const void* src, int srcWidByt, int srcBpp,	// 入力のピクセル,横幅バイト数,縦幅,bpp  ※ srcWidBytは0だとwとbppからジャストの値を求める.
		const void *clut, 							// 入力の clut と
		int dir,									// 入力のピクセルが 0:上ラインから 1:下ラインから詰められている.
		unsigned* pSize)							// 生成したサイズ
{
	unsigned    dbpp = (dstBpp > 0) ? dstBpp : srcBpp;
	unsigned	bpp  = bmp_chkDstBpp(dbpp);
	unsigned 	sz   = bmp_encodeWorkSize(w, h, bpp);
	void		*m	 = calloc(1, sz);
	if (m == NULL)
		return NULL;

	sz = bmp_write(m, w, h, bpp, src, srcWidByt, srcBpp, clut, dir);
	if (sz == 0) {
		free(m);
		m = NULL;
	}
	if (pSize)
		*pSize = sz;
	return m;
}
#endif


// 予め stdio.h をincludeしているときのみ利用可能.
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_/*GCC*/) || (defined _STDIO_H_INCLUDED/*watcom*/)
#include <stdio.h>

#if defined __cplusplus
static int bmp_write_file(const char *fname, int w, int h, int dstBpp, const void* src, int srcWidByt=0, int srcBpp=0, const void* clut=0, int dir=0);
#endif

/// bmpファイル生成.
static inline int bmp_write_file(
	const char* fname,							// 出力ファイル名
	int w, int h, int dstBpp,					// 出力の横幅,縦幅, bpp
	const void* src, int srcWidByt, int srcBpp,	// 入力のピクセル,横幅バイト数,縦幅,bpp  ※ srcWidBytは0だとwとbppからジャストの値を求める.
	const void *clut, 							// 入力の clut
	int dir)									// 入力のピクセルが 0:上ラインから 1:下ラインから詰められている.
{
	size_t   l  = (unsigned)-1;
	unsigned sz = 0;
	void*	 m  = bmp_writeMalloc(w,h,dstBpp, src, srcWidByt, srcBpp, clut, dir, &sz);
	if (m) {
		if (sz) {
			FILE* fp = fopen(fname, "wb");
			if (fp) {
				l = fwrite(m, 1, sz, fp);
				fclose(fp);
			}
		}
		free(m);
	}
	return l == (size_t)sz;
}
#endif

#endif	// inlineが使える場合.



#endif	// BMP_WRT_H
