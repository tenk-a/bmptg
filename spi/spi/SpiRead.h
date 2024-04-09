/**
 *	@file	SpiMain.h
 *	@brief	Susie �v���O�C���̃��C������.
 *	@author Masashi KITAMURA.(tenk*)
 *	@note
 *	(1999-?	iftga,iftim���̏���)
 *	(2000	�啝������)
 *	2007	�قƂ�Ǔ����\�[�X����������̂͏C����ƂŖʓ|�������̂ŁA
 *			���ʏ��������݂̂ɂ��A�񋤒ʕ�����SpiRead.h�ɒǂ��o����.
 */

#ifndef SPIREAD_H
#define SPIREAD_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SPIREAD_MALLOC(p)		LocalAlloc(LMEM_FIXED, (p))
#define SPIREAD_FREE(p)			LocalFree(p)


/// �摜�t�H�[�}�b�g����ɕK�v�ȃw�b�_�T�C�Y. (�t�@�C���Ǎ����p)
extern DWORD		SpiRead_headerSize;



/// �v���O�C�����̕�����z��.
extern const char*	SpiRead_pluginInfoMsg[];



/// pData�̉摜�f�[�^���T�|�[�g���Ă��邩(1)�ۂ�(0)��Ԃ�. fname�͔���⏕�̃t�@�C����.
BOOL SpiRead_isSupported(const char* fname, const BYTE* pData);



/** SpiRead_getInfo �ŁA�I�v�V�����v�f�I�ȏ������ĕԂ����߂̍\����.
 *	�Ăь������������Ă�̂ŁA�f�t�H���g�ł悯��ΐݒ�̕K�v�Ȃ�.
 */
typedef struct SpiRead_InfoOpt {
	int			clutSize;	///< ����0. 16�Fclut n�{�̂悤�ȏꍇ���v�F����ݒ�. bpp��菭�Ȃ��͕̂s��.
	int			trColor;	///< ���ߐF������ꍇ�ݒ�. �������-1. clut�Ȃ�ԍ�. ���F�Ȃ�RGB24�r�b�g�l.
	int			alphaFlag;	///< ��f��clut���Ƀ���񂪂���Ȃ�1, �����Ȃ�0. -1�Ȃ�f�t�H���g.
	int			x0;			///< �n�_x. �f�t�H���g0.
	int			y0;			///< �n�_y. �f�t�H���g0.
	WORD		x_density;	///<  �f�t�H���g0.
	WORD		y_density;	///<  �f�t�H���g0.
	int			dstBpp;		///< ����0. getPix()�ŏ����bpp���傫���F��(��:16�F��256�F)�ŕԂ��ꍇ��bpp��ݒ�.( 4,8,24,32 �̉��ꂩ )
	char* 		pText;		///< �e�L�X�g�������ĕԂ�.(�K���Ăь������g��0�N���A���ăA�h���X��ݒ�)
	DWORD 		textSize;	///< pText�̎w���������̃o�C�g��. (�Ăь����ݒ�,getInfo���͎Q�Ƃ̂�)
	const char*	fname;		///< (�Q�Ɨp)�f�[�^�̃t�@�C����. NULL�̏ꍇ������̂Œ���.
} SpiRead_InfoOpt;


/// pData�̉摜����Ԃ�.
BOOL SpiRead_getInfo(const BYTE* pData, unsigned dataSize, int*  pWidth, int*  pHeight, int* pBpp, DWORD* pClut256, SpiRead_InfoOpt* infoOpt);
					// GetPictureInfo�� GetPicture�̗�������Ă΂��.
					// pClut256 �̓A�h���X���ݒ肳��Ă���΁A
					// 256�F�܂ł�clut�������ĕԂ�.(NULL�Ȃ疳��)
					// 1�F�� ��8R8G8B8 �ŁA��=0���� 1�`254������ 255�s�������K���ݒ�.
					// ����ȊO�̃|�C���^�͕K���Ăь��ŏ��������ꂽ�A�h���X��p��.



/// pData�� pixBpp�r�b�g�F�摜�ɓW�J����pPix�ɓ���ĕԂ�. 
BOOL SpiRead_getPix(const BYTE* pData, unsigned dataSize,  BYTE* pPix, DWORD pixBpp, const char* fname);
					//  fname�͕����\���̃f�[�^�����A�����Afname=NULL�̏󋵂�����̂Œ���.
					//  �s�N�Z���z�u��bmp�ɍ��킹�邱��. pixBpp�́A
					//		��bpp 1	     �Ȃ� pixBpp �� 1 or 8
					//		��bpp 2�`4   �Ȃ� pixBpp �� 4 or 8
					//		��bpp 5�`8   �Ȃ� pixBpp �� 8
					//		��bpp 9�ȏ�  �Ȃ� pixBpp �� 24 or 32
					//	���ݒ肳���.

#ifdef __cplusplus
}
#endif

#endif	// SPIREAD_H
