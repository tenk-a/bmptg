/**
 *  @file   pix32_rotateBilinear.c
 *  @brief  ‰ñ“](•âŠ®‚ÍƒoƒCƒŠƒjƒA–@)
 *  @author Masashi KITAMURA
 */

#include "pix32_rotate.h"
#include "pix32.h"
#include "def.h"


#define USE_SUM_I64
#ifdef USE_SUM_I64
typedef int64_t         sum_t;
#define DBL_TO_SUM(x)   (sum_t)((x) * 4096.0)
#else
typedef double          sum_t;
#define DBL_TO_SUM(x)   (x)
#endif


// bilinear
#define CALC_WEI(d)     (((d) <= 0.0) ? 1.0 : ((d) >= 1.0) ? 0.0 : 1.0 - (d))


static int  pix32_rotateBilinearSub(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);


/** ‰ñ“]
 */
int  pix32_rotateBilinear(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol)
{
    if (!dst || !src || !srcW || !srcH) {
        assert(0 && "ERROR pix32_rotateBilinear bad param.\n");
        return 0;
    }

    rot  = fmod(rot, 360.0);

    return pix32_rotateBilinearSub(dst, src, srcW, srcH, rot, dcol);
}

/** ‰ñ“]
 */
static int  pix32_rotateBilinearSub(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol)
{
    uint32_t    dstX, dstY;
    double const N   = 0.5;
    double      rad  = 2 * M_PI * rot / 360.0;
    double      sinR = sin(rad);
    double      cosR = cos(rad);
    uint32_t    dstW = (uint32_t)(fabs(srcW * cosR) + fabs(srcH * sinR) + 0.5);
    uint32_t    dstH = (uint32_t)(fabs(srcW * sinR) + fabs(srcH * cosR) + 0.5);
    uint32_t*   dstP = (uint32_t*)malloc(dstW * dstH * sizeof(uint32_t));
    double      dstCX = dstW / 2.0;
    double      dstCY = dstH / 2.0;
    double      srcCX = srcW / 2.0;
    double      srcCY = srcH / 2.0;

    if (!dstP) {
        return 0;
    }

    dst->mallocMem  = dstP;
    dst->w          = dstW;
    dst->h          = dstH;

    for (dstY = 0; dstY < dstH; ++dstY) {
        for (dstX = 0; dstX < dstW; ++dstX) {
            int    x, y;
            sum_t  r, g, b, a;
            sum_t  wei_total;
            double tx = (dstX + 0.5 - dstCX);
            double ty = (dstY + 0.5 - dstCY);
            double x0 = tx * cosR - ty * sinR + srcCX;
            double y0 = tx * sinR + ty * cosR + srcCY;
            double x1d= x0 - N;
            double x2d= x0 + N;
            double y1d= y0 - N;
            double y2d= y0 + N;
            int    x1 = (int)(x1d + 8) - 8;
            int    x2 = (int)(x2d + 8) - 8;
            int    y1 = (int)(y1d + 8) - 8;
            int    y2 = (int)(y2d + 8) - 8;

            if (x2 < 0 || x1 >= (int)srcW || y2 < 0 || y1 >= (int)srcH) {
                dstP[dstY*dstW + dstX] = dcol;
                continue;
            }

            r = 0, g = 0, b = 0, a = 0;
            wei_total = 0;
            for (y = y1; y <= y2; ++y) {
                for (x = x1; x <= x2; ++x) {
                    uint32_t c    = (x >= 0 && x < (int)srcW && y >= 0 && y < (int)srcH) ? src[y * srcW + x] : dcol;
                    double   lenX = fabs((x + 0.5) - x0);
                    double   lenY = fabs((y + 0.5) - y0);
                    double   weiX = CALC_WEI(lenX);
                    double   weiY = CALC_WEI(lenY);
                    sum_t    wei  = DBL_TO_SUM(weiX * weiY);

                    wei_total += wei;
                    r += PIX32_GET_R(c) * wei;
                    g += PIX32_GET_G(c) * wei;
                    b += PIX32_GET_B(c) * wei;
                    a += PIX32_GET_A(c) * wei;
                }
            }

            if (wei_total) {
                r /= wei_total;
                g /= wei_total;
                b /= wei_total;
                a /= wei_total;
            }

            {
             #if defined(USE_SUM_I64) && defined(CPU64)
                sum_t       ir, ig, ib, ia;
             #else
                uint32_t    ir, ig, ib, ia;
             #endif
                ir = r; if (ir > 255) ir = 255;
                ig = g; if (ig > 255) ig = 255;
                ib = b; if (ib > 255) ib = 255;
                ia = a; if (ia > 255) ia = 255;
                dstP[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
            }
        }
    }
    return 1;
}
