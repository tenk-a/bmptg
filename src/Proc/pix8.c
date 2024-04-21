/**
 *  @file   pix8.c
 *  @biref  8ビット色画像をいろいろ変換
 *  @author Masashi Kitamura
 *  @date   2000
 */
#include "def.h"
#include "mem_mac.h"
#include "pix8.h"


void pix8_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH)
{
    int n;

    // ソース範囲チェック.
    if (rctX+rctW <= 0 || rctX >= srcW || rctY+rctH <= 0 || rctY >= srcH) { // 画面の範囲外だから描画しない.
        return;
    }
    if (rctX < 0) {
        n = -rctX;
        rctW -= n;
        dstX += n;
        rctX =  0;
    }
    if (rctY < 0) {
        n = -rctY;
        rctH -= n;
        dstY += n;
        rctY =  0;
    }
    if (rctX+rctW > srcW) {         // 後ろが切れるなら、表示幅を減らす.
        rctW = srcW - rctX;
    }
    if (rctY+rctH > srcH) {         // 後ろが切れるなら、表示幅を減らす.
        rctH = srcH - rctY;
    }

    // 出力範囲チェック.
    if (dstX+rctW <= 0 || dstX >= dstW || dstY+rctH <= 0 || dstY >= dstH) { // 画面の範囲外だから描画しない.
        return;
    }
    if (dstX < 0) {
        rctW -= (-dstX);
        dstX =  0;
    }
    if (dstY < 0) {
        rctH -= (-dstY);
        dstY =  0;
    }
    if (dstX+rctW > dstW) {
        rctW = dstW - dstX;
    }
    if (dstY+rctH > dstH) {
        rctH = dstH - dstY;
    }

    pix8_copyWH((uint8_t*)dst+dstY*dstW+dstX, dstW, (uint8_t*)src+rctY*srcW+rctX, srcW, rctW, rctH);
}



/** src画像(横幅srcW)の矩形(rctW*rctH)をdst画像(横幅dstW)にコピー
 *  範囲チェックなどはしないので使い手側で調整のこと
 */
void pix8_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH)
{
    uint8_t *d, *s;
    int x,y;

    d = (uint8_t*)dst;
    s = (uint8_t*)src;
    for (y = 0; y < rctH; y++) {
        for (x = 0; x < rctW; x++) {
            d[x] = s[x];
        }
        s += srcW;
        d += dstW;
    }
}


#if 0
/** 背景色以外が使っている矩形を求める。抜き色はインデックス・カラー */
int pix8_getNukiRectI(void *pix0, int xsz, int ysz, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p)
{
    int x,y,x0,y0,x1,y1;
    uint8_t *pix = (uint8_t*)pix0;

    if (pix == NULL || xsz == 0 || ysz == 0) {
        return -1;
    }

    x0 = y0 =  0x7fff;
    x1 = y1 = -0x7fff;

    for (y = 0; y < ysz; y++) {
        for (x = 0; x < xsz; x++) {
            if (pix[y*xsz+x] != nukiClut) {
                if (x0 > x) x0 = x;
                if (y0 > y) y0 = y;
                if (x1 < x) x1 = x;
                if (y1 < y) y1 = y;
            }
        }
    }

    if (x0 == 0x7fff || x1 == -0x7fff) {    /* 背景色のみのとき */
        *w_p = 0;
        *h_p = 0;
        *x_p = 0;
        *y_p = 0;
    } else {
        *x_p = x0;
        *y_p = y0;
        *w_p = x1+1 - x0;
        *h_p = y1+1 - y0;
    }
    return 0;
}
#endif


