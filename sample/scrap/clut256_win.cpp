#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <string.h>
#include <map>

static uint32_t clut_win256[] = {
        0xFF000000,0xFF800000,0xFF008000,0xFF808000,0xFF000080,0xFF800080,0xFF008080,0xFF808080,0xFFc0dcc0,0xFFa6caf0,0xFF2a3faa,0xFF2a3fff,0xFF2a5f00,0xFF2a5f55,0xFF2a5faa,0xFF2a5fff,
        0xFF2a7f00,0xFF2a7f55,0xFF2a7faa,0xFF2a7fff,0xFF2a9f00,0xFF2a9f55,0xFF2a9faa,0xFF2a9fff,0xFF2abf00,0xFF2abf55,0xFF2abfaa,0xFF2abfff,0xFF2adf00,0xFF2adf55,0xFF2adfaa,0xFF2adfff,
        0xFF2aff00,0xFF2aff55,0xFF2affaa,0xFF2affff,0xFF550000,0xFF550055,0xFF5500aa,0xFF5500ff,0xFF551f00,0xFF551f55,0xFF551faa,0xFF551fff,0xFF553f00,0xFF553f55,0xFF553faa,0xFF553fff,
        0xFF555f00,0xFF555f55,0xFF555faa,0xFF555fff,0xFF557f00,0xFF557f55,0xFF557faa,0xFF557fff,0xFF559f00,0xFF559f55,0xFF559faa,0xFF559fff,0xFF55bf00,0xFF55bf55,0xFF55bfaa,0xFF55bfff,
        0xFF55df00,0xFF55df55,0xFF55dfaa,0xFF55dfff,0xFF55ff00,0xFF55ff55,0xFF55ffaa,0xFF55ffff,0xFF7f0000,0xFF7f0055,0xFF7f00aa,0xFF7f00ff,0xFF7f1f00,0xFF7f1f55,0xFF7f1faa,0xFF7f1fff,
        0xFF7f3f00,0xFF7f3f55,0xFF7f3faa,0xFF7f3fff,0xFF7f5f00,0xFF7f5f55,0xFF7f5faa,0xFF7f5fff,0xFF7f7f00,0xFF7f7f55,0xFF7f7faa,0xFF7f7fff,0xFF7f9f00,0xFF7f9f55,0xFF7f9faa,0xFF7f9fff,
        0xFF7fbf00,0xFF7fbf55,0xFF7fbfaa,0xFF7fbfff,0xFF7fdf00,0xFF7fdf55,0xFF7fdfaa,0xFF7fdfff,0xFF7fff00,0xFF7fff55,0xFF7fffaa,0xFF7fffff,0xFFaa0000,0xFFaa0055,0xFFaa00aa,0xFFaa00ff,
        0xFFaa1f00,0xFFaa1f55,0xFFaa1faa,0xFFaa1fff,0xFFaa3f00,0xFFaa3f55,0xFFaa3faa,0xFFaa3fff,0xFFaa5f00,0xFFaa5f55,0xFFaa5faa,0xFFaa5fff,0xFFaa7f00,0xFFaa7f55,0xFFaa7faa,0xFFaa7fff,
        0xFFaa9f00,0xFFaa9f55,0xFFaa9faa,0xFFaa9fff,0xFFaabf00,0xFFaabf55,0xFFaabfaa,0xFFaabfff,0xFFaadf00,0xFFaadf55,0xFFaadfaa,0xFFaadfff,0xFFaaff00,0xFFaaff55,0xFFaaffaa,0xFFaaffff,
        0xFFd40000,0xFFd40055,0xFFd400aa,0xFFd400ff,0xFFd41f00,0xFFd41f55,0xFFd41faa,0xFFd41fff,0xFFd43f00,0xFFd43f55,0xFFd43faa,0xFFd43fff,0xFFd45f00,0xFFd45f55,0xFFd45faa,0xFFd45fff,
        0xFFd47f00,0xFFd47f55,0xFFd47faa,0xFFd47fff,0xFFd49f00,0xFFd49f55,0xFFd49faa,0xFFd49fff,0xFFd4bf00,0xFFd4bf55,0xFFd4bfaa,0xFFd4bfff,0xFFd4df00,0xFFd4df55,0xFFd4dfaa,0xFFd4dfff,
        0xFFd4ff00,0xFFd4ff55,0xFFd4ffaa,0xFFd4ffff,0xFFff0055,0xFFff00aa,0xFFff1f00,0xFFff1f55,0xFFff1faa,0xFFff1fff,0xFFff3f00,0xFFff3f55,0xFFff3faa,0xFFff3fff,0xFFff5f00,0xFFff5f55,
        0xFFff5faa,0xFFff5fff,0xFFff7f00,0xFFff7f55,0xFFff7faa,0xFFff7fff,0xFFff9f00,0xFFff9f55,0xFFff9faa,0xFFff9fff,0xFFffbf00,0xFFffbf55,0xFFffbfaa,0xFFffbfff,0xFFffdf00,0xFFffdf55,
        0xFFffdfaa,0xFFffdfff,0xFFffff55,0xFFffffaa,0xFFccccff,0xFFffccff,0xFF33ffff,0xFF66ffff,0xFF99ffff,0xFFccffff,0xFF007f00,0xFF007f55,0xFF007faa,0xFF007fff,0xFF009f00,0xFF009f55,
        0xFF009faa,0xFF009fff,0xFF00bf00,0xFF00bf55,0xFF00bfaa,0xFF00bfff,0xFF00df00,0xFF00df55,0xFF00dfaa,0xFF00dfff,0xFF00ff55,0xFF00ffaa,0xFF2a0000,0xFF2a0055,0xFF2a00aa,0xFF2a00ff,
        0xFF2a1f00,0xFF2a1f55,0xFF2a1faa,0xFF2a1fff,0xFF2a3f00,0xFF2a3f55,0xFFfffbf0,0xFFa0a0a4,0xFF808080,0xFFff0000,0xFF00ff00,0xFFffff00,0xFF0000ff,0xFFff00ff,0xFF00ffff,0xFFffffff,
};


