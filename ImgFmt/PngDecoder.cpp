/**
 *  @file PngDecoder.cpp
 *  @brief  ���������jpg�摜�f�[�^��W�J����.
 *  @author Masashi KITAMURA
 *  @note
 *
 *      - �g����
 *          - setData(binData,size) �Ń������[���jpg�f�[�^��ݒ肵�A
 *            ���̌� read()�����malloc������������24�r�b�g�F�摜�𓾂�.
 *          - ���邢��setData()��, �T�C�YwidthByte()*height() �ȏ�̃�������
 *            �Ăь��ŗp�ӂ���read(pix, ...)���g��.
 */

#include <stdlib.h>     // calloc���w�b�_�Ŏg�p���邽��.
#include <string.h>
#include <assert.h>

#include "libpng/png.h"
#include "libpng/pngstruct.h"

#include "PngDecoder.hpp"


#define ARGB(a,r,g,b)   ((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))
#define WID2BYT(w,bpp)  (((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)

#ifdef __WATCOMC__
#  undef png_jmpbuf
#  define png_jmpbuf(png_ptr) (*png_set_longjmp_fn((png_ptr), (png_longjmp_ptr)longjmp, (sizeof (jmp_buf))))
#endif
#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr_) ((png_ptr_)->jmpbuf)
#endif


PngDecoder::PngDecoder()
{
    memset(this, 0, sizeof *this);
}


PngDecoder::~PngDecoder()
{
    release();
}


PngDecoder::PngDecoder(const void* binData, unsigned binDataSize)
{
    memset(this, 0, sizeof *this);
    setData(binData, binDataSize);
}


/// �����Ŋm�ۂ��Ă��郁�������J��.
void    PngDecoder::release()
{
    if (png_ptr_) {
        png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
        memset(this, 0, sizeof *this);
    }
}


unsigned    PngDecoder::imageByte()  const {    ///< �W�J��̃o�C�g����Ԃ�.
    unsigned wb = widthByte(align_);
    return wb * height_;
}


void        PngDecoder::setWidthAlign(unsigned align) {
    assert(align == 1 || align == 2 || align == 4 || align == 8 || align == 16);
    align_ = align;
}


/// algn�o�C�g�ɃA���C�����g�ς̉����o�C�g��.
unsigned    PngDecoder::widthByte(unsigned algn) const
{
    if (algn == 0)
        algn = align_;
    if (algn == 0)
        algn = 1;
    assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
    unsigned    wb  = WID2BYT(width_, bpp_);
    wb = (wb + algn-1) & ~(algn-1);
    return wb;
}


bool PngDecoder::isSupported(const void *binData) {
    enum { PNG_BYTES_TO_CHECK = 4 };
    return (png_sig_cmp((png_bytep)binData, (png_size_t) 0, PNG_BYTES_TO_CHECK) == 0);
}


/// ���������png�f�[�^��ݒ�. �|�C���^��ێ����邾���Ȃ̂�read()���I����O��binData��j�󂵂Ȃ�����.
bool    PngDecoder::setData(const void* binData, unsigned binDataSize)
{
    if (isSupported(binData) == false)
        return false;

    binData_     = (const unsigned char*)binData;
    binDataSize_ = binDataSize;

    png_ptr_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr_ == NULL)
        return 0;

    info_ptr_ = png_create_info_struct(png_ptr_);
    if (info_ptr_ == NULL) {
        png_destroy_read_struct(&png_ptr_, NULL, NULL);
        return 0;
    }

 #ifdef PNG_SETJMP_SUPPORTED
    if (setjmp(png_jmpbuf(png_ptr_))) {
        png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
        return 0;
    }
 #endif

    png_set_read_fn(png_ptr_, (void *)this, (png_rw_ptr)raw_read_data);

    png_set_sig_bytes(png_ptr_, sig_read_);

    png_read_info(png_ptr_, info_ptr_);
    png_get_IHDR(png_ptr_, info_ptr_, (png_uint_32*)&width_, (png_uint_32*)&height_, &bpp_, &color_type_, &interlace_type_, NULL, NULL);

    clutSize_   = 0;
    haveAlpha_  = false;
    orgBpp_     = bpp_;

    switch (color_type_) {
    case 0: // �O���[�X�P�[��. bpp=1,2,4,8,16. clut�戵���ɂ���.
        //x orgBpp_     = bpp_;
        if (bpp_ > 8)   // 16�͌Ăё����[�`�������Ή���8�����ɂ���.
            bpp_ = 8;
        clutSize_   = 1 << bpp_;
        haveAlpha_ = png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS) != 0;
        break;

    case 2: // rgb True Color. bpp=8,16. rgb888�݂̂ɂ���.
        orgBpp_     = bpp_ * 3;
        bpp_        = 24;   // 3*8
        haveAlpha_  = png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS) != 0;
        if (haveAlpha_)
            bpp_    = 32;
        break;

    case 3: // clut. �C���f�b�N�X�J���[ 1,2,4,8.  bpp=2�͔����������̏�ł͂��̂܂܂ɂ��Ƃ�.
        {
            //x orgBpp_     = bpp_;
            if (bpp_ > 8)   // 16�͌Ăё����[�`�������Ή���8�����ɂ���.
                bpp_ = 8;
            png_colorp  palette     = 0;
            png_get_PLTE(png_ptr_, info_ptr_, &palette, (int*)&clutSize_);
            haveAlpha_ = png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS) != 0;
        }
        break;

    case 4: // ���t���O���C�X�P�[��. argb�摜�Ƃ��Ĉ���.
        haveAlpha_  = true;
        orgBpp_     = bpp_ * 2;
        bpp_        = 32;
        png_set_gray_to_rgb(png_ptr_);
        break;

    case 6: // argb. ���t��True Color. 8888�݂̂ɂ��Ƃ�.
        haveAlpha_  = true;
        orgBpp_     = bpp_ * 4;
        bpp_        = 32;
        break;

    default:
        return 0;
    }

    return 1;
}


