/**
 *	@file	mem_mac.h
 *	@brief	メモリアクセスのためのマクロ
 *	@author	北村雅史	NBB00541@nifty.com
 */

#ifndef MEM_MAC_H
#define MEM_MAC_H

// 基本型としてuint8_t,uint16_t,uint32_t,uint64_tが定義されていること
#include "def.h"


//-------------------------------------------------------------
// int8やint16の値をつなげたりばらしたりするマクロ
//-------------------------------------------------------------

/// 8bit数二つを上下につなげて16ビット数にする
#define BB(a,b) 		((((uint8_t)(a))<<8)|(uint8_t)(b))

/// 16bit数二つを上下につなげて32ビット数にする
#define WW(a,b) 		((((uint16_t)(a))<<16)|(uint16_t)(b))

/// 32bit数二つを上下につなげて64ビット数にする
#define LL(a,b)			((((uint64_t)(a))<<32)|(uint32_t)(b))

/// 8bit数4つを上位から順につなげて32ビット数にする
#define BBBB(a,b,c,d)	((((uint8_t)(a))<<24)|(((uint8_t)(b))<<16)|(((uint8_t)(c))<<8)|((uint8_t)(d)))

#ifdef BIG_ENDIAN
#define CC(a,b)			BB(a,b)			///< 2バイト文字列をint16として扱った場合の、定数比較用マクロ
#define CCCC(a,b,c,d)	BBBB(a,b,c,d)	///< 4バイト文字列をint32として扱った場合の、定数比較用マクロ
#else	// LITTLE_ENDIAN
#define CC(a,b)			BB(b,a)			///< 2バイト文字列をint16として扱った場合の、定数比較用マクロ
#define CCCC(a,b,c,d)	BBBB(d,c,b,a)	///< 4バイト文字列をint32として扱った場合の、定数比較用マクロ
#endif

#define GLB(a)			((uint8_t)(a))						///< aを int16型として下位バイトの値を取得
#define GHB(a)			((uint8_t)(((uint16_t)(a))>>8))		///< aを int16型として上位バイトの値を取得
#define GLLB(a) 		((uint8_t)(a))						///< aを int32型として最下位バイトの値を取得
#define GLHB(a) 		((uint8_t)(((uint16_t)(a))>>8))		///< aを int32型として下2バイト目の値を取得
#define GHLB(a) 		((uint8_t)(((uint32_t)(a))>>16))	///< aを int32型として下3バイト目の値を取得
#define GHHB(a) 		((uint8_t)(((uint32_t)(a))>>24))	///< aを int32型として下4バイト目の値を取得
#define GLW(a)			((uint16_t)(a))						///< int32型としての a の下16ビットの値を取得
#define GHW(a)			((uint16_t)(((uint32_t)(a))>>16))	///< int32型としての a の上16ビットの値を取得



//-------------------------------------------------------------
// アライメントを気にせず、メモリへアクセスするためのマクロ
//-------------------------------------------------------------

#define	PEEKB(a)		(*(const uint8_t *)(a))			///< アドレス a から 1バイト読み込む
#define	POKEB(a,b)		(*(uint8_t *)(a) = (b))			///< アドレス a に 1バイト書き込む

//	インテル/リトルエンディアン・データにアクセスする場合
#if defined _M_IX86	 || defined _X86_ 	// X86 は、アライメントを気にする必要がないので直接アクセス
#define	PEEKiW(a)		(*(const uint16_t *)(a))
#define	PEEKiD(a)		(*(const uint32_t *)(a))
#define PEEKiB3(s)		((*(const uint16_t*)(s)) | ((*(const uint8_t*)((s)+2))<<16))
#define	POKEiW(a,b)		(*(uint16_t *)(a) = (b))
#define	POKEiD(a,b)		(*(uint32_t *)(a) = (b))
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEiW(a,GLW(b)))
#else					//アライメント対策で、1バイトづつアクセス
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#define PEEKiB3(s)		BBBB(0, ((const uint8_t*)(s))[2], ((const uint8_t*)(s))[1], ((const uint8_t*)(s))[0])
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a), GLLB(b)))
#endif

