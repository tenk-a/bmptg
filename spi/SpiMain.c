/**
 *	@file	SpiMain.c
 *	@brief	Susie �v���O�C���̃��C������.
 *	@author Masashi KITAMURA.(tenk*)
 *	@note
 *	(1999-?	iftga,iftim���̏���)
 *	(2000	�啝������)
 *	2007-6	�قƂ�Ǔ����\�[�X����������̂͏C����ƂŖʓ|�������̂ŁA
 *			���ʏ��������݂̂ɂ��A�񋤒ʕ�����SpiRead.h�ɒǂ��o����.
 */

#include <windows.h>
#include <string.h>
#include "SpiRead.h"


#ifdef USE_REGOPT	// TSPIRO�ɂ�郌�W�X�g���𔽉f����Ƃ�.
#include "RegOpt.h"
#endif


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C		extern "C"
#else
#define EXTERN_C
#endif
#endif



/* SUSIE �v���O�C���֌W -----------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
struct PictureInfo {				/* Susie �v���O�C���p�\���� 			*/
	long	left;					/* �摜��W�J����ʒux					*/
	long	top;					/* �摜��W�J����ʒuy					*/
	long	width;					/* �摜�̕�(pixel)						*/
	long	height; 				/* �摜�̍���(pixel)					*/
	WORD	x_density;				/* ��f�̐����������x					*/
	WORD	y_density;				/* ��f�̐����������x					*/
	short	colorDepth; 			/* �P��f�������bit��					*/
	HLOCAL	hInfo;					/* �摜���̃e�L�X�g��� 				*/
};
#pragma pack(pop)

#define ER_OK				0		/* ����I�� 			  				*/
#define ER_NO_FUNCTION	   -1		/* ���̋@�\�̓C���v�������g����Ă��Ȃ� */
#define ER_ABORT			1		/* �R�[���o�b�N�֐�����0��Ԃ��A�W�J���~*/
#define ER_NOT_SUPPROT		2		/* ���m�̃t�H�[�}�b�g	  				*/
#define ER_OUT_OF_ORDER 	3		/* �f�[�^�����Ă���	  				*/
#define ER_NOT_ENOUGH_MEM	4		/* �������[���m�ۏo���Ȃ� 				*/
#define ER_MEMORY			5		/* �������[�G���[�iLock�o���Ȃ��A���j	*/
#define ER_FILE_READ		6		/* �t�@�C�����[�h�G���[   				*/
/*#define ER_RESERVE		7*/ 	/* (�\��)				  				*/
#define ER_ETC				8		/* �����G���[			  				*/


EXTERN_C __declspec(dllexport) int PASCAL IsSupported(LPSTR filename, DWORD dw);
EXTERN_C __declspec(dllexport) int PASCAL GetPluginInfo(int infono, LPSTR buf, int buflen);
EXTERN_C __declspec(dllexport) int PASCAL GetPictureInfo(LPSTR buf, long len, unsigned flag, struct PictureInfo *lpInfo);
EXTERN_C __declspec(dllexport) int PASCAL GetPicture(
		LPSTR buf, long len, unsigned flag, HLOCAL *pHBInfo, HLOCAL *pHBm,
		int (CALLBACK *lpPrgressCallback)(int,int,long),long lData
		);
EXTERN_C __declspec(dllexport) int PASCAL GetPreview(
		LPSTR buf, long len, unsigned flag, HANDLE *pHBInfo, HANDLE *pHBm,
		FARPROC lpPrgressCallback, long lData
		);


/*---------------------------------------------------------------------------*/


#define GR_WID2BYT(w,bpp)	(((w) * "\1\4\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define GR_WID2BYT4(w,bpp)	((GR_WID2BYT(w,bpp) + 3) & ~3)


static int	FileLoad(char *name, DWORD ofs, DWORD sz, BYTE **ppBuf, DWORD* pSz);
static LPBITMAPINFO SetBmi(int w, int h, int bpp, DWORD **clutp, BYTE **pixp);

#ifdef USE_REGOPT
static int checkRegOpt(HLOCAL *pHBInfo, HLOCAL *pHBm, BYTE *gdat, DWORD sz, int w, int h, int bpp,
						DWORD *clut, SpiRead_InfoOpt* pInfoOpt, int* pDib32Mode);
