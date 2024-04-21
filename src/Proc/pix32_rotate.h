/**
 *  @file   pix32_rotate.h
 *  @brief  �摜��C�ӊp�ŉ�].
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_ROTATE_H
#define PIX32_ROTATE_H

#include "def.h"

/// �摜����]�������ʂ̐V�K�摜�̏��
typedef struct pix32_rotate_dst_t {
    uint32_t*   mallocMem;          ///< �摜. malloc���ꂽ������
    uint32_t    w;                  ///< ����
    uint32_t    h;                  ///< �c��
} pix32_rotate_dst_t;

#if defined __cplusplus
extern "C" {
#endif

int  pix32_rotateBilinear(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
int  pix32_rotateBicubic(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int hasAlpha);
int  pix32_rotateSpline36(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int hasAlpha);

/// ���łɉ�]�ς݂̉摜�̃��`�����l�������o�C���j�A�Őݒ肵�������߂̃��[�`��
int  pix32_rotateBilinearAlpha(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
#if defined __cplusplus
}
#endif


/** �摜�̉�].
 * @param dst   �o�͉摜
 * @param src   ���͉摜
 * @param srcW  ���͉���
 * @param srcH  ���͏c��
 * @param rot   �E�ւ̉�]�p�x
 * @param dcol  �͈͊O�̌��Ԃ��o�������ɖ��߂�F
 * @param type  �ϊ��̎��: 0,1=�o�C���j�A 2=�o�C�L���[�r�b�N 3,4=Spline36
 */
inline int  pix32_rotate(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int hasAlpha, int type)
{
    switch (type) {
    case 0:
    case 1: return pix32_rotateBilinear(dst, src, srcW, srcH, rot, dcol);
    case 2: return pix32_rotateBicubic (dst, src, srcW, srcH, rot, dcol, hasAlpha);
    case 3:
    case 4: return pix32_rotateSpline36(dst, src, srcW, srcH, rot, dcol, hasAlpha);
    default: assert(0);
    }
    return 0;
}

#if 0
// ===========================================================================
// C++��
// image(), witdh(),height(),bpp() �������o�[�Ɏ��摜�N���X�𑀍�.

#if defined __cplusplus

template<class IMG>
void pix32_rotate(IMG& dst, const IMG& src, double rot, int type=1) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
}

#endif  // __cplusplus
#endif

#endif
