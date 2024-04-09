/**
 *  @file	tga_read.h
 *  @brief	���������tga�摜��W�J����.
 *	@author	Masashi Kitamura
 * @note
 *  ��  ���o�͂ŐF����摜�T�C�Y�̈Ⴄ���k�f�[�^�̓W�J�� malloc&free���g�p
 */
#ifndef TGA_READ_H
#define TGA_READ_H


#if defined __cplusplus
extern "C" {
#endif


/// ���������tga�f�[�^����A�����A�c���A�r�b�g/�s�N�Z���Aclut�̐F�����擾����.
int	 tga_getHdr(const void *tga_data, int *w_p, int *h_p, int *bpp_p, int *clutSize_p);
int	tga_getHdrEx(const void *tga_data, int *wp, int *hp, int *bppp, int *clutNump, int* alpModep);

/// ���������tga�f�[�^����A�s�N�Z���f�[�^�Aclut���擾����. (�T�C�Y�͗\��tga_getHdr�Ŏ擾���ėp�ӂ̂���)
int  tga_read(const void *tga_data, void *dst, int widthByte, int h, int bpp, void *clut, int dir);

/// ���������tga�f�[�^����Aclut �݂̂�clutSize�܂Ŏ擾����. 1�F��A8R8G8B8.
int  tga_getClut(const void *tga_data, void *clut, int clutSize);


#if defined __cplusplus
/// tga_getHdr�Ŏ擾��������,bpp��艡���o�C�g�����v�Z����.
inline int  tga_width2byte(int w, int bpp, int algnByte) { return (w * ((bpp+7)>>3) + algnByte-1) & ~algnByte; }
#else
#define tga_width2byte(w,bpp,algnByte)		((((w) * (((bpp)+7)>>3)) + (algnByte)-1) & ~(algnByte))
#endif


#if defined __cplusplus
}
#endif




// ===========================================================================
// (��L�֐��̎g�p��)

// inline ���w��ł���ꍇ.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)
#include <assert.h>

// �\�� stdlib.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) \
	|| (defined _STDLIB_H_INCLUDED/*watcom*/) || (defined _MSL_STDLIB_H/*CW*/)
#include <stdlib.h>		// calloc,free�̂���.

#if defined __cplusplus
static inline void* tga_readMalloc(const void *d, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=0, int dir=0);
#endif

/** tga_data���摜��W�J�Amalloc�����������ɓ���ĕԂ�.
 *  ���������� *w_p,*h_p,*bpp_p,clut�ɉ���,�c��,bpp,clut(4*256)������. ��clut��clut�t�摜�̎��̂�.
 *  dstBpp �͐��������摜��bpp��ݒ�. 0�Ȃ�摜���g��bpp. 		   ���F��������ݒ�͒P�������őΏ�.
 *  algn �͉����o�C�g���̃A���C�����g. 1,2,4,8,16��z��. �ʏ��1, win-bitmap�p�Ȃ�4��ݒ�̂���.
 *  dir �� 0�Ȃ�ド�C������A1�Ȃ牺���C������擾.  win-bitmap�p�Ȃ�1��ݒ�̂���.
 */
static inline void* tga_readMalloc(const void *tga_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
	int		w, h, bpp, csz, wb;
	void	*m = NULL;

	assert(tga_data && dstBpp >= 0);
	assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);

	if (tga_getHdr(tga_data,&w,&h,&bpp,&csz) == 0)
		return NULL;

	if (dstBpp <= 0)
		dstBpp = bpp;

	wb = tga_width2byte(w, dstBpp, algn);
	m  = calloc(1, wb*h);

	if (tga_read(tga_data, m, wb, h, dstBpp, clut, dir) == 0) {
		free(m);
		return NULL;
	}

	if (w_p)   *w_p   = w;
	if (h_p)   *h_p   = h;
	if (bpp_p) *bpp_p = bpp;
	return m;
}
#endif


// �\�� stdio.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_/*GCC*/) \
	|| (defined _STDIO_H_INCLUDED/*watcom*/) || (defined _MSL_STDIO_H/*CW*/)
#include <stdio.h>

#if defined __cplusplus
static void* tga_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=1, int dir=0);
#endif

static inline void* tga_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
	void*	pix = NULL;
	FILE*	fp  = fopen(fname, "rb");
	if (fp) {
		size_t l;
		fseek(fp, 0, SEEK_END);
		l = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (l) {
			void* dat = malloc(l);
			if (dat) {
				size_t sz = fread(dat, 1, l, fp);
				if (sz == l)
					pix  = tga_readMalloc(dat, w_p, h_p, bpp_p, clut, dstBpp, algn, dir);
				free(dat);
			}
		}
		fclose(fp);
	}
	return pix;

}
#endif

#endif	// inline���g����ꍇ.



#endif	// TGA_READ_H
