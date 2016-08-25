#include <stdio.h>
#include <stdlib.h>
#include "../ImgFmt/PngDecoder.hpp"
#include "../ImgFmt/tga_wrt.h"
#include "../misc/misc.hpp"

int main(int argc, char* argv[])
{
    const char *src_name = argv[1];
    const char *dst_name = argv[2];

    if (argc != 3) {
        printf("usage> smp_png2tga.exe in_file.png out_file.tga\n");
        return 1;
    }

    std::vector<unsigned char> buf;
    file_load(src_name, buf);
    PngDecoder dec(&buf[0], buf.size());
    std::vector<unsigned char> img( dec.imageByte() );
    bool rc = dec.read(&img[0]);
    if (rc) {
        unsigned clut[1024] = {0};
        int dst_bpp    = dec.bpp() <=8 ? 8 : dec.bpp();         // tga‚Í2F16F‚ª–³‚¢‚Ì‚Å256F‚É.
        if (dst_bpp <= 8)
            dec.getClut(clut, 1024);
        printf("%s %d*%d bpp=%d wb=%d dec.bpp=%d\n", dst_name, dec.width(), dec.height(), dst_bpp, dec.widthByte(), dec.bpp());
        tga_write_file(dst_name, dec.width(), dec.height(), dst_bpp, &img[0], dec.widthByte(), dec.bpp(), clut, 0);
    }
    return 0;
}

