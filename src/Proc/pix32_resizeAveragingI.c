/**
 *  @file   pix32_resizeAveragingI.c
 *  @brief  出力サイズで入力サイズが割り切れる場合用の平均画素法での縮小.
 *  @author Masashi KITAMURA
 */

#include "pix32_resizeAveragingI.h"
#include "pix32.h"
//#include "pix32_kyuv.h"
#include "def.h"


/** 出力サイズで入力サイズが割り切れる場合、平均画素法で縮小.
 */
void pix32_resizeAveragingI(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
    enum { K = 1 << 5 };
    unsigned    mw, mh;
    unsigned    msz;
    unsigned    dx, dy, sxx, syy;
    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
        return;
    mw = srcW / dstW;
    mh = srcH / dstH;
    if (mw == 0 || mh == 0)
        return;
    //printf("[%d*%d->(%d,%d)->%d*%d]\n", srcW,srcH,mw,mh,dstW,dstH);
    msz = mw * mh;
    syy = 0;
    for (dy = 0; dy < dstH; ++dy) {
        sxx = 0;
        for (dx = 0; dx < dstW; ++dx) {
            uint64_t    ta=0, tr=0, tg=0, tb=0;
            unsigned    x, y, a, r, g, b;
            for (y = syy; y < syy+mh; ++y) {
                for (x = sxx; x < sxx+mw; ++x) {
                    uint32_t c = src[y*srcW + x];
                    tr += PIX32_GET_R(c);
                    tg += PIX32_GET_G(c);
                    tb += PIX32_GET_B(c);
                    ta += PIX32_GET_A(c);
                }
            }
            a = ta / msz;
            r = tr / msz;
            g = tg / msz;
            b = tb / msz;
            dst[dy * dstW + dx] = PIX32_ARGB(a, r, g, b);
            sxx += mw;
        }
        syy += mh;
    }
}