/** ARGB*clutSize��clut���擾.
 */
unsigned PngDecoder::getClut(unsigned* clut, unsigned clutSize)
{
    // clut����.
    unsigned plttN = 1 << bpp_;
    if (bpp_ <= 8) {
        if (clutSize == 0)
            clutSize = plttN;
        clutSize = (clutSize < plttN) ? clutSize : plttN;
    } else {
        clutSize = 0;
    }
    if (clut && clutSize > 0) {
        for (unsigned i = 0; i < clutSize; ++i)
            clut[i] = 0x00000000;   // 0xFF000000;
    }

    switch (color_type_) {
    case 0:
        {
            assert(clut != NULL);
            int trn  = -1;
            if (haveAlpha_) {
                int             n = 0;
                png_color_16p   p = 0;
                png_get_tRNS(png_ptr_, info_ptr_, NULL, &n, &p);
                if (p && n >= 1) {
                    trn = p->gray >> 8;
                }
            }
            if (bpp_ == 1) {
                clut[0] = (trn == 0x00) ? 0x00000000 : 0xFF000000;
                clut[1] = (trn == 0xff) ? 0x00FFFFFF : 0xFFFFFFFF;
            } else {
                unsigned c;
                unsigned m = (1<<bpp_) - 1;
                for (unsigned i = 0; i < clutSize; i++) {
                    c = ((i&m) << (8-bpp_));
                    c = c | (c >> bpp_) | (c >> (bpp_+bpp_)) | (c >> (bpp_+bpp_+bpp_));
                    int a = (int(c) == trn) ? 0x00 : 0xff;
                    clut[i] = ARGB(a, c,c,c);
                }
            }
        }
        break;

    case 3: // r,g,b�p���b�g.
        {
            assert(clut != NULL);
            png_colorp  palette     = 0;
            if (png_get_PLTE(png_ptr_, info_ptr_, &palette, (int*)&plttN)) {
                if (plttN > clutSize)
                    plttN = clutSize;
                for (unsigned i = 0; i < plttN; ++i) {
                    clut[i] = ARGB(0xFF, palette->red, palette->green, palette->blue);
                    ++palette;
                }
                // ���l������Δ��f.
                if (png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS)) {
                    int                 n = 0;
                    png_bytep           p = 0;
                    png_get_tRNS(png_ptr_, info_ptr_, &p, &n, NULL);
                    for (unsigned i = 0; i < plttN && i < unsigned(n) && p != 0; ++i) {
                        int a = *p++;
                        clut[i] = ARGB(a,0,0,0) | (clut[i] & 0xFFFFFF);
                    }
                }
            }
          #if 1 // �K�v�Ȃ�?�Ƃ������ԈႢ?
            else if (png_get_valid(png_ptr_, info_ptr_, PNG_INFO_sPLT)) {
                png_sPLT_tp     p = 0;
                int n = png_get_sPLT(png_ptr_, info_ptr_, &p);
                if (n > 0 && p) {
                    for (unsigned i = 0; i < clutSize_ && i < unsigned(n); ++i) {
                        int a = p->entries[i].alpha >> 8;
                        int r = p->entries[i].red   >> 8;
                        int g = p->entries[i].green >> 8;
                        int b = p->entries[i].blue  >> 8;
                        clut[i] = ARGB(a,r,g,b);
                    }
                }
            }
          #endif
        }
        break;

    default:
        break;
    }

    return clutSize;
}


/** malloc �����������ɉ摜�����ĕԂ�.
 *  @param  widAlgn     �����o�C�g���̃A���C�����g. 1,2,4,8��z��.
 *                      �f�t�H���g 1. bmp�ɍ��킹�����ꍇ�� 4��ݒ�̂���.
 *  @param  dir         0:���ォ��l�߂�.  1:��������l�߂�.bmp�ɍ��킹�����ꍇ�� 1��ݒ�̂���.
 */
