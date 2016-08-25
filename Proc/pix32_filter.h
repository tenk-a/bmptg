/**
 *  @file   pix32_filter.h
 *  @brief  フィルタ
 */
#ifndef PIX32_FILTER_H
#define PIX32_FILTER_H

#ifndef PIX32_ARGB  // a,r,g,b結合＆分解マクロ

/// 8bit値のa,r,g,bを繋げて 32bit の色値にする
#define PIX32_ARGB(a,r,g,b)     ((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)       ((unsigned char)(argb))         ///< argb値中の blueの値を取得
#define PIX32_GET_G(argb)       ((unsigned char)((argb) >>  8)) ///< argb値中の greenの値を取得
#define PIX32_GET_R(argb)       ((unsigned char)((argb) >> 16)) ///< argb値中の greenの値を取得
#define PIX32_GET_A(argb)       (((unsigned)(argb)) >> 24)      ///< argb値中の alphaの値を取得

#endif


inline
void pix32_filter(unsigned* dst, const unsigned* src, unsigned w, unsigned h, const int flt3x3[], int waight)
{
    //typedef double    val_t;
 #if defined(_MSC_VER) || defined(__BORLANDC__)
    typedef __int64     val_t;
 #else
    typedef long long   val_t;
 #endif
    int             x;
    int             y;

    for (y = 0; (unsigned)y < h; ++y) {
        for (x = 0; (unsigned)x < w; ++x) {
            int         a,r,g,b;
            unsigned    c  = src[y*w + x];
            val_t       ft = flt3x3[4];
            val_t       fa = PIX32_GET_A(c) * ft;
            val_t       fr = PIX32_GET_R(c) * ft;
            val_t       fg = PIX32_GET_G(c) * ft;
            val_t       fb = PIX32_GET_B(c) * ft;

            if (x - 1 >= 0 && y - 1 >= 0) {
                c   = src[(y - 1) * w + (x - 1)];
            } else if (x - 1 >= 0) {
                c   = src[(y - 0) * w + (x - 1)];
            } else if (y - 1 >= 0) {
                c   = src[(y - 1) * w + (x - 0)];
            } else {
                c   = src[(y - 0) * w + (x - 0)];
            }
            ft  = flt3x3[0];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (y - 1 >= 0) {
                c   = src[(y - 1) * w + (x    )];
            } else {
                c   = src[(y - 0) * w + (x    )];
            }
            ft  = flt3x3[1];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (x + 1U < w && y - 1 >= 0) {
                c   = src[(y - 1) * w + (x + 1)];
            } else if (x + 1U < w) {
                c   = src[(y - 0) * w + (x + 1)];
            } else if (y - 1 >= 0) {
                c   = src[(y - 1) * w + (x + 0)];
            } else {
                c   = src[(y - 0) * w + (x + 0)];
            }
            ft  = flt3x3[2];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (x - 1 >= 0) {
                c   = src[(y    ) * w + (x - 1)];
            } else {
                c   = src[(y    ) * w + (x - 0)];
            }
            ft  = flt3x3[3];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (x + 1U < w) {
                c   = src[(y    ) * w + (x + 1)];
            } else {
                c   = src[(y    ) * w + (x + 0)];
            }
            ft  = flt3x3[5];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (x - 1 >= 0 && y + 1U <  h) {
                c   = src[(y + 1) * w + (x - 1)];
            } else if (x - 1 >= 0) {
                c   = src[(y + 0) * w + (x - 1)];
            } else if (y + 1U <  h) {
                c   = src[(y + 1) * w + (x - 0)];
            } else {
                c   = src[(y + 0) * w + (x - 0)];
            }
            ft  = flt3x3[6];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (y + 1U <  h) {
                c   = src[(y + 1) * w + (x    )];
            } else {
                c   = src[(y + 0) * w + (x    )];
            }
            ft  = flt3x3[7];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            if (x + 1U < w && y + 1U <  h) {
                c   = src[(y + 1) * w + (x + 1)];
            } else if (x + 1U < w) {
                c   = src[(y + 0) * w + (x + 1)];
            } else if (y + 1U < h) {
                c   = src[(y + 1) * w + (x + 0)];
            } else {
                c   = src[(y + 0) * w + (x + 0)];
            }
            ft  = flt3x3[8];
            fa += PIX32_GET_A(c) * ft;
            fr += PIX32_GET_R(c) * ft;
            fg += PIX32_GET_G(c) * ft;
            fb += PIX32_GET_B(c) * ft;

            a = (int)(fa / waight);
            r = (int)(fr / waight);
            g = (int)(fg / waight);
            b = (int)(fb / waight);

            if (a > 255) a = 255; else if (a < 0) a = 0;
            if (r > 255) r = 255; else if (r < 0) r = 0;
            if (g > 255) g = 255; else if (g < 0) g = 0;
            if (b > 255) b = 255; else if (b < 0) b = 0;

            dst[y * w + x] = PIX32_ARGB(a,r,g,b);   //(a << 24) | (r << 16) | (g << 8) | b;
        }
    }
}



inline
void pix32_bokashi(unsigned* dst, const unsigned* src, unsigned w, unsigned h)
{
    static const int flt3x3[] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };

    pix32_filter(dst, src, w, h, flt3x3, 9);
}



inline
void pix32_sharp(unsigned* dst, const unsigned* src, unsigned w, unsigned h)
{
    static const int flt3x3[] = {
        -1, -1, -1,
        -1, 15, -1,
        -1, -1, -1,
    };

    pix32_filter(dst, src, w, h, flt3x3, 7);
}


#endif
