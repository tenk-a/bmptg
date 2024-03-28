/**
 *  @file   pix32_kyuv.h
 *  @brief  rgb <=> yuv 変換
 *  @author Masashi KITAMURA
 */

#include "pix32_kyuv.h"
#include "def.h"


/** rgb画像を yuv に変換. 出力サイズが入力サイズより大きければ、辺のピクセルをコピーして埋める.
 */
void pix32_kyuvFromRgb(pix32_kyuv_t *dst, unsigned dstW, unsigned dstH, unsigned ofsX, unsigned ofsY, unsigned const* src, unsigned srcW, unsigned srcH)
{
    unsigned        ofsXbtm, ofsYbtm;
    unsigned const* s;
    pix32_kyuv_t*          d;
    pix32_kyuv_t*          p;
    unsigned        sx, sy;

    if (dst == NULL || dstW == 0 || dstH == 0 || src == NULL || srcW == 0 || srcH == 0 || dstW < srcW+ofsX || dstH < srcH+ofsY) {
        assert(0 && "ERROR: pix32_kyuvFromRgb param");
        return;
    }
    ofsXbtm = dstW - ofsX - srcW;
    ofsYbtm = dstH - ofsY - srcH;

    s = src;
    d = dst + ofsY * dstW;
    p = d;
    for (sy = 0; sy < srcH; ++sy) {
        pix32_kyuv_t*  top = d;
        d += ofsX;
        for (sx = 0; sx < srcW; ++sx) {
            unsigned c = *s++;
            unsigned r = PIX32_GET_R(c);
            unsigned g = PIX32_GET_G(c);
            unsigned b = PIX32_GET_B(c);
            d->at[0]   = PIX32_RGB_TO_KYUV_Y(r,g,b);
            d->at[1]   = PIX32_RGB_TO_KYUV_U(r,g,b);
            d->at[2]   = PIX32_RGB_TO_KYUV_V(r,g,b);
            d->at[3]   = PIX32_GET_A(c);
            ++d;
        }
        for (sx = 0; sx < ofsX; ++sx)
            top[sx] = top[ofsX];
        for (sx = 0; sx < ofsXbtm; ++sx)
            d[sx]   = d[-1];
        d += ofsXbtm;
    }

    d = dst;
    for (sy = 0; sy < ofsY; ++sy) {
        for (sx = 0; sx < dstW; ++sx)
            *d++ = p[sx];
    }

    p = dst + (ofsY + srcH - 1) * dstW;
    d = p   + dstW;
    for (sy = 0; sy < ofsYbtm; ++sy) {
        for (sx = 0; sx < dstW; ++sx)
            *d++ = p[sx];
    }
}
