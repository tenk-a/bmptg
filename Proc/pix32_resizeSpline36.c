/**
 *  @file   pix32_resizeSpline36.c
 *  @brief  spline-36 Ç≈ägëÂèkè¨.
 *  @author Masashi KITAMURA
 *	@note
 *		http://www.rainorshine.asia/2015/10/12/post2602.html
 *		ÇéQçl
 */

#include "pix32_resizeSpline36.h"
#include "pix32_resizeAveragingI.h"
#include "pix32.h"
#include "def.h"
#include <stdlib.h>


#define USE_SUM_I64
#ifdef USE_SUM_I64
typedef int64_t			sum_t;
#define DBL_TO_SUM(x)	(sum_t)((x) * 4096.0)
#else
typedef double			sum_t;
#define DBL_TO_SUM(x)	(x)
#endif


// spline-36 ( see> http://www.rainorshine.asia/2015/10/12/post2602.html )
#define CALC_WEI(x)		(  ((x) < 1.0) ? (((( 247.0 * (x) - 453.0) * (x) -    3.0) * (x) + 209.0) / 209.0)	\
						 : ((x) < 2.0) ? ((((-114.0 * (x) + 612.0) * (x) - 1038.0) * (x) + 540.0) / 209.0)	\
						 : ((x) < 3.0) ? ((((  19.0 * (x) - 159.0) * (x) +  434.0) * (x) - 384.0) / 209.0)	\
						 :               0.0 )


static void pix32_resizeSpline36Sub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);

#if 1	// èkè¨Ç…âÊëfïΩãœñ@ÇópÇ¢ÇÈÉoÅ[ÉWÉáÉì
static void pix32_resizeSpline36Reduc(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);


/** ägëÂèkè¨
 */
int  pix32_resizeSpline36(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    if (!dst || !src || !srcW || !srcH || !dstW || !dstH) {
        assert(0 && "ERROR pix32_resizeSpline36 bad param.\n");
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
	if (dstW >= srcW && dstH >= srcH)	// ägëÂÇÃÇ›
    	pix32_resizeSpline36Sub(dst, dstW, dstH, src, srcW, srcH);
	else								// ägëÂèkè¨
		pix32_resizeSpline36Reduc(dst, dstW, dstH, src, srcW, srcH);
    return 1;
}


/** spline-36 ägëÂÇÃÇ›
 */
static void  pix32_resizeSpline36Sub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
	enum { N = 3 };
	// spline-36
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
			int    x1 = (int)(x0 - N);
			int    x2 = (int)(x0 + N);
			int    y1 = (int)(y0 - N);
			int    y2 = (int)(y0 + N);
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
			if (scaleType == 0) {	// (rscaleX != 1.0 && rscaleY != 1.0)
				for (y = y1; y <= y2; ++y) {
					for (x = x1; x <= x2; ++x) {
						uint32_t c    = src[y * srcW + x];
						double   lenX = fabs((x + 0.5) - x0);
						double   lenY = fabs((y + 0.5) - y0);
					 #if 1
				        double   weiX = CALC_WEI(lenX);
				        double   weiY = CALC_WEI(lenY);
		          		sum_t    wei  = DBL_TO_SUM(weiX * weiY);
					 #else	//é¿å±: http://koujinz.cocolog-nifty.com/blog/2009/05/post-d4b2.html ÇéQçl.
					 		// X Y Ç≤Ç∆Ç≈Ç»Ç≠íºê¸ãóó£Ç≈weightÇãÅÇﬂÇÈèÍçá. (ÉVÉÉÅ[ÉvÉlÉXÇ™å¯Ç¢ÇΩÇÊÇ§Ç»âÊÇ…Ç»ÇÈ)
					 	double   len  = sqrt(lenX*lenX + lenY*lenY);
		          		sum_t    wei  = DBL_TO_SUM(CALC_WEI(len));
					 #endif

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
			} else {	// (scaleType == 2)	// (rscaleX == 1.0 && rscaleY != 1.0)
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
				sum_t ir, ig, ib, ia;
			 #else
				int   ir, ig, ib, ia;
			 #endif
				ir = r; if (ir < 0) ir = 0; else if (ir > 255) ir = 255;
				ig = g; if (ig < 0) ig = 0; else if (ig > 255) ig = 255;
				ib = b; if (ib < 0) ib = 0; else if (ib > 255) ib = 255;
				ia = a; if (ia < 0) ia = 0; else if (ia > 255) ia = 255;
				dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
			}
		}
	}
}