void* PngDecoder::read(unsigned widAlgn, unsigned dir)
{
    setWidthAlign(widAlgn);
    if (dir&1)
        revY();
    void*   m = calloc(1, imageByte());
    return m && this->read(m) ? m : 0;
}


/// pix ��24�r�b�g�F�摜��W�J����. �T�C�Y��0���ƃf�t�H���g�̂܂�. dir��0�����ォ��1�Ȃ獶������.
bool    PngDecoder::read(void* pix)
{
   // 1�v���[���� 16bit �̏ꍇ�́A8�r�b�g�ɂ���悤�ɐݒ�.
    png_set_strip_16(png_ptr_);

  #if 1 // little endian ABGR �Ȃ̂� ARGB��.(big endian RGBA->BGRA)
    if (color_type_ & PNG_COLOR_MASK_COLOR)
        png_set_bgr(png_ptr_);
  #endif

    // 1�o�C�g���̋l�ߏ���ύX.
    if (bigEndian_ == false)    // png�̃f�t�H���g�̓r�b�O�G���f�B�A���Ȃ̂ŁA���g���G���f�B�A������swap��.
        png_set_packswap(png_ptr_);

    if (toClutBpp8_)
        png_set_packing(png_ptr_);
    if (color_type_ == PNG_COLOR_TYPE_GRAY && toClutBpp8_)
         png_set_expand_gray_1_2_4_to_8(png_ptr_);

    /* Expand paletted colors into true RGB triplets */
    if (color_type_ == PNG_COLOR_TYPE_PALETTE && toTrueColor_)
        png_set_palette_to_rgb(png_ptr_);

    if (png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS) && (toTrueColor_ || bpp_ >= 24))
        png_set_tRNS_to_alpha(png_ptr_);

    if (stripAlpha_)
        png_set_strip_alpha(png_ptr_);


 #if 0
    png_color_16    my_background;
    memset(&my_background, 0, sizeof my_background);
    png_color_16*   image_background;
    if (png_get_bKGD(png_ptr_, info_ptr_, &image_background))
        png_set_background(png_ptr_, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    else
        png_set_background(png_ptr_, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

  #if 1
    double screen_gamma = 2.2;
  #else
    if ( /* We have a user-defined screen gamma value */ ) {
        screen_gamma = user - defined screen_gamma;
    }
    else if ((gamma_str = getenv("SCREEN_GAMMA")) != NULL) {
        screen_gamma = atof(gamma_str);
    } else {
        screen_gamma = 2.2;                     /* A good guess for a PC monitors in a dimly lit room */
        screen_gamma = 1.7 or 1.0;              /* A good guess for Mac systems */
    }
  #endif

    int         intent;

    if (png_get_sRGB(png_ptr_, info_ptr_, &intent))
        png_set_gamma(png_ptr_, screen_gamma, 0.45455);
    else {
        double      image_gamma;

        if (png_get_gAMA( png_ptr_, info_ptr_,   &image_gamma))
            png_set_gamma(png_ptr_, screen_gamma, image_gamma);
        else
            png_set_gamma(png_ptr_, screen_gamma, 0.45455);
    }

    //x png_set_invert_mono(png_ptr_);

    png_set_swap_alpha(png_ptr_);
    png_set_swap(png_ptr_);
    png_set_filler(png_ptr_, 0xff, PNG_FILLER_AFTER);
    // number_passes = png_set_interlace_handling(png_ptr_);
    png_read_update_info(png_ptr_, info_ptr_);
 #endif

    png_bytep*  row_pointers = new png_bytep[height_];
    //x unsigned    wb0      = png_get_rowbytes(png_ptr_, info_ptr_);
    unsigned    wb           = widthByte();
    if (revY_) {
        for (int y = int(height_); --y >= 0; )
            row_pointers[y] = (png_bytep)pix + (height_-1-y) * wb;
    } else {
        for (int y = 0; y < int(height_); ++y)
            row_pointers[y] = (png_bytep)pix + y * wb;
    }

    png_read_image(png_ptr_, row_pointers);

    memset(row_pointers, 0, sizeof(png_bytep) * height_);
    png_read_end(png_ptr_, info_ptr_);

    png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
    delete row_pointers;

    return 1;
}


void PngDecoder::raw_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PngDecoder* pOwn = (PngDecoder*)png_ptr->io_ptr;
  #if 1 // �G���[�����K���ɑΏ�.
    if (pOwn->cur_ >= pOwn->binDataSize_) {
        if (data != 0 && length > 0)
            memset(data, 0, length);
        return;
    }
    unsigned rest = pOwn->binDataSize_ - pOwn->cur_;
    if (length > rest) {
        if (data != 0 && length-rest > 0)
            memset(data+rest, 0, length-rest);
        length = rest;
    }
    if (length == 0)
        return;
  #endif
    memcpy(data, pOwn->binData_ + pOwn->cur_, length);
    assert(length <= ~0U);
    pOwn->cur_ += (unsigned)length;
}
