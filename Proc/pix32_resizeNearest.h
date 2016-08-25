/**
 *  @file   pix32_resizeBilinear.c
 *  @brief  バイリニア法で拡大縮小.
 *  @author Masashi KITAMURA
 */

#include "pix32_resizeBilinear.h"
#include "pix32_resizeAveragingI.h"
#include "pix32_kyuv.h"
#include "def.h"
#include <stdlib.h>

#if defined _MSC_VER || defined __WATCOMC__ || defined __BORLANDC__
#include <malloc.h>
#endif

static void pix32_bilinearMagific(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);
static void pix32_bilinearMagificReduc(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);
static void pix32_bilinearMagificReducWidth(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);
static void pix32_bilinearMagificReducHeight(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);


/** 拡大縮小
 */
void pix32_resizeBilinear(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
        return;
    if (dstW == srcW && dstH == srcH) {
        // 同じサイズならメモリコピーですます
        memcpy(dst, src, dstW*srcH*sizeof(*dst));
    } else if ((srcW % dstW) == 0 && (srcH % dstH) == 0) {
        // 整数割りですむ場合は、それ専用の処理にして、滲みを減らす
        pix32_resizeAveragingI(dst, dstW, dstH, src, srcW, srcH);
    } else if (dstH == srcH) {
        // 横だけバイリニアで拡縮
        pix32_bilinearMagificReducWidth(dst, dstW, dstH, src, srcW, srcH);
    } else if (dstW == srcW) {
        // 縦だけバイリニアで拡縮
        pix32_bilinearMagificReducHeight(dst, dstW, dstH, src, srcW, srcH);
    } else if (dstW >= srcW && dstH >= srcH) {
        // バイリニアで拡大(縮小無)
        pix32_bilinearMagific(dst, dstW, dstH, src, srcW, srcH);
    } else {
        // バイリニアで拡大縮小
        pix32_bilinearMagificReduc(dst, dstW, dstH, src, srcW, srcH);
    }
}

/** バイリニア拡大(縮小無).
 */
