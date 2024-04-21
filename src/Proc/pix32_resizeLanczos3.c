/**
 *  @file   pix32_resizeLanczos3.c
 *  @brief  lanczos-3 法で拡大縮小.
 *  @author Masashi KITAMURA
 *  @note
    http://www.rainorshine.asia/2015/10/12/post2602.html
    http://d.hatena.ne.jp/gioext/20090414/1239720615
 */

#include "pix32_resizeLanczos3.h"
#include "pix32_resizeBilinear.h"
#include "pix32_resizeAveragingI.h"
#include "pix32.h"
#include "def.h"
#include <stdlib.h>

void pix32_resizeLanczos3Sub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);

/** 拡大縮小
 */
int pix32_resizeLanczos3(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH, int hasAlpha)
{
    if (!dst || !src || !srcW || !srcH || !dstW || !dstH) {
        assert(0 && "ERROR pix32_resizeLanczos3 bad param.\n");
        return 0;
    }

    if (dstW == srcW && dstH == srcH) {
        memcpy(dst, src, dstW*srcH*sizeof(*dst));                       // 同じサイズならメモリコピーですます
        return 1;
    }

    if ((srcW % dstW) == 0 && (srcH % dstH) == 0) {
        pix32_resizeAveragingI(dst, dstW, dstH, src, srcW, srcH);       // 整数割りですむ場合は、それ専用の処理にして、滲みを減らす
        return 1;
    }

    pix32_resizeLanczos3Sub(dst, dstW, dstH, src, srcW, srcH);

    if (hasAlpha) { // αチャンネル部分はバイリニアで処理
        pix32_resizeBilinearAlpha(dst, dstW, dstH, src, srcW, srcH);
    }
    return 1;
}

/** lanczos-3
 */
void pix32_resizeLanczos3Sub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
    enum { N = 3 };
    #define SINC(x)         (sin(M_PI * (x)) / (M_PI * (x)))
    // lanczos-3
    #define CALC_WEI(absL)  ( ((absL) == 0.0) ? 1.0                     \
                            : ((absL) >= N  ) ? 0.0                     \
                            :               SINC(absL) * SINC((absL) / N))
    double   scaleX , scaleY;
    double   scaleX1, scaleY1;
    double   scaleX2, scaleY2;
    uint32_t dstX   , dstY;
    int      scaleType;

    assert(dst && dstW && dstH && src && srcW && srcH);
    scaleX    = (double)dstW / srcW;
    scaleY    = (double)dstH / srcH;
    scaleType = (scaleX == 1.0) * 2 + (scaleY == 1.0);
    if (scaleX > 1.0) {
        scaleX1 = scaleX;
        scaleX2 = 1.0;
    } else {
        scaleX1 = 1.0;
        scaleX2 = scaleX;
    }
    if (scaleY > 1.0) {
        scaleY1 = scaleY;
        scaleY2 = 1.0;
    } else {
        scaleY1 = 1.0;
        scaleY2 = scaleY;
    }

 #if 0
    if (dstW == srcW && dstH == srcH) { // (scaleType == 3)
        memcpy(dst, src, srcW * srcH * sizeof(*src));
        return;
    }
 #endif

    for (dstY = 0; dstY < dstH; ++dstY) {
        for (dstX = 0; dstX < dstW; ++dstX) {
            double x0 = (dstX + 0.5) / scaleX1;
            double y0 = (dstY + 0.5) / scaleY1;
            int    x1 = (int)((x0 - N) / scaleX2);
            int    x2 = (int)((x0 + N) / scaleX2);
            int    y1 = (int)((y0 - N) / scaleY2);
            int    y2 = (int)((y0 + N) / scaleY2);
            int    x, y;
            double r, g, b;
          #ifdef USE_ALPHA
            double a;
          #endif
            double wei_total;

            if (x1 < 0)
                x1 = 0;
            if (x2 >= (int)srcW)
                x2 = srcW - 1;
            if (y1 < 0)
                y1 = 0;
            if (y2 >= (int)srcH)
                y2 = srcH - 1;

            r = 0.0, g = 0.0, b = 0.0;
         #ifdef USE_ALPHA
            a = 0.0;
         #endif
            wei_total = 0.0;
            if (scaleType == 0) {   // (scaleX != 1.0 && scaleY != 1.0)
                for (y = y1; y <= y2; ++y) {
                    for (x = x1; x <= x2; ++x) {
                        uint32_t c    = src[y * srcW + x];
                        double   lenX = fabs((x + 0.5) * scaleX2 - x0);
                        double   lenY = fabs((y + 0.5) * scaleY2 - y0);
                        double   weiX = CALC_WEI(lenX);
                        double   weiY = CALC_WEI(lenY);
                        double   wei  = weiX * weiY;

                        wei_total += wei;
                        r += PIX32_GET_R(c) * wei;
                        g += PIX32_GET_G(c) * wei;
                        b += PIX32_GET_B(c) * wei;
                     #ifdef USE_ALPHA
                        a += PIX32_GET_A(c) * wei;
                     #endif
                    }
                }
            } else if (scaleType == 1) { // (scaleX != 1.0 && scaleY == 1.0)
                int y = (int)dstY; //y0;
                for (x = x1; x <= x2; ++x) {
                    uint32_t c    = src[y * srcW + x];
                    double   lenX = fabs((x + 0.5) * scaleX2 - x0);
                    double   wei  = CALC_WEI(lenX);

                    wei_total += wei;
                    r += PIX32_GET_R(c) * wei;
                    g += PIX32_GET_G(c) * wei;
                    b += PIX32_GET_B(c) * wei;
                 #ifdef USE_ALPHA
                    a += PIX32_GET_A(c) * wei;
                 #endif
                }
            } else {    // (scaleType == 2) // (scaleX == 1.0 && scaleY != 1.0)
                int x = (int)dstX; //x0;
                for (y = y1; y <= y2; ++y) {
                    uint32_t c    = src[y * srcW + x];
                    double   lenY = fabs((y + 0.5) * scaleY2 - y0);
                    double   wei  = CALC_WEI(lenY);

                    wei_total += wei;
                    r += PIX32_GET_R(c) * wei;
                    g += PIX32_GET_G(c) * wei;
                    b += PIX32_GET_B(c) * wei;
                 #ifdef USE_ALPHA
                    a += PIX32_GET_A(c) * wei;
                 #endif
                }
            }

            r /= wei_total;
            g /= wei_total;
            b /= wei_total;
         #ifdef USE_ALPHA
            a /= wei_total;
         #endif

            {
                int ir, ig, ib, ia;
                ir = (int)r; if (ir < 0) ir = 0; else if (ir > 255) ir = 255;
                ig = (int)g; if (ig < 0) ig = 0; else if (ig > 255) ig = 255;
                ib = (int)b; if (ib < 0) ib = 0; else if (ib > 255) ib = 255;
             #ifdef USE_ALPHA
                ia = (int)a; if (ia < 0) ia = 0; else if (ia > 255) ia = 255;
             #else
                ia = 255;
             #endif
                dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
            }
        }
    }
    return;

    #undef SINC
    #undef CALC_WEI
}
