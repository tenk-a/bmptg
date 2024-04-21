/**
 *  @file   pix8.h
 *  @biref  8�r�b�g�F�摜�����낢��ϊ�
 *  @author Masashi Kitamura
 *  @date   2000
 */
#ifndef PIX8_H
#define PIX8_H

#ifdef __cplusplus
extern "C" {
#endif

void pix8_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH);

/// src�摜(����srcW)�̋�`(rctW*rctH)��dst�摜(����dstW)�ɃR�s�[
void pix8_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH);

/// �P���Ȋg��
void pix8_resize(uint8_t *pix2, unsigned rszW, unsigned rszH, const uint8_t *pix, unsigned w, unsigned h);

/// src�摜���� bpp �͈̔͊O�ɂȂ�ԍ������邩�`�F�b�N. ����Ε�����Ԃ�.
int pix8_hasPixOutOfIdx(uint8_t const* src, int w, int h, int idx);

//x /// �w�i�F�ȊO���g���Ă����`�����߂�B�����F�̓C���f�b�N�X�E�J���[
//x int pix8_getNukiRectI(void *pix0, int xsz, int ysz, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);

//x /// �w�i�F�ȊO���g���Ă����`�����߂�. �����F�̓t���J���[
//x int pix8_getNukiRectF(void *pix0, int xsz, int ysz, int *clut, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);

//x int pix8_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p);
//x int  pix8_revXY(void *pix, int w, int h, int dir);
//x void pix8_bitCom(void *pix, int w, int h);

#ifdef __cplusplus
};
#endif

#endif  /* PIX8_H */
