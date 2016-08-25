/**
 *	@file	BmImgSave.h
 *	@brief	各種フォーマット(bmp,tga ..)のセーブ
 *	@author	Masashi Kitamura
 *	@date	2000
 */

#ifndef BMIMGSAVE_H
#define BMIMGSAVE_H


/// 画像変換のオプション
typedef struct bm_opt_t {
	int x0,y0;
	int clutNum;
	int tx,ty;
	int srcW, srcH;
	int clutX,clutY, clutW, clutH;
	int clutBpp;
	int nukiCo,nukiCoI;
	int resolX, resolY;
	int celW, celH;
	int mapW, mapH;
	int	lvlY, lvlUV;
	int mapTexW, mapTexH;
	int quality;
	int quality_grey;
	int alpBitForBpp8;
	int mono;
} bm_opt_t;



int  bm_write(int fmt, void *bm_data, int w, int h, int bpp, const void *src, int srcWb, int srcBpp, const void *clut, int dir, const bm_opt_t *o);
int  bm_chkDstBpp(int fmt, int bpp);
int  bm_encodeWorkSize(int fmt, int w, int h, int bpp);

#endif
