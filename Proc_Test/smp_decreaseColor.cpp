/**
 *  @file   smp_decreaseColor.cpp
 *  @brief  減色ルーチンの使用サンプル.
 *  @author Masashi Kitamura
 *  @date   2007
 */

//#include "DecreaseColor.h"        // コンパイラのデフォルトinclude機能を使うので、コメントアウト.
#include <stdio.h>
#include <stdlib.h>
#include "tga_read.h"
#include "tga_wrt.h"
#include <vector>


int conv(const char* srcname, const char* dstname)
{
    struct SrcPtr {
        SrcPtr(unsigned* ptr) : ptr_(ptr) {;}
        ~SrcPtr() { if (ptr_) free(ptr_); }
        unsigned* ptr_;
    };
    int w=0, h=0, bpp=0;
    SrcPtr src ( (unsigned*)tga_read_file(srcname, &w, &h, &bpp, NULL, 32) );
    if (src.ptr_ == NULL) {
        printf("%s : load error\n", srcname);
        return -1;
    }
    std::vector<unsigned char> dst( w * h );
    std::vector<unsigned>      clut( 256+1 );

    DecreaseColor<>     dc;
    int rc = dc.conv( &dst[0], src.ptr_, w, h, &clut[0], 256 );
    if (rc == false) {
        printf("%s : convert error\n", srcname);
        return -1;
    }

    rc = tga_write_file(dstname, w, h, 8, &dst[0], w, 8, &clut[0], 0x40);
    if (rc == 0) {
        printf("%s : save error\n", dstname);
        return -1;
    }
    return 0;
}



int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("usage> %s in_file.tga out_file.tga\n", argv[0]);
        return 1;
    }

    return conv(argv[1], argv[2]);
}


