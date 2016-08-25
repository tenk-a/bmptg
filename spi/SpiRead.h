/**
 *	@file	SpiMain.h
 *	@brief	Susie プラグインのメイン処理.
 *	@author Masashi KITAMURA.(tenk*)
 *	@note
 *	(1999-?	iftga,iftim等の初版)
 *	(2000	大幅改正版)
 *	2007	ほとんど同じソースが複数あるのは修正作業で面倒だったので、
 *			共通処理部分のみにし、非共通部分をSpiRead.hに追い出した.
 */

#ifndef SPIREAD_H
#define SPIREAD_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SPIREAD_MALLOC(p)		LocalAlloc(LMEM_FIXED, (p))
#define SPIREAD_FREE(p)			LocalFree(p)


/// 画像フォーマット判定に必要なヘッダサイズ. (ファイル読込時用)
extern DWORD		SpiRead_headerSize;



/// プラグイン情報の文字列配列.
extern const char*	SpiRead_pluginInfoMsg[];



/// pDataの画像データをサポートしているか(1)否か(0)を返す. fnameは判定補助のファイル名.
BOOL SpiRead_isSupported(const char* fname, const BYTE* pData);



/** SpiRead_getInfo で、オプション要素的な情報を入れて返すための構造体.
 *	呼び元が初期化してるので、デフォルトでよければ設定の必要なし.
 */
typedef struct SpiRead_InfoOpt {
	int			clutSize;	///< 普通0. 16色clut n本のような場合合計色数を設定. bppより少ないのは不可.
	int			trColor;	///< 透過色がある場合設定. 無ければ-1. clutなら番号. 多色ならRGB24ビット値.
	int			alphaFlag;	///< 画素かclut中にα情報があるなら1, 無しなら0. -1ならデフォルト.
	int			x0;			///< 始点x. デフォルト0.
	int			y0;			///< 始点y. デフォルト0.
	WORD		x_density;	///<  デフォルト0.
	WORD		y_density;	///<  デフォルト0.
	int			dstBpp;		///< 普通0. getPix()で所定のbppより大きい色数(例:16色→256色)で返す場合のbppを設定.( 4,8,24,32 の何れか )
	char* 		pText;		///< テキスト情報を入れて返す.(必ず呼び元が中身を0クリアしてアドレスを設定)
	DWORD 		textSize;	///< pTextの指すメモリのバイト数. (呼び元が設定,getInfo側は参照のみ)
	const char*	fname;		///< (参照用)データのファイル名. NULLの場合もあるので注意.
} SpiRead_InfoOpt;


/// pDataの画像情報を返す.
BOOL SpiRead_getInfo(const BYTE* pData, unsigned dataSize, int*  pWidth, int*  pHeight, int* pBpp, DWORD* pClut256, SpiRead_InfoOpt* infoOpt);
					// GetPictureInfoと GetPictureの両方から呼ばれる.
					// pClut256 はアドレスが設定されていれば、
					// 256色までのclut情報を入れて返す.(NULLなら無視)
					// 1色は α8R8G8B8 で、α=0透明 1～254半透明 255不透明も必ず設定.
					// それ以外のポインタは必ず呼び元で初期化されたアドレスを用意.



/// pDataを pixBppビット色画像に展開してpPixに入れて返す. 
BOOL SpiRead_getPix(const BYTE* pData, unsigned dataSize,  BYTE* pPix, DWORD pixBpp, const char* fname);
					//  fnameは複数構成のデータ向け、だが、fname=NULLの状況もあるので注意.
					//  ピクセル配置はbmpに合わせること. pixBppは、
					//		元bpp 1	     なら pixBpp は 1 or 8
					//		元bpp 2～4   なら pixBpp は 4 or 8
					//		元bpp 5～8   なら pixBpp は 8
					//		元bpp 9以上  なら pixBpp は 24 or 32
					//	が設定される.

#ifdef __cplusplus
}
#endif

#endif	// SPIREAD_H