#endif




//x BOOL WINAPI   DllEntryPoint(HINSTANCE dummy_hInst, DWORD dummy_flag, LPVOID dummy_rsv)
BOOL APIENTRY DllMain(HANDLE dummy_hInst,  DWORD dummy_flag, LPVOID dummy_rsv)
{
	dummy_hInst;
	dummy_flag;
	dummy_rsv;
  #if 0
	switch (flag) {
	case DLL_PROCESS_ATTACH: break;
	case DLL_THREAD_ATTACH:  break;
	case DLL_THREAD_DETACH:  break;
	case DLL_PROCESS_DETACH: break;
	}
  #endif
	return TRUE;
}



EXTERN_C __declspec(dllexport) int PASCAL GetPluginInfo(int infono, LPSTR buf, int bufSize)
{
	DWORD l = 0;
	DWORD n = 0;
	for (n  = 0; SpiRead_pluginInfoMsg[n] != NULL; ++n)
		;
	if ((DWORD)infono < n && buf) {
		l = strlen(SpiRead_pluginInfoMsg[infono]);
		if (l >= (DWORD)bufSize)
			l = bufSize - 1;
		buf[0] = 0;
		if (l > 0) {
			ZeroMemory(buf, l);
			CopyMemory(buf, SpiRead_pluginInfoMsg[infono], l);
			buf[l] = 0;
		}
	}
	return l;
}



/**	dw�Ŏ������f�[�^���A�T�|�[�g���Ă�����̂��ǂ�����Ԃ�(fname�͔���⏕�p)
 */
EXTERN_C __declspec(dllexport) int PASCAL IsSupported(LPSTR fname, DWORD dw)
{
	DWORD 	size	= SpiRead_headerSize;
	char	*tmp	= NULL;
	BYTE*	p;
	BOOL	rc;

	/* �t�@�C������������ */
	if ((dw & 0xFFFF0000) == 0) {	/* �t�@�C���n���h�� */
		DWORD sz;
		if (size == 0)
			size = GetFileSize((HANDLE)dw, NULL);
		tmp	= (char*)LocalAlloc(LMEM_FIXED, size + 4);
		if (tmp == 0)
			return ER_NOT_ENOUGH_MEM;
		ReadFile((HANDLE)dw, tmp, size, &sz, NULL);
		p = (BYTE*)tmp;
	} else {
		p = (BYTE*)dw;
	}
	rc = SpiRead_isSupported((const char*)fname, p);
	if (tmp)
		LocalFree(tmp);
	return rc;
}



/** �摜����Ԃ�. (�W�J���bmp�̌`���łȂ��A�摜�t�@�C���������Ă������Ԃ�)
 */
EXTERN_C __declspec(dllexport) int PASCAL GetPictureInfo(LPSTR buf, long ofs, unsigned flag, struct PictureInfo *info)
{
	SpiRead_InfoOpt	infoOpt;			// SpiRead_getInfo�ɓn���I�v�V�������̂��߂̍\���̕ϐ�.
	int 	 w 		= 0;
	int 	 h 		= 0;
	int		 bpp 	= 0;
	BYTE*	 tmp	= NULL;
	unsigned sz		= 0;
	char	 text[0x4000];

	memset(&text   , 0, sizeof text   );
	memset(&infoOpt, 0, sizeof infoOpt);
	infoOpt.trColor  = -1;
	infoOpt.pText    = text;
	infoOpt.textSize = sizeof text;

	flag   &= 7;
	if (flag == 0) {	/* �t�@�C�����w�b�_��������� */
		int		rc;
		infoOpt.fname = (char*)buf;
		rc = FileLoad(buf, ofs, SpiRead_headerSize, &tmp, (DWORD*)&sz);
		if (rc)
			return rc;
		buf = (LPSTR)tmp;
	}

	if (SpiRead_getInfo((const BYTE*)buf, sz, &w,&h, &bpp, NULL, &infoOpt) == 0) {
		if (tmp)
			LocalFree(tmp);
		return ER_NOT_SUPPROT;
	}
	if (tmp)
		LocalFree(tmp);

	info->left		 = infoOpt.x0;
	info->top		 = infoOpt.y0;
	info->width 	 = w;
	info->height	 = h;
	info->x_density  = (WORD)infoOpt.x_density;
	info->y_density  = (WORD)infoOpt.y_density;
	info->colorDepth = (short)bpp;
	info->hInfo 	 = NULL;
	if (text[0]) {
		//DWORD l = strlen(text) + 1;
		//buf = (LPSTR**)LocalAlloc(LMEM_FIXED, l);
		//memcpy(buf, text, l);
		//* ���
	}
	return ER_OK;
}



