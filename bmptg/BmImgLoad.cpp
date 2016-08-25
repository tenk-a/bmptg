/**
 *  @file   BmImgLoad.h
 *  @brief  各種フォーマット(bmp,tga ..)のロード
 *  @author Masashi Kitamura
 *  @date   2000,2007
 */

#include "def.h"

#include "bmp_read.h"
#include "tga_read.h"
#include "pix32_colCnv.h"
#include "beta.h"
#include "BmImgLoad.h"


#if defined USE_JPG
#include "JpgDecoder.hpp"
#endif
#if defined USE_PNG
#include "PngDecoder.hpp"
#endif

#ifdef USE_MY_FMT
#include "my.h"
#endif



#include "mem_mac.h"

/** 画像のヘッダ情報の取得
 */
int  bm_getHdr(const void *bm_data, unsigned sz, int *w_p, int *h_p, int *bpp_p, int *clutNum_p)
{
    const uint8_t *s = (const uint8_t *)bm_data;

    if (PEEKmW(s) == BB('B','M')) {
        if (bmp_getHdr(s, w_p, h_p,bpp_p, clutNum_p))
            return BM_FMT_BMP;
  #ifdef MY_H
    } else if (MY_IS_FMT_MY6_HDR(s)) {
        if (my6_getHdr(s, w_p, h_p,bpp_p, clutNum_p))
            return BM_FMT_MY6;
  #endif
  #if defined USE_JPG
    } else if (JpgDecoder::isSupported(s)) {    // if (memcmp(s, "\xFF\xD8\xFF\xE0\x00\x10JFIF", 10) == 0)
        JpgDecoder  dec(s, sz);
        if (w_p)        *w_p       = dec.width();
        if (h_p)        *h_p       = dec.height();
        if (bpp_p)      *bpp_p     = dec.bpp();
        if (clutNum_p)  *clutNum_p = 0;
        return BM_FMT_JPG;
  #endif
  #if defined USE_PNG
    } else if (PngDecoder::isSupported(s)) {
        PngDecoder  dec(s, sz);
        if (w_p)        *w_p       = dec.width();
        if (h_p)        *h_p       = dec.height();
        if (bpp_p)      *bpp_p     = dec.bpp();
        if (clutNum_p)  *clutNum_p = 0;
        return BM_FMT_PNG;
  #endif
    } else {
      #ifdef MY_H
		int fmt = MY_bm_getHdr(bm_data, sz, w_p, h_p, bpp_p, clutNum_p);
		if (fmt)
			return fmt;
      #endif
        if (tga_getHdr(s, w_p, h_p,bpp_p, clutNum_p))
            return BM_FMT_TGA;
    }
    return BM_FMT_NON;
}


int  bm_read(const void *bm_data, unsigned dataSz, void *dst, int wb, int h, int bpp, void *clut, int dir)
{
    int fmt = bm_getHdr(bm_data, dataSz, NULL,NULL,NULL,NULL);
    int n   = 0;

    switch (fmt) {
    case BM_FMT_BMP: n = bmp_read(bm_data, dst, wb, h, bpp, clut, dir); break;
    case BM_FMT_TGA: n = tga_read(bm_data, dst, wb, h, bpp, clut, dir); break;
  #if defined USE_JPG
    case BM_FMT_JPG:
        {
            JpgDecoder dec(bm_data, dataSz);
            uint8_t* s = (uint8_t*)dec.read();
            if (s) {
				if (dec.bpp() == 8) {
					memcpy(clut, dec.clut(), sizeof(unsigned)*256);
				}
                n = beta_conv(dst, wb, h, bpp, s, dec.widthByte(), dec.bpp(), clut, dir, 0,0);
                free(s);
                if (bpp != 8) {
                	unsigned w = BYT2WID(wb,32);
                	pix32_swapARGB((unsigned*)dst, w, h, (PIX32_SWAPARGB)14);  // abgr -> argb
				}
            }
        }
        break;
  #endif
  #if defined USE_PNG
    case BM_FMT_PNG:
        {
            PngDecoder dec(bm_data, dataSz);
            if (dec.clutSize())
                dec.getClut((unsigned*)clut);
            if (dec.bpp() < 8)
                dec.setBigEndian();
            //if (bpp== 8)
            //  dec.toClutBpp8();
            //if (bpp == 32)    // αの有無でbppが24,32どちらになるかわかりにくいので、普通に展開後自前で行う.
            //  dec.toTrueColor();
            uint8_t* s = (uint8_t*)dec.read();
            if (s != 0) {
                if ((unsigned)wb == dec.widthByte() && (unsigned)bpp == dec.bpp()) {
                    memcpy(dst, s, wb * h);
                    n = 1;
                } else {
                    n = beta_conv(dst, wb, h, bpp, s, dec.widthByte(1), dec.bpp(), clut, dir, 0,0);
                }
                free(s);
            }
        }
        break;
  #endif

    default:
	  #ifdef MY_H
		n = MY_bm_read(bm_data, dataSz, dst, wb, h, bpp, clut, dir, fmt);
	  #endif
        break;
    }

    //if (dbgExLog_getSw()) printf("[%d]:%d\n", fmt, n);
    //if (dbgExLog_getSw()) for (int i = 0; i < 256; i++) printf("clut[%d]=%#x\n", i, ((uint32_t*)clut)[i]);

    if (n)
        return fmt;
    return BM_FMT_NON;
}




/** 32ビット色画像にしてロード.
 */
unsigned*  bm_load32(const void *bm_data, unsigned sz, int *w_p, int *h_p, int* bpp_p)
{
    int bpp     = 0;
    int clutNum = 0;
    int w       = 0;
    int h       = 0;
    if (bm_getHdr(bm_data, sz, &w, &h, &bpp, &clutNum) == BM_FMT_NON)
        return NULL;
    unsigned wb  = WID2BYT(w, bpp);
    unsigned*  buf = (unsigned*)calloc(1, 4 * wb * h);
    if (buf == NULL)
        return NULL;
    if (bm_read(bm_data, sz, buf, wb, h, 32, NULL, 0) == 0) {
        free(buf);
        return NULL;
    }
    *w_p = w;
    *h_p = h;
    *bpp_p = bpp;
    return buf;
}
