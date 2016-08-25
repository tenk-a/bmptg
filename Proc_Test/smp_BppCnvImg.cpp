#include "../Proc/BppCnvImg.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "../ImgFmt/tga_read.h"
#include "../ImgFmt/tga_wrt.h"
#include <string>
#include <vector>
#include "../misc/misc.h"
// #include "ReduceColorMC.h"


using namespace std;
//x using namespace MISC;


int tga_load(const std::string fname, BppCnvImg& img)
{
    std::vector<unsigned char> buf;
    if (file_load(fname.c_str(), buf) == 0)
        return 0;

    int w, h, bpp, clutSize;
    if ( tga_getHdr(&buf[0], &w, &h, &bpp, &clutSize) == 0 )
        return 0;

    if (bpp == 16)
        bpp = 15;
    img.create(w, h, bpp, 1, clutSize);
    return tga_read(&buf[0], img.image(), img.widthByte(), img.height(), bpp, img.clut(), 0);
}



void tga_save(const std::string fname, const BppCnvImg& img)
{
    int dst_bpp    = img.fmt() <=8 ? 8 : img.fmt() <= 16 ? 16 : img.fmt();

    tga_write_file(fname.c_str(), img.width(), img.height(), dst_bpp,
                img.image(), img.widthByte(), img.fmt(), img.clut(), 0x40);
}


