/**
 *	@file	RegOpt.h
 *	@brief	tspiro.exeと専用susieプラグインがレジストリを介してやり取りする.
 *	@author	Masashi KITAMURA
 *	date	200?
 *	note
 *		2007	bpp32use関係の実装
 */

#ifndef REGOPT_H
#define REGOPT_H
// #include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


#define REGOPT_VER			0x0100
#define REGOPT_S_VER		0x0100
#define REGOPT_CLUT_NUM 	2048
#define REGOPT_FILE_NAME	"Software\\tenk\\TSPIRO"	//レジストリに作られる

typedef struct regopt_t {
	// regoptコントローラからの指示。xは未実装（or廃案)
	SHORT	ver;		// 上位WORD. 互換制を維持している間は固定の値. 下位 バージョン番号
	SHORT	size;		// この構造体のサイズ
	BYTE	enable; 	// bit0:レジストリの内容を	  0=使わない 1=使う
						// bit1:画像情報の取得を要求  0=しない	 1=する
	BYTE	alpType;	// α情報の扱い 0:α無視  1:ブレンド  2:加算  3:減算  4:PS風擬似αブレンド
	BYTE	alpMode;	// 0:全体にα値を反映  1:α値が 0以外のピクセルのみに反映
	BYTE	alpRev; 	// α値の扱いを 0:0xFFなら不透明～0なら透明  1:0なら不透明～0xFFなら透明
	BYTE	alpHalf;	// 0:通常 1:α値は 128が100%となるように扱う
	BYTE	keyMode;	// 抜色処理 0:無し	1:すべてダイレクトカラー  2:256色以下はclut番号,多色はダイレクト
	BYTE	aspReq; 	//x 予定:元画像のドット比が1:1でないとき調整 0:しない 2:する
	BYTE	bpp32use;	// Bpp 24でなく 32 として、画像を呼び出し元へ返す.
	SHORT	tone;		// 輝度. 1000 を 100%とする
	SHORT	alpVal; 	// ピクセル固有のα値をさらに0..255段階にする
	SHORT	clutOfs;	// clutテーブルをこの値分ずらして描画。
	SHORT	clutKey;	// clutでの抜き色番号
	DWORD	colKey; 	// 抜き色 RGB値。
	//
	DWORD	gridCol;	// 色
	DWORD	bgCol1; 	// BG色1
	DWORD	bgCol2; 	// BG色2
	SHORT	gridW;		// グリッドの横幅 2～1024
	SHORT	gridH;		// グリッドの縦幅 2～1024
	BYTE	bgFileFlg;	//x 予定:BG表示要求 0:しない  1:する
	BYTE	bgGridMode; // BG色やグリッドを一括 0:off  1:on
	BYTE	gridFlg;	// グリッド線描画を 0:off  1:on
	BYTE	rngChk; 	//x (グリッド範囲を求める. 廃案かも)
	//char bgname[1024];	//x bg画像名. bmpのみ。		// bgnameは別の専用キーにする
	BYTE	mapOffSw;	// マップ化画像のマップを無視(内部テクスチャをそのまま表示)
	BYTE	dmy[3];
	DWORD	rsv[16-12]; //
} regopt_t;


typedef struct regopt_s_t {
	// regoptコントローラへのレポート. TSPIRO v0.50ではまだ未参照。また x付きは将来の予定
	SHORT	var;			// 上位WORD. 互換制を維持している間は固定の値. 下位 バージョン番号
	SHORT	size;			// この構造体のサイズ

	DWORD	time;			// timeGetTime() の値。更新チェック用
	SHORT	bpp;			// 元の BPP. spi呼び出し側にはすべて 24ビット色として渡されるため
	SHORT	clutNum;		// 元のclutの数(ただし最大 REGOPT_CLUT_NUM)
	LONG	w;				// 元画像の横幅
	LONG	h;				// 元画像の縦幅
	LONG	x0; 			// 元画像の始点x
	LONG	y0; 			// 元画像の始点y
	LONG	xresol; 		// 元画像にあった横ドット比(bmp式)
	LONG	yresol; 		// 元画像にあった縦ドット比(bmp式)
	short	clutKey;		// 元画像のclut時の抜き色番号+1 (0の時抜き色無し. 1以上の時、-1すれば抜き色番号)
	short	dmy0;			//x
	LONG	colorKey;		// 元画像のダイレクトカラーでの抜き色 (0の時抜き色無し. 1以上の時, -1すれば抜き色)
	//int clut[CLUT_NUM];	// レジストリに設定するclut // clutは別の専用キーにする
	DWORD	rsv2[16-9]; 	//
} regopt_s_t;


int RegOpt_Get(regopt_t *ro);
int RegOpt_GetPicture(regopt_t *ro, BYTE *pix, int w, int h, int dbpp, DWORD *clut0, int clutNum, const BYTE* src, int srcWidByt, int srcBpp);
int RegOpt_Put(regopt_s_t *rs, int w, int h, int bpp, const DWORD *clut, int clutNum, const char *fname);


#ifdef __cplusplus
}
#endif

#endif
