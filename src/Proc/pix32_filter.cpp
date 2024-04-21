/**
 *  @file   pix32_filter.cpp
 *  @brief  フィルタ
 */

#ifndef PIX32_FILTER_H
#define PIX32_FILTER_H

#include "pix32_filter.h"

#include "../misc/def.h"


void pix32_bokasi(uint32_t* dst, const uint32_t* src, unsigned w, unsigned h)
{
    static const int ft3x3[] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };

    pix32_filter(dst, src, w, h, ft3x3, 9);
}



void pix32_sharp(uint32_t* dst, const uint32_t* src, unsigned w, unsigned h)
{
    static const int ft3x3[] = {
        -1, -1, -1,
        -1, 15, -1,
        -1, -1, -1,
    };

    pix32_filter(dst, src, w, h, ft3x3, 7);
}



void pix32_filter(uint32_t* dst, const uint32_t* src, unsigned w, unsigned h, const int ft3x3[], int waight)
{
    //typedef double    val_t;
    typedef   int64_t   val_t;

    const uint8_t*  srcB = (uint8_t*)src;
    unsigned        wb   = w * 4;
    unsigned x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned    a,r,g,b;
            unsigned    c  = src[y*w + x];
            val_t       ft = ft3x3[4];
            val_t       fa = PIX32_GET_A(c) * ft;
            val_t       fr = PIX32_GET_R(c) * ft;
            val_t       fg = PIX32_GET_G(c) * ft;
            val_t       fb = PIX32_GET_B(c) * ft;
            if (x - 1 >= 0 && y - 1 >= 0) {
                c   = src[(y - 1) * w + (x - 1)];
                ft  = ft3x3[0];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (y - 1 >= 0) {
                c   = src[(y - 1) * w + (x    )];
                ft  = ft3x3[1];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (x + 1 < w && y - 1 >= 0) {
                c   = src[(y - 1) * w + (x + 1)];
                ft  = ft3x3[2];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (x - 1 >= 0) {
                c   = src[(y    ) * w + (x - 1)];
                ft  = ft3x3[3];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (x + 1 < w) {
                c   = src[(y    ) * w + (x + 1)];
                ft  = ft3x3[5];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (x - 1 >= 0 && y + 1 <  w) {
                c   = src[(y + 1) * w + (x - 1)];
                ft  = ft3x3[6];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (y + 1 <  w) {
                c   = src[(y + 1) * w + (x    )];
                ft  = ft3x3[7];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }
            if (x + 1 < w && y + 1 <  w) {
                c   = src[(y + 1) * w + (x + 1)];
                ft  = ft3x3[8];
                fa += PIX32_GET_A(c) * ft;
                fr += PIX32_GET_R(c) * ft;
                fg += PIX32_GET_G(c) * ft;
                fb += PIX32_GET_B(c) * ft;
            }

            a = (int)(fa / waight);
            r = (int)(fr / waight);
            g = (int)(fg / waight);
            b = (int)(fb / waight);

            if (a > 255) a = 255; else if (a < 0) a = 0;
            if (r > 255) r = 255; else if (r < 0) r = 0;
            if (g > 255) g = 255; else if (g < 0) g = 0;
            if (b > 255) b = 255; else if (b < 0) b = 0;

            dst[y * w + x] = ARGB(a,r,g,b); //(a << 24) | (r << 16) | (g << 8) | b;
        }
    }
}


#endif