static void pix32_bilinearMagific(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    double      rw, rh;
    unsigned    dx,dy;
#if 0   // 予めサンプルする２行分をyuv化して処理するバージョン
    pix32_kyuv_t*   yuvbuf;
    unsigned        lineSize;
    int             scanLine = 0;

    if (dstW < srcW || dstH < srcH || srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
        return;
    lineSize = srcW + 1;
    yuvbuf = (pix32_kyuv_t*) alloca(sizeof(yuvbuf[0]) * lineSize * 2);
    if (!yuvbuf)
        return;

    rw  = (double)srcW / dstW;
    rh  = (double)srcH / dstH;
    for (dy = 0; dy < dstH; ++dy) {
        pix32_kyuv_t*  curr;
        pix32_kyuv_t*  next;
        int     x0, y0;
        double  kw, kh;
        double  sy = dy * rh;
        y0  = (int)sy;
        if (scanLine <= y0+1 && (unsigned)y0 < srcH) {
            while (scanLine <= y0+1) {
                int x,y;
                curr = &yuvbuf[(scanLine & 1) * lineSize];
                y    = (scanLine >= (int)srcH) ? srcH - 1 : scanLine;
                for (x = 0; (unsigned)x < srcW; ++x) {
                    uint32_t c = src[y*srcW + x];
                    uint32_t r = PIX32_GET_R(c);
                    uint32_t g = PIX32_GET_G(c);
                    uint32_t b = PIX32_GET_B(c);
                    pix32_kyuv_t* yuv = &curr[x];
                    yuv->at[0] = PIX32_RGB_TO_KYUV_Y(r,g,b);
                    yuv->at[1] = PIX32_RGB_TO_KYUV_U(r,g,b);
                    yuv->at[2] = PIX32_RGB_TO_KYUV_V(r,g,b);
                    yuv->at[3] = PIX32_GET_A(c);
                }
                curr[srcW] = curr[srcW-1];
                ++scanLine;
            }
        }
        kh   = sy - y0;
        curr = &yuvbuf[((y0+0) & 1) * lineSize];
        next = &yuvbuf[((y0+1) & 1) * lineSize];
        for (dx = 0; dx < dstW; ++dx) {
            double  sx = dx * rw;
            x0 = (int)sx;
            kw = sx - x0;
            {
                pix32_kyuv_t*   c0 = &curr[x0    ];
                pix32_kyuv_t*   c1 = &curr[x0 + 1];
                pix32_kyuv_t*   c2 = &next[x0    ];
                pix32_kyuv_t*   c3 = &next[x0 + 1];
                double      krw = 1.0 - kw;
                double      krh = 1.0 - kh;
                double      k0 = krw * krh;
                double      k1 = kw  * krh;
                double      k2 = krw * kh;
                double      k3 = kw  * kh;
                uint32_t    a  = (uint32_t)(c0->at[3]*k0 + c1->at[3]*k1 + c2->at[3]*k2 + c3->at[3]*k3 + 0.5);
                int32_t     i  = (int32_t )(c0->at[0]*k0 + c1->at[0]*k1 + c2->at[0]*k2 + c3->at[0]*k3);
                int32_t     u  = (int32_t )(c0->at[1]*k0 + c1->at[1]*k1 + c2->at[1]*k2 + c3->at[1]*k3) - PIX32_KYUV_MUL_K*128;
                int32_t     v  = (int32_t )(c0->at[2]*k0 + c1->at[2]*k1 + c2->at[2]*k2 + c3->at[2]*k3) - PIX32_KYUV_MUL_K*128;
                int32_t     g  = PIX32_KYUV_TO_RGB_G(i,u,v);
                int32_t     r  = PIX32_KYUV_TO_RGB_R(i,u,v);
                int32_t     b  = PIX32_KYUV_TO_RGB_B(i,u,v);
                g = PIX32_CLAMP(g, 0, 255);
                r = PIX32_CLAMP(r, 0, 255);
                b = PIX32_CLAMP(b, 0, 255);
                if (a > 255) a = 255;
                dst[dy * dstW + dx] = PIX32_ARGB(a,r,g,b);
            }
        }
    }
#elif 1
    rw  = (double)srcW / dstW;
    rh  = (double)srcH / dstH;
    for (dy = 0; dy < dstH; dy++) {
        double  sy = dy * rh;
        int     y0 = (int)sy;
        for (dx = 0; dx < dstW; dx++) {
            double  sx = dx * rw;
            int     x0 = (int)sx;
            double      kw = sx - x0;
            double      kh = sy - y0;
            double      krw= 1.0 - kw;
            double      krh= 1.0 - kh;
            double      k0 = krw * krh;
            double      k1 = kw  * krh;
            double      k2 = krw * kh;
            double      k3 = kw  * kh;
            unsigned    ccX0a1lW = x0+1U < srcW;
            unsigned    ccY0a1lH = y0+1U < srcH;
            unsigned    c0 = src[y0 * srcW + x0];
            unsigned    c1 = ccX0a1lW ? src[y0 * srcW + x0 + 1] : c0;
            unsigned    c2 = ccY0a1lH ? src[(y0+1) * srcW + x0] : c0;
            unsigned    c3 = (ccX0a1lW & ccY0a1lH) ? src[(y0+1) * srcW + x0 + 1]
                           : ccX0a1lW ?              c1
                           : ccY0a1lH ?              c2
                           :                         c0;
            unsigned    a;
            int         r,g,b;
            double      i,u,v;

            a   = (unsigned)(PIX32_GET_A(c0)*k0 + PIX32_GET_A(c1)*k1 + PIX32_GET_A(c2)*k2 + PIX32_GET_A(c3)*k3 + 0.5);
            if (a > 255) a = 255;

            r   = PIX32_GET_R(c0);
            g   = PIX32_GET_G(c0);
            b   = PIX32_GET_B(c0);
            i   = PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k0);
            u   = PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k0);
            v   = PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k0);
            r   = PIX32_GET_R(c1);
            g   = PIX32_GET_G(c1);
            b   = PIX32_GET_B(c1);
            i  += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k1);
            u  += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k1);
            v  += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k1);
            r   = PIX32_GET_R(c2);
            g   = PIX32_GET_G(c2);
            b   = PIX32_GET_B(c2);
            i  += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k2);
            u  += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k2);
            v  += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k2);
            r   = PIX32_GET_R(c3);
            g   = PIX32_GET_G(c3);
            b   = PIX32_GET_B(c3);
            i  += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k3);
            u  += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k3);
            v  += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k3);

            u  -= 128.0;
            v  -= 128.0;
            g   = PIX32_YUV_TO_RGB_G(i, u, v);
            r   = PIX32_YUV_TO_RGB_R(i, u, v);
            b   = PIX32_YUV_TO_RGB_B(i, u, v);
            g   = PIX32_CLAMP(g, 0, 255);
            r   = PIX32_CLAMP(r, 0, 255);
            b   = PIX32_CLAMP(b, 0, 255);

            dst[dy * dstW + dx] = PIX32_ARGB(a,r,g,b);
        }
    }
