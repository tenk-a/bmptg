/**
 *  @file   pix32_resizeBicubic.h
 *  @brief  �o�C�L���[�r�b�N�@�Ŋg��k��.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_RESIZEBICUBIC_H
#define PIX32_RESIZEBICUBIC_H

#ifdef __cplusplus
extern "C" {
#endif

/** �g��k��
 */
int pix32_resizeBicubic(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);

#ifdef __cplusplus
}
#endif

#if defined __cplusplus
// C++��
// image(), witdh(),height(),bpp() �������o�[�Ɏ��摜�N���X�𑀍�.

template<class IMG>
void pix32_resizeBicubic(IMG& dst, const IMG& src) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    pix32_resizeBicubic(dst.image(), dst.width(), src.image(), src.width(), src.height());
}

#endif  // __cplusplus

#endif
