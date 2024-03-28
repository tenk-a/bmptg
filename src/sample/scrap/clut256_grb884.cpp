#include <stdio.h>
#include <stdint.h>

int main()
{
    size_t idx = 0;
    for (idx = 0; idx < 256; ++idx) {
     #if 0
        static uint8_t tbl1[] = { 0x00, 0x2f, 0x4f, 0x6f, 0x8f, 0xaf, 0xcf, 0xff };
        static uint8_t tbl2[] = { 0x00,       0x4f,       0x8f,             0xff };
     #else
        uint8_t g = (idx >> 5) & 7;
        uint8_t r = (idx >> 2) & 7;
        uint8_t b = (idx     ) & 3;
        uint32_t c;
        g = (g << 5) | (g << 2) | (g >> 1);
        r = (r << 5) | (r << 2) | (r >> 1);
        b = (b << 6) | (b << 4) | (b << 2) | b;
        c = (0xff << 24) | (r << 16) | (g << 8) | b;
        printf("0x%08x,", c);
        if ((idx % 16) == 15) {
            printf("\n");
        }
     #endif
    }
    return 0;
}
