/**
 *  @file JpgEncoder.hpp
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
#ifndef JPGENCODER_HPP
#define JPGENCODER_HPP

#include <assert.h>

/// 24�r�b�g�F�摜��jpg�f�[�^�Ɉ��k. (libjpeg.lib�̃��b�p�[)
/// ��1�s�N�Z���́A��������R,G,B���ɕ��񂾃��m�����g���G���f�B�A���l�Ƃ��Ă�BGR��
class JpgEncoder {
public:
    JpgEncoder() {;}
    ~JpgEncoder() {;}

    JpgEncoder(unsigned char* dst, unsigned dstSz, const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned* pSz=0) {
        unsigned sz = write(dst,dstSz, src, wid, hei, quality, widByt, dir);
        if (pSz)
            *pSz = sz;
    }

    /// �����Ŋm�ۂ��Ă��郁�������J��. ���덇�킹�p�̃_�~�[.
    void    release() {;}

    /// pix ��24�r�b�g�F�摜��W�J����. �T�C�Y��0���ƃf�t�H���g�̂܂�. dir��0�����ォ��1�Ȃ獶������.
    static unsigned     write(unsigned char* dst, unsigned dstSz, const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned mono=0);

    /// malloc ������������jpg�f�[�^�����ĕԂ�.
    static unsigned char*  write(const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned*pSz=0);

    /// write�Ń|�C���^���󂯎�����ꍇ�̑΂ƂȂ郁�����J��.
    static void     deallocate(void* p);

    /// �������o�C�g���ɕϊ�.
    static unsigned wid2byt(unsigned wid, unsigned algn=1) {
        assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
        return (wid*3 + algn-1) & ~(algn-1);
    }
};

#endif  // JPGENCODER_H
