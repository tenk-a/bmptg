/**
 *  @file PngDecoder.hpp
 *  @brief  ���������png�摜�f�[�^��W�J����.
 *  @author Masashi KITAMURA
 *  @note
 *
 *      - �g����
 *          - setData(binData,size) �Ń������[���png�f�[�^��ݒ肵�A
 *            ���̌� read()�����malloc������������24�r�b�g�F�摜�𓾂�.
 *          - ���邢��setData()��, �T�C�YwidthByte()*height() �ȏ�̃�������
 *            �Ăь��ŗp�ӂ���read(pix, ...)���g��.
 */
#ifndef PNGDECODER_HPP
#define PNGDECODER_HPP

#include <stddef.h>
#include <assert.h>

#include "libpng/png.h"


/// ���������png�摜�f�[�^��W�J����. (libjpeg.lib�̃��b�p�[)
class PngDecoder {
public:
    PngDecoder();
    ~PngDecoder();
    PngDecoder(const void* binData, unsigned binDataSize);

    static bool isSupported(const void* data);

    /// �����Ŋm�ۂ��Ă��郁�������J��.
    void        release();

    /// ���������png�f�[�^��ݒ�. �|�C���^��ێ����邾���Ȃ̂�read()���I����O��binData��j�󂵂Ȃ�����.
    bool        setData(const void* binData, unsigned binDataSize) throw();

    unsigned    bpp()        const { return bpp_; }         ///< �r�b�g/�s�N�Z��. 24�̂�.
    unsigned    width()      const { return width_; }       ///< ����.
    unsigned    height()     const { return height_; }      ///< �c��.
    unsigned    widthByte(unsigned algn=0) const;           ///< algn�o�C�g�ɃA���C�����g�ς̉����o�C�g��.

    void        setBigEndian() { bigEndian_  = true; }
    void        revY()         { revY_ = true; }
    void        stripAlpha()   { stripAlpha_  = true; }     ///< ���t���摜�̏ꍇ�ł�,���𖳎�.
    void        toTrueColor()  { toTrueColor_ = true; bpp_ = haveAlpha_ ? 32 : 24; }
    void        setWidthAlign(unsigned align);

    // ���֌W�̋������ւ�ɂȂ��Ă��܂�.
    void        toClutBpp8()   { toClutBpp8_  = true; bpp_ = 8; }

    unsigned    clutSize()   const { return clutSize_; }
    unsigned    getClut(unsigned* clut, unsigned clutSize=0);

    unsigned    imageByte()  const;                         ///< �W�J�ɕK�v�ȃo�C�g����Ԃ�.
    bool        read(void* img);

    void*       read(unsigned widAlgn=1, unsigned dir=0);

private:
    static void raw_read_data(struct png_struct_def* png_ptr, unsigned char* data, size_t length);

private:
    const unsigned char*    binData_;
    struct png_struct_def*  png_ptr_;
    struct png_info_def*    info_ptr_;
    unsigned                binDataSize_;
    unsigned                cur_;

    unsigned                width_;
    unsigned                height_;
    int                     orgBpp_;
    int                     bpp_;
    unsigned                clutSize_;
    bool                    haveAlpha_;

    int                     color_type_;
    int                     interlace_type_;
    unsigned                sig_read_;

    unsigned                align_;
    bool                    revY_;
    bool                    bigEndian_;
    bool                    toTrueColor_;
    bool                    toClutBpp8_;
    bool                    stripAlpha_;

    bool                    closeRq_;
};

#endif  // PNGDECODER_H
