/**
 *	@file	SpiRead_Bmp.c
 *	@brief	Susie�p bmp �v���O v0.83
 *	@author Masashi KITAMURA (tenk*)
 *	@note
 *	1999-?	(ifbmp.c����)
 *	2000	�啝������
 *	2000-10 0.81 bmp_read.c�̎d�l�ύX�ɔ����C��
 *			���W�X�g���o�R�̎w����ł���悤�ɂ����
 *  2007-06	0.83 Spi�e�v���O�C���̋��ʏ��������Ɣ񋤒ʕ����𕪗�
 *			(���̔񋤒ʕ���)
 *			���J�����˂Ă��̂��A�h�T�N�T�Ɍ��J.
 */

#include "bmp_read.h"
#include "SpiRead.h"

#define GR_WID2BYT(w,bpp)	(((w) * "\1\4\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define GR_WID2BYT4(w,bpp)	((GR_WID2BYT(w,bpp) + 3) & ~3)


#ifdef USE_REGOPT
 #define ABOUT_MSG "bmp to DIB v0.83 (TSPIRO) writen by tenk*"
#else
 #define ABOUT_MSG "bmp to DIB ver 0.83 writen by tenk*"
#endif



/// IsSupported,GetPictureInfo�Ńt�@�C���Ǎ�������ꍇ�́A
/// �摜�t�H�[�}�b�g����ɕK�v�ȃw�b�_�T�C�Y. clut�͓ǂ܂Ȃ��̂ŏ��ʂł���..����.
DWORD		SpiRead_headerSize = 0x1000;



/// �v���O�C�����̕�����z��.
extern const char*	SpiRead_pluginInfoMsg[] = {
	"00IN", 									/* 0 : Plug-in API ver. */
	ABOUT_MSG,									/* 1 : About.. */
	"*.bmp;*.dib",								/* 2 : �g���q */
	"bmp",										/* 3 : �`���� */
	"",
};



/// pData�̉摜�f�[�^���T�|�[�g���Ă��邩(1)�ۂ�(0)��Ԃ�. fname�͔���⏕�̃t�@�C����.
BOOL SpiRead_isSupported(const char* fname, const BYTE* pData)
{
	fname;
	return bmp_getHdr(pData, NULL,NULL,NULL,NULL) != 0;
}



/** pData�̉摜����Ԃ�.
 *  GetPictureInfo�� GetPicture�̗�������Ă΂��.
 *	�l�����ĕԂ����A�f�t�H���g�l�ł悯��Ή������Ȃ��Ă悢.
 *	pClut256�ȊO�͕K���A�h���X������.
 *	clut��ݒ肷��ꍇ�́A1�F�� ��8R8G8B8 �ŁA
 *	���l(0���� 1�`254������ 255�s����)���ݒ肵�ĕԂ�.
 */
BOOL SpiRead_getInfo(
		const BYTE* 		pData,			// �W�J���f�[�^.
		unsigned			dataSize,		// �W�J���f�[�^�̃o�C�g��.
		int*   				pWidth,			// ���������ĕԂ�.
		int*  				pHeight,		// �c�������ĕԂ�.
		int* 	 			pBpp,			// �摜�̂P�s�N�Z��������̃r�b�g��.(�W�J�\��́A�ł͂Ȃ�)
		DWORD* 				pClut256,		// 256�F�܂ł̐F�p���b�g(clut)�����ĕԂ�. �A�h���X0�Ȃ疳��.
		SpiRead_InfoOpt* 	pInfoOpt)		// �I�v�V�����I�ȏ���Ԃ��ꍇ�p.
{
	int	resolX=0, resolY=0;
	if (bmp_getHdrEx((void*)pData, pWidth,pHeight, pBpp, &pInfoOpt->clutSize, &resolX, &resolY) == 0)
		return 0;

	pInfoOpt->x_density = resolX;
	pInfoOpt->y_density = resolY;

	if (pClut256) {
		int clutSize = pInfoOpt->clutSize;
		if (clutSize > 256)
			clutSize = 256;
		if (clutSize > 0) {
			int rc = bmp_getClut(pData, pClut256, clutSize);
		}
	}

	return 1;
}



/** pData�� pixBpp�r�b�g�F�摜�ɓW�J����pPix�ɓ���ĕԂ�.
 *	- bmp�Ɠ��l�ɁA�����o�C�g����4�̔{��, �����C������l�߂�.
 *
 *  - �s�N�Z���z�u��bmp�ɍ��킹��.
 *  - pixBpp�́A�ʏ��
 *		��bpp�� 1�Ȃ� 1, ��bpp 2�`4 �Ȃ� 4, ��bpp 5�`8�Ȃ� 8�A��9�ȏ�Ȃ�24
 *	  ���ݒ肳��Ă���B������ USE_DIB32 ����`�ݒ肳��Ă���ꍇ�́A
 *		�����F|���t�摜�ɑ΂��Ă� bpp 32
 *	  ���ݒ肳�ꂤ��B�܂��A
 *	  tspiro���W�X�g���I�v�V�����p�� USE_REGOPT ����`����Ă���ꍇ��
 *		��bpp�� 8�ȉ��Ȃ� 8�A��9�ȏ�� 32
 *	  ���ݒ肳�ꂤ��. (USE_BMP32��USE_REGOPT����`�Ȃ�C�ɂ��Ȃ�)
 *
 *  - fname�͕����t�@�C���\���̃f�[�^�����A�����Afname=NULL�̏󋵂�����̂Œ���.
 */
int	 SpiRead_getPix(const BYTE* pData, unsigned dataSize, BYTE* pPix, DWORD pixBpp, const char* fname)
{
	int 	w;
	int		h;
	int		widByt;
	fname;

	if (bmp_getHdr(pData, &w, &h, NULL, NULL) == 0)
		return 0;

	widByt = GR_WID2BYT4(w,pixBpp);
	return bmp_read(pData, pPix, widByt, h, pixBpp, NULL, 1) != 0;
}
