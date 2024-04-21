/**
 *  @file   pix32_resizeBilinear.c
 *  @brief  ÉoÉCÉäÉjÉAñ@Ç≈ägëÂèkè¨.
 *  @author Masashi KITAMURA
 */

#include "pix32_resizeBilinear.h"
#include "pix32_resizeAveragingI.h"
#include "pix32.h"
#include "def.h"


#define USE_SUM_I64
#ifdef USE_SUM_I64
typedef uint64_t        sum_t;
#define DBL_TO_SUM(x)   (sum_t)((x) * 4096.0)
#else
typedef double          sum_t;
#define DBL_TO_SUM(x)   (x)
#endif


// bilinear
#define CALC_WEI(d)     (((d) <= 0.0) ? 1.0 : ((d) >= 1.0) ? 0.0 : 1.0 - (d))


static void pix32_resizeBilinearSub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);
static void pix32_resizeBilinearReduc(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);


/** ägëÂèkè¨
 */
int  pix32_resizeBilinear(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    if (!dst || !src || !srcW || !srcH || !dstW || !dstH) {
        assert(0 && "ERROR pix32_resizeBilinear bad param.\n");
        return 0;
    }

    if (dstW == srcW && dstH == srcH) {
        memcpy(dst, src, dstW*srcH*sizeof(*dst));                       // ìØÇ∂ÉTÉCÉYÇ»ÇÁÉÅÉÇÉäÉRÉsÅ[Ç≈Ç∑Ç‹Ç∑
        return 1;
    }

    if ((srcW % dstW) == 0 && (srcH % dstH) == 0) {
        pix32_resizeAveragingI(dst, dstW, dstH, src, srcW, srcH);       // êÆêîäÑÇËÇ≈Ç∑ÇﬁèÍçáÇÕÅAÇªÇÍêÍópÇÃèàóùÇ…ÇµÇƒÅAü¯Ç›Çå∏ÇÁÇ∑
        return 1;
    }
    if (dstW >= srcW && dstH >= srcH)
        pix32_resizeBilinearSub(dst, dstW, dstH, src, srcW, srcH);
    else
        pix32_resizeBilinearReduc(dst, dstW, dstH, src, srcW, srcH);
    return 1;
}


/** Bilinear ägëÂÇÃÇ›
 */
static void  pix32_resizeBilinearSub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
    double const R = 0.5;
    double   rscaleX, rscaleY;
    uint32_t dstX   , dstY;
    int      scaleType;

    assert(dst && dstW && dstH && src && srcW && srcH);
    rscaleX   = (double)srcW / dstW;
    rscaleY   = (double)srcH / dstH;
    scaleType = (rscaleX == 1.0) * 2 + (rscaleY == 1.0);

    for (dstY = 0; dstY < dstH; ++dstY) {
        for (dstX = 0; dstX < dstW; ++dstX) {
            double x0 = (dstX + 0.5) * rscaleX;
            double y0 = (dstY + 0.5) * rscaleY;
            int    x1 = (int)(x0 - R);
            int    x2 = (int)(x0 + R);
            int    y1 = (int)(y0 - R);
            int    y2 = (int)(y0 + R);
            int    x, y;
            sum_t  r, g, b, a;
            sum_t  wei_total;

            if (x1 < 0)
                x1 = 0;
            if (x2 >= (int)srcW)
                x2 = srcW - 1;
            if (y1 < 0)
                y1 = 0;
            if (y2 >= (int)srcH)
                y2 = srcH - 1;

            r = 0, g = 0, b = 0, a = 0;
            wei_total = 0;
            if (scaleType == 0) {   // (rscaleX != 1.0 && rscaleY != 1.0)
                for (y = y1; y <= y2; ++y) {
                    for (x = x1; x <= x2; ++x) {
                        uint32_t c    = src[y * srcW + x];
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
            } else if (scaleType == 1) { // (rscaleX != 1.0 && rscaleY == 1.0)
                int y = (int)dstY; //y0;
                for (x = x1; x <= x2; ++x) {
                    uint32_t c    = src[y * srcW + x];
                    double   lenX = fabs((x + 0.5) - x0);
                    sum_t    wei  = DBL_TO_SUM( CALC_WEI(lenX) );

                    wei_total += wei;
                    r += PIX32_GET_R(c) * wei;
                    g += PIX32_GET_G(c) * wei;
                    b += PIX32_GET_B(c) * wei;
                    a += PIX32_GET_A(c) * wei;
                }
            } else {    // (scaleType == 2) // (rscaleX == 1.0 && rscaleY != 1.0)
                int x = (int)dstX; //x0;
                for (y = y1; y <= y2; ++y) {
                    uint32_t c    = src[y * srcW + x];
                    double   lenY = fabs((y + 0.5) - y0);
                    sum_t    wei  = DBL_TO_SUM( CALC_WEI(lenY) );

                    wei_total += wei;
                    r += PIX32_GET_R(c) * wei;
                    g += PIX32_GET_G(c) * wei;
                    b += PIX32_GET_B(c) * wei;
                    a += PIX32_GET_A(c) * wei;
                }
            }

            r /= wei_total;
            g /= wei_total;
            b /= wei_total;
            a /= wei_total;

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
                dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
            }
        }
    }
}