/** �v���r���[�摜��Ԃ�. �̃_�~�[�i���T�|�[�g)
 */
EXTERN_C __declspec(dllexport) int PASCAL GetPreview(
		LPSTR	 dummy_buf,
		long	 dummy_len,
		unsigned dummy_flag,
		HLOCAL*  dummy_pHBInfo,
		HLOCAL*  dummy_pHBm,
		FARPROC  dummy_lpPrgressCallback,
		long	 dummy_lData
){
	dummy_buf;
	dummy_len;
	dummy_flag;
	dummy_pHBInfo;
	dummy_pHBm;
	dummy_lpPrgressCallback;
	dummy_lData;
	return ER_NO_FUNCTION;
}



/** �W�J�����摜��Ԃ�.
 */
EXTERN_C __declspec(dllexport) int PASCAL GetPicture(
		LPSTR			nameOrData,
		long			ofs,
		unsigned		flag,
		HLOCAL			*pHBInfo,
		HLOCAL			*pHBm,
		int (CALLBACK	*dummy_lpPrgressCallback)(int,int,long),
		long			dummy_lData
){
	SpiRead_InfoOpt	 infoOpt;
	BITMAPINFO	*bm;
	DWORD		clut0[8*256];
	DWORD		*clut    = clut0;
	int 		dbpp;
	unsigned	sz		 = 0;
	BYTE		*gdat	 = 0;
	BYTE		*pix	 = 0;
	int 		w 		 = 0;
	int 		h 		 = 0;
	int			bpp 	 = 0;
  #if defined USE_DIB32
	int			dib32Mode= 1;
  #else
	int			dib32Mode= 0;
  #endif
	char		text[0x4000];

	dummy_lpPrgressCallback;
	dummy_lData;

	for (h = 0; h < 256; ++h)
		clut[h] = 0x00000000;	// 0xFF000000;
	memset(&text   , 0, sizeof text   );
	memset(&infoOpt, 0, sizeof infoOpt);
	infoOpt.trColor = -1;
	infoOpt.pText   = text;
	infoOpt.textSize= sizeof text;

	flag &= 7;
	if (flag == 0) { /* �t�@�C���Ǎ� */
		infoOpt.fname = (char*)nameOrData;
		h = FileLoad((char*)nameOrData, ofs, 0, &gdat, (DWORD*)&sz);
		if (h != ER_OK) {
			return h;
		}
	} else {
		gdat = (BYTE*)nameOrData;
		sz   = (unsigned)-1;
	}

	if (SpiRead_getInfo((const BYTE*)gdat, sz, &w,&h, &bpp, clut, &infoOpt) == 0) {
		if (flag == 0) LocalFree(gdat);
		return ER_NOT_SUPPROT;
	}

	if (infoOpt.clutSize == 0 && bpp <= 8)
		infoOpt.clutSize = 1 << bpp;

  #ifdef USE_REGOPT // regopt.h��include���Ă���A���W�X�g���o�R�ŁA�����F�┲�F�̏���������
	{
		int rc = checkRegOpt(pHBInfo, pHBm, gdat, sz, w, h, bpp, clut, &infoOpt, &dib32Mode);
		if (rc >= -1) {
			return rc;
		}
	}
  #endif

	// �ʏ�̉摜���[�h
	// bpp����. 1,4,8,24,(32)�r�b�g�F�ɂ���.
	dbpp	 = (bpp <= 8) ? bpp : 24;
	if (dib32Mode && infoOpt.alphaFlag) {		// ���t�摜�̎���32bit bmp�ɂ��郂�[�h�̎�
		if ((dib32Mode & 2) && bpp <= 8)		// ���L���clut�t��dib32��.
			dbpp = 32;
		else if ((dib32Mode & 1) && bpp > 8)	// ���L��̑��F��Ȃ�dib32��.
			dbpp = 32;
	}
	if (infoOpt.dstBpp > 0)						// �W�J���[�`�������Ή���������A����ɍ��킹��.
		dbpp = infoOpt.dstBpp;
	dbpp = (dbpp > 24) ? 32 : (dbpp > 8) ? 24 : (dbpp > 4) ? 8 : (dbpp > 1) ? 4 : 1;

	bm		= SetBmi(w, h, dbpp, &clut, &pix);
	if (bm == NULL) {
		if (flag == 0) LocalFree(gdat);
		return ER_NOT_ENOUGH_MEM;
	}
	*pHBInfo = (HLOCAL)bm;
	*pHBm	 = (HLOCAL)pix;

	SpiRead_getPix(gdat, sz, pix, dbpp, infoOpt.fname);

	/* �t�@�C���ǂݍ��݂�������Γǂݍ��݃o�b�t�@�J�� */
	if (flag == 0) LocalFree(gdat);
	return ER_OK;
}




