#include <stdio.h>
#include <stdint.h>
#include <algorithm>

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
 #if 1
    uint32_t yn = 7;
    uint32_t un = 6;
    uint32_t vn = 6;
 #else
    uint32_t yn = 10;
    uint32_t un = 5;
    uint32_t vn = 5;
 #endif
    printf("// yuv %d,%d,%d\n", yn, un, vn);
    uint32_t clut[1024] = {0};
    uint32_t count = 0;
    for (int yi = 0; yi < yn; ++yi) {
        for (int ui = 0; ui < un; ++ui) {
            for (int vi = 0; vi < vn; ++vi) {
                uint64_t y = yi * 0x0ff0 / (yn-1);
                uint64_t u = ui * 0x07f8 / (un-1);
                uint64_t v = vi * 0x07f8 / (vn-1);
                uint64_t yuv = (y << 32) | (u << 16) | (v);
                uint32_t rgb = yuv2rgb(yuv);
                rgb |= 0xff000000;
                clut[count] = rgb;
                ++count;
            }
        }
    }
    if (count > 0)
        std::sort(&clut[0], &clut[count]);
    for (size_t idx = 0; idx < count; ++idx) {
        printf("0x%08x,", clut[idx]);
        if (idx % 8 == 7)
            printf("\n");
    }
    for (size_t idx = count; idx < 256; ++idx) {
        printf("0x%08x,", 0);
        if (idx % 8 == 7)
            printf("\n");
    }
    printf("\n");
    return 0;
}
