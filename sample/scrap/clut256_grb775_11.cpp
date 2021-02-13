#include <stdio.h>
#include <stdint.h>

static uint32_t const clut[] = {
    0xff2a2a2a,
    0xff555555,
    0xffaaaaaa,
    0xffd4d4d4,
    0xff3f3f3f,
    0xff3f553f,
    0xff553f3f,
    0xff553f55,
    0xff55553f,
    0xffbfbfbf,
    0x00000000,
};

int main()
{
    size_t idx = 0;
    for (size_t g = 0; g < 7; ++g) {
        for (size_t r = 0; r < 7; ++r) {
            for (size_t b = 0; b < 5; ++b) {
                uint8_t gg = g * 255 / 6;
                uint8_t rr = r * 255 / 6;
                uint8_t bb = b * 255 / 4;
                uint32_t c = (0xff << 24) | (rr << 16) | (gg << 8) | bb;
                printf("0x%08x,", c);
                if ((idx % 5) == 4) {
                    printf("\n");
                }
                ++idx;
            }
        }
    }
    for (size_t i = 0; i < 11; ++i) {
        uint32_t c = clut[i];
        printf("0x%08x,", c);
        if ((idx % 5) == 4) {
            printf("\n");
        }
        ++idx;
    }
    return 0;
}
