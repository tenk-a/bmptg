/**
 *	@file	SpiRead_Tga.c
 *	@brief	Susie用 tga プラグ v0.84
 *	@author Masashi KITAMURA (tenk*)
 *	@note
 *	1999-?	(iftga.c初版)
 *	2000	大幅改正版
 *	2000-10 0.81 tga_read.cの仕様変更に伴う修正
 *			レジストリ経由の指定をできるようにする版
 *	2001	0.82 tga_getHdrの引数でbppとclutSizeの順番が誤っていたのを修正.
 *			IsSupported()で読込バッファが0x12でなく0x10で溢れてたのを修正.
 *	2007-05 0.83 tgaのxy反転フラグのビットの解釈を逆にしていたのを修正.
 *			 (Issiki氏の改造版 iftgaxよりバックポート)
 *  2007-06	0.84 Spi各プラグインの共通処理部分と非共通部分を分離
 *			(その非共通部分)
 */

#include "tga_read.h"
#include "SpiRead.h"

#define GR_WID2BYT(w,bpp)	(((w) * "\1\4\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define GR_WID2BYT4(w,bpp)	((GR_WID2BYT(w,bpp) + 3) & ~3)


#ifdef USE_REGOPT
 #define ABOUT_MSG "tga to DIB v0.84b (TSPIRO) writen by tenk*"
#else
 #define ABOUT_MSG "tga to DIB ver 0.84b writen by tenk*"
#endif



/// IsSupported,GetPictureInfoでファイル読込をする場合の、
/// 画像フォーマット判定に必要なヘッダサイズ. clutは読まないので少量ですむ..かも.
DWORD		SpiRead_headerSize = 0x1000;		// 32バイトもあればいが..念のため4K.



/// プラグイン情報の文字列配列.
extern const char*	SpiRead_pluginInfoMsg[] = {
	"00IN", 									/* 0 : Plug-in API ver. */
	ABOUT_MSG,									/* 1 : About.. */
	"*.tga",									/* 2 : 拡張子 */
	"tga(targa)",								/* 3 : 形式名 */
	"",
};



/// pDataの画像データをサポートしているか(1)否か(0)を返す. fnameは判定補助のファイル名.
BOOL SpiRead_isSupported(const char* fname, const BYTE* pData)
{
	fname;
	return tga_getHdr(pData, NULL,NULL,NULL,NULL) != 0;
}



/** pDataの画像情報を返す.
 *  GetPictureInfoと GetPictureの両方から呼ばれる.
 *	値を入れて返すが、デフォルト値でよければ何もしなくてよい.
 *	pClut256以外は必ずアドレスがある.
 *	clutを設定する場合は、1色は α8R8G8B8 で、
 *	α値(0透明 1～254半透明 255不透明)も設定して返す.
 */
BOOL SpiRead_getInfo(
		const BYTE* 		pData,			// 展開元データ.
		unsigned			dataSize,		// 展開元データのバイト数.
		int*   				pWidth,			// 横幅を入れて返す.
		int*  				pHeight,		// 縦幅を入れて返す.
		int* 	 			pBpp,			// 画像の１ピクセルあたりのビット数.(展開予定の、ではなく)
		DWORD* 				pClut256,		// 256色までの色パレット(clut)を入れて返す. アドレス0なら無視.
		SpiRead_InfoOpt* 	pInfoOpt)		// オプション的な情報を返す場合用.
{
	int alphaFlag = 0;		// tgaのピクセルおよびclutの bpp が 16か32の時はα付き.
	if (tga_getHdrEx((void*)pData, pWidth,pHeight, pBpp, &pInfoOpt->clutSize, &alphaFlag) == 0)
		return 0;

	if (pClut256) {
		int clutSize = pInfoOpt->clutSize;
		if (clutSize > 256)
			clutSize = 256;
		if (clutSize > 0) {
			int rc = tga_getClut(pData, pClut256, clutSize);
			alphaFlag = (rc == 2);		// α付きのclutだったら2が帰ってきている.
		}
	}

	pInfoOpt->alphaFlag = alphaFlag;

	return 1;
}



/** pDataを pixBppビット色画像に展開してpPixに入れて返す.
 *	- bmpと同様に、横幅バイト数は4の倍数, 下ラインから詰める.
 *
 *  - ピクセル配置はbmpに合わせる.
 *  - pixBppは、通常は
 *		元bppが 1なら 1, 元bpp 2～4 なら 4, 元bpp 5～8なら 8、元9以上なら24
 *	  が設定されてくる。ただし USE_DIB32 が定義設定されている場合は、
 *		透明色|α付画像に対しては bpp 32
 *	  が設定されうる。また、
 *	  tspiroレジストリオプション用に USE_REGOPT が定義されている場合は
 *		元bppが 8以下なら 8、元9以上は 32
 *	  が設定されうる. (USE_BMP32やUSE_REGOPT未定義なら気にしない)
 *
 *  - fnameは複数ファイル構成のデータ向け、だが、fname=NULLの状況もあるので注意.
 */
int	 SpiRead_getPix(const BYTE* pData, unsigned dataSize, BYTE* pPix, DWORD pixBpp, const char* fname)
{
	int 	w;
	int		h;
	int		widByt;
	fname;

	if (tga_getHdr(pData, &w, &h, NULL, NULL) == 0)
		return 0;

	widByt = GR_WID2BYT4(w,pixBpp);
	return tga_read(pData, pPix, widByt, h, pixBpp, NULL, 1) != 0;
}
