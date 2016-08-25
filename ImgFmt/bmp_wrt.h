/**
 *	@file	bmp_wrt.h
 *	@brief	BMP �摜�f�[�^�o��
 *	@author	Masashi Kitamura
 *	@date	199?
 */
#ifndef BMP_WRT_H
#define BMP_WRT_H

#ifdef __cplusplus
extern "C" {
#endif

/// bmp�摜�C���[�W�̐���.
int  bmp_write( void *bmp_data, 							// �������ꂽbmp�C���[�W���i�[����o�b�t�@.
				int w, int h, int bpp,						// �o�͂̉���,�c��, bpp
				const void *src, int srcWb, int srcBpp,		// ���͂̃s�N�Z��,�����o�C�g��,bpp
				const void *clut, int flags_dir);			// ���͂� clut, �e��t���O�w��.

/// bmp�摜�C���[�W�̐���.
int  bmp_writeEx(void *bmp_data,							// �������ꂽbmp�C���[�W���i�[����o�b�t�@.
				int w, int h, int dstBpp,					// �o�͂̉���,�c��, bpp
				const void* src, int srcWb, int srcBpp,		// ���͂̃s�N�Z��,�����o�C�g��,�c��,bpp
				const void *clut, int clutSize,				// ���͂� clut �ƁAclut�̐F��,
				int   flags,								// �e��t���O�w��.
				int   resolX, int resolY);					// �o�͂̃w�b�_�ɓ���� �s�N�Z��/���[�g�� �� �������A�c����.

				// �t���O�w��.
				//	bit


/// �w��bpp���A����𖞂������ۂ� bmp �ŃT�|�[�g����� bpp �ɕϊ�.
int  bmp_chkDstBpp(int bpp);

/// w,h,bpp����bmp�C���[�W�ɕK�v�ȃo�C�g����Ԃ�(��ڂɕԂ�).
int  bmp_encodeWorkSize(int w, int h, int bpp);


#ifdef __cplusplus
}
#endif




// ===========================================================================
// (��L�֐��̎g�p��)

// inline ���w��ł���ꍇ.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)

// �\�� stdlib.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) || (defined _STDLIB_H_INCLUDED/*watcom*/)
#include <stdlib.h>		// calloc,free�̂���.

/** �w�肵���摜���Amalloc������������bmp�摜�ɂ��ĕԂ�. ���s�����NULL.
 */
static inline void* bmp_writeMalloc(
		int w, int h, int dstBpp,					// �o�͂̉���,�c��, bpp
		const void* src, int srcWidByt, int srcBpp,	// ���͂̃s�N�Z��,�����o�C�g��,�c��,bpp  �� srcWidByt��0����w��bpp����W���X�g�̒l�����߂�.
		const void *clut, 							// ���͂� clut ��
		int dir,									// ���͂̃s�N�Z���� 0:�ド�C������ 1:�����C������l�߂��Ă���.
		unsigned* pSize)							// ���������T�C�Y
{
	unsigned    dbpp = (dstBpp > 0) ? dstBpp : srcBpp;
	unsigned	bpp  = bmp_chkDstBpp(dbpp);
	unsigned 	sz   = bmp_encodeWorkSize(w, h, bpp);
	void		*m	 = calloc(1, sz);
	if (m == NULL)
		return NULL;

	sz = bmp_write(m, w, h, bpp, src, srcWidByt, srcBpp, clut, dir);
	if (sz == 0) {
		free(m);
		m = NULL;
	}
	if (pSize)
		*pSize = sz;
	return m;
}
#endif


// �\�� stdio.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_/*GCC*/) || (defined _STDIO_H_INCLUDED/*watcom*/)
#include <stdio.h>

#if defined __cplusplus
static int bmp_write_file(const char *fname, int w, int h, int dstBpp, const void* src, int srcWidByt=0, int srcBpp=0, const void* clut=0, int dir=0);
#endif

/// bmp�t�@�C������.
static inline int bmp_write_file(
	const char* fname,							// �o�̓t�@�C����
	int w, int h, int dstBpp,					// �o�͂̉���,�c��, bpp
	const void* src, int srcWidByt, int srcBpp,	// ���͂̃s�N�Z��,�����o�C�g��,�c��,bpp  �� srcWidByt��0����w��bpp����W���X�g�̒l�����߂�.
	const void *clut, 							// ���͂� clut
	int dir)									// ���͂̃s�N�Z���� 0:�ド�C������ 1:�����C������l�߂��Ă���.
{
	size_t   l  = (unsigned)-1;
	unsigned sz = 0;
	void*	 m  = bmp_writeMalloc(w,h,dstBpp, src, srcWidByt, srcBpp, clut, dir, &sz);
	if (m) {
		if (sz) {
			FILE* fp = fopen(fname, "wb");
			if (fp) {
				l = fwrite(m, 1, sz, fp);
				fclose(fp);
			}
		}
		free(m);
	}
	return l == (size_t)sz;
}
#endif

#endif	// inline���g����ꍇ.



#endif	// BMP_WRT_H