//		モトローラ/ビッグエンディアン・データにアクセスする場合
#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define PEEKmB3(s)		BBBB(0, ((const uint8_t*)(s))[0], ((const uint8_t*)(s))[1], ((const uint8_t*)(s))[2])
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))
#define POKEmB3(a,b)	(POKEB((a)+0, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a)+2, GLLB(b)))

//		そのCPUの、デフォルトのエンディアンでアクセスする場合(バイト単位のアドレス可能)
#ifdef BIG_ENDIAN
#define	PEEKW(a)		PEEKmW(a)
#define	PEEKB3(a)		PEEKmB3(a)
#define	PEEKD(a)		PEEKmD(a)
#define	POKEW(a,b)		POKEmW(a,b)
#define	POKEB3(a,b)		POKEmB3(a,b)
#define	POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define	PEEKW(a)		PEEKiW(a)
#define	PEEKB3(a)		PEEKiB3(a)
#define	PEEKD(a)		PEEKiD(a)
#define	POKEW(a,b)		POKEiW(a,b)
#define	POKEB3(a,b)		POKEiB3(a,b)
#define	POKED(a,b)		POKEiD(a,b)
#endif


//-------------------------------------------------------------
// メモリ・コピーマクロ. アライメントは利用者側で気をつけること
//-------------------------------------------------------------

#define STREND(p)			((p)+ strlen(p))

/// ty型の0を アドレス dから sz/sizeof(ty)個書き込む
#define MEMCLR_TY(d, sz, ty) do {ty *d__ = (ty *)(d); unsigned c___ = (sz)/sizeof(ty); do {*d___++ = 0;} while(--c___);} while(0)
#define MEMCLR(d, sz)		MEMCLR_TY(d, s, sz, uint8_t)			///< dからのszバイトを0クリア.
#define MEMCLR2(d, sz)		MEMCLR_TY(d, s, sz, uint16_t)			///< dからのszバイトを0クリア. アドレス,サイズともに2の倍数のこと。
#define MEMCLR4(d, sz)		MEMCLR_TY(d, s, sz, uint32_t)			///< dからのszバイトを0クリア. アドレス,サイズともに4の倍数のこと。
#define MEMCLR8(d, sz)		MEMCLR_TY(d, s, sz, uint64_t)			///< dからのszバイトを0クリア. アドレス,サイズともに8の倍数のこと。
#define MEMCLR16(d, sz)		MEMCLR_TY(d, s, sz, __UINT128)			///< dからのszバイトを0クリア. アドレス,サイズともに16の倍数のこと。

//#ifndef ZeroMemory
//#define ZeroMemory(a,sz)	MEMCLR(a,sz)
//#endif

/// ty型のsを アドレス dから sz/sizeof(ty)個書き込む
#define MEMSET_TY(d, s, sz, ty) do {ty *d___ = (ty *)(d); ty s___  = (ty)(s);   unsigned c___ = (sz)/sizeof(ty); do {*d___++ = s___;} while(--c___);} while(0)
#define MEMSET(d, s, sz)	MEMSET_TY(d, s, sz, uint8_t)			///< d へ値sをszバイト書き込む.
#define MEMSET2(d, s, sz)	MEMSET_TY(d, s, sz, uint16_t)			///< d へ値sをsz/2個書き込む. アドレス,サイズともに2の倍数のこと。
#define MEMSET4(d, s, sz)	MEMSET_TY(d, s, sz, uint32_t)			///< d へ値sをsz/4個書き込む. アドレス,サイズともに4の倍数のこと。
#define MEMSET8(d, s, sz)	MEMSET_TY(d, s, sz, uint64_t)			///< d へ値sをsz/8個書き込む. アドレス,サイズともに8の倍数のこと。

