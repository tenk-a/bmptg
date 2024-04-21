#include <stdio.h>
#include <stdlib.h>
#define  inline __inline
#include "../ImgFmt/bmp_read.h"
#include "../ImgFmt/tga_wrt.h"


int main(int argc, char* argv[])
{
    const char *src_name = argv[1];
    const char *dst_name = argv[2];
    unsigned    clut[256];
    void        *pix;
    int         w, h, bpp;

    if (argc != 3) {
        printf("usage> smp_bmp2tga.exe in_file.bmp out_file.tga\n");
        return 1;
    }

    // bmp���t�@�C������Ǎ�.
    // �o��bpp=0�œ��͂ɓ���. �����A���C�����g��4�̂܂܂ɂ��Ă���.
    pix = bmp_read_file(src_name, &w, &h, &bpp, clut, 0, 4, 0);
    if (pix) {
        int width_byte = bmp_width2byte(w, bpp, 4); // ���͉摜�̉����o�C�g��.
        int dst_bpp    = bpp <=8 ? 8 : bpp;         // tga��2�F16�F�������̂�256�F��.
        tga_write_file(dst_name, w, h, dst_bpp, pix, width_byte, bpp, clut, 0);
        free(pix);
    }
    return 0;
}