#ifdef USE_REGOPT	// regopt.h��include���Ă���A���W�X�g���o�R�ŁA�����F�┲�F�̏���������
static int checkRegOpt(HLOCAL *pHBInfo, HLOCAL *pHBm, BYTE *gdat, DWORD sz, int w, int h, int bpp,
						DWORD *clut, SpiRead_InfoOpt* pInfoOpt, int* pDib32Mode)
{
	BITMAPINFO	*bm;
	BYTE		*pix;
	regopt_t	ro_body;
	regopt_t	*ro = &ro_body;
	int 		n	= RegOpt_Get(ro);

	// RegOpt�̑��݂̗L���Ɋւ�炸�Abpp32use�͎擾.
	*pDib32Mode = ro->bpp32use;

	if (n & 2) {
		regopt_s_t rs_body, *rs = &rs_body;
		ZeroMemory(rs, sizeof(*rs));
		// rs->indColKey=0; rs->drctColKey=0; rs->x0 = x0; rs->y0 = y0; rs->xresol = xr; rs->yresol = yr;
		if (pInfoOpt->trColor >= 0) {
			if (bpp <= 8)
				rs->clutKey  = (short)(pInfoOpt->trColor + 1);
			else
				rs->colorKey = pInfoOpt->trColor + 1;
		}
		rs->x0 = pInfoOpt->x0;
		rs->y0 = pInfoOpt->y0;
		rs->xresol = pInfoOpt->x_density;
		rs->yresol = pInfoOpt->y_density;
		RegOpt_Put(rs, w, h, bpp, clut, pInfoOpt->clutSize, pInfoOpt->fname);
	}
	if (n & 1) {
		int 	dstBpp = 24;	//x ro->bpp32use ? 32 : 24;
		int		wkBpp  = (bpp <= 8) ? 8 : 32;
		BYTE	*wkPix;
		*pHBInfo = bm = SetBmi(w, h, dstBpp, NULL, &pix);
		if (bm == NULL)
			return ER_NOT_ENOUGH_MEM;
		*pHBm	 = (HLOCAL)pix;
		wkPix	 = LocalAlloc(LMEM_FIXED, GR_WID2BYT4(w,wkBpp)*h);
		if (wkPix == NULL) {
			LocalFree(pix);
			LocalFree(bm);
			return ER_NOT_ENOUGH_MEM;
		}
		SpiRead_getPix(gdat, sz, wkPix, wkBpp, pInfoOpt->fname);
		LocalFree(gdat);
		return RegOpt_GetPicture(ro, pix, w, h, dstBpp, clut, pInfoOpt->clutSize, wkPix, GR_WID2BYT4(w,wkBpp), wkBpp);
	}
	return -0x80;
}
#endif



/** DIB�́i��������K�v�Ȃ�Ίm�ۂ��āj�w�b�_��ݒ�Bclut�ʒu, �s�N�Z���ʒu��Ԃ�.
 */
