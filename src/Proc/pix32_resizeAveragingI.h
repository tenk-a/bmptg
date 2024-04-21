/**
 *  @file   pix32_resizeAveragingI.c
 *  @brief  出力サイズで入力サイズが割り切れる場合用の平均画素法での縮小.
 *  @author Masashi KITAMURA
 */
#ifndef PIX32_RESIZEAVARAGINGI_H
#define PIX32_RESIZEAVARAGINGI_H

#ifdef __cplusplus
extern "C" {
#endif

/** 拡大縮小
 */
void pix32_resizeAveragingI(unsigned *dst, unsigned dstW, unsigned dstH, const unsigned *src, unsigned srcW, unsigned srcH);


#ifdef __cplusplus
}
#endif

#endif
