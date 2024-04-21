/**
 *  @file gen.h
 *  @brief  ピクセルデータのサイズ調整コピー等
 *  @author Masashi Kitamura
 *  @date   199?
 */
#ifndef GEN_H
#define GEN_H
#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

void gen_newSizePix8(uint8_t **a_pix, int *a_w, int *a_h, int sw,int sh,int sx,int sy, int colKey, int vv_w,int vv_h,int vv_x,int vv_y);
void gen_newSizePix32(uint8_t **a_pix, int *a_w,int *a_h, int sw,int sh,int sx,int sy, int colKey, int vv_w,int vv_h,int vv_x,int vv_y);
void gen_celSz8(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey);
void gen_celSz32(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey);
void gen_nukiRect8(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int nukiClut, int grdMd);
void gen_nukiRect32(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int grdMd);

int gen_clmp8(uint8_t **a_map, int *a_mapSz, int *a_celNum, uint8_t **a_pix, int *a_w, int *a_h, int x0, int y0, int texW, int texH, int cw, int ch, int md, int bpp, int nukiCo, int styl, void *clut, int w0, int h0);
int gen_clmp32(uint8_t **a_map, int *a_mapSz, int *a_celNum, uint8_t **a_pix, int *a_w, int *a_h, int x0, int y0, int texW, int texH, int cw, int ch, int md, int bpp, int nukiCo, int styl, void *clut, int w0, int h0);

#ifdef __cplusplus
};
#endif

/*
■ セル(チップ)＆マップ化。
たとえば640x480 の画像を16x16のセル単位にばらしてダブりなくして
並べなおした256x256等のテクスチャ画像と 独自のマップファイルを
生成可能。
　-mh,-mpにより、画像を、指定矩形(-mcN:M)でセルに分割して、
同一画のセルを統一して画像を減らして詰め合わせたテクスチャ画
を生成できるが、そのとき対になるマップ（ヘッダ）情報は以下の
ようなる。
（値はリトルエンディアン(インテル式)。ヘッダ部は３２バイト)

　OFFS  SIZE    name    補足
　00    4       id      "MAP\0" or "MAP\1"
　04    4       offs    テクスチャ画像へのオフセット(-mp時のみ）
　08    4       rsv.    予約. 値=0
　0C    1       flags   フラグ. 2=1セル属性は1バイト. 以外は2バイト
　0D    1       bpp     テクスチャのBPP
　0E    2       celNum  セルの数
　10    2       mapW    マップの横幅(セル数)
　12    2       mapH    マップの縦幅(セル数)
　14    2       celW    セルの横幅(ピクセル数)
　16    2       celH    セルの横幅(ピクセル数)
　18    2       orgW    元画像の横幅(ピクセル数)
　1A    2       orgH    元画像の横幅(ピクセル数)
　1C    2       x0      表示座標オフセットＸ(ピクセル数)
　1E    2       y0      表示座標オフセットＹ(ピクセル数)
　30    mapW*mapH       マップ情報

・１セルの属性情報は
　　　1バイトのとき、0:透明セル 1..255:セル番号
　　　2バイトのとき
　　　　bit 13-0:　0:透明セル 1～ セル番号
　　　　bit 14:　　半透明を 0=含まないセル　1=含むセル
　　　　bit 15:　　0:透明色,半透明色を含むセル　1=無透明色のみのセル

・テクスチャは、セル１番からが小さい順に左から右に上から下に詰めて
　配置される。0番の透明セルようの画像は用意されないので注意。
*/

#endif
