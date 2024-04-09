/**
 *  @file JpgDecoder.hpp
 *  @brief  ���������jpg�摜�f�[�^��W�J����.
 *  @author Masashi KITAMURA
 *  @note
 *      - Independent JPEG Group��libjpeg.lib��p���Ă��܂��B
 *
 *      - �g����
 *          - setData(binData,size) �Ń������[���jpg�f�[�^��ݒ肵�A
 *            ���̌� read()�����malloc������������24�r�b�g�F�摜�𓾂�.
 *          - ���邢��setData()��, �T�C�YwidthByte()*height() �ȏ�̃�������
 *            �Ăь��ŗp�ӂ���read(pix, ...)���g��.
 */
#ifndef JPGDECODER_HPP
#define JPGDECODER_HPP

#include <cstdlib>      // calloc���w�b�_�Ŏg�p���邽��.
#include <cassert>
#include <string.h>
#include <memory.h>



/// ���������jpg�摜�f�[�^��W�J����. (libjpeg.lib�̃��b�p�[)
class JpgDecoder {
public:
    JpgDecoder();
    ~JpgDecoder();
    JpgDecoder(const void* binData, unsigned binDataSize, bool thumbnail=false);

    /// �����Ŋm�ۂ��Ă��郁�������J��.
    void        release();

    static bool isSupported(const void* data) {
        const unsigned char* b = (unsigned char*)data;
      #if 1
        return (b[0] == 0xFF && b[1] == 0xD8);
      #else
        if (b[0] != 0xFF || b[1] != 0xD8)
            return false;
        if (b[2] == 0xFF && b[3] == 0xE1 && memcmp(b+6,"JFIF", 4) == 0)
            return true;
        return (b[2] == 0xFF && b[3] == 0xE1 && memcmp(b+6,"Exif", 4) == 0);
      #endif
    }

    /// ���������jpg�f�[�^��ݒ�. �|�C���^��ێ����邾���Ȃ̂�read()���I����O��binData��j�󂵂Ȃ�����.
    bool        setData(const void* binData, unsigned binDataSize, bool thumbnail=false);

    unsigned    bpp()        const { return bpp_; }         	///< �r�b�g/�s�N�Z��. 24�̂�.
    unsigned    width()      const;                         	///< ����.
    unsigned    height()     const;                         	///< �c��.
    unsigned    widthByte()  const { return width()*bpp_>>3; }	///< ���o�C�g��.
    unsigned    widthByte(unsigned algn) const;             	///< algn�o�C�g�ɃA���C�����g�ς̉���.

    unsigned    imageByte()  const { return widthByte() * height(); }

    /// pix ��24�r�b�g�F�摜��W�J����. �T�C�Y��0���ƃf�t�H���g�̂܂�. dir��0�����ォ��1�Ȃ獶������.
    bool        read(void* pix, unsigned widByt=0, unsigned hei=0, unsigned dir=0);

    /// malloc �����������ɉ摜�����ĕԂ�.
    void*       read(unsigned widAlgn=1, unsigned dir=0);

	unsigned	originalWidth() const { return originalWidth_; }	// thumbnail�w�莞�̌��T�C�Y
	unsigned	originalHeight() const { return originalHeight_; }	// thumbnail�w�莞�̌��T�C�Y

	unsigned const*	clut() const { return gray_clut_; }

private:
    struct jpeg_decompress_struct*  info_;
    struct jpeg_error_mgr*          jerr_;
    void*                           binData_;
    unsigned                        binDataSize_;
    bool                            closeRq_;
    bool                            thumbnail_;
	unsigned						originalWidth_;
	unsigned						originalHeight_;
	int								bpp_;
	unsigned						gray_clut_[256];
};



/** algn�o�C�g�ɃA���C�����g�ς݂̉����o�C�g����Ԃ�.
 */
inline unsigned     JpgDecoder::widthByte(unsigned algn) const {
    assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
    return (widthByte() + algn-1) & ~(algn-1);
}



#endif  // JPGDECODER_H
