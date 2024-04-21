/**
 *  @file   pix32.c
 *  @biref  32ビット色画像をいろいろ変換
 *  @author Masashi Kitamura
 *  @date   2000
 *  @note
 *      2001-07 v2.00 拡縮をバイリニアサンプリングを用いた方法に変更。
 */

#include "def.h"
#include "subr.h"
#include "mem_mac.h"
#include "pix32.h"
#include <stdlib.h>


/** dstW*dstHな画像dst の(dstX,dstY) に srcW*srcHな画像srcの(srcX,srcY)の矩形rctW*rctH をコピーする
 */
void pix32_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH)
{
    int n;

    // ソース範囲チェック
    if ((rctX+rctW <= 0) | (rctX >= srcW) | (rctY+rctH <= 0) | (rctY >= srcH)) {    // 画面の範囲外だから描画しない
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
    if (rctX+rctW > srcW) {         // 後ろが切れるなら、表示幅を減らす
        rctW = srcW - rctX;
    }
    if (rctY+rctH > srcH) {         // 後ろが切れるなら、表示幅を減らす
        rctH = srcH - rctY;
    }

    // 出力範囲チェック
    if ((dstX+rctW <= 0) | (dstX >= dstW) | (dstY+rctH <= 0) | (dstY >= dstH)) {    // 画面の範囲外だから描画しない
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

    pix32_copyWH((uint32_t*)dst+dstY*dstW+dstX, dstW, (uint32_t*)src+rctY*srcW+rctX, srcW, rctW, rctH);
}



/** src画像(横幅srcW)の矩形(rctW*rctH)をdst画像(横幅dstW)にコピー
 *  範囲チェックなどはしないので使い手側で調整のこと
 */
void pix32_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH)
{
    uint32_t *d, *s;
    int x,y;

    d = (uint32_t*)dst;
    s = (uint32_t*)src;
    for (y = 0; y < rctH; y++) {
        for (x = 0; x < rctW; x++) {
            d[x] = s[x];
        }
        s += srcW;
        d += dstW;
    }
}



#if 0
/** α情報が0以外の矩形を求める. 抜き色はフルカラー */
int pix32_getAlpRect(void *pix0, int xsz, int ysz, int *x_p, int *y_p, int *w_p, int *h_p)
{
    int x,y,x0,y0,x1,y1;
    uint32_t *pix = (uint32_t*)pix0;

    if (pix == NULL || xsz == 0 || ysz == 0) {
        return -1;
    }

    x0 = y0 =  0x7fff;
    x1 = y1 = -0x7fff;

    for (y = 0; y < ysz; y++) {
        for (x = 0; x < xsz; x++) {
            uint32_t    c = pix[y*xsz+x];
            c &= 0xFF000000;
            if (c != 0) {
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



/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

/** 画像のαがsikiiA以下なら α=0の抜色, 色が0ならちょっと明るい色に,
 * r,g,bの各値が bppビット色の0以外の最小値未満なら、その最小値に変換する.
 */
void pix32_blackAMskGen(unsigned *pix, unsigned w, unsigned h, unsigned sikiiA, unsigned bpp)
{
    unsigned    *px = (unsigned*)pix;
    unsigned    l   = (bpp >= 24) ? 1 : (bpp >= 21) ? 2 : (bpp >= 18) ? 4 : (bpp >= 15) ? 8 : (bpp >= 12) ? 16 : (bpp >= 9) ? 32 : (bpp >= 6) ? 64 : 128;
    unsigned    n;

    for (n = 0; n < w * h; n++) {
        unsigned    c = *px;
        unsigned    a = PIX32_GET_A(c);
        c = c & 0xFFFFFF;
        if (a <= sikiiA) {
            c = 0;
            a = 0;
        } else {
            unsigned r,g,b,rr,gg,bb;
            rr = r = PIX32_GET_R(c);
            gg = g = PIX32_GET_G(c);
            bb = b = PIX32_GET_B(c);
            if (0 < r && r < l) r = (r < l/2) ? 0 : l;
            if (0 < g && g < l) g = (g < l/2) ? 0 : l;
            if (0 < b && b < l) b = (b < l/2) ? 0 : l;
            c = PIX32_ARGB(0,r,g,b);
            if (c == 0) {
                if (gg > rr && gg > bb)
                    c = PIX32_ARGB(0,0,l,0);
                else if (rr > gg && rr > bb)
                    c = PIX32_ARGB(0,l,0,0);
                else
                    c = PIX32_ARGB(0,0,0,l);
            }
            c |= PIX32_ARGB(a,0,0,0);
        }
        *px++ = c;
    }
}









/*--------------------------------------------------------------------------*/
/* マージ                                                                   */
/*--------------------------------------------------------------------------*/

void pix32_merge(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rate2)
{
    unsigned rate2a = PIX32_GET_A(rate2);
    unsigned rate2r = PIX32_GET_R(rate2);
    unsigned rate2g = PIX32_GET_G(rate2);
    unsigned rate2b = PIX32_GET_B(rate2);
    int x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned c1 = *src1++;
            unsigned c2 = *src2++;
            unsigned a  = (unsigned)(PIX32_GET_A(c1) * (255-rate2a) + PIX32_GET_A(c2) * rate2a);
            unsigned r  = (unsigned)(PIX32_GET_R(c1) * (255-rate2r) + PIX32_GET_R(c2) * rate2r);
            unsigned g  = (unsigned)(PIX32_GET_G(c1) * (255-rate2g) + PIX32_GET_G(c2) * rate2g);
            unsigned b  = (unsigned)(PIX32_GET_B(c1) * (255-rate2b) + PIX32_GET_B(c2) * rate2b);
            unsigned c;
            a /= 255;
            r /= 255;
            g /= 255;
            b /= 255;
            c         = PIX32_ARGB(a, r, g, b);
            *dst++    = c;
        }
    }
}


/// rgb1,rgb2の２色から、各r,g,bがその範囲に収まるピクセルのみα=0xffとなる画像を生成
void pix32_genCol2MaskAlp(unsigned *dst, const unsigned *src, int w, int h, unsigned rgb1, unsigned rgb2)
{
    unsigned    r1 = PIX32_GET_R(rgb1);
    unsigned    g1 = PIX32_GET_G(rgb1);
    unsigned    b1 = PIX32_GET_B(rgb1);
    unsigned    r2 = PIX32_GET_R(rgb2);
    unsigned    g2 = PIX32_GET_G(rgb2);
    unsigned    b2 = PIX32_GET_B(rgb2);
    unsigned    t;
    int x, y;

    if (r1 > r2)
        t = r1, r1 = r2, r2 = t;
    if (g1 > g2)
        t = g1, g1 = g2, g2 = t;
    if (b1 > b2)
        t = b1, b1 = b2, b2 = t;

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned c = *src++;
            unsigned r = PIX32_GET_R(c);
            unsigned g = PIX32_GET_G(c);
            unsigned b = PIX32_GET_B(c);
            int    a = (r1 <= r) & (r <= r2) & (g1 <= g) & (g <= g2) & (b1 <= b) & (b <= b2);
            c        = PIX32_ARGB( (unsigned char)(-a), r, g, b);
            *dst++   = c;
        }
    }
}


/// src1とsrc2をrgbRate2とsrc2のα値を用いて合成. dstのαは src1の値となる(rgbRate2はαには反映しない)
void pix32_mergeSrc2Alp(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rate2)
{
    //x unsigned rate2a = PIX32_GET_A(rate2);
    unsigned rate2r = PIX32_GET_R(rate2);
    unsigned rate2g = PIX32_GET_G(rate2);
    unsigned rate2b = PIX32_GET_B(rate2);
    int x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            unsigned c1 = *src1++;
            unsigned c2 = *src2++;
            unsigned a  = (unsigned)PIX32_GET_A(c1);
            unsigned a2 = (unsigned)PIX32_GET_A(c2);
            unsigned r  = (unsigned)(PIX32_GET_R(c1) * (255*255-a2*rate2r) + PIX32_GET_R(c2) * a2 * rate2r);
            unsigned g  = (unsigned)(PIX32_GET_G(c1) * (255*255-a2*rate2g) + PIX32_GET_G(c2) * a2 * rate2g);
            unsigned b  = (unsigned)(PIX32_GET_B(c1) * (255*255-a2*rate2b) + PIX32_GET_B(c2) * a2 * rate2b);
            unsigned c;
            r /= 255*255;
            g /= 255*255;
            b /= 255*255;
            c         = PIX32_ARGB(a, r, g, b);
            *dst++    = c;
        }
    }
}





