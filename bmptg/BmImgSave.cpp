/**
 *  @file   BmImgSave.h
 *  @brief  各種フォーマット(bmp,tga ..)のセーブ
 *  @author Masashi Kitamura
 *  @date   2000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "subr.h"
#include "mem_mac.h"
#include "BmImgLoad.h"
#include "BmImgSave.h"

#include "beta.h"
#include "bmp_wrt.h"
#include "tga_wrt.h"
#include "pix32_colCnv.h"
#if defined USE_JPG
#include "JpgEncoder.hpp"
#endif
//#undef USE_PNG
#if defined USE_PNG
#include "PngEncoder.hpp"
#endif


#ifdef USE_MY_FMT
#include "my.h"
#endif


int  bm_write(int fmt, void *bm_data, int w, int h, int bpp, const void *src, int srcWb, int srcBpp, const void *clut, int dir_flags, const bm_opt_t *o)
{
    // bppが8ビット以下のときはsrcBpp=8, 多色画のときは srcBpp=32 であること
    int n = 0;

    switch (fmt) {
    case BM_FMT_NON:
        break;

    case BM_FMT_BMP:
        n = bmp_write(bm_data, w, h, bpp, src, srcWb, srcBpp, clut, dir_flags);
        break;

    case BM_FMT_TGA:
      #if 0
        n = tga_write(bm_data, w, h, bpp, src, srcWb, srcBpp, clut, dir_flags);
      #else
        {
            int clutBpp = o->clutBpp;
            if (dir_flags & 0x40)   // clutをα付にする.
                clutBpp = 32;
            n = tga_writeEx(bm_data, 0x7FFFFFFF, w, h, bpp, src, srcWb, srcBpp, clut, clutBpp, dir_flags, 0,0);
        }
      #endif
        break;

    case BM_FMT_BETA:   // ヘッダ無しの生データ
        n = beta_conv(bm_data, WID2BYT(w,bpp), h, bpp, src, srcWb, srcBpp, clut, dir_flags, 0,0);
        break;

  #if defined USE_JPG
    case BM_FMT_JPG:
		if (o->mono) {
            unsigned    wb  = WID2BYT(w,24);
            uint8_t*    tmp = (uint8_t*)callocE(1, wb * h);
            n = beta_conv(tmp, wb, h, 24, src, srcWb, srcBpp, clut, dir_flags, 0,0);
            for (unsigned i = 0; i < (unsigned)w*h; ++i) {
                tmp[i] = tmp[i*3];
            }
			int quality = (o->quality_grey >= 0) ? o->quality_grey : o->quality;
            n = JpgEncoder().write((uint8_t*)bm_data, w*h, tmp, w, h, quality, w, 0, 1);
			freeE(tmp);
		} else {
            unsigned    wb  = WID2BYT(w,24);
            uint8_t*    tmp = (uint8_t*)callocE(1, wb * h);
            n = beta_conv(tmp, wb, h, 24, src, srcWb, srcBpp, clut, dir_flags, 0,0);
            for (unsigned i = 0; i < (unsigned)w*h; ++i) {
                unsigned char c = tmp[i*3+0];
                tmp[i*3+0]      = tmp[i*3+2];
                tmp[i*3+2]      = c;
            }
            n = JpgEncoder().write((uint8_t*)bm_data, wb*h, tmp, w, h, o->quality, wb, 0);
			freeE(tmp);
        }
        break;
  #endif

  #if defined USE_PNG
    case BM_FMT_PNG:
        {
            bpp = (bpp <= 8) ? bpp : (bpp == 15) ? 32 : (bpp == 24) ? 24 : 32;
            unsigned    wb      = WID2BYT(w, bpp);
            unsigned    dataSz  = wb * h + 0x10;
            uint8_t*    tmp     = (uint8_t*)callocE(1, dataSz);
            n = beta_conv(tmp, wb, h, bpp, src, srcWb, srcBpp, clut, dir_flags, 0,0);
            dataSz = bm_encodeWorkSize(BM_FMT_PNG,w,h,bpp);
            n = PngEncoder().write((uint8_t*)bm_data, dataSz, tmp, w, h, bpp, (unsigned*)clut, wb, 0);
			freeE(tmp);
        }
        break;
  #endif

    default:
	  #ifdef MY_H
		n = MY_bm_write(fmt, bm_data, w, h, bpp, src, srcWb, srcBpp, clut, dir_flags, o);
	  #endif
        break;
    }
    return n;
}


int bm_chkDstBpp(int fmt, int bpp)
{
    // 減色の色数のためにdstBppを流用して値が範囲外の場合があるので、強引に辻褄合わせ
    if (bpp <= 1)       bpp = 1;
    else if (bpp <=  2) bpp = 2;
    //else if(bpp <= 3) bpp = 3;
    else if (bpp <=  4) bpp = 4;
    //else if(bpp <= 5) bpp = 5;
    //else if(bpp <= 6) bpp = 6;
    else if (bpp <=  8) bpp = 8;
    else if (bpp <= 12) bpp = 12;
    else if (bpp <= 15) bpp = 15;
    else if (bpp <= 16) bpp = 16;
    //else if(bpp <=18) bpp = 18;
    else if (bpp <= 24) bpp = 24;
    else                bpp = 32;

    switch (fmt) {
    case BM_FMT_BMP: return bmp_chkDstBpp(bpp);
    case BM_FMT_TGA: return tga_chkDstBpp(bpp);
    default: ;
	  #ifdef MY_H
	  	bpp = MY_bm_chkDstBpp(bpp, fmt);
	  #endif
    }
	return bpp;
}


int bm_encodeWorkSize(int fmt, int w, int h, int bpp)
{
    switch (fmt) {
    case BM_FMT_BMP: return bmp_encodeWorkSize(w,h,bpp);
    case BM_FMT_TGA: return tga_encodeWorkSize(w,h,bpp);
    case BM_FMT_JPG: return 2 * WID2BYT4(w,24) * h + 0x10000;
    case BM_FMT_PNG: return 3 * WID2BYT4(w,32) * h + 0x10000;
    default: ;
	  #ifdef MY_H
		{
			int n = MY_bm_encodeWorkSize(fmt, w, h, bpp);
			if (n)
				return n;
		}
	  #endif
    }
    return w * h + WID2BYT4(w,bpp) * h +  sizeof(unsigned)*256+4096;
}
