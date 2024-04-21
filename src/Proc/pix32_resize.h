/**
 *  @file   pix32_resize.h
 *  @brief  バイリニアサンプリングで拡大縮小.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_RESIZE_H
#define PIX32_RESIZE_H

#include "pix32_resizeNearestNeighbor.h"
#include "pix32_resizeBilinear.h"
#include "pix32_resizeBicubic.h"
#include "pix32_resizeSpline36.h"
#include "pix32_resizeLanczos3.h"

/** 画像の拡大縮小
 * @param dst   出力画像
 * @param dstW  出力横幅
 * @param dstH  出力縦幅
 * @param src   入力画像
 * @param srcW  入力横幅
 * @param srcH  入力縦幅
 * @param type  変換の種類: 0,1=バイリニア 2=バイキュービック 3,4=Spline36
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
// C++向
// image(), witdh(),height(),bpp() をメンバーに持つ画像クラスを操作.

#if defined __cplusplus

template<class IMG>
void pix32_resize(IMG& dst, const IMG& src, int type=1) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    pix32_resize(dst.image(), dst.width(), src.image(), src.width(), src.height(), type);
}


#endif  // __cplusplus

#endif
