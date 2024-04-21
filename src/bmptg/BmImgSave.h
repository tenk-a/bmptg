/**
 *  @file   BmImgSave.h
 *  @brief  各種フォーマット(bmp,tga ..)のセーブ
 *  @author Masashi Kitamura
 *  @date   2000
 */

#ifndef BMIMGSAVE_H
#define BMIMGSAVE_H


/// 画像変換のオプション
typedef struct bm_opt_t {
    int x0,y0;
    int clutNum;
    int tx,ty;
    int srcW, srcH;
    int clutX,clutY, clutW, clutH;
    int clutBpp;
    int nukiCo,nukiCoI;
    int resolX, resolY;
    int celW, celH;
    int mapW, mapH;
    int lvlY, lvlUV;
    int mapTexW, mapTexH;
    int quality;
    int quality_grey;
    int alpBitForBpp8;
    int mono;
} bm_opt_t;


typedef enum BM_FLAG_T {
    BM_FLAG_COLKEY_SH   = 5,
    BM_FLAG_CLUT_ALP_SH = 6,
    BM_FLAG_EX_ENC_SH   = 7,
    BM_FLAG_COLKEY      = 1 << BM_FLAG_COLKEY_SH,       ///< 抜き色があるか
    BM_FLAG_CLUT_ALP    = 1 << BM_FLAG_CLUT_ALP_SH,     ///< alpha付clutにするか
    BM_FLAG_EX_ENC      = 1 << BM_FLAG_EX_ENC_SH,       ///< 特別の圧縮をするか
} BM_FLAG_T;


int  bm_write(int fmt, void *bm_data, int w, int h, int bpp, const void *src, int srcWb, int srcBpp, const void *clut, int dir_flags, const bm_opt_t *o);
int  bm_chkDstBpp(int fmt, int bpp);
int  bm_encodeWorkSize(int fmt, int w, int h, int bpp);

#endif
