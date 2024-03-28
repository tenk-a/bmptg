/**
 *	@file	DecreaseColorT1.h
 *	@brief	減色処理. 頻度カウントして多いもの順.
 *	@author	Masashi KITAMURA
 */

#ifndef DECREASECOLORT1_H
#define DECREASECOLORT1_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <functional>
#if defined _WIN32 == 0
#include <stdint.h>
#endif



/// 減色処理. 32ビット色画を8ビット色画に変換.
template<class A=void>
class DecreaseColor {
public:
	DecreaseColor() {; }
	DecreaseColor(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize=256) {
		conv(pSrc,w,h,pDst,clut,clutSize);
	}

	static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize=256);

private:
	static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
		return (a << 24) | (r << 16) | (g << 8) | (b);
	}

	static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
	static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
	static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
	static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

private:
	struct Hst {
	  #if defined _WIN32				// vcにstdint.hがないのでその対策.
		typedef unsigned __int64 sum_t;	// win系コンパイラはvc互換で古くから__int64がある.
	  #else
		typedef uint64_t		 sum_t;
	  #endif
		sum_t			g;
		sum_t			r;
		sum_t			b;
		std::size_t 	num;
		short			no;
		// short		a;

		bool operator<(const Hst& r) const {
			return this->num > r.num;
		}
	};
};



/// 32ビット色画を8ビット色画に変換.
template<class A>
bool DecreaseColor<A>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize)
{
	assert(clutSize <= 256);
	enum { BI=4, N=1<<BI, SH=8-BI };
	enum { NUM = 1 + N*N*N };
	assert( NUM >= 256 );
	Hst* pHst = new Hst[ NUM ];
	std::memset(pHst, 0, sizeof(Hst) * NUM);
	for (unsigned i = 0; i < w * h; ++i) {
		unsigned c = pSrc[i];
		unsigned g = argb_g(c);
		unsigned r = argb_r(c);
		unsigned b = argb_b(c);
		unsigned no;
		no  = 1 + (g>>SH) * N*N + (r>>SH) * N + (b>>SH);
		Hst* h = &pHst[no];
		++h->num;
		h->no = no;
		h->g += g;
		h->r += r;
		h->b += b;
	}

	// 多いモノからclutSize個を選出(先頭に移動).
	std::nth_element( &pHst[0], &pHst[clutSize], &pHst[NUM], std::less<Hst>() );

	// 多いものから色を取得.
	std::memset(pClut, 0, clutSize*sizeof(pClut[0]));
	unsigned n = 0;
	for (unsigned j = 0; j < clutSize; ++j) {
		const Hst*	h   = &pHst[j];
		std::size_t num = h->num;
		if (num == 0)
			continue;	// break;
		unsigned g	  = unsigned(h->g / num);
		unsigned r	  = unsigned(h->r / num);
		unsigned b	  = unsigned(h->b / num);
		pClut[n] = argb(0xff,r,g,b);
		++n;
	}
	// 実際に使用されているclut数を求める.
	if (n < 2)	// ただし最低2色は使うことに.
		n = 2;
	clutSize = n;

	// clutを暗いもの順に並べ直す.
	std::sort( &pClut[0], &pClut[clutSize] );

	// 現状のclutで、すべてのピクセルをclutSize色化.
	// 色選択の条件として、
	//   - a,r,g,bの値が近いモノ(色同士の差が小さいモノ)
	//   - 1色内のr,g,bの輝度の大小関係が、近いモノ
	// をチェック.
	std::memset(pHst, 0, sizeof(Hst) * clutSize);
	for (unsigned j = 0; j < w * h; ++j) {
		unsigned c	= pSrc[j];
		int		 g  = argb_g(c);
		int		 r  = argb_r(c);
		int		 b  = argb_b(c);
		int		 o  = g - r;
		int		 p  = r - b;
		int		 q  = b - g;

		unsigned ii = 0;
		unsigned k  = 0xFFFFFFFF;
		for (unsigned i = 0; i < clutSize; ++i) {
			unsigned ic	= pClut[i];
			int			ig = argb_g(ic);
			int			ir = argb_r(ic);
			int			ib = argb_b(ic);
			int			io = ig - ir;
			int			ip = ir - ib;
			int			iq = ib - ig;

			int 		gg = ig - g;
			int 		rr = ir - r;
			int 		bb = ib - b;
			int 		oo = io - o;
			int 		pp = ip - p;
			int 		qq = iq - q;
			unsigned 	ik = gg*gg + rr*rr + bb*bb + oo*oo + pp*pp + qq*qq;
			if (ik < k) {
				k  = ik;
				ii = i;
			}
		}

		pDst[j] = ii;

		// 使用数や色値の合計.
		Hst* t = &pHst[ ii ];
		++t->num;
		t->g += g;
		t->r += r;
		t->b += b;
	}

	// clutの各色を実際のピクセルの色の平均にする.
	for (unsigned i = 0; i < clutSize; ++i) {
		const Hst*	t	 = &pHst[i];
		unsigned    n    = t->num;
		unsigned	g=0, r=0, b=0;
		if (n) {
			g = unsigned(t->g / n);
			r = unsigned(t->r / n);
			b = unsigned(t->b / n);
		}
		pClut[i] = argb(0xff,r,g,b);
	}

	delete pHst;
	return true;
}


#endif
