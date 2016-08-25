/**
 *  @file   pix32_resizeNearestNeighbor.h
 *  @brief  �j�A���X�g�l�C�o�[�@�Ŋg��k��.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_RESIZENEARESTNEIGHBOR_H
#define PIX32_RESIZENEARESTNEIGHBOR_H

#ifdef __cplusplus
extern "C" {
#endif

/** �g��k��
 */
int  pix32_resizeNearestNeighbor(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);

#ifdef __cplusplus
}
#endif

#if defined __cplusplus
// C++��
// image(), witdh(),height(),bpp() �������o�[�Ɏ��摜�N���X�𑀍�.

template<class IMG>
int  pix32_resizeNearestNeighbor(IMG& dst, const IMG& src) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    return pix32_resizeNearestNeighbor(dst.image(), dst.width(), src.image(), src.width(), src.height());
}

#endif  // __cplusplus

#endif
