/**
 *
 */
#ifndef PIX32_H
#define PIX32_H


#ifndef PIX32_ARGB	// a,r,g,b�����������}�N��

/// 8bit�l��a,r,g,b���q���� 32bit �̐F�l�ɂ���
#define PIX32_ARGB(a,r,g,b)		((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)		((unsigned char)(argb))			///< argb�l���� blue�̒l���擾
#define PIX32_GET_G(argb)		((unsigned char)((argb)>>8))	///< argb�l���� green�̒l���擾
#define PIX32_GET_R(argb)		((unsigned char)((argb)>>16))	///< argb�l���� green�̒l���擾
#define PIX32_GET_A(argb)		(((unsigned)(argb))>>24)		///< argb�l���� alpha�̒l���擾

#endif

#ifdef __cplusplus
extern "C" {
#endif


void pix32_copyRect(void *dst, int dstW, int dstH, int dstX, int dstY, void *src, int srcW, int srcH, int rctX, int rctY, int rctW, int rctH);
void pix32_copyWH(void *dst, int dstW, void *src, int srcW, int rctW, int rctH);
	// src�摜(����srcW)�̋�`(rctW*rctH)��dst�摜(����dstW)�ɃR�s�[
	// �͈̓`�F�b�N�Ȃǂ͂��Ȃ��̂Ŏg���葤�Œ����̂���

// int pix32_getNukiRect(void *pix0, int xsz, int ysz, int nukiClut, int *x_p, int *y_p, int *w_p, int *h_p);
//	/* �w�i�F�ȊO���g���Ă����`�����߂�. */

// int pix32_getAlpRect(void *pix0, int xsz, int ysz, int *x_p, int *y_p, int *w_p, int *h_p);
//	/* ��!=0���g���Ă����`�����߂�. */

//x int pix32_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p);
//x	/* �O���b�h�T�C�Y gw*gh �Ƃ��āA�O���b�h�ɍ����悤�ɋ�`�T�C�Y�𒲐� */


void pix32_blackAMskGen(unsigned *pix, unsigned w, unsigned h, unsigned sikiiA, unsigned bpp);

void pix32_merge(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rgbRate2);
void pix32_genCol2MaskAlp(unsigned *dst, const unsigned *src, int w, int h, unsigned rgb1, unsigned rgb2);
void pix32_mergeSrc2Alp(unsigned *dst, const unsigned *src1, const unsigned *src2, int w, int h, unsigned rgbRate2);


#ifdef __cplusplus
}
#endif


#endif
