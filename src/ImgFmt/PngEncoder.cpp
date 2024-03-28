/**
 *  @file PngEncoder.cpp
 *  @brief  メモリ上にpng画像データを作成.
 *  @author Masashi KITAMURA
 */
#include <stdlib.h>     // callocをヘッダで使用するため.
#include <string.h>
#include <assert.h>

#include "libpng/png.h"
#include "libpng/pngstruct.h"

#include "PngEncoder.hpp"

#ifdef __WATCOMC__
#  undef png_jmpbuf
#  define png_jmpbuf(png_ptr) (*png_set_longjmp_fn((png_ptr), (png_longjmp_ptr)longjmp, (sizeof (jmp_buf))))
#endif


PngEncoder::PngEncoder() {
    memset(this, 0, sizeof *this);
}


/// writeでポインタを受け取った場合の対となるメモリ開放.
void    PngEncoder::deallocate(void* p) {
    if (p)
        free(p);
}


/** malloc したメモリにpngデータを入れて返す. メモリは、かなり大きめに取っているので注意.
 */
unsigned char* PngEncoder::write(unsigned& rSz, const void* src, unsigned w, unsigned h, unsigned bpp, const unsigned* clut, unsigned widByt, unsigned dir) {
    if (widByt == 0)
        widByt = wid2byt(w, bpp);
    rSz = 0;
    unsigned        size = widByt * h;
    unsigned char*  m    = (unsigned char*)calloc(1, size);
    if (m)
        rSz = write( m, size, src, w, h, bpp, clut, widByt, dir);
    return m;
}


/// pix に画像を展開する. サイズは0だとデフォルトのまま. dirは0が左上から1なら左下から.
unsigned    PngEncoder::write(unsigned char* dst, unsigned dstSz, const void* src, unsigned width, unsigned height, unsigned bpp, const unsigned* clut, unsigned widByt, unsigned dir)
{
    assert(bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8 || bpp == 24 || bpp == 32 || bpp == 13);
    unsigned        clutSize  = (bpp <= 8) ? 1 << bpp : 0;
    bool            alpFlag   = (clut && clutSize) ? haveAlpha(clut, 1, clutSize) : 0;

    png_structp     png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
        return 0;

    png_infop       info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        return 0;
    }

 #ifdef PNG_SETJMP_SUPPORTED
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 0;
    }
 #endif

    data_     = dst;
    dataSize_ = dstSz;

    //x png_init_io(png_ptr, fp);
    png_set_write_fn(png_ptr, (void *)this, (png_rw_ptr)raw_write_fn, (png_flush_ptr)raw_IO_flush_function);

    png_set_bgr(png_ptr);

    int typ = (bpp == 24) ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGB_ALPHA;
    if (bpp <= 8) {
        if (clut) {
            typ = PNG_COLOR_TYPE_PALETTE;
            png_color   palette[256];
            memset(palette, 0, sizeof palette);
            for (unsigned i = 0; i < clutSize; ++i) {
                palette[i].red   = (png_byte)(clut[i] >> 16);
                palette[i].green = (png_byte)(clut[i] >>  8);
                palette[i].blue  = (png_byte)clut[i];
            }
            png_set_PLTE(png_ptr, info_ptr, &palette[0], clutSize);
            if (alpFlag) {
                unsigned char alpBuf[256];
                memset(alpBuf, 0, sizeof alpBuf);
                for (unsigned i = 0; i < clutSize; ++i) {
                    alpBuf[i] = clut[i] >> 24;
                }
                png_set_tRNS(png_ptr, info_ptr, alpBuf, clutSize, NULL);
            }
        } else {
            typ = PNG_COLOR_TYPE_GRAY;
        }
    } else if (bpp == 13) {
		typ  = PNG_COLOR_TYPE_GRAY_ALPHA;
		bpp  = 16;
    }
    int pngbpp = (bpp <= 8) ? bpp : 8;
    png_set_IHDR(png_ptr, info_ptr, width, height, pngbpp, typ, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytepp  lines = (png_bytepp)png_malloc(png_ptr, height*sizeof(png_bytep));  //x new png_bytep[height];
    if (dir & 1) {  //上下反転するとき.
        for (int i = height; --i >= 0;)
            lines[i] = (png_bytep)src + (height-1-i) * widByt;
    } else {
        for (unsigned i = 0; i < height; ++i)
            lines[i] = (png_bytep)src + i * widByt;
    }

    png_write_image(png_ptr, (png_bytepp)lines);

    png_write_end(png_ptr, info_ptr);
    png_free(png_ptr, lines);   //x delete lines;
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return cur_;
}


void PngEncoder::raw_write_fn(png_structp png_ptr, png_bytep buf, png_size_t size)
{
    PngEncoder  *pOwn = (PngEncoder*)png_ptr->io_ptr;
    assert(pOwn != NULL);
    if (pOwn->cur_+size > pOwn->dataSize_) {
        png_error(png_ptr, "PNG ENCODE ERROR(not enough memory)");
        return;
    }
    memcpy(pOwn->data_+pOwn->cur_, buf, size);
    assert(size <= ~0U);
    pOwn->cur_ += (unsigned)size;
}


void PngEncoder::raw_IO_flush_function(png_structp /*png_ptr*/)
{
}


bool PngEncoder::haveAlpha(const unsigned* p, unsigned w, unsigned h)
{
    int             chk;
    const unsigned* e  = p + w * h;
    assert(p != 0 && w != 0 && h != 0);
    chk = (unsigned char)(*p++ >> 24);
    if (0 < chk && chk < 255)
        return true;
    while (p < e) {
        int a   = (unsigned char)(*p++ >> 24);
        if (a != chk)
            return true;
    }
    return false;
}