void conv(const char* fname)
{
    std::string src_name = fname;
    BppCnvImg   img;
    BppCnvImg   img2;
    BppCnvImg   img0;
    if (tga_load(src_name, img0) == 0) {
        printf("%s : tgaƒtƒ@ƒCƒ‹‚Ì“Ç‚Ýž‚Ý‚ÉŽ¸”s\n", src_name);
        exit(1);
    }
    std::string bname = fname_chgExt(src_name, NULL);
    img = img0;
    tga_save(bname+"_0.tga", img);

    img.create(img0.width(), img0.height(), 8, 1, 256);
    BppCnvImg::convEx(img, img0);
    img.swapToAGRB();
    img.swapFromAGRB();
    img.revY();
    img.revY();
    tga_save(bname+"_08.tga", img);

    img.create(img0.width(), img0.height(), 8, 1, 256);
    convGRB332(img, img0);
    tga_save(bname+"_08jp.tga", img);

    img.create(img0.width(), img0.height(), 8, 1, 256);
    img.setDitherMode(1);
    convGRB332(img, img0);
    tga_save(bname+"_08jp_dit.tga", img);

    img.create(img0.width(), img0.height(), 8, 1, 256);
    convGray(img, img0);
    tga_save(bname+"_08gray.tga", img);



    img.create(img0.width(), img0.height(), 4, 1);
    BppCnvImg::convEx(img, img0);
    img.swapBitOrder();
    img.swapToAGRB();
    img.swapFromAGRB();
    img.revY();
    img.revY();
    tga_save(bname+"_04.tga", img);

    img.create(img0.width(), img0.height(), 4, 1);
    convGRB111(img, img0);
    img.swapBitOrder();
    tga_save(bname+"_04jp.tga", img);

    img.create(img0.width(), img0.height(), 4, 1);
    img.setDitherMode(1);
    convGRB111(img, img0);
    img.swapBitOrder();
    tga_save(bname+"_04jp_dit.tga", img);

    img.create(img0.width(), img0.height(), 4, 1);
    convGray(img, img0);
    img.swapBitOrder();
    tga_save(bname+"_04gray.tga", img);

    img.create(img0.width(), img0.height(), 4, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img.swapBitOrder();
    tga_save(bname+"_04grayDit.tga", img);


    img.create(img0.width(), img0.height(), 2, 1);
    BppCnvImg::convEx(img, img0);
    img2.create(img, 8 );
    img.swapToAGRB();
    img.swapFromAGRB();
    img.revY();
    img.revY();
    tga_save(bname+"_02.tga", img2);

    img.create(img0.width(), img0.height(), 2, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img2.create(img, 8 );
    tga_save(bname+"_02gray.tga", img2);
    /*
    img.create(img0.width(), img0.height(), 2, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img2.create(img, 24 );
    tga_save(bname+"_02dit.tga", img2);
    */

    img.create(img0.width(), img0.height(), 1, 1);
    BppCnvImg::convEx(img, img0);
    img2.create(img, 8 );
    img.swapToAGRB();
    img.swapFromAGRB();
    img.revY();
    img.revY();
    tga_save(bname+"_01.tga", img2);

    img.create(img0.width(), img0.height(), 1, 1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_01gray.tga", img2);

    img.create(img0.width(), img0.height(), 1, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_01gray_dit.tga", img2);



    img.create(img0.width(), img0.height(), 12, 1);
    img.convEx(img, img0);
    img.revY();
    img.revY();
    img2.create(img, 32 );
    tga_save(bname+"_12.tga", img2);

    img.create(img0.width(), img0.height(), 12, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_12dit.tga", img2);

    img.create(img0.width(), img0.height(), 12, 1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_12gray.tga", img2);

    img.create(img0.width(), img0.height(), 12, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_12grayDit.tga", img2);



    img.create(img0.width(), img0.height(), 15, 1);
    img.convEx(img, img0);
    img.swapByteOrder();
    img.swapByteOrder();
    img2.create(img, 32 );
    tga_save(bname+"_15.tga", img2);

    img.create(img0.width(), img0.height(), 15, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img.revY();
    img.revY();
    img.swapToAGRB();
    img.swapFromAGRB();
    img2.create(img, 32 );
    tga_save(bname+"_15dit.tga", img2);

    img.create(img0.width(), img0.height(), 15, 1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_15gray.tga", img2);

    img.create(img0.width(), img0.height(), 15, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_15grayDit.tga", img2);


    img.create(img0.width(), img0.height(), 16, 1);
    img.convEx(img, img0);
    img.revY();
    img.revY();
    img.swapByteOrder();
    img.swapByteOrder();
    img.swapToABGR();
    img.swapFromABGR();
    img2.create(img, 32 );
    tga_save(bname+"_16.tga", img2);

    img.create(img0.width(), img0.height(), 16, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img.swapToAGRB();
    img.swapFromAGRB();
    img2.create(img, 32 );
    tga_save(bname+"_16dit.tga", img2);

    img.create(img0.width(), img0.height(), 16, 1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_16gray.tga", img2);

    img.create(img0.width(), img0.height(), 16, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img2.create(img, 32 );
    tga_save(bname+"_16grayDit.tga", img2);


    img.create(img0.width(), img0.height(), 24, 1);
    img.convEx(img, img0);
    img.swapToAGRB();
    img.swapFromAGRB();
    img.revY();
    img.revY();
    tga_save(bname+"_24.tga", img);

    img.create(img0.width(), img0.height(), 24, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img.swapToAGRB();
    img.swapFromAGRB();
    tga_save(bname+"_24dit.tga", img);

    img.create(img0.width(), img0.height(), 24, 1);
    convGray(img, img0);
    tga_save(bname+"_24gray.tga", img);

    img.create(img0.width(), img0.height(), 24, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    tga_save(bname+"_24grayDit.tga", img);


    img.create(img0.width(), img0.height(), 32, 1);
    img.convEx(img, img0);
    img.swapByteOrder();
    img.swapToBGRA();
    img.revY();
    img.revY();
    tga_save(bname+"_32.tga", img);

    img.create(img0.width(), img0.height(), 32, 1);
    img.setDitherMode(1);
    img.convEx(img, img0);
    img.swapByteOrder();
    img.swapFromBGRA();
    tga_save(bname+"_32dit.tga", img);

    img.create(img0.width(), img0.height(), 32, 1);
    convGray(img, img0);
    img.swapToAGRB();
    img.swapFromAGRB();
    tga_save(bname+"_32gray.tga", img);

    img.create(img0.width(), img0.height(), 32, 1);
    img.setDitherMode(1);
    convGray(img, img0);
    img.swapToGRBA();
    img.swapFromGRBA();
    img.revY();
    img.revY();
    tga_save(bname+"_32grayDit.tga", img);
}




int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("usage> smp_bppCnvImg.exe in_file.tga\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        const char* p = argv[i];
        conv(p);
    }
    return 0;
}


