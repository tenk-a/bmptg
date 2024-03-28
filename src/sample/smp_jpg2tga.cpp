#include <stdio.h>
#include <stdlib.h>
#include "../ImgFmt/JpgDecoder.hpp"
#include "../ImgFmt/tga_wrt.h"
#include "../misc/misc.hpp"
#include "../Proc/BppCnvImg.hpp"


static void convRGB(void* pic, unsigned bytes);


int main(int argc, char* argv[])
{
    const char *src_name = argv[1];
    const char *dst_name = argv[2];

    if (argc != 3) {
        printf("usage> smp_jpg2tga.exe in_file.jpg out_file.tga\n");
        return 1;
    }

    std::vector<unsigned char> buf;
    file_load(src_name, buf);
    JpgDecoder dec(&buf[0], buf.size());
    std::vector<unsigned char> img( dec.imageByte() );
    bool rc = dec.read(&img[0]);
    if (rc) {
        unsigned clut[1024] = {0};
        int dst_bpp    = dec.bpp() <=8 ? 8 : dec.bpp();         // tga‚Í2F16F‚ª–³‚¢‚Ì‚Å256F‚É.
        printf("%s %d*%d bpp=%d wb=%d dec.bpp=%d\n", dst_name, dec.width(), dec.height(), dst_bpp, dec.widthByte(), dec.bpp());
        convRGB(&img[0], dec.imageByte() );
        tga_write_file(dst_name, dec.width(), dec.height(), dst_bpp, &img[0], dec.widthByte(), dec.bpp(), clut, 0);
    }
    return 0;
}


static void convRGB(void* pic, unsigned bytes)
{
    unsigned n = bytes / 3;
    unsigned char* p = (unsigned char*)pic;
    unsigned char* e = p + n * 3;
    while (p < e) {
        unsigned r = p[0];
        unsigned g = p[1];
        unsigned b = p[2];
        p[0] = b;
        p[1] = g;
        p[2] = r;
        p += 3;
    }
}
