/**
 *  @file pix32_bokashi.h
 *  @brief  32ビット色画をぼかす.
 *  @author Masashi KITAMURA
 */
#ifndef PIX32_BOKASHI
#define PIX32_BOKASHI


#include <string.h>


#ifndef PIX32_ARGB   // a,r,g,b結合＆分解マクロ.

/// 8bit値のa,r,g,bを繋げて 32bit の色値にする.
#define PIX32_ARGB(a,r,g,b)     ((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)       ((unsigned char)(argb))         ///< argb値中の blueの値を取得.
#define PIX32_GET_G(argb)       ((unsigned char)((argb)>>8))    ///< argb値中の greenの値を取得.
#define PIX32_GET_R(argb)       ((unsigned char)((argb)>>16))   ///< argb値中の greenの値を取得.
#define PIX32_GET_A(argb)       (((unsigned)(argb))>>24)        ///< argb値中の alphaの値を取得.

#endif




/** 適当にぼかし画像に変換.
 */
inline void pix32_bokashi1(unsigned *dst, const unsigned *src, int w, int h)
{
    unsigned y;

    // 上辺と下辺はそのままコピー(手抜き)
    memcpy(dst, src, w*sizeof(unsigned));
    memcpy(&dst[(h-1)*w], &src[(h-1)*w], w*sizeof(unsigned));

    for (y = 1; y < h-1U; ++y) {
        unsigned        x;
        unsigned        *pd  = dst + (y*w);
        const unsigned  *ps  = src + (y*w);

        *pd++ = *ps++;          // 左辺はそのまま(手抜き)

        for (x = 1; x < w-1U; ++x) {
            //x enum { K0 =  4, K1 =  2, K2 =  1 };     // total=16
            //x enum { K0 = 12, K1 =  8, K2 =  5 };     // total=64
            //x enum { K0 = 44, K1 = 32, K2 = 21 };     // total=256
            enum     { K0 = 40, K1 = 32, K2 = 25 };     // total=256
            //x enum { KTOTAL = K0 + K1*4 + K2*4 };
            unsigned    k  = K0;
            unsigned    c  = ps[0];
            unsigned    aa = PIX32_GET_A(c);
          #if 0
            if (aa == 0) {
                pd[0] = 0;
                ++ps;
                ++pd;
                continue;
            }
          #endif
            unsigned    a = aa        * K0;
            unsigned    r = PIX32_GET_R(c) * K0;
            unsigned    g = PIX32_GET_G(c) * K0;
            unsigned    b = PIX32_GET_B(c) * K0;

            c = ps[-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[-w];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[w];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[-w-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[-w+1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[ w-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[ w+1];
            aa = PIX32_GET_A(c);
            if (aa) {
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            a /= k;         //x a/= KTOTAL;
            r /= k;         //x r/= KTOTAL;
            g /= k;         //x g/= KTOTAL;
            b /= k;         //x b/= KTOTAL;

            if (ps[0] == 0) {
                if (PIX32_ARGB(0,r,g,b) == 0) {
                    a = 0;
                }
            }
            c = PIX32_ARGB(a, r, g, b);
            *pd = c;
            ++pd;
            ++ps;
        }
        *pd++ = *ps++;          // 右辺はそのまま(手抜き)
    }
}




/** α 0..thre のピクセル付近だけぼかし変換
 */
inline void pix32_bokashiAlpLtEqThreshold(unsigned *dst, const unsigned *src, int w, int h, int thre)
{
    unsigned y;

    // 上辺と下辺はそのままコピー(手抜き)
    memcpy(dst, src, w*sizeof(unsigned));
    memcpy(&dst[(h-1)*w], &src[(h-1)*w], w*sizeof(unsigned));

    for (y = 1; y < h-1U; ++y) {
        unsigned        x;
        unsigned        *pd  = dst + (y*w);
        const unsigned  *ps  = src + (y*w);

        *pd++ = *ps++;          // 左辺はそのまま(手抜き)

        for (x = 1; x < w-1U; ++x) {
            //x enum { K0 =  4, K1 =  2, K2 =  1 };     // total=16
            //x enum { K0 = 12, K1 =  8, K2 =  5 };     // total=64
            //x enum { K0 = 44, K1 = 32, K2 = 21 };     // total=256
            enum     { K0 = 40, K1 = 32, K2 = 25 };     // total=256
            //x enum { KTOTAL = K0 + K1*4 + K2*4 };
            unsigned    k  = K0;
            unsigned    c  = ps[0];
            unsigned    aa = PIX32_GET_A(c);
          #if 0
            if (aa == 0) {
                pd[0] = 0;
                ++ps;
                ++pd;
                continue;
            }
          #endif
            unsigned    a = aa        * K0;
            unsigned    r = PIX32_GET_R(c) * K0;
            unsigned    g = PIX32_GET_G(c) * K0;
            unsigned    b = PIX32_GET_B(c) * K0;
            unsigned    af=1;

            c = ps[-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[-w];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[w];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K1;
                r += PIX32_GET_R(c) * K1;
                g += PIX32_GET_G(c) * K1;
                b += PIX32_GET_B(c) * K1;
                k += K1;
            }

            c = ps[-w-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[-w+1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[ w-1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            c = ps[ w+1];
            aa = PIX32_GET_A(c);
            if (aa) {
                af &= (aa > thre);
                a += aa        * K2;
                r += PIX32_GET_R(c) * K2;
                g += PIX32_GET_G(c) * K2;
                b += PIX32_GET_B(c) * K2;
                k += K2;
            }

            if (af == 0) {
                a /= k;         //x a/= KTOTAL;
                r /= k;         //x r/= KTOTAL;
                g /= k;         //x g/= KTOTAL;
                b /= k;         //x b/= KTOTAL;

                if (ps[0] == 0) {
                    if (PIX32_ARGB(0,r,g,b) == 0) {
                        a = 0;
                    }
                }
                c = PIX32_ARGB(a, r, g, b);
            } else {
                c = ps[0];
            }
            *pd = c;
            ++pd;
            ++ps;
        }
        *pd++ = *ps++;          // 右辺はそのまま(手抜き)
    }
}






/**バストアップなどで、縁をαでぼかした画像を生成する. 簡易処理
 */
inline void pix32_alpBokasi(unsigned *pix, unsigned wid, unsigned hei, int dmy_sikii)
{
    int  w = (int)wid;
    int  h = (int)hei;
    int  n,x,y,c,m,a,aa,k,kk;
    int  qr;
    #undef  A
    #undef  P
    #undef  R
    #undef  K
    #undef  DK
    #define P(x,y)          pix[(y)*w + (x)]
    #define R(x,y)          (((x) >= 0 && (y) >= 0 && (x) < w && (y) < h))
    #define K(c)            ((PIX32_GET_G(c) * 9 + PIX32_GET_R(c) * 5 + PIX32_GET_B(c) * 2)>>4)
    #define A(x,y)          ((qr = R(x,y)) == 0 || (qr && PIX32_GET_A(P(x,y))))
    //#define DK(x,y,kk,k)  {if (A(x,y)) {int c_ = P(x,y); kk += K(c_) - k;}}
    #define DK(x,y,kk,k)    {if (R(x,y) && PIX32_GET_A(P(x,y))) {int c_ = P(x,y); kk += K(c_) - k;}}
    (void)dmy_sikii;

    // α=0,c=0のピクセルに隣接する画像のα値を調整.
    //for (y = 0; y < h; ++y) {
    for (y = 1; y < h-1; ++y) {
        //for (x = 0; x < w; ++x) {
        for (x = 1; x < w-1; ++x) {
            n = y*w+x;
            c = pix[n];
            k = K(c);
            a = PIX32_GET_A(c);
            if (a == 0) {       // マスクで抜き色のところを,
                pix[n] = 0;     // ソースも抜き色にする.
            } else {
                m =(  A(x-1,y-1)*1 + A(x,y-1)*1 + A(x+1,y-1)*1
                    + A(x-1,y  )*1 + A(x,y  )*1 + A(x+1,y  )*1
                    + A(x-1,y+1)*1 + A(x,y+1)*1 + A(x+1,y+1)*1 );
                if (m < 9) {
                    m =(                 A(x-1,y-2)*3 + A(x,y-2)*3 + A(x+1,y-2)*3
                        + A(x-2,y-1)*3 + A(x-1,y-1)*4 + A(x,y-1)*4 + A(x+1,y-1)*4 + A(x+2,y-1)*3
                        + A(x-2,y  )*3 + A(x-1,y  )*4 + A(x,y  )*5 + A(x+1,y  )*4 + A(x+2,y  )*3
                        + A(x-2,y+1)*3 + A(x-1,y+1)*4 + A(x,y+1)*4 + A(x+1,y+1)*4 + A(x+2,y+1)*3
                                       + A(x-1,y+2)*3 + A(x,y+2)*3 + A(x+1,y+2)*3
                    );
                    kk = 0;
                    DK(x-1,y-1,kk,k); DK(x,y-1,kk,k); DK(x+1,y-1,kk,k);
                    DK(x-1,y  ,kk,k);                 DK(x+1,y  ,kk,k);
                    DK(x-1,y+1,kk,k); DK(x,y+1,kk,k); DK(x+1,y+1,kk,k);
                    kk = abs(kk) >> 3;
                    kk = 255 - kk;
                    if (k < 48) kk = kk * kk / 255;
                    m  = (255 * m) / 73;
                    if (k < 64) m  = m * m / 255;
                    aa = m * kk / 255;
                    //aa = aa * aa / 255;
                    aa = a * aa / 255;
                    //aa = aa * aa / 255;
                    //if (aa < 8)
                    //    aa = 8;
                    //printf("@(%d,%d)=%x  %x,%x,%x\n",x,y,aa, a,m,kk);
                    pix[n] = ((aa << 24) | (c&0xFFFFFF));
                }
            }
        }
    }
    #undef  A
    #undef  P
    #undef  R
    #undef  K
    #undef  DK
}







// ===========================================================================
// C++向
// image(), witdh(),height(),bpp() をメンバーに持つ画像クラスを操作.

#if defined __cplusplus

template<class IMG>
void pix32_bokashi1(IMG& dst, const IMG& src) {
    assert(dst.bpp() == src.bpp() && src.bpp == 32 && dst.width() == src.width() && dst.width() == src.width());
    pix32_bokashi(dst.image(), src.image(), dst.width(), dst.height());
}


template<class IMG>
void pix32_alpBokashi(IMG& img) {
    assert(img.bpp() == 32);
    pix32_alpBokasi(img.image(), img.width(), img.height(), 0);
}


#endif  // __cplusplus

#endif