/** Bilinear ägëÂèkè¨
 */
static void  pix32_resizeBilinearReduc(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
    double const R = 0.5;
    double   rscaleX, rscaleY;
    double   mrscaleX, mrscaleY;
    double   mw, mh;
    double   mx, my;
    uint32_t dstX, dstY;
    int      scaleType;

    assert(dst && dstW && dstH && src && srcW && srcH);

    // ägëÂÇµÇΩÇ†Ç∆ êÆêîî{Ç≈èkè¨Ç∑ÇÈÇΩÇﬂÇÃêÆêîî{ó¶ÇãÅÇﬂÇÈ.
    rscaleX  = (double)srcW / dstW;
    rscaleY  = (double)srcH / dstH;
    mw       = (rscaleX <= 1.0) ? 1 : (int)rscaleX + 1;
    mh       = (rscaleY <= 1.0) ? 1 : (int)rscaleY + 1;
    mrscaleX = rscaleX / mw;
    mrscaleY = rscaleY / mh;
    scaleType= (mrscaleX == 1.0) * 2 + (mrscaleY == 1.0);

    for (dstY = 0; dstY < dstH; ++dstY) {
        for (dstX = 0; dstX < dstW; ++dstX) {
            sum_t r = 0, g = 0, b = 0, a = 0;
            sum_t wei_total = 0;
            for (my = dstY*mh; my < (dstY+1)*mh; ++my) {
                for (mx = dstX*mw; mx < (dstX+1)*mw; ++mx) {
                    double x0 = (mx + 0.5) * mrscaleX;
                    double y0 = (my + 0.5) * mrscaleY;
                    int    x1 = (int)(x0 - R);
                    int    x2 = (int)(x0 + R);
                    int    y1 = (int)(y0 - R);
                    int    y2 = (int)(y0 + R);
                    int    x, y;

                    if (x1 < 0)
                        x1 = 0;
                    if (x2 >= (int)srcW)
                        x2 = srcW - 1;
                    if (y1 < 0)
                        y1 = 0;
                    if (y2 >= (int)srcH)
                        y2 = srcH - 1;

                    if (scaleType == 0) {   // (rscaleX != 1.0 && rscaleY != 1.0)
                        for (y = y1; y <= y2; ++y) {
                            for (x = x1; x <= x2; ++x) {
                                uint32_t c    = src[y * srcW + x];
                                double   lenX = fabs((x + 0.5) - x0);
                                double   lenY = fabs((y + 0.5) - y0);
                                double   weiX = CALC_WEI(lenX);
                                double   weiY = CALC_WEI(lenY);
                                sum_t    wei  = DBL_TO_SUM( weiX * weiY );

                                wei_total += wei;
                                r += PIX32_GET_R(c) * wei;
                                g += PIX32_GET_G(c) * wei;
                                b += PIX32_GET_B(c) * wei;
                                a += PIX32_GET_A(c) * wei;
                            }
                        }
                    } else if (scaleType == 1) { // (rscaleX != 1.0 && rscaleY == 1.0)
                        int y = (int)my; //y0;
                        for (x = x1; x <= x2; ++x) {
                            uint32_t c    = src[y * srcW + x];
                            double   lenX = fabs((x + 0.5) - x0);
                            sum_t    wei  = DBL_TO_SUM( CALC_WEI(lenX) );

                            wei_total += wei;
                            r += PIX32_GET_R(c) * wei;
                            g += PIX32_GET_G(c) * wei;
                            b += PIX32_GET_B(c) * wei;
                            a += PIX32_GET_A(c) * wei;
                        }
                    } else {    // (scaleType == 2) // (rscaleX == 1.0 && rscaleY != 1.0)
                        int x = (int)mx; //x0;
                        for (y = y1; y <= y2; ++y) {
                            uint32_t c    = src[y * srcW + x];
                            double   lenY = fabs((y + 0.5) - y0);
                            sum_t    wei  = DBL_TO_SUM( CALC_WEI(lenY) );

                            wei_total += wei;
                            r += PIX32_GET_R(c) * wei;
                            g += PIX32_GET_G(c) * wei;
                            b += PIX32_GET_B(c) * wei;
                            a += PIX32_GET_A(c) * wei;
                        }
                    }
                }
            }

            r /= wei_total;
            g /= wei_total;
            b /= wei_total;
            a /= wei_total;

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
                dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
            }
        }
    }
}
