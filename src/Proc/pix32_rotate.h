/**
 *  @file   pix32_rotate.h
 *  @brief  画像を任意角で回転.
 *  @author Masashi KITAMURA
 */

#ifndef PIX32_ROTATE_H
#define PIX32_ROTATE_H

#include "def.h"

/// 画像を回転した結果の新規画像の情報
typedef struct pix32_rotate_dst_t {
    uint32_t*   mallocMem;          ///< 画像. mallocされたメモリ
    uint32_t    w;                  ///< 横幅
    uint32_t    h;                  ///< 縦幅
} pix32_rotate_dst_t;

#if defined __cplusplus
extern "C" {
#endif

int  pix32_rotateBilinear(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
int  pix32_rotateBicubic(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int hasAlpha);
int  pix32_rotateSpline36(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol, int hasAlpha);

/// すでに回転済みの画像のαチャンネルだけバイリニアで設定し直すためのルーチン
int  pix32_rotateBilinearAlpha(pix32_rotate_dst_t* dst, const unsigned *src, unsigned srcW, unsigned srcH, double rot, uint32_t dcol);
#if defined __cplusplus
}
#endif


/** 画像の回転.
 * @param dst   出力画像
 * @param src   入力画像
 * @param srcW  入力横幅
 * @param srcH  入力縦幅
 * @param rot   右への回転角度
 * @param dcol  範囲外の隙間が出来た時に埋める色
 * @param type  変換の種類: 0,1=バイリニア 2=バイキュービック 3,4=Spline36
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
// C++向
// image(), witdh(),height(),bpp() をメンバーに持つ画像クラスを操作.

#if defined __cplusplus

template<class IMG>
void pix32_rotate(IMG& dst, const IMG& src, double rot, int type=1) {
    assert(dst.bpp() == 32 && src.bpp() == 32);
}

#endif  // __cplusplus
#endif

#endif