#if 0
/** 背景色以外が使っている矩形を求める. 抜き色はフルカラー */
int pix8_getNukiRectF(void *pix0, int xsz, int ysz, int *clut, int colKey, int *x_p, int *y_p, int *w_p, int *h_p)
{
    int x,y,x0,y0,x1,y1;
    uint8_t *pix = (uint8_t*)pix0;

    if (pix == NULL || xsz == 0 || ysz == 0) {
        return -1;
    }

    x0 = y0 =  0x7fff;
    x1 = y1 = -0x7fff;

    for (y = 0; y < ysz; y++) {
        for (x = 0; x < xsz; x++) {
            if (clut[pix[y*xsz+x]] != colKey) {
                if (x0 > x) x0 = x;
                if (y0 > y) y0 = y;
                if (x1 < x) x1 = x;
                if (y1 < y) y1 = y;
            }
        }
    }

    if (x0 == 0x7fff || x1 == -0x7fff) {    /* 背景色のみのとき */
        *w_p = 0;
        *h_p = 0;
        *x_p = 0;
        *y_p = 0;
    } else {
        *x_p = x0;
        *y_p = y0;
        *w_p = x1+1 - x0;
        *h_p = y1+1 - y0;
    }
    return 0;
}
#endif



#if 0
/** グリッドサイズ gw*gh として、グリッドに合うように矩形サイズを調整 */
int pix8_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p)
{
    int x0,y0,x1,y1;

    if (gw > 1) {
        x1 = ((*x_p + *w_p + gw - 1) / gw) * gw;
        x0 = (*x_p / gw) * gw;
        *x_p = x0;
        *w_p = x1 - x0;
    }
    if (gh > 1) {
        y1 = ((*y_p + *h_p + gh - 1) / gh) * gh;
        y0 = (*y_p / gh) * gh;
        *y_p = y0;
        *h_p = y1 - y0;
    }

    return 0;
}
#endif

#if 0
int pix8_revXY(void *pix, int w, int h, int dir)
{
    int   wb = w;
    uint8_t *p = (uint8_t*)pix, *q = (p + (h - 1) * wb);
    int    c,x,y;

    wb = (wb) ? wb : WID2BYT4(w, 8);
    switch (dir) {
    case 1:
        for (y = 0; y < h; y++) {
            for (x = 0; x < w/2; x++) {
                c    = p[x];
                p[x] = p[w-1-x];
                p[w-1-x] = c;
            }
            p = (uint8_t*)((uint8_t*)p + wb);
        }
        break;
    case 2:
        for (y = 0; y < h/2; y++) {
            for (x = 0; x < w; x++) {
                c    = p[x];
                p[x] = q[x];
                q[x] = c;
            }
            p = (uint8_t*)((uint8_t*)p + wb);
            q = (uint8_t*)((uint8_t*)q - wb);
        }
        break;
    case 3:
        for (y = 0; y < h/2; y++) {
            for (x = 0; x < w; x++) {
                c    = p[x];
                p[x] = q[w-1-x];
                q[w-1-x] = c;
            }
            p = (uint8_t*)((uint8_t*)p + wb);
            q = (uint8_t*)((uint8_t*)q - wb);
        }
        break;
    default:
        break;
    }
    return 1;
}
#endif


#if 0
void pix8_bitCom(void *pix, int w, int h)
{
    uint8_t *p = (uint8_t*)pix;
    int i;

    for (i = 0; i < w * h; i++) {
        *p = ~(*p);
        p++;
    }
}
#endif


/// 単純な拡大.
void pix8_resize(uint8_t *pix2, unsigned rszW, unsigned rszH, const uint8_t *pix, unsigned w, unsigned h)
{
    unsigned nw = rszW / w;
    unsigned nh = rszH / h;
    unsigned x,y,u,v,c;

    if (nw == 0 || nh == 0) {
        printf("CLUT付画像のままでは整数倍拡大以外の拡縮できない\n");
        return;
    }
    if (rszW % w != 0 || rszH % h != 0) {
        printf("CLUT付画像のままでは整数倍拡大以外の拡縮できない\n");
    }

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            c = pix[ y * w + x ];
            for (v = 0; v < nh; ++v) {
                for (u = 0; u < nw; ++u) {
                    pix2[ (y*nh+v)*rszW + (x*nw+u) ] = c;
                }
            }
        }
    }
}


/// src画像中に bpp の範囲外になる番号があるかチェック. あれば正数を返す.
int pix8_hasPixOutOfIdx(uint8_t const* src, int w, int h, int idxMax) {
    unsigned wh = w * h;
    unsigned i;
    for (i = 0; i < wh; ++i) {
        if (src[i] > idxMax)
            return 1;
    }
    return 0;
}
