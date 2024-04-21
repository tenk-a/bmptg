/**
 *  @file   pix32_resizeNearestNeighbor.c
 *  @brief  �j�A���X�g�l�C�o�[�@�Ŋg��k��.
 *  @author Masashi KITAMURA
 */

#include "pix32_resizeNearestNeighbor.h"
#include "pix32_resizeAveragingI.h"
#include "pix32.h"
#include "def.h"

static void pix32_resizeNearestNeighborSub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH);


/** �g��k��
 */
int  pix32_resizeNearestNeighbor(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH)
{
    if (!dst || !src || !srcW || !srcH || !dstW || !dstH) {
        assert(0 && "ERROR pix32_resizeNearestNeighbor bad param.\n");
        return 0;
    }

    if (dstW == srcW && dstH == srcH) {
        memcpy(dst, src, dstW*srcH*sizeof(*dst));                       // �����T�C�Y�Ȃ烁�����R�s�[�ł��܂�
        return 1;
    }

    pix32_resizeNearestNeighborSub(dst, dstW, dstH, src, srcW, srcH);
    return 1;
}


/** Nearest Neighbor �@�Ŋg��k��
 */
static void  pix32_resizeNearestNeighborSub(unsigned* dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH)
{
    double   rscaleX, rscaleY;
    uint32_t dstX   , dstY;

    assert(dst && dstW && dstH && src && srcW && srcH);
    rscaleX   = (double)srcW / dstW;
    rscaleY   = (double)srcH / dstH;

    for (dstY = 0; dstY < dstH; ++dstY) {
        for (dstX = 0; dstX < dstW; ++dstX) {
            int x = (int)((dstX + 0.5) * rscaleX);
            int y = (int)((dstY + 0.5) * rscaleY);
            dst[dstY*dstW + dstX] = src[y * srcW + x];
        }
    }
}