#endif
}


/** バイリニア法+平均画素法で拡大縮小(主に縮小用)
 */
static void pix32_bilinearMagificReduc(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    // 縮小が整数倍で行えるように、計算途中は一旦バイリニアサンプリングで拡大
    double      rw, rh;
    unsigned    msz;
    unsigned    dmw,dmh;
    unsigned    dx, dy;
    unsigned    mw, mh;

    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
        return;

    rw  = (double)srcW / dstW;
    rh  = (double)srcH / dstH;

    mw  = (int)rw + 1;      // 何分の一にするか
    mh  = (int)rh + 1;

    mw += (mw < 2);
    mh += (mh < 2);

    dmw = dstW * mw;        // 出力幅の整数倍かつ元サイズよりも大きい値
    dmh = dstH * mh;
    rw  = (double)srcW / dmw;
    rh  = (double)srcH / dmh;

    msz = mw * mh;

    for (dy = 0; dy < dstH; ++dy) {
        for (dx = 0; dx < dstW; ++dx) {
            double      ta=0, ti=0, tu=0, tv=0;
            unsigned    dmx, dmy ;
            // 仮想的に一旦dmw*dmhサイズの画像に拡大したとして、
            // その中のmw*mh ピクセルのargbの画素平均を出力の１ピクセルにする.
            for (dmy = dy*mh; dmy < (dy+1)*mh; ++dmy) {
                double  sy = dmy * rh;
                double  kh;
                int     y0;
                y0 = (int)sy;
                kh = sy - y0;
                for (dmx = dx*mw; dmx < (dx+1)*mw; ++dmx) {
                    double  sx = dmx * rw;
                    int         x0 = (int)sx;
                    double      kw = sx - x0;
                    double      krw= 1.0 - kw;
                    double      krh= 1.0 - kh;
                    double      k0 = krw * krh;
                    double      k1 = kw  * krh;
                    double      k2 = krw * kh;
                    double      k3 = kw  * kh;
                    unsigned    ccX0a1lW = (x0+1U < srcW);
                    unsigned    ccY0a1lH = (y0+1U < srcH);
                    unsigned    c0  = src[y0 * srcW + x0];
                    unsigned    c1  = ccX0a1lW ? src[y0 * srcW + x0 + 1] : c0;
                    unsigned    c2  = ccY0a1lH ? src[(y0+1) * srcW + x0] : c0;
                    unsigned    c3  =(ccX0a1lW & ccY0a1lH) ? src[(y0+1) * srcW + (x0 + 1)]
                                    : ccX0a1lW             ? c1
                                    : ccY0a1lH             ? c2
                                    :                        c0;
                    unsigned    r,g,b;
                    r   = PIX32_GET_R(c0);
                    g   = PIX32_GET_G(c0);
                    b   = PIX32_GET_B(c0);
                    ta += PIX32_GET_A(c0)*k0;
                    ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k0);
                    tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k0);
                    tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k0);
                    r   = PIX32_GET_R(c1);
                    g   = PIX32_GET_G(c1);
                    b   = PIX32_GET_B(c1);
                    ta += PIX32_GET_A(c1)*k1;
                    ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k1);
                    tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k1);
                    tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k1);
                    r   = PIX32_GET_R(c2);
                    g   = PIX32_GET_G(c2);
                    b   = PIX32_GET_B(c2);
                    ta += PIX32_GET_A(c2)*k2;
                    ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k2);
                    tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k2);
                    tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k2);
                    r   = PIX32_GET_R(c3);
                    g   = PIX32_GET_G(c3);
                    b   = PIX32_GET_B(c3);
                    ta += PIX32_GET_A(c3)*k3;
                    ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k3);
                    tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k3);
                    tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k3);
                }
            }
            PIX32_RGB_FROM_TOTAL_YUV(dst[dy * dstW + dx], ti, tu, tv, ta, msz, double);
        }
    }
}

