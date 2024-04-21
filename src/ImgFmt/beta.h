/**
 *  @file   beta.h
 *  @brief  生バイナリの生成。パレット無し. 読み込み系は未対応
 *  @author Masashi Kitamura
 */
#ifndef BETA_H
#define BETA_H


#ifdef __cplusplus
extern "C" {
#endif

int  beta_getHdr(const void *beta_data, int *w_p, int *h_p, int *bpp_p, int *clutNum_p);    //dummy
int  beta_getClut(const void *tga_data, void *clut, int clutNum);   //dummy
int  beta_read(const void *tga_data, void *dst, int wb, int h, int dbpp, void *clut, int dir);  //dummy
int  beta_write(const void *beta_data, int w, int h, int dbpp, const void *src, int srcWb, int sbpp, void *clut, int dir);

int  beta_conv(const void *beta_data, int dstWb, int h, int dbpp, const void *src, int srcWb, int sbpp, const void *clut, int flags, int boI, int boO);
void beta_genClut(unsigned *clut, int dbpp);
void beta_genMonoClut(unsigned *clut, int bpp);

#ifdef __cplusplus
};
#endif


#endif
