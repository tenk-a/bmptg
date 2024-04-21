/**
 *  @file JpgDecoder.cpp
 *  @brief  ���������jpg�摜�f�[�^��W�J����.
 *  @author Masashi KITAMURA
 *  @note
 *      - Independent JPEG Group��libjpeg.lib
 *             �܂��� libjpeg-turbo.lib ��p���Ă��܂��B
 *
 *      - �g����
 *          - setData(binData,size) �Ń������[���jpg�f�[�^��ݒ肵�A
 *            ���̌� read()�����malloc������������24�r�b�g�F�摜�𓾂�.
 *          - ���邢��setData()��, �T�C�YwidthByte()*height() �ȏ�̃�������
 *            �Ăь��ŗp�ӂ���read(pix, ...)���g��.
 *
 *      - �܂��A�G���[������[�܂��ăf�t�H���g�̂܂܂Ȃ̂ŁA
 *        �R�}���h���C���c�[���ȊO�ł͎g���Ȃ�.
 */


#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#if 1 //def USE_JPG_TURBO
#include "jpeglib.h"
#elif defined USE_MOZJPEG
#include "jpeglib.h"
#else
extern "C" {
#include "jpeglib.h"
}
#endif

#include "JpgDecoder.hpp"



// ===========================================================================
// ===========================================================================
// ���������jpeg�f�[�^���������߂̃��[�`��.

// namespace JpgOnMem {



struct JpgOnMemMgr {
    struct jpeg_source_mgr  pub;                    /* public fields */
    JOCTET*                 buffer;
};



static void jpgOnMem_init_source(j_decompress_ptr cinfo)
{
}



static boolean jpgOnMem_fill_input_buffer(j_decompress_ptr cinfo)
{
    JpgOnMemMgr* src = (JpgOnMemMgr*) cinfo->src;

    src->buffer[0]              = (JOCTET) 0xFF;
    src->buffer[1]              = (JOCTET) JPEG_EOI;
    src->pub.next_input_byte    = src->buffer;
    src->pub.bytes_in_buffer    = 2;
    return TRUE;
}



static void jpgOnMem_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    JpgOnMemMgr* src = (JpgOnMemMgr*) cinfo->src;

    if (num_bytes > 0) {
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}



static void jpgOnMem_term_source(j_decompress_ptr cinfo)
{
}




/**
 */
