/**
 *  @file   pix32_resizeBicubic.h
 *  @brief  バイキュービック法で拡大縮小.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_RESIZEBICUBIC_H
#define PIX32_RESIZEBICUBIC_H

#ifdef __cplusplus
extern "C" {
#endif

/** 拡大縮小
 */
int pix32_resizeBicubic(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);

#ifdef __cplusplus
}
#endif

#if defined __cplusplus
// C++向
// image(), witdh(),height(),bpp() をメンバーに持つ画像クラスを操作.

template<class IMG>
void pix32_resizeBicubic(IMG& dst, const IMG& src) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    pix32_resizeBicubic(dst.image(), dst.width(), src.image(), src.width(), src.height());
}

#endif  // __cplusplus

#endif