/** spline-36 ägëÂèkè¨
 */
static void  pix32_resizeSpline36Reduc(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
	enum { N = 3 };
	double   rscaleX, rscaleY;
	double	 mrscaleX, mrscaleY;
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
					int    x1 = (int)(x0 - N);
					int    x2 = (int)(x0 + N);
					int    y1 = (int)(y0 - N);
					int    y2 = (int)(y0 + N);
					int    x, y;

					if (x1 < 0)
						x1 = 0;
					if (x2 >= (int)srcW)
						x2 = srcW - 1;
					if (y1 < 0)
						y1 = 0;
					if (y2 >= (int)srcH)
						y2 = srcH - 1;

					if (scaleType == 0) {	// (rscaleX != 1.0 && rscaleY != 1.0)
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
					} else {	// (scaleType == 2)	// (rscaleX == 1.0 && rscaleY != 1.0)
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
				sum_t ir, ig, ib, ia;
			 #else
				int   ir, ig, ib, ia;
			 #endif
				ir = r; if (ir < 0) ir = 0; else if (ir > 255) ir = 255;
				ig = g; if (ig < 0) ig = 0; else if (ig > 255) ig = 255;
				ib = b; if (ib < 0) ib = 0; else if (ib > 255) ib = 255;
				ia = a; if (ia < 0) ia = 0; else if (ia > 255) ia = 255;
				dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
			}
		}
	}
}

#else	// èkè¨Ç‡spline36Ç≈çsÇ§èÍçá

/** ägëÂèkè¨
 */
int  pix32_resizeSpline36(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    if (!dst || !src || !srcW || !srcH || !dstW || !dstH) {
        assert(0 && "ERROR pix32_resizeSpline36 bad param.\n");
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
   	pix32_resizeSpline36Sub(dst, dstW, dstH, src, srcW, srcH);
    return 1;
}

/** spline-36
 */
void  pix32_resizeSpline36Sub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
	enum { N = 3 };
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
	if (dstW == srcW && dstH == srcH) {	// (scaleType == 3)
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

			r = 0.0, g = 0.0, b = 0.0, a = 0.0;
			wei_total = 0.0;
			if (scaleType == 0) {	// (scaleX != 1.0 && scaleY != 1.0)
				for (y = y1; y <= y2; ++y) {
					for (x = x1; x <= x2; ++x) {
						uint32_t c    = src[y * srcW + x];
						double   lenX = fabs((x + 0.5) * scaleX2 - x0);
						double   lenY = fabs((y + 0.5) * scaleY2 - y0);
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
			} else if (scaleType == 1) { // (scaleX != 1.0 && scaleY == 1.0)
				int y = (int)dstY; //y0;
				for (x = x1; x <= x2; ++x) {
					uint32_t c    = src[y * srcW + x];
					double   lenX = fabs((x + 0.5) * scaleX2 - x0);
			        sum_t    wei  = DBL_TO_SUM( CALC_WEI(lenX) );

			        wei_total += wei;
			        r += PIX32_GET_R(c) * wei;
			        g += PIX32_GET_G(c) * wei;
			        b += PIX32_GET_B(c) * wei;
			        a += PIX32_GET_A(c) * wei;
			    }
			} else {	// (scaleType == 2)	// (scaleX == 1.0 && scaleY != 1.0)
				int x = (int)dstX; //x0;
				for (y = y1; y <= y2; ++y) {
					uint32_t c    = src[y * srcW + x];
					double   lenY = fabs((y + 0.5) * scaleY2 - y0);
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
				sum_t ir, ig, ib, ia;
			 #else
				int   ir, ig, ib, ia;
			 #endif
				ir = r; if (ir < 0) ir = 0; else if (ir > 255) ir = 255;
				ig = g; if (ig < 0) ig = 0; else if (ig > 255) ig = 255;
				ib = b; if (ib < 0) ib = 0; else if (ib > 255) ib = 255;
				ia = a; if (ia < 0) ia = 0; else if (ia > 255) ia = 255;
				dst[dstY*dstW + dstX] = PIX32_ARGB(ia,ir,ig,ib);
			}
		}
	}
}
#endif
