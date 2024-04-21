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

    // tga�t�@�C���Ǎ�.
    // �o��bpp=0�œ��͂ɓ���. �����A���C�����g��1�̂܂܂ɂ��Ă���.
    pix = tga_read_file(srcname, &w, &h, &bpp, clut, 0, 1, 0);

    if (pix) {
        // bmp����.
        int width_byte = tga_width2byte(w, bpp, 1); // ���͉摜�̉����o�C�g��.
        int dst_bpp    = bpp;                       // �o��bpp.

        bmp_write_file(dstname, w, h, dst_bpp, pix, width_byte, bpp, clut, 0);

        free(pix);
    }
    return 0;
}
