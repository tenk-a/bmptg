/**
 *  @file gen.c
 *  @brief  ピクセルデータのサイズ調整コピー等
 *  @author Masashi Kitamura
 *  @date   199?
 */
#include "subr.h"
#include "pix32.h"
#include "pix8.h"
#include "pix_subr.h"
#include "gen.h"

// dmc v8.41 で mem_mac.h の版だと、理不尽なエラー(コンパイラのバグ)になるので、その回避
#undef MEMSET4
#define MEMSET4(d, s, sz)   do {int  *d__ = (int *)(d); int c__ = (unsigned)(sz)>>2; do { *d__++ = (int) (s); } while(--c__); } while (0)



// ===========================================================================

/// 指定サイズの画像領域に、pixを矩形転送する
void gen_newSizePix8(
    uint8_t **a_pix,
    int   *a_w  , int *a_h,
    int   sw    , int sh,
    int   sx    , int sy,
    int   colKeyIdx,
    int   vv_w  , int vv_h,
    int   vv_x  , int vv_y)
{
    int srcw = *a_w;
    int srch = *a_h;
    int x,y,w,h;
    uint8_t *p, *src = *a_pix;

    sw = (sw) ? sw : srcw;
    sh = (sh) ? sh : srch;
    if (vv_w == 0) vv_w = sw;
    if (vv_h == 0) vv_h = sh;
    w = vv_w;
    h = vv_h;
    x = vv_x;
    y = vv_y;
    p  = (uint8_t*)callocE(1, w * h + 16);
    if (colKeyIdx != -1)
        memset(p, colKeyIdx, w*h+16);
    pix8_copyRect(p, w, h, x, y, src, srcw, srch, sx,sy,sw,sh);
    freeE(src);
    *a_pix = p;
    *a_w   = w;
    *a_h   = h;
}


/// 指定サイズの画像領域に、pixを矩形転送する
void gen_newSizePix32(
    uint8_t **a_pix,
    int   *a_w  , int *a_h,
    int   sw    , int sh,
    int   sx    , int sy,
    int   backColor,
    int   vv_w  , int vv_h,
    int   vv_x  , int vv_y)
{
    int srcw = *a_w;
    int srch = *a_h;
    int x,y,w,h;
    uint8_t *p, *src = *a_pix;

    sw = (sw) ? sw : srcw;
    sh = (sh) ? sh : srch;
    if (vv_w == 0) vv_w = sw;
    if (vv_h == 0) vv_h = sh;
    w = vv_w;
    h = vv_h;
    x = vv_x;
    y = vv_y;
    p  = (uint8_t*)mallocE(4*(w * h + 16));
    MEMSET4(p, backColor, ((w*h+16)*4));
    pix32_copyRect(p, w, h, x, y, src, srcw, srch, sx,sy,sw,sh);
    freeE(src);
    *a_pix = p;
    *a_w   = w;
    *a_h   = h;
}


/// 指定ドット単位のサイズに合わせ直す
void gen_celSz8(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey)
{
    int sw = *a_w;
    int sh = *a_h;
    int w,h; //x,y;
    uint8_t *p, *src = *a_pix;

    if (celW <= 0 || celH <= 0)
        return;
    w = ((sw + celW-1) / celW) * celW;
    h = ((sh + celH-1) / celH) * celH;
    if (dbgExLog_getSw()) printf("\t画像を %d*%d に変更します\n", w, h);
    p  = (uint8_t*)callocE(1, w * h + 16);
    if (colKey != -1)
        memset(p, colKey, w*h+16);
    pix8_copyRect(p, w, h, 0, 0, src, sw, sh, 0,0,sw,sh);
    freeE(src);
    *a_pix = p;
    *a_w   = w;
    *a_h   = h;
}


/// 指定ドット単位のサイズに合わせ直す
void gen_celSz32(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey)
{
    int sw = *a_w;
    int sh = *a_h;
    int w,h;    // x,y
    uint8_t *p, *src = *a_pix;

    if (celW <= 0 || celH <= 0)
        return;
    w = ((sw + celW-1) / celW) * celW;
    h = ((sh + celH-1) / celH) * celH;
    if (dbgExLog_getSw()) printf("\t画像を %d*%d に変更します\n", w, h);
    p  = (uint8_t*)callocE(4, w * h + 16);
    if (colKey != -1)
        MEMSET4(p, colKey, 4*(w*h+16));
    pix32_copyRect(p, w, h, 0, 0, src, sw, sh, 0,0,sw,sh);
    freeE(src);
    *a_pix = p;
    *a_w   = w;
    *a_h   = h;
}



/// 抜き色による矩形領域の縮小
void gen_nukiRect8(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int nukiClut, int grdMd)
{
    int sw = *a_w;
    int sh = *a_h;
    int x  = 0;
    int y  = 0;
    int w,h;
    uint8_t *src = *a_pix;
    uint8_t *p;

    w = sw, h = sh;
    pix_getRectWithoutColKey(src, (unsigned)sw, (unsigned)sh, (uint8_t)nukiClut, &x, &y, &w, &h);   // 抜き色番号で矩形サイズを求める
    if (celW && celH) {
        if ((grdMd&1) == 0) {
            pix_gridRect(celW,celH, &x,&y, &w,&h);                  // グリッド単位に直す
        } else {
            w = ((w + celW-1) / celW) * celW;
            h = ((h + celH-1) / celH) * celH;
        }
    }

    if (grdMd & 2) {
        h += y;
        y =  0;
    } else if (grdMd & 4) {
        w += x;
        x =  0;
    }

    if (x || y || w < sw || h < sh) {
        if (dbgExLog_getSw()) printf("\t画像を(%d,%d) %d*%d に変更します\n", x,y, w,h);
        p = (uint8_t*)callocE(1, w * h);
        //pix32_copyRect(p, w, h, 0,0, src, sw, sh, x, y, w, h);
        pix8_copyRect(p, w, h, 0,0, src, sw, sh, x, y, w, h);
        *a_x += x;
        *a_y += y;
        *a_w =  w;
        *a_h =  h;
        *a_pix = p;
        freeE(src);
    }
}


/// 抜き色による矩形領域の縮小
void gen_nukiRect32(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int grdMd)
{
    int     sw = *a_w;
    int     sh = *a_h;
    int     x  = 0;
    int     y  = 0;
    int     w  = sw;
    int     h  = sh;
    uint8_t *src = *a_pix;
    uint8_t *p;

    pix32_getAlpRect((unsigned*)src, sw, sh, &x, &y, &w, &h);               // α情報で矩形サイズを求める
    if (celW && celH) {
        if ((grdMd&1) == 0) {
            pix_gridRect(celW,celH, &x,&y, &w,&h);              // グリッド単位に直す
        } else {
            w = ((w + celW-1) / celW) * celW;
            h = ((h + celH-1) / celH) * celH;
        }
    }

    if (grdMd & 2) {
        h += y;
        y =  0;
    } else if (grdMd & 4) {
        w += x;
        x =  0;
    }

    if (x || y || w < sw || h < sh) {
        if (dbgExLog_getSw()) printf("\t画像を(%d,%d) %d*%d に変更します\n", x,y, w,h);
        p = (uint8_t*)callocE(4, w * h);
        pix32_copyRect(p, w, h, 0,0, src, sw, sh, x, y, w, h);
        *a_x += x;
        *a_y += y;
        *a_w =  w;
        *a_h =  h;
        *a_pix = p;
        freeE(src);
    }
}
