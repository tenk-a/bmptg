/**
 *  @file JpgEncoder.cpp
 *  @brief  画像をjpgデータに変換.
 *  @author Masashi KITAMURA
 *  @note
 *      - Independent JPEG Groupのlibjpeg.libを用いています。
 *
 *      - まだ、エラー処理を端折ってデフォルトのままなので、
 *        コマンドラインツール以外では使えない.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if 1 //def USE_JPG_TURBO
#include "libjpeg-turbo/jpeglib.h"
#elif defined USE_MOZJPEG
#include "mozjpeg/jpeglib.h"
#else
extern "C" {
#include "jpeg/jpeglib.h"
}
#endif

#include "JpgEncoder.hpp"



// ===========================================================================
// ===========================================================================
// メモリ上のjpegデータを扱うためのルーチン.

// namespace JpgOnMem {



struct JpgOnMemMgr {
    struct jpeg_destination_mgr     pub;                    /* public fields */
};



static void jpgOnMem_init_destination(j_compress_ptr cinfo)
{
}



static boolean  jpgOnMem_empty_output_buffer (j_compress_ptr cinfo)
{
    //x exit(1);
    return FALSE;
}



static void jpgOnMem_term_destination(j_compress_ptr cinfo)
{
}




/**
 */
static void jpgOnMem_dst(j_compress_ptr cinfo, void *data, size_t size)
{
    JpgOnMemMgr* dest;

    if (cinfo->dest == NULL) {
        dest        = (JpgOnMemMgr*)(*cinfo->mem->alloc_small) ( (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(JpgOnMemMgr) );
        cinfo->dest = (struct jpeg_destination_mgr *)dest;
    }

    dest = (JpgOnMemMgr*) cinfo->dest;

    // 使用するルーチンを設定.
    dest->pub.init_destination    = jpgOnMem_init_destination;
    dest->pub.empty_output_buffer = jpgOnMem_empty_output_buffer;
    dest->pub.term_destination    = jpgOnMem_term_destination;

    // バッファの準備.
    dest->pub.next_output_byte    = (JOCTET *) data;
    dest->pub.free_in_buffer      = size;
}



// }    // namespce JpgOnMem;







// ===========================================================================
// ===========================================================================
// 外部に見せるクラス.

/** メモリ上のjpgデータを設定.
 * ポインタを保持するだけなのでread()を終える前にbinDataを破壊しないこと.
 */
unsigned JpgEncoder::write(
        unsigned char*  binData,
        unsigned        binDataSize,
        const void*     pixBGR,
        unsigned        wid,
        unsigned        hei,
        unsigned        quality,
        unsigned        widByt,
        unsigned        dir,
        unsigned        mono)
{
    unsigned char*          pix = (unsigned char*) pixBGR;
    jpeg_compress_struct    info;
    jpeg_error_mgr          jerr;

    memset(&info, 0, sizeof info);
    info.err            = jpeg_std_error(&jerr);

    jpeg_create_compress(&info);
    jpgOnMem_dst(&info, (void*)binData, binDataSize );

    info.image_width        = wid;
    info.image_height       = hei;
    if (mono) {
        info.input_components   = 1;
        info.in_color_space     = JCS_GRAYSCALE;
    } else {
        info.input_components   = 3;
        info.in_color_space     = JCS_RGB;
    }
    jpeg_set_defaults(&info);
    jpeg_set_quality(&info, quality, TRUE /* limit to baseline-JPEG values */ );
    jpeg_start_compress(&info, TRUE);

    if (widByt == 0)
        widByt = wid * info.input_components;
    assert( wid*info.input_components <= widByt );

    JSAMPROW    row_pointer[2];
    row_pointer[1] = NULL;
    if ((dir & 1) == 0) {   // 左上からピクセルを詰める.
        while (info.next_scanline < info.image_height) {
            row_pointer[0] = &pix[info.next_scanline * widByt];
            jpeg_write_scanlines(&info, row_pointer, 1);
        }
    } else {
        while (info.next_scanline < info.image_height) {
            row_pointer[0] = &pix[(hei-1-info.next_scanline) * widByt];
            jpeg_write_scanlines(&info, row_pointer, 1);
        }
    }

    jpeg_finish_compress(&info);
    size_t sz = binDataSize - ((JpgOnMemMgr*)info.dest)->pub.free_in_buffer;
    assert(sz <= ~0U);
    jpeg_destroy_compress(&info);
    return (unsigned)sz;
}


/// writeでポインタを受け取った場合の対となるメモリ開放.
void JpgEncoder::deallocate(void* p) {
    if (p)
        free(p);
}


/** malloc したメモリにjpgデータを入れて返す. メモリは、かなり大きめに取っているので注意.
 */
unsigned char* JpgEncoder::write(const void* src, unsigned w, unsigned h, unsigned quality, unsigned widByt, unsigned dir, unsigned* pSz) {
    if (widByt == 0)
        widByt = w * 3;
    unsigned        size = widByt * h;
    unsigned char*  m    = (unsigned char*)calloc(1, size);
    unsigned        sz   = 0;
    if (m)
        sz = write( m, size, src, w, h, quality, widByt, dir);
    if (pSz)
        *pSz = sz;
    return m;
}
