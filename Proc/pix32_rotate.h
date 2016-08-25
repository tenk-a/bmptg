/**
 *  @file   pix32_rotate.h
 *  @brief  ‰æ‘œ‚ğ”CˆÓŠp‚Å‰ñ“].
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_ROTATE_H
#define PIX32_ROTATE_H

#include "def.h"

typedef struct pix32_rotate_dst_t {
	uint32_t*	mallocMem;
	uint32_t	w;
	uint32_t	h;
} pix32_rotate_dst_t;

#if defined __cplusplus
extern "C" {
#endif

int  pix32_rotateBilinear(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
int  pix32_rotateBicubic(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
int  pix32_rotateSpline36(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);

#if defined __cplusplus
}
#endif


/** Šg‘åk¬
 */
inline int  pix32_rotate(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int type)
{
	switch (type) {
	case 0:
	case 1:	return pix32_rotateBilinear(dst, src, srcW, srcH, rot, dcol);
	case 2: return pix32_rotateBicubic (dst, src, srcW, srcH, rot, dcol);
	case 3:
	case 4: return pix32_rotateSpline36(dst, src, srcW, srcH, rot, dcol);
	default: assert(0);
	}
	return 0;
}

#if 0
// ===========================================================================
// C++Œü
// image(), witdh(),height(),bpp() ‚ğƒƒ“ƒo[‚É‚Â‰æ‘œƒNƒ‰ƒX‚ğ‘€ì.

#if defined __cplusplus

template<class IMG>
void pix32_rotate(IMG& dst, const IMG& src, double rot, int type=1) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
}

#endif  // __cplusplus
#endif

#endif
