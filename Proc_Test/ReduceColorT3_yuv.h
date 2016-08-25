/**
 *	@file	ReduceColorHist.h
 *	@brief	減色処理. rgb を yuv にして頻度カウントして多いもの順.
 *	@author	Masashi KITAMURA
 */

#ifndef REDUCECOLORT3_H
#define REDUCECOLORT3_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <algorithm>


/// 減色処理.(一旦 rgb を yuv にし、yuv[45][19][19]の頻度をカウントして、多いものを選ぶ)
template<class A=void>
class ReduceColorT3 {
public:
	ReduceColorT3() : minA_(-1) {; }
	~ReduceColorT3(){;}

	bool conv(const unsigned* pSrc, unsigned w, unsigned h, unsigned char* pDst, unsigned* clut, unsigned clutSize);
	void setMinAlpha(int a) { assert(0 <= a && a <= 255); minA_ = a; }

private:
	struct Yuv {
		short			v;
		short			u;
		short			y;
		short			a;

		bool operator<(const Yuv& r) const {
			if (this->a != r.a)
				return this->a < r.a;
			if (this->y != r.y)
				return this->y < r.y;
			if (this->u != r.u)
				return this->u < r.u;
			return this->v < r.v;
		}
	};

	struct Hst {
		double			y;
		double			u;
		double			v;
		std::size_t 	num;
		short			no;
		short			a;

		bool operator<(const Hst& r) const {
			if (this->a < r.a)
				return true;
			if (this->num != r.num)
				return this->num > r.num;
			return this->y > r.y;
		}
	};

private:
	void yuv_to_rgb(const Yuv* yuv, std::size_t size, unsigned* argb);
	void rgb_to_yuv(const unsigned* pRgb, std::size_t size, Yuv* yuv);
	bool choseColor(const Yuv* yuv, unsigned w, unsigned h, Yuv* clutWk, unsigned clutSize);
	bool convPix32to8(const Yuv *pSrc, int w, int h, unsigned char *pDst, Yuv *pClut, unsigned clutSize);

	static int clamp(int val, int mi, int ma) {
		if (val < mi)
			return mi;
		else if (ma < val)
			return ma;
		return val;
	}

private:
	int		minA_;
};



/// 32ビット色画を8ビット色画に変換.
template<class A>
bool ReduceColorT3<A>::conv(const unsigned* pSrc, unsigned w, unsigned h, unsigned char* pDst, unsigned* pClut, unsigned clutSize)
{
	Yuv		aClutYuv[256];
	Yuv*	pYuv   = new Yuv[ w * h ];
	std::memset(aClutYuv, 0, sizeof(aClutYuv));
	rgb_to_yuv(pSrc, w * h, pYuv);
	choseColor(pYuv, w, h, aClutYuv, clutSize);
	convPix32to8(pYuv, w, h, pDst, aClutYuv, clutSize);
	yuv_to_rgb(aClutYuv, clutSize, pClut);
	delete pYuv;
	return true;
}



/// ピクセルrgbをyuvに変換.
template<class A>
void ReduceColorT3<A>::rgb_to_yuv(const unsigned* pRgb, std::size_t size, typename ReduceColorT3<A>::Yuv* yuv)
{
	for (unsigned i = 0; i < size; ++i) {
		unsigned  c		= pRgb[i];
		int 	a		= (c >> 24);
		int 	g		= (unsigned char)(c >>  8);
		int 	r		= (unsigned char)(c >> 16);
		int	 	b		= (unsigned char)(c);
		if (a <= minA_)
			a = r = g = b = 0;
		int y,u,v;
		yuv[i].a    = a;
		yuv[i].y 	= y = ( 58661*g +  29891*r + 11448*b) * (2048>>5) / ( 255 * (100000>>5) );
		yuv[i].u 	= u = (-33126*g -  16874*r + 50000*b) * (1024>>5) / ( 255 * (100000>>5) ) + 512;
		yuv[i].v 	= v = (-41869*g +  50000*r -  8131*b) * (1024>>5) / ( 255 * (100000>>5) ) + 512;
		//x printf("%3d %3d,%3d,%3d  %2x,%2x,%2x\n", i, y,u,v, r,g,b);
		assert(0 <= y && y <= 2*1024);
		assert(0 <= u && u <= 1024);
		assert(0 <= v && v <= 1024);
	}
}



template<class A>
void ReduceColorT3<A>::yuv_to_rgb(const typename ReduceColorT3<A>::Yuv* yuv, std::size_t size, unsigned* argb)
{
	for (unsigned i = 0; i < size; ++i) {
		const Yuv* s = &yuv[i];
		int y = s->y >> 1;
		int u = (int)s->u - 512;
		int v = (int)s->v - 512;
		int g = ((100000*y -  34414*u -  71414*v)>>(10+5)) * 255 / (100000>>5);
		int r = ((100000*y            + 140200*v)>>(10+5)) * 255 / (100000>>5);
		int b = ((100000*y + 177200*u           )>>(10+5)) * 255 / (100000>>5);
		r = clamp(r, 0, 255);
		g = clamp(g, 0, 255);
		b = clamp(b, 0, 255);
		if (s->a == 0)
			r = g = b = 0;
		assert(0 <= r && r <= 255);
		assert(0 <= g && g <= 255);
		assert(0 <= b && b <= 255);
		argb[i] = (s->a << 24) | (r << 16) | (g << 8) | (b);
	}
}



