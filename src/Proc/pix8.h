/**
 *  @file   pix8.h
 *  @biref  8ビット色画像をいろいろ変換
 *  @author Masashi Kitamura
 *  @date   2000
 */
#ifndef PIX8_H
#define PIX8_H

#ifdef __cplusplus
extern "C" {
#endif

void pix8_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH);

/// src画像(横幅srcW)の矩形(rctW*rctH)をdst画像(横幅dstW)にコピー
void pix8_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH);

/// 単純な拡大
void pix8_resize(uint8_t *pix2, unsigned rszW, unsigned rszH, const uint8_t *pix, unsigned w, unsigned h);

/// src画像中に bpp の範囲外になる番号があるかチェック. あれば負数を返す.
int pix8_hasPixOutOfIdx(uint8_t const* src, int w, int h, int idx);

//x /// 背景色以外が使っている矩形を求める。抜き色はインデックス・カラー
//x int pix8_getNukiRectI(void *pix0, int xsz, int ysz, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);

//x /// 背景色以外が使っている矩形を求める. 抜き色はフルカラー
//x int pix8_getNukiRectF(void *pix0, int xsz, int ysz, int *clut, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);

//x int pix8_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p);
//x int  pix8_revXY(void *pix, int w, int h, int dir);
//x void pix8_bitCom(void *pix, int w, int h);

#ifdef __cplusplus
};
#endif

#endif  /* PIX8_H */