static LPBITMAPINFO SetBmi(int w, int h, int bpp, DWORD **clutp, BYTE **pixp)
{
	LPBITMAPINFO	bm;
	BYTE*			d;
	DWORD*			clut;
	int 			wb		 = GR_WID2BYT4(w, bpp);
	int 			clutSize = (bpp <= 8) ? (1 << bpp) : 0;
	int 			hdrSz	 = (bpp <= 24) ? sizeof(BITMAPINFOHEADER) : sizeof(BITMAPV4HEADER);
	int				alcHdrSz = hdrSz + clutSize * sizeof(RGBQUAD);

	// �w�b�_ & clut�������m��
	d = (BYTE*)LocalAlloc(LMEM_FIXED, alcHdrSz);
	if (d == NULL) {
		return NULL;
	}
	ZeroMemory(d, alcHdrSz);
	bm	 = (LPBITMAPINFO)d;

	if (clutp && clutSize > 0) {
		clut   = (DWORD*)(d + hdrSz);
		CopyMemory(clut, *clutp, clutSize*sizeof(DWORD));
		*clutp = clut;
	}

	// �s�N�Z���f�[�^�p�������m��
	if (pixp) {
		d	 = (BYTE*)LocalAlloc(LMEM_FIXED, wb * h);
		if (d == NULL) {
			LocalFree(bm);
			return NULL;
		}
		*pixp = d;
	}

	// �w�b�_�ݒ�
	bm->bmiHeader.biBitCount		= (WORD)bpp;
	bm->bmiHeader.biSize			= hdrSz;
	bm->bmiHeader.biWidth			= w;
	bm->bmiHeader.biHeight			= h;
	bm->bmiHeader.biPlanes			= 1;
	bm->bmiHeader.biCompression 	= BI_RGB;
	bm->bmiHeader.biSizeImage		= wb * h;
	bm->bmiHeader.biXPelsPerMeter	= 0;
	bm->bmiHeader.biYPelsPerMeter	= 0;
	bm->bmiHeader.biClrImportant	= 0;
	bm->bmiHeader.biClrUsed 		= clutSize;

	if (bpp == 32) {
		//x DWORD*	dd				= (DWORD*)(d + sizeof(BITMAPV4HEADER));
		LPBITMAPV4HEADER bmV4hdr 	= (LPBITMAPV4HEADER)d;	//x &bm->bmiHeader;
		bmV4hdr->bV4V4Compression 	= BI_BITFIELDS;
		bmV4hdr->bV4RedMask			= 0x00FF0000;
		bmV4hdr->bV4GreenMask		= 0x0000FF00;
		bmV4hdr->bV4BlueMask 		= 0x000000FF;
		bmV4hdr->bV4AlphaMask		= 0xFF000000;
		bmV4hdr->bV4CSType			= 0;
		memset(&bmV4hdr->bV4Endpoints, 0, sizeof bmV4hdr->bV4Endpoints);
		bmV4hdr->bV4GammaRed		= 0;
		bmV4hdr->bV4GammaGreen		= 0;
		bmV4hdr->bV4GammaBlue		= 0;
	}
	return bm;
}



/** �t�@�C���ǂݍ���.
 */
static int	FileLoad(char *name, DWORD ofs, DWORD sz, BYTE **ppBuf, DWORD* pSz)
{
	HANDLE	hdl;
	DWORD	l;
	BOOL	rc;
	BYTE*	buf;

	hdl = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if (hdl == INVALID_HANDLE_VALUE)
		return ER_FILE_READ;
	l = GetFileSize(hdl, NULL);
  	/* MAC�o�C�i�����΍�... */
	if (ofs > 0 && l > ofs) {
		SetFilePointer(hdl, ofs, NULL, FILE_BEGIN);
		l -= ofs;
	}
	if (sz > 0)
		l = sz;
	buf = (BYTE*)LocalAlloc(LMEM_FIXED, l + 32);
	if (buf == NULL) {
		CloseHandle(hdl);
		return ER_NOT_ENOUGH_MEM;
	}
	if (pSz == NULL)
		pSz = &l;
	rc = ReadFile(hdl, buf, l, pSz, NULL);
	CloseHandle(hdl);
	*ppBuf = buf;
	if (rc == 0) {
		LocalFree(buf);
		return ER_FILE_READ;
	}
	return ER_OK;
}

