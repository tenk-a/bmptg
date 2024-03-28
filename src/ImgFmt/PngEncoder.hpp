/**
 *  @file PngEncoder.hpp
 *  @brief  メモリ上にpng画像データを作成.
 *  @author Masashi KITAMURA
 */
#ifndef PNGENCODER_HPP
#define PNGENCODER_HPP

#include "libpng/png.h"
#include <assert.h>

class PngEncoder {
public:
    PngEncoder();
    ~PngEncoder() {;}

    PngEncoder(unsigned char* dst, unsigned& dstSz, const void* src, unsigned wid, unsigned hei, unsigned bpp, const unsigned* clut=0, unsigned widByt=0, unsigned dir=0) {
        unsigned sz = write(dst,dstSz, src, wid, hei, bpp, clut, widByt, dir);
        dstSz = sz;
    }

    /// 内部で確保しているメモリを開放. 辻褄合わせ用のダミー.
    void    release() {;}

    /// pix に24ビット色画像を展開する. サイズは0だとデフォルトのまま. dirは0が左上から1なら左下から.
    unsigned    write(unsigned char* dst, unsigned dstSz, const void* src, unsigned wid, unsigned hei, unsigned bpp, const unsigned* clut=0, unsigned widByt=0, unsigned dir=0);

    /// malloc したメモリにpngデータを入れて返す.
    unsigned char*  write(unsigned& rSz, const void* src, unsigned w, unsigned h, unsigned bpp, const unsigned* clut=0, unsigned widByt=0, unsigned dir=0);

    /// writeでポインタを受け取った場合の対となるメモリ開放.
    static void     deallocate(void* p);

    unsigned    workSize(unsigned w, unsigned h, unsigned bpp);
    void        setInterlace(bool sw) { interlacing_ = sw; }
    void        setWidthAlign(unsigned algn);
    void        setWidthByte(unsigned widByt);

    /// 横幅をバイト数に変換.
    static unsigned wid2byt(unsigned wid, unsigned algn=1) {
        assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
        return (wid*3 + algn-1) & ~(algn-1);
    }

private:
    static void raw_write_fn(png_structp png_ptr, png_bytep buf, png_size_t size);
    static void raw_IO_flush_function(png_structp png_ptr);
    static bool haveAlpha(const unsigned* pix, unsigned w, unsigned h);

private:
    unsigned char*  data_;
    unsigned        dataSize_;
    bool            interlacing_;
    unsigned        align_;
    unsigned        widByt_;
    unsigned        cur_;
};


#endif  // PNGENCODER_H