template<class A>
bool ReduceColorT3<A>::choseColor(const struct ReduceColorT3::Yuv* pYuv, unsigned w, unsigned h, struct ReduceColorT3::Yuv* pClutYuv, unsigned clutSize)
{
	//enum { YN=14, UN= 7, VN= 7 };
	enum { YN=10, UN= 5, VN= 5 };
	//enum { YN= 7, UN= 3, VN= 3 };
	//enum { YN=11, UN= 5, VN= 5 };
	//enum { YN=25, UN=13, VN=13 };
	//enum { YN=35, UN=15, VN=15 };
	//enum { YN=45, UN=19, VN=19 };
	//enum { YN=51, UN=25, VN=25 };
	enum   { NUM = 1 + YN*UN*VN };
	Hst* pHst = new Hst[ NUM ];
	std::memset(pHst, 0, sizeof(Hst) * NUM);
	int top = 0;
	for (unsigned i = 0; i < w * h; ++i) {
		const Yuv* p = &pYuv[i];
		unsigned   no;
		if (p->a <= minA_) {
			no  = 0;
			top = 1;
		} else {
		  #if 0
			unsigned y = (p->y * (YN-1) + 2*1024/(YN*2)) >> 11;
			unsigned u = (p->u * (UN-1) + 1024/(UN*2)) >> 10;
			unsigned v = (p->v * (VN-1) + 1024/(VN*2)) >> 10;
		  #else
			unsigned y = (p->y * (YN-1)) >> 11;
			unsigned u = (p->u * (UN-1)) >> 10;
			unsigned v = (p->v * (VN-1)) >> 10;
		  #endif
			if (y > YN-1) y = YN-1;
			if (u > UN-1) u = UN-1;
			if (v > VN-1) v = VN-1;
			no = 1 + y * UN*VN + u * VN + v;
		}
		Hst* h = &pHst[no];
		++h->num;
		h->no = no;
		h->a  = p->a;
		h->y += p->y;
		h->u += p->u;
		h->v += p->v;
	}

	// 多い順に並べる.
	std::sort( &pHst[top], &pHst[NUM] );

	// 多いものから色を取得.
	for (unsigned j = 0; j < clutSize; ++j) {
		const Hst*	h = &pHst[j];
		std::size_t num = h->num;
		if (num == 0)
			continue;	// break;
		Yuv*	t = &pClutYuv[j];
		t->a	  = h->a;
	  #if 0
		// とりあえずのyuvを設定.(あとで計算し直す).
		unsigned no = h->no;
		if (no > 0) {
			--no;
			unsigned y = no / (UN*VN);
			unsigned uv= no % (UN*VN);
			unsigned u = uv / VN;
			unsigned v = uv % VN;
			t->y	   = ((y * 2*1024) / (YN-1));
			t->u	   = ((u * 1024) / (UN-1));
			t->v	   = ((v * 1024) / (VN-1));
		}
	  #else
		t->y	  = h->y / num;
		t->u	  = h->u / num;
		t->v	  = h->v / num;
	  #endif
	}

	// 暗いもの順に並べ直す.
	std::sort( &pClutYuv[top], &pClutYuv[clutSize] );

	delete pHst;
	return true;
}



/** 32ビット色画を clut に従って近い色に置換.
 *	αは、抜き色１色のみ考慮.
 */ 
template<class A>
bool ReduceColorT3<A>::convPix32to8(const typename ReduceColorT3<A>::Yuv *pSrc, int w, int h
		, unsigned char *pDst, typename ReduceColorT3<A>::Yuv *pClut, unsigned clutSize)
{
	// 現状のclutで、すべてのピクセルをclutSize色化.
	for (unsigned n = 0; n < w * h; ++n) {
		unsigned ii = 0;
		const Yuv*	s	= &pSrc[n];
		int		 	a	= s->a;
		if (a > minA_) {	// 透明以外のピクセルだったとき.
			unsigned k = 0xFFFFFFFF;
			for (unsigned i = 0; i < clutSize; ++i) {
				const Yuv*	t    = &pClut[i];
				if (t->a > minA_) {
					int 		y = t->y - s->y;
					int 		u = t->u - s->u;
					int 		v = t->v - s->v;
					unsigned 	ik = /* a*a+ */ y*y + u*u + v*v;
					if (ik < k) {
						k  = ik;
						ii = i;
					}
				}
			}
		}
		pDst[n] = ii;
	}

 #if 0
	// clutの各色を実際のピクセルの色の平均にする.
	Hst*	pHst = new Hst[ clutSize ];
	std::memset(pHst, 0, sizeof(Hst) * clutSize);
	for (unsigned j = 0; j < w * h; ++j) {
		Hst*		t = &pHst[ pDst[j] ];
		const Yuv*	s = &pSrc[j];
		++t->num;
		t->y += s->y;
		t->u += s->u;
		t->v += s->v;
	}
	for (unsigned i = 0; i < clutSize; ++i) {
		const Hst*	t	 = &pHst[i];
		Yuv*		p    = &pClut[i];
		unsigned    n    = t->num;
		if (n) {
			p->y = t->y / n;
			p->u = t->u / n;
			p->v = t->v / n;
		}
	}
	delete pHst;
  #endif

	return 1;
}


#endif