/// ty型のポインタとしてsからdへ sz/sizeof(ty)個コピーする
#define MEMCPY_TY_TR(d, s, sz, ty, EXPR) do {ty *d___ = (ty *)(d); ty *s___ = (ty *)(s); unsigned c___ = ((unsigned)(sz)/sizeof(ty)); do {(EXPR);} while(--c___);} while(0)
#define MEMCPY_TY(d, s, sz, ty) MEMCPY_TY_TR(d,s,sz,ty, (*d___++ = *s___++))
#define MEMCPY(d, s, sz)	MEMCPY_TY(d, s, sz, int8_t)			///< s から d へ szバイトコピー.
#define MEMCPY2(d, s, sz)	MEMCPY_TY(d, s, sz, int16_t)			///< s から d へ szバイトコピー. アドレス,サイズともに2の倍数のこと。
#define MEMCPY4(d, s, sz)	MEMCPY_TY(d, s, sz, int32_t)			///< s から d へ szバイトコピー. アドレス,サイズともに4の倍数のこと。
#define MEMCPY8(d, s, sz)	MEMCPY_TY(d, s, sz, int64_t)			///< s から d へ szバイトコピー. アドレス,サイズともに8の倍数のこと。
#define MEMCPY16(d, s, sz)	MEMCPY_TY(d, s, sz, __INT128)		///< s から d へ szバイトコピー. アドレス,サイズともに16の倍数のこと。
///< s から d へ szバイトコピー. アドレス,サイズともに32の倍数のこと。
#define MEMCPY32(d, s, sz)	MEMCPY_TY_TR(d, s, sz, __INT128, (*d___++ = *s___++, *d___++ = *s___++))
///< s から d へ szバイトコピー. アドレス,サイズともに64の倍数のこと。
#define MEMCPY64(d, s, sz)	MEMCPY_TY_TR(d, s, sz, __INT128, (*d___++ = *s___++, *d___++ = *s___++, *d___++ = *s___++, *d___++ = *s___++))

/// ty型のポインタとしてsからdへ後ろから sz/sizeof(ty)個コピーする
#define MEMRCPY_TY(d,s, sz, ty) do {ty *d___ = (ty *)(d); ty *s___ = (ty *)(s); unsigned c___ = ((unsigned)(sz)/sizeof(ty)); while (c___-- > 0) {d___[c___] = s___[c___]; } } while (0)
#define MEMRCPY(d, s, sz)	MEMRCPY_TY(d, s, sz, int8_t)
#define MEMRCPY2(d, s, sz)	MEMRCPY_TY(d, s, sz, int16_t)
#define MEMRCPY4(d, s, sz)	MEMRCPY_TY(d, s, sz, int32_t)
#define MEMRCPY8(d, s, sz)	MEMRCPY_TY(d, s, sz, int64_t)
#define MEMRCPY16(d, s, sz)	MEMRCPY_TY(d, s, sz, __INT128)



//-------------------------------------------------------------
// 画像処理向けの、マクロ
//-------------------------------------------------------------
//	色値 argb は winのbmp/dibを基準に8ビットの a,r,g,b順
//  ※ bpp   1ピクセルあたりのビット数

/// bpp の バイト数に計算
#define BPP2BYT(bpp)		(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)

/// 横幅とbppよりバイト数を計算
#define WID2BYT(w,bpp)		(((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)

/// 横幅とbppよりバイト数を計算.このときbmp等向けに4の倍数にする
#define WID2BYT4(w,bpp) 	((WID2BYT(w,bpp) + 3) & ~3)

/// バイト数とbpp より横幅ピクセル数を求める
#define BYT2WID(w,bpp)		(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

/// 8bit値のa,r,g,bを繋げて 32bit の色値にする
#define ARGB(a,r,g,b)		((((uint8_t)(a))<<24)|(((uint8_t)(r))<<16)|(((uint8_t)(g))<<8)|((uint8_t)(b)))

/// argb値中の blueの値を取得
#define ARGB_B(argb)		((uint8_t)(argb))

/// argb値中の greenの値を取得
#define ARGB_G(argb)		((uint8_t)((argb)>>8))

/// argb値中の greenの値を取得
#define ARGB_R(argb)		((uint8_t)((argb)>>16))

/// argb値中の alphaの値を取得
#define ARGB_A(argb)		(((uint32_t)(argb))>>24)

/// argb値の各値同士を掛けて/256で割る
#define ARGBxARGB(x,y)		BBBB((GHHB(x)*GHHB(y))>>8, GHLB(x)*(GHLB(y))>>8, GLHB(x)*(GLHB(y))>>8, GLLB(x)*(GLLB(y))>>8)

