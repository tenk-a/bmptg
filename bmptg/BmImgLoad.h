/**
 *	@file	BmImgLoad.h
 *	@brief	各種フォーマット(bmp,tga ..)のロード
 *	@author	Masashi Kitamura
 *	@date	2000,2007
 */

#ifndef BMIMGLOAD_H
#define BMIMGLOAD_H

typedef enum {
	BM_FMT_NON=0,
	BM_FMT_BMP,		///< bmp
	BM_FMT_TGA,		///< targa
	BM_FMT_BETA,	///< non-header raw
	BM_FMT_JPG,		///< jpeg
	BM_FMT_PNG,		///< peng
	BM_FMT_MY1,		///< ex...
	BM_FMT_MY2,		///< ...
	BM_FMT_MY3,		///< ...
	BM_FMT_MY4,		///< ...
	BM_FMT_MY5,		///< ...
	BM_FMT_MY6,		///< ...
	BM_FMT_MY7,		///< ...
	//BM_FMT_NUM
} BM_FMT;


int  bm_getHdr(const void *bm_data, unsigned dataBytes, int *w_p, int *h_p, int *bpp_p, int *clutNum_p);
int  bm_getClut(const void *bm_data, void *clut0, int num);
int  bm_read(const void *bm_data, unsigned dataSize, void *dst, int wb, int h, int bpp, void *clut, int dir);

unsigned*  bm_load32(const void *bm_data, unsigned sz, int *w_p, int *h_p, int* bpp_p);

#endif


