// rgb <=> yuv ïœä∑Ç≈ÇÃåÎç∑ãÔçáÇéãîF.
// r8g8b8 Ç…ëŒÇµ yuv äe11bità»è„Ç»ÇÁ åÎç∑Ç»Ç≠Ç»ÇÈñÕól
#include <stdio.h>

#ifndef PIX32_CLAMP
#define PIX32_CLAMP(c,mi,ma)    (((c) < (mi)) ? (mi) : ((ma) < (c)) ? (ma) : (c))
#endif

//enum { SHIFT_K = 2 }; // NG
//enum { SHIFT_K = 3 }; // OK
enum { SHIFT_K   = 4 }; // OK
enum { K = 1 << SHIFT_K };

unsigned __int64  rgb2yuv(unsigned argb)
{
    int r = (unsigned char)(argb >> 16);
    int g = (unsigned char)(argb >> 8);
    int b = (unsigned char)(argb);
  #if 1
    unsigned y = (K * (38444U * g + 19589U * r +  7503U * b                )) >> 16;   // Y
    unsigned u = (K * (-21709 * g + -11059 * r +  32768 * b +  255*65536/2 )) >> 16;   // Cb
    unsigned v = (K * (-27439 * g +  32768 * r +  -5329 * b +  255*65536/2 )) >> 16;   // Cr
  #elif 1
    unsigned y = (K * (38444U * g + 19589U * r +  7503U * b                )) / 65536;   // Y
    unsigned u = (K * (-21709 * g + -11059 * r +  32768 * b +  255*65536/2 )) / 65536;   // Cb
    unsigned v = (K * (-27439 * g +  32768 * r +  -5329 * b +  255*65536/2 )) / 65536;   // Cr
  #elif 1
    unsigned y = (unsigned)(K*( 0.299 * r + 0.587 * g + 0.114 * b));        // Y
    unsigned u = (unsigned)(K*(-0.1687 * r - 0.3313 * g + 0.500 * b + 128));   // Cb
    unsigned v = (unsigned)(K*( 0.500 * r - 0.4187 * g - 0.0813 * b + 128));    // Cr
  #endif
    return ((unsigned __int64)(unsigned short)y << 32) | ((unsigned short)u << 16) | (unsigned short)v;
}

unsigned    yuv2rgb(unsigned __int64 yuv)
{
    int     y  = (unsigned short)(yuv >> 32);
    int     u  = (short)(yuv >> 16);
    int     v  = (short)(yuv);
 #if 1
    u -= K * 255 / 2;
    v -= K * 255 / 2;
    int     g  = (( 65536 * y + -22554 * u + -46802 * v) + K*65536/2) >> (SHIFT_K + 16);
    int     r  = (( 65536 * y +            +  91881 * v) + K*65536/2) >> (SHIFT_K + 16);
    int     b  = (( 65536 * y + 116130 * u             ) + K*65536/2) >> (SHIFT_K + 16);
 #elif 1
    u -= K * 255 / 2;
    v -= K * 255 / 2;
    int     g  = (( 65536 * y + -22554 * u + -46802 * v) + K*65536/2) / (K*65536);
    int     r  = (( 65536 * y +            +  91881 * v) + K*65536/2) / (K*65536);
    int     b  = (( 65536 * y + 116130 * u             ) + K*65536/2) / (K*65536);
 #elif 1
    u -= 128*K;
    v -= 128*K;
    int     r  = (( 1.0 * y               + 1.402   * v ) + K/2) / K;
    int     g  = (( 1.0 * y - 0.34414 * u - 0.71414 * v ) + K/2) / K;
    int     b  = (( 1.0 * y + 1.772   * u               ) + K/2) / K;
 #endif
    g = PIX32_CLAMP(g, 0, 255);
    r = PIX32_CLAMP(r, 0, 255);
    b = PIX32_CLAMP(b, 0, 255);
    return (r << 16) | (g << 8) | b;
}


int main()
{
    enum { STEP = 1 };
    unsigned r,g,b;
    int      err = 0;
    printf("check start\n");
    for (r = 0; r < 255; r += STEP) {
        for (g = 0; g < 255; g += STEP) {
            for (b = 0; b < 255; b += STEP) {
                unsigned          n   = (r << 16) | (g << 8) | b;
                unsigned __int64  yuv = rgb2yuv(n);
                unsigned          rgb = yuv2rgb(yuv);
                if (n != rgb) {
                    printf("%06x => yuv=%012llx => %06x\n", n, yuv, rgb);
                    ++err;
                }
            }
        }
    }
    if (err == 0)
        printf("ok!\n");
    return 0;
}

