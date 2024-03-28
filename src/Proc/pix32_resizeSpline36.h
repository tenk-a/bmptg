/**
 *  @file   pix32_resizeSpline36.h
 *  @brief  spline-36 ‚ÅŠg‘åk¬.
 *  @author Masashi KITAMURA
 */
#ifndef PIX32_RESIZESPLINE36_H
#define PIX32_RESIZESPLINE36_H

#ifdef __cplusplus
extern "C" {
#endif

/** Šg‘åk¬
 */
int pix32_resizeSpline36(unsigned *dst, unsigned dstW, unsigned dstH, unsigned const* src, unsigned srcW, unsigned srcH, int hasAlpha);

#ifdef __cplusplus
}
#endif

#if defined __cplusplus
// C++Œü
// image(), witdh(),height(),bpp() ‚ğƒƒ“ƒo[‚É‚Â‰æ‘œƒNƒ‰ƒX‚ğ‘€ì.

template<class IMG>
int pix32_resizeSpline36(IMG& dst, const IMG& src) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
    return pix32_resizeLanczos3(dst.image(), dst.width(), src.image(), src.width(), src.height());
}

#endif  // __cplusplus

#endif