static void jpgOnMem_src(j_decompress_ptr cinfo, void *data, size_t size)
{
    JpgOnMemMgr* src;

    if (cinfo->src == NULL) {
        // �������m��.
        src         = (JpgOnMemMgr*)(*cinfo->mem->alloc_small) ( (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(JpgOnMemMgr) );
        cinfo->src  = (struct jpeg_source_mgr *)src;
        src->buffer = (JOCTET *)(*cinfo->mem->alloc_small)( (j_common_ptr)cinfo, JPOOL_PERMANENT, size * sizeof(JOCTET) );
    }

    src = (JpgOnMemMgr*) cinfo->src;

    // �g�p���郋�[�`����ݒ�.
    src->pub.init_source        = jpgOnMem_init_source;
    src->pub.fill_input_buffer  = jpgOnMem_fill_input_buffer;
    src->pub.skip_input_data    = jpgOnMem_skip_input_data;
    src->pub.resync_to_restart  = jpeg_resync_to_restart;   /* use default method */
    src->pub.term_source        = jpgOnMem_term_source;

    // �o�b�t�@�̏���.
    src->pub.next_input_byte    = (JOCTET *) data;
    src->pub.bytes_in_buffer    = size;
}



// }    // namespce JpgOnMem;







// ===========================================================================
// ===========================================================================
// �O���Ɍ�����N���X.



JpgDecoder::JpgDecoder()
    :   info_(0)
    ,   jerr_(0)
    ,   binData_(0)
    ,   binDataSize_(0)
    ,   closeRq_(0)
    ,   thumbnail_(false)
    ,   originalWidth_(0)
    ,   originalHeight_(0)
{
    memset(gray_clut_, 0, sizeof gray_clut_);
}



JpgDecoder::JpgDecoder(const void* binData, unsigned binDataSize, bool thumbnail)
    :   info_(0)
    ,   jerr_(0)
    ,   binData_(0)
    ,   binDataSize_(0)
    ,   closeRq_(0)
    ,   thumbnail_(false)
    ,   originalWidth_(0)
    ,   originalHeight_(0)
    ,   bpp_(0)
{
    setData(binData, binDataSize, thumbnail);
}




JpgDecoder::~JpgDecoder()
{
    release();
}



/// �����Ŋm�ۂ��Ă��郁�������J��.
void JpgDecoder::release() {
    if (closeRq_) {
        //jpeg_finish_decompress(info_);
        jpeg_destroy_decompress(info_);
        closeRq_ = false;
    }
    delete info_;
    info_ = NULL;
    delete jerr_;
    jerr_ = NULL;
}



unsigned    JpgDecoder::width()  const
{
    assert(info_ != NULL);
    return info_->output_width;
}



unsigned    JpgDecoder::height() const
{
    assert(info_ != NULL);
    return info_->output_height;
}




/** ���������jpg�f�[�^��ݒ�.
 * �|�C���^��ێ����邾���Ȃ̂�read()���I����O��binData��j�󂵂Ȃ�����.
 */
bool    JpgDecoder::setData(const void* binData, unsigned binDataSize, bool thumbnail)
{
    jpeg_decompress_struct* info = new jpeg_decompress_struct();
    if (!info)
        return false;
    jpeg_error_mgr*         jerr = new jpeg_error_mgr();
    if (!jerr)
        return false;
    info_       = info;
    jerr_       = jerr;
    info_->err  = jpeg_std_error(jerr_);

    jpeg_create_decompress(info_);
    jpgOnMem_src(info_, (void*)binData, binDataSize );
    jpeg_read_header(info_, TRUE);

    int w = info_->image_width;
    int h = info_->image_height;
    originalWidth_  = w;
    originalHeight_ = h;
    enum { K = 1 << 8 };
    if (thumbnail && w >= 2*K && h >= 2*K) {
        thumbnail_          = thumbnail;
        info_->dct_method   = (J_DCT_METHOD)JDCT_IFAST;
        info_->scale_num    = 1;
        info_->scale_denom  = (w >= 8*K && h >= 8*K) ? 8 : (w >= 4*K && 4*K >= 256) ? 4 : 2;
    }

    jpeg_start_decompress(info_);

    bpp_ = info_->output_components * 8;
    if (bpp_ != 24 && bpp_ != 8) {
        assert(bpp_ == 24 || bpp_ == 8);
        bpp_ = 24;
    }
    if (bpp_ == 8) {
        for (unsigned i = 0; i < 256; ++i) {
            gray_clut_[i] = (0xFF<<24)|(i << 16)|(i << 8)|i;
        }
    }

    closeRq_            = true;         // ������������Ō��close���K�v..
    return true;
}




/** pix ��24�r�b�g�F�摜��W�J����.
 * widByt,hei�̃T�C�Y��0���ƃf�t�H���g�̂܂�.
 * dir��0�����ォ��1�Ȃ獶������.
 */
bool  JpgDecoder::read(void* pix, unsigned widByt, unsigned hei, unsigned dir)
{
    assert(closeRq_ == true);           // setData��ʉ߂��Ă��Ȃ��Ƒʖ�.

    unsigned    row_stride  = info_->output_width * info_->output_components;
    JSAMPARRAY  buffer      = (*info_->mem->alloc_sarray) ((j_common_ptr)info_, JPOOL_IMAGE, row_stride, 1);

    if (hei == 0)
        hei    = height();

    if (widByt == 0)
        widByt = widthByte();

    if (row_stride > widByt)
        row_stride = widByt;

    int             i   = 0;
    unsigned char*  m   = 0;
    int             rst = int(widByt - row_stride);

    if ((dir & 1) == 0) {   // ���ォ��s�N�Z�����l�߂�.
        while (info_->output_scanline < info_->output_height) {
            jpeg_read_scanlines(info_, buffer, 1);
            m = (unsigned char*)pix + i * widByt;
            memcpy(m, buffer[0], row_stride);
            if (rst > 0)
                memset(m + row_stride, 0, rst);
            ++i;
        }
    } else {                // ��������s�N�Z�����l�߂�.
        while (info_->output_scanline < info_->output_height) {
            jpeg_read_scanlines(info_, buffer, 1);
            m = (unsigned char*)pix + (hei - 1 - i) * widByt;
            memcpy(m, buffer[0], row_stride);
            if (rst > 0)
                memset(m + row_stride, 0, rst);
            ++i;
        }
    }

    jpeg_finish_decompress(info_);
    jpeg_destroy_decompress(info_);
    closeRq_ = true;

    return 1;
}



/** malloc �����������ɉ摜�����ĕԂ�.
 *  @param  widAlgn     �����o�C�g���̃A���C�����g. 1,2,4,8��z��.
 *                      �f�t�H���g 1. bmp�ɍ��킹�����ꍇ�� 4��ݒ�̂���.
 *  @param  dir         0:���ォ��l�߂�.  1:��������l�߂�.bmp�ɍ��킹�����ꍇ�� 1��ݒ�̂���.
 */
void* JpgDecoder::read(unsigned widAlgn, unsigned dir) {
    unsigned wb = this->widthByte(widAlgn);
    unsigned h  = this->height();
    void*   m = calloc(1, wb * h);
    if (m)
        this->read(m, wb, h, dir);
    return m;
}