/** 横のみ拡縮
 */
static void pix32_bilinearMagificReducWidth(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    double      rw;
    unsigned    msz;
    unsigned    dmw;
    unsigned    dx, dy;

    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0 || dstH != srcH)
        return;

    rw  = (double)srcW / dstW;

    msz = (int)rw + 1;
    msz += (msz < 2);

    dmw = dstW * msz;        // 出力幅の整数倍かつ元サイズよりも大きい値
    rw  = (double)srcW / dmw;

    for (dy = 0; dy < dstH; ++dy) {
        for (dx = 0; dx < dstW; ++dx) {
            double ta=0, ti=0, tu=0, tv=0;
            unsigned   dmx;
            for (dmx = dx*msz; dmx < (dx+1)*msz; ++dmx) {
                double      sx  = dmx * rw;
                int         x0  = (int)sx;
                double      k1  = sx - x0;
                double      k0  = 1.0 - k1;
                unsigned    c0  = src[dy * srcW + x0];
                unsigned    c1  = (x0+1U < srcW) ? src[dy * srcW + x0 + 1] : c0;
                unsigned    r,g,b;
                r   = PIX32_GET_R(c0);
                g   = PIX32_GET_G(c0);
                b   = PIX32_GET_B(c0);
                ta += PIX32_GET_A(c0)*k0;
                ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k0);
                tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k0);
                tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k0);
                r   = PIX32_GET_R(c1);
                g   = PIX32_GET_G(c1);
                b   = PIX32_GET_B(c1);
                ta += PIX32_GET_A(c1)*k1;
                ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k1);
                tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k1);
                tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k1);
            }
            PIX32_RGB_FROM_TOTAL_YUV(dst[dy * dstW + dx], ti, tu, tv, ta, msz, double);
        }
    }
}


/** 縦のみ拡大縮小
 */
static void pix32_bilinearMagificReducHeight(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    double      rh;
    unsigned    msz;
    unsigned    dmh;
    unsigned    dx, dy;

    if (srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0 || dstW != srcW)
        return;

    rh  = (double)srcH / dstH;

    msz = (int)rh + 1;
    msz += (msz < 2);

    dmh = dstH * msz;        // 出力幅の整数倍かつ元サイズよりも大きい値
    rh  = (double)srcH / dmh;

    for (dy = 0; dy < dstH; ++dy) {
        for (dx = 0; dx < dstW; ++dx) {
            double ta=0, ti=0, tu=0, tv=0;
            unsigned    dmy;
            for (dmy = dy*msz; dmy < (dy+1)*msz; ++dmy) {
                double      sy  = dmy * rh;
                int         y0  = (int)sy;
                double      k1  = sy - y0;
                double      k0  = 1.0 - k1;
                unsigned    c0  = src[y0 * srcW + dx];
                unsigned    c1  = (y0+1U < srcH) ? src[(y0+1) * srcW + dx] : c0;
                unsigned    r,g,b;
                r   = PIX32_GET_R(c0);
                g   = PIX32_GET_G(c0);
                b   = PIX32_GET_B(c0);
                ta += PIX32_GET_A(c0)*k0;
                ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k0);
                tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k0);
                tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k0);
                r   = PIX32_GET_R(c1);
                g   = PIX32_GET_G(c1);
                b   = PIX32_GET_B(c1);
                ta += PIX32_GET_A(c1)*k1;
                ti += PIX32_RGB_TO_YUV_Y_x_VAL(r,g,b, k1);
                tu += PIX32_RGB_TO_YUV_U_x_VAL(r,g,b, k1);
                tv += PIX32_RGB_TO_YUV_V_x_VAL(r,g,b, k1);
            }
            PIX32_RGB_FROM_TOTAL_YUV(dst[dy * dstW + dx], ti, tu, tv, ta, msz, double);
        }
    }
}