int main()
{
    size_t idx = 0;
    printf("uint32_t clut[] = {\n");
    for (idx = 0; idx < 256; ++idx) {
        uint32_t c = clut_win256[idx];
        if ((idx % 8) == 0) {
            printf("\t");
        }
        printf("0x%08x,", c);
        if ((idx % 8) == 7) {
            printf("\n");
        }
    };
    printf("};\n");

    printf("\n\n\n");

    idx = 0;
    printf("uint32_t clut_rgb[] = {\n");
    for (idx = 0; idx < 256; ++idx) {
        uint32_t c = clut_win256[idx];
        uint8_t     a = uint8_t(c >> 24);
        uint8_t     r = uint8_t(c >> 16);
        uint8_t     g = uint8_t(c >>  8);
        uint8_t     b = uint8_t(c);
        //printf("ARGB(%3d,%3d,%3d,%d),\n", a,r,g,b);
        printf("\tARGB(0x%02x,0x%02x,0x%02x,0x%02x),\n", a,r,g,b);
        if ((idx % 16) == 15) {
            printf("\n");
        }
    };
    printf("};\n");

    printf("\n\n\n");
    printf("// sort\n\n\n");
    uint32_t clut[256];
    memcpy(clut, clut_win256, sizeof clut);
    std::sort(&clut[0], &clut[256]);

    std::map<int,int> cols[3];
    idx = 0;
    printf("uint32_t clut_rgb_sort[] = {\n");
    for (idx = 0; idx < 256; ++idx) {
        uint32_t    c = clut[idx];
        uint8_t     a = uint8_t(c >> 24);
        uint8_t     r = uint8_t(c >> 16);
        uint8_t     g = uint8_t(c >>  8);
        uint8_t     b = uint8_t(c);
        //printf("ARGB(%3d,%3d,%3d,%d),\n", a,r,g,b);
        printf("\tARGB(0x%02x,0x%02x,0x%02x,0x%02x),\n", a,r,g,b);
        if ((idx % 16) == 15) {
            printf("\n");
        }
        cols[0][r] = 1;
        cols[1][g] = 1;
        cols[2][b] = 1;
    };
    printf("};\n");

    for (idx = 0; idx < 3; ++idx) {
        printf("\t");
        auto& col1 = cols[idx];
        for (auto& co : col1) {
            printf("0x%02x,", co.first);
        }
        printf("\n");
    }

    return 0;
}