/// argb値の並びをabgr 並びに変換する
#define ARGBtoABGR(c)		(((c) & 0xFF00FF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// a8r8g8b8だったものを a4r4g4b4 に変換
#define ARGB_8888to4444(c)	((((c)>>16)&0xF000)|(((c)>>12)&0x0F00)|(((c)>>8)&0xF0)|((uint8_t)(c)>>4))

/// a4r4g4b4だったものを a8r8g8b8 に変換
#define ARGB_4444to8888(c)	((((c)&0xF000)<<16)|(((c)&0x0F00)<<12)|(((c)&0xF0)<<8)|((uint8_t)((c)<<4)))

/// 8ビット値のa,r,g,bの各上位4ビットを繋げて16ビット整数に変換
#define ARGB4444(a,r,g,b)	(((((a)<<8) | (g)) & 0xF0F0) | (((((r)<<8)|(b)) & 0xF0F0) >> 4))
	//#define ARGB4444(a,r,g,b) ((((a)&0xF0)<<8) | (((r)&0xF0)<<4) | ((g)&0xF0) | (((b)>>4)&0xF))

/// a8r8g8b8 を a1r5g5b5 の16ビット値に変換
#define ARGB_8888to1555(c)	((((c)>>16)&0x8000)|(((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((uint8_t)((c)>>3)))

/// a1r5g5b5 を a8r8g8b8 に変換
#define ARGB_1555to8888(c)	((((c)&0x8000)<<16)|(((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((uint8_t)(c)<<3)))

/// 8ビット値のa,r,g,b の上位 1,5,5,5ビットを取得して 16ビット値に変換
#define ARGB1555(a,r,g,b)	(((a>>7)<<15)|(((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))

/// 8ビット値のr,g,b の上位 r5,g6,b5ビットを取得して 16ビット値に変換
#define RGB565(r,g,b)		((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|(((b)>>3)&0x1f))

//#define COLORPACK(r,g,b,a)	((((uint32_t)((a) * 255)) << 24) | (((uint32_t)((r) * 255)) << 16) | (((uint32_t)((g) * 255)) << 8) | (uint32_t)((b) * 255))

#define ARGB_FA(argb)		(((uint8_t)((uint32_t)(argb)>>24))* (1/255.f))	///< argb値より、alphaを 0～1.0FのFloat値として取得
#define ARGB_FR(argb)		(((uint8_t)((uint32_t)(argb)>>16))* (1/255.f))	///< argb値より、red  を 0～1.0FのFloat値として取得
#define ARGB_FG(argb)		(((uint8_t)((uint32_t)(argb)>>8)) * (1/255.f))	///< argb値より、greenを 0～1.0FのFloat値として取得
#define ARGB_FB(argb)		(((uint8_t)(argb))              * (1/255.f))	///< argb値より、blue を 0～1.0FのFloat値として取得

#define ARGB_ALP(a)			ARGB((a),0xFF,0xFF,0xFF)					///< αのみ掛け合わせる定数を生成
#define ARGB_DFLT			ARGB(0xFF,0xFF,0xFF,0xFF)					///< 色を掛け合わせるときの、デフォルト値
#define ARGB_WHITE			ARGB(0xFF,0xFF,0xFF,0xFF)					///< 白のARGB値
#define ARGB_BLACK			ARGB(0xFF,0x00,0x00,0x00)					///< 黒のARGB値

/// PS2のABGR値を(winの)通常のARGB値に変換
#define ARGB_FROM_PS2COL(c)	(((((uint32_t)(c)>>24)*255>>7)<<24)|((c)&0xFF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// (winの)通常のARGB値をPS2のABGRに変換
#define ARGB_TO_PS2COL(c)	(((((uint32_t)(c)>>24)*128/255)<<24)|((c)&0xFF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// ARGB並びをRGBA並びに変換
#define ARGB_TO_RGBA(c)		((uint8_t)((c) >> 24)|(uint32_t)((c)<<8))

/// RGBA並びをARGB並びに変換
#define ARGB_FROM_RGBA(c)	(((uint8_t)(c) << 24)|((uint32_t)(c)>>8))


#endif	// MEM_MAC_H
