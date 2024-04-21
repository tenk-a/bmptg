#include <stdio.h>
#include <stdlib.h>
#define  inline __inline
#include "../ImgFmt/tga_read.h"
#include "../ImgFmt/bmp_wrt.h"


int main(int argc, char* argv[])
{
    const char *srcname = argv[1];
    const char *dstname = argv[2];
    unsigned    clut[256];
    void        *pix;
    int         w, h, bpp;

    if (argc != 3) {
        printf("usage> smp_tga2bmp.exe in_file.tga out_file.bmp\n");
        return 1;
    }

    // tgaファイル読込.
    // 出力bpp=0で入力に同じ. 横幅アライメントは1のままにしておく.
    pix = tga_read_file(srcname, &w, &h, &bpp, clut, 0, 1, 0);

    if (pix) {
        // bmp書込.
        int width_byte = tga_width2byte(w, bpp, 1); // 入力画像の横幅バイト数.
        int dst_bpp    = bpp;                       // 出力bpp.

        bmp_write_file(dstname, w, h, dst_bpp, pix, width_byte, bpp, clut, 0);

        free(pix);
    }
    return 0;
}
