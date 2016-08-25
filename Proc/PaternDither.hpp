/**
 *	@file	PaternDither.h
 *	@brief	パターンディザを施す
 *	@author	Masashi KITAMURA
 */

#pragma once

#include <stddef.h>
#include <string.h>
#include <memory.h>


class PaternDither {
public:
	/** 画像にパターン・ディザを施す。パターンはBayerのみ.
	 * @param flgs	フラグ
	 *	 		- bit1,0: 0=ディザ無 1=2x2 2=4x4 3=8x8
	 *	 		- bit7:   1:AGとRBとでマトリクスを逆にする 0:しない
	 *	 		- bit8:   1:αプレーンもディザする 0:しない
	 */
	void conv(
			unsigned*		dst,		///< 出力バッファ
			const unsigned*	src,		///< 入力バッファ
			unsigned		w,			///< 横幅
			unsigned		h,			///< 縦幅
			int 			ditBpp,		///< bpp ディザをかけた後の色のビット数.
			int 			flgs)
	{
		enum { R  = 0, G  = 1, B  = 2, A  = 3, };

		ditBpp = clamp(ditBpp, 3, 18/* 3*6 */);

		//r_bit,g_bit,b_bit は有効ビット数. 1～6. G,R,Bの順に多く割り当てる
		int b_bit = ditBpp / 3;
		int g_bit = b_bit;
		int r_bit = g_bit;
		if ((ditBpp % 3) >= 1)
			++g_bit;
		if ((ditBpp % 3) == 2)
			++r_bit;

		// パターンディザを施すためのテーブルを作成
		int revf = (flgs >> 7) & 1;
		int typ  = flgs & 3;
		int af   = (flgs>>8)&1;
		//x unsigned char 	(*dt)[8][8][256] = new unsigned char[4][8][8][256];
		unsigned char 	dt[4][8][8][256];
		memset(dt, 0, (sizeof *dt) * 4);
		makePtnDitherTable(dt[R], r_bit, typ, 0);
		makePtnDitherTable(dt[G], g_bit, typ, revf);
		makePtnDitherTable(dt[B], b_bit, typ, 0);
		if (af)
			makePtnDitherTable(dt[A], g_bit, typ, revf);

		for (unsigned y = 0; y < h; ++y) {
			for (unsigned x = 0; x < w; ++x) {
				unsigned c = src[y*w+x];
				unsigned u = x & 7;
				unsigned v = y & 7;
				unsigned a = argb_a(c);	if (af)	 a = dt[A][v][u][a];
				unsigned r = argb_r(c); r = dt[R][v][u][r];
				unsigned g = argb_g(c); g = dt[G][v][u][g];
				unsigned b = argb_b(c); b = dt[B][v][u][b];
				dst[y*w+x] = argb(a,r,g,b);
			}
		}
		//x delete[] dt;
	}

private:
	/// パターンテーブルの作成.
	/// typ  : 0=ディザ無し 1=2x2  2=4x4  3=8x8
	/// mode : ptnディザ値の計算 0=標準 1:正のみ 2:ハーフ 3:ハーフ加算のみ
	void makePtnDitherTable(unsigned char ct[8][8][256], int c_bit, int typ, int revf) {
		static const signed char dmPtn[4][8][8] = {
			//パターンディザ無し
			{
				{0}
			}, {	//Bayer 2x2
				{  -96,  32, -96,  32, -96,  32, -96,  32,},
				{   96, -32,  96, -32,  96, -32,  96, -32,},
				{  -96,  32, -96,  32, -96,  32, -96,  32,},
				{   96, -32,  96, -32,  96, -32,  96, -32,},
				{  -96,  32, -96,  32, -96,  32, -96,  32,},
				{   96, -32,  96, -32,  96, -32,  96, -32,},
				{  -96,  32, -96,  32, -96,  32, -96,  32,},
				{   96, -32,  96, -32,  96, -32,  96, -32,},
			}, {	//Bayer 4x4
				{ -120,   8, -88,  40,-120,   8, -88,  40,},
				{   72, -56, 104, -24,  72, -56, 104, -24,},
				{  -72,  56,-104,  24, -72,  56,-104,  24,},
				{  120,  -8,  88, -40, 120,  -8,  88, -40,},
				{ -120,   8, -88,  40,-120,   8, -88,  40,},
				{   72, -56, 104, -24,  72, -56, 104, -24,},
				{  -72,  56,-104,  24, -72,  56,-104,  24,},
				{  120,  -8,  88, -40, 120,  -8,  88, -40,},
			}, {	//Bayer 8x8
				{ -126,   2, -94,  34,-118,  10, -86,  42,},
				{   66, -62,  98, -30,  74, -54, 106, -22,},
				{  -78,  50,-110,  18, -70,  58,-102,  26,},
				{  114, -14,  82, -46, 122,  -6,  90, -38,},
				{ -114,  14, -82,  46,-122,   6, -90,  38,},
				{   78, -50, 110, -18,  70, -58, 102, -26,},
				{  -66,  62, -98,  30, -74,  54,-106,  18,},
				{  126,  -2,  94, -34, 118, -10,  86, -42,},
			},
		};

		// まず、指定ビット数の精度の値のテーブルを作る
		unsigned char	 ct0[256];
		static const int msk0[] = {0x80,0xC0,0xE0,0xF0, 0xF8,0xFC,0xFE,0xFF};
		int				 msk	= msk0[c_bit-1];
		for (unsigned i = 0; i < 256; ++i) {
			unsigned  k = (i & msk);
			if (c_bit > 3)			k = k | (k >> c_bit);
			else if (c_bit == 3)	k = k | (k >> 3) | (k >> (2*3));
			else if (c_bit == 2)	k = k | (k >> 2) | (k >> (2*2)) | (k >> (3*2));
			else					k = k ? 0xff : 0x00;
			ct0[i] = k;
			//x DBG_PRINTF(("%02x", k)); if (i % 16 == 15) DBG_PRINTF(("\n")); else DBG_PRINTF((" "));
		}

		// パターンディザを反映したテーブル
		const signed char (*pPtn)[8] = dmPtn[typ];
		for (int y = 0; y < 8; y++) {
			for (int x 	= 0; x < 8; x++) {
				int yy 	= (revf == 0) ? y : 7 - y;
				int dm 	= pPtn[yy][x];
				dm 		= dm >> c_bit;
				for (int i = 0; i < 256; i++) {
					int c 		= i+dm;
					c     		= clamp(c, 0, 0xff);
					ct[y][x][i] = ct0[c];
				}
			}
		}
	}

	static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
		return (a << 24) | (r << 16) | (g << 8) | (b);
	}

	static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
	static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
	static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
	static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

	static int clamp(int val, int mi, int ma) {
		if (val < mi)
			return mi;
		else if (ma < val)
			return ma;
		return val;
	}
};

