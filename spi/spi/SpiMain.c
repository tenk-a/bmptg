/**
 *	@file	SpiMain.c
 *	@brief	Susie プラグインのメイン処理.
 *	@author Masashi KITAMURA.(tenk*)
 *	@note
 *	(1999-?	iftga,iftim等の初版)
 *	(2000	大幅改正版)
 *	2007-6	ほとんど同じソースが複数あるのは修正作業で面倒だったので、
 *			共通処理部分のみにし、非共通部分をSpiRead.hに追い出した.
 */

#include <windows.h>
#include <string.h>
#include "SpiRead.h"


#ifdef USE_REGOPT	// TSPIROによるレジストリを反映するとき.
#include "RegOpt.h"
#endif


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C		extern "C"
#else
#define EXTERN_C
#endif
#endif



/* SUSIE プラグイン関係 -----------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
struct PictureInfo {				/* Susie プラグイン用構造体 			*/
	long	left;					/* 画像を展開する位置x					*/
	long	top;					/* 画像を展開する位置y					*/
	long	width;					/* 画像の幅(pixel)						*/
	long	height; 				/* 画像の高さ(pixel)					*/
	WORD	x_density;				/* 画素の水平方向密度					*/
	WORD	y_density;				/* 画素の垂直方向密度					*/
	short	colorDepth; 			/* １画素当たりのbit数					*/
	HLOCAL	hInfo;					/* 画像内のテキスト情報 				*/
};
#pragma pack(pop)

#define ER_OK				0		/* 正常終了 			  				*/
#define ER_NO_FUNCTION	   -1		/* その機能はインプリメントされていない */
#define ER_ABORT			1		/* コールバック関数が非0を返し、展開中止*/
#define ER_NOT_SUPPROT		2		/* 未知のフォーマット	  				*/
#define ER_OUT_OF_ORDER 	3		/* データが壊れている	  				*/
#define ER_NOT_ENOUGH_MEM	4		/* メモリーが確保出来ない 				*/
#define ER_MEMORY			5		/* メモリーエラー（Lock出来ない、等）	*/
#define ER_FILE_READ		6		/* ファイルリードエラー   				*/
/*#define ER_RESERVE		7*/ 	/* (予約)				  				*/
#define ER_ETC				8		/* 内部エラー			  				*/


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



/**	dwで示されるデータが、サポートしているものかどうかを返す(fnameは判定補助用)
 */
EXTERN_C __declspec(dllexport) int PASCAL IsSupported(LPSTR fname, DWORD dw)
{
	DWORD 	size	= SpiRead_headerSize;
	char	*tmp	= NULL;
	BYTE*	p;
	BOOL	rc;

	/* ファイルかメモリか */
	if ((dw & 0xFFFF0000) == 0) {	/* ファイルハンドル */
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



/** 画像情報を返す. (展開後のbmpの形式でなく、画像ファイルが持っている情報を返す)
 */
EXTERN_C __declspec(dllexport) int PASCAL GetPictureInfo(LPSTR buf, long ofs, unsigned flag, struct PictureInfo *info)
{
	SpiRead_InfoOpt	infoOpt;			// SpiRead_getInfoに渡すオプション情報のための構造体変数.
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
	if (flag == 0) {	/* ファイルよりヘッダ部分を入力 */
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
		//* 後で
	}
	return ER_OK;
}



/** プレビュー画像を返す. のダミー（未サポート)
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



/** 展開した画像を返す.
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
	if (flag == 0) { /* ファイル読込 */
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

  #ifdef USE_REGOPT // regopt.hをincludeしてたら、レジストリ経由で、透明色や抜色の処理をする
	{
		int rc = checkRegOpt(pHBInfo, pHBm, gdat, sz, w, h, bpp, clut, &infoOpt, &dib32Mode);
		if (rc >= -1) {
			return rc;
		}
	}
  #endif

	// 通常の画像ロード
	// bpp調整. 1,4,8,24,(32)ビット色にする.
	dbpp	 = (bpp <= 8) ? bpp : 24;
	if (dib32Mode && infoOpt.alphaFlag) {		// α付画像の時は32bit bmpにするモードの時
		if ((dib32Mode & 2) && bpp <= 8)		// α有りのclut付をdib32に.
			dbpp = 32;
		else if ((dib32Mode & 1) && bpp > 8)	// α有りの多色画ならdib32に.
			dbpp = 32;
	}
	if (infoOpt.dstBpp > 0)						// 展開ルーチンが未対応だったら、それに合わせる.
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

	/* ファイル読み込みだったらば読み込みバッファ開放 */
	if (flag == 0) LocalFree(gdat);
	return ER_OK;
}




#ifdef USE_REGOPT	// regopt.hをincludeしてたら、レジストリ経由で、透明色や抜色の処理をする
static int checkRegOpt(HLOCAL *pHBInfo, HLOCAL *pHBm, BYTE *gdat, DWORD sz, int w, int h, int bpp,
						DWORD *clut, SpiRead_InfoOpt* pInfoOpt, int* pDib32Mode)
{
	BITMAPINFO	*bm;
	BYTE		*pix;
	regopt_t	ro_body;
	regopt_t	*ro = &ro_body;
	int 		n	= RegOpt_Get(ro);

	// RegOptの存在の有無に関わらず、bpp32useは取得.
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



/** DIBの（メモリを必要ならば確保して）ヘッダを設定。clut位置, ピクセル位置を返す.
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

	// ヘッダ & clutメモリ確保
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

	// ピクセルデータ用メモリ確保
	if (pixp) {
		d	 = (BYTE*)LocalAlloc(LMEM_FIXED, wb * h);
		if (d == NULL) {
			LocalFree(bm);
			return NULL;
		}
		*pixp = d;
	}

	// ヘッダ設定
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



/** ファイル読み込み.
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
  	/* MACバイナリ等対策... */
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

