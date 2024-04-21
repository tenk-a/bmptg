/**
 *  @file   pix32_resize.h
 *  @brief  �o�C���j�A�T���v�����O�Ŋg��k��.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_RESIZE_H
#define PIX32_RESIZE_H

#include "pix32_resizeNearestNeighbor.h"
#include "pix32_resizeBilinear.h"
#include "pix32_resizeBicubic.h"
#include "pix32_resizeSpline36.h"
#include "pix32_resizeLanczos3.h"

/** �摜�̊g��k��
 * @param dst   �o�͉摜
 * @param dstW  �o�͉���
 * @param dstH  �o�͏c��
 * @param src   ���͉摜
 * @param srcW  ���͉���
 * @param srcH  ���͏c��
 * @param type  �ϊ��̎��: 0,1=�o�C���j�A 2=�o�C�L���[�r�b�N 3,4=Spline36
 */
inline void pix32_resize(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH, int type, int hasAlpha)
{
    switch (type) {
    case 0: pix32_resizeNearestNeighbor(dst, dstW, dstH, src, srcW, srcH);  break;
    case 1: pix32_resizeBilinear(dst, dstW, dstH, src, srcW, srcH); break;
    case 2: pix32_resizeBicubic( dst, dstW, dstH, src, srcW, srcH, hasAlpha); break;
    case 3: pix32_resizeSpline36(dst, dstW, dstH, src, srcW, srcH, hasAlpha); break;
    case 4: pix32_resizeLanczos3(dst, dstW, dstH, src, srcW, srcH, hasAlpha); break;
    default: assert(0);
    }
}

// ===========================================================================
// C++��
// image(), witdh(),height(),bpp() �������o�[�Ɏ��摜�N���X�𑀍�.

#if defined __cplusplus

template<class IMG>
void pix32_resize(IMG& dst, const IMG& src, int type=1) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    pix32_resize(dst.image(), dst.width(), src.image(), src.width(), src.height(), type);
}


#endif  // __cplusplus

#endif
