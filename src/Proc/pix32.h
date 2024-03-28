/**
 *
 */
#ifndef PIX32_H
#define PIX32_H


#ifndef PIX32_ARGB	// a,r,g,b結合＆分解マクロ

/// 8bit値のa,r,g,bを繋げて 32bit の色値にする
#define PIX32_ARGB(a,r,g,b)		((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)		((unsigned char)(argb))			///< argb値中の blueの値を取得
#define PIX32_GET_G(argb)		((unsigned char)((argb)>>8))	///< argb値中の greenの値を取得
#define PIX32_GET_R(argb)		((unsigned char)((argb)>>16))	///< argb値中の greenの値を取得
#define PIX32_GET_A(argb)		(((unsigned)(argb))>>24)		///< argb値中の alphaの値を取得

#endif

#ifdef __cplusplus
extern "C" {
#endif


void pix32_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH);
void pix32_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH);
	// src画像(横幅srcW)の矩形(rctW*rctH)をdst画像(横幅dstW)にコピー
	// 範囲チェックなどはしないので使い手側で調整のこと

// int pix32_getNukiRect(void *pix0, int xsz, int ysz, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);
//	/* 背景色以外が使っている矩形を求める. */

// int pix32_getAlpRect(void *pix0, int xsz, int ysz, int *x_p, int *y_p, int *w_p, int *h_p);
//	/* α!=0が使っている矩形を求める. */

//x int pix32_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p);
//x	/* グリッドサイズ gw*gh として、グリッドに合うように矩形サイズを調整 */


void pix32_blackAMskGen(unsigned *pix, unsigned w, unsigned h, unsigned sikiiA, unsigned bpp);

void pix32_merge(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rgbRate2);
void pix32_genCol2MaskAlp(unsigned *dst, const unsigned *src, int w, int h, unsigned rgb1, unsigned rgb2);
void pix32_mergeSrc2Alp(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rgbRate2);


#ifdef __cplusplus
}
#endif


#endif
