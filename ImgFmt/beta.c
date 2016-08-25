/**
 *	@file	beta.c
 *	@brief	生バイナリの生成。パレット無し. 読み込み系は未対応
 *	@author	Masashi Kitamura
 */


#include "beta.h"
#include <stddef.h>


/*---------------------------------------------------------------------------*/
/* コンパイル環境の辻褄あわせ. */


#if (defined _MSC_VER) || (__BORLANDC__ <= 0x0551)
typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned		uint32_t;
typedef	short			int16_t;
#else
#include <stdint.h>
#endif


#if !defined(inline) && !(defined __GNUC__) && (__STDC_VERSION__ < 199901L)
#define	inline			__inline
#endif



/*---------------------------------------------------------------------------*/

//#define	BB(a,b)		((((unsigned char)(a))<<8)+(unsigned char)(b))
//#define	WW(a,b)		((((unsigned short)(a))<<16)+(unsigned short)(b))
#define	BBBB(a,b,c,d)	((((uint8_t)(a))<<24)+(((uint8_t)(b))<<16)+(((uint8_t)(c))<<8)+((uint8_t)(d)))

#define	GLB(a)			((unsigned char)(a))
#define	GHB(a)			GLB(((unsigned short)(a))>>8)
#define	GLW(a)			((unsigned short)(a))
#define	GHW(a)			GLW(((unsigned long)(a))>>16)

/* バイト単位のアドレスから値を取り出すメモリアクセス用マクロ */
#define	PEEKB(a)		(*(const unsigned char  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#if defined _M_IX86	 || defined _X86_ 	// X86 は、アライメントを気にする必要がないので直接アクセス
#define	PEEKiW(a)		(*(const unsigned short *)(a))
#define	PEEKiD(a)		(*(const unsigned long  *)(a))
#define	POKEiW(a,b)		(*(unsigned short *)(a) = (b))
#define	POKEiD(a,b)		(*(unsigned       *)(a) = (b))
#else
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#endif

#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))

// 画像用
#define WID2BYT(w,bpp)	(((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define	BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w) << 1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

#define	ARGB(a,r,g,b)	((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))




/*---------------------------------------------------------------------------*/

static uint32_t *beta_clut;



void beta_genMonoClut(uint32_t *clut, int bpp)
{
	int num = 1 << bpp;
	int i,c,spd;

	if (clut == NULL || num <= 0 || bpp <= 0 || bpp > 16)
		return;
	spd = 256*0x1000 / num;
	for (i = 0; i < num; i++) {
		c = (spd*i) >> 12;
		if (c > 255) c = 255;
		clut[i] = BBBB(0, c, c, c);
	}
}


void beta_genClut(uint32_t *clut, int dbpp)
{
	int i,c, r,g,b, n=0;
	if (clut == NULL)
		return;
	if (dbpp == 1) {						// 黒白
		clut[0] = 0;
		clut[1] = 0xFFFFFF;
	} else if (2 <= dbpp && dbpp <= 5) {	// 単色濃淡
		int m = (1<<dbpp) - 1;
		for (i = 0; i < (1<<dbpp); i++) {
			c = ((i&m) << (8-dbpp));
			c = c | (c >> dbpp) | (c >> (dbpp+dbpp)) | (c >> (dbpp+dbpp+dbpp));
			clut[i] = BBBB(0, c,c,c);
		}
	} else if (dbpp == 6) {
		for (i = 0; i < 4; ++i) {
			for (g = 0; g < 4; ++g) {
				for (r = 0; r < 4; ++r) {
					for (b = 0; b < 4; ++b) {
						int gg = (g << 6) | (g << 4) | (g << 2) | g;
						int rr = (r << 6) | (r << 4) | (r << 2) | r;
						int bb = (b << 6) | (b << 4) | (b << 2) | b;
						clut[n++] = BBBB(0, rr,gg,bb);
					}
				}
			}
		}
	} else if (dbpp == 7) {
		for (i = 0; i < 2; ++i) {
			for (g = 0; g < 8; ++g) {
				for (r = 0; r < 4; ++r) {
					for (b = 0; b < 4; ++b) {
						int gg = (g << 5) | (g << 2) | (g >> 1);
						int rr = (r << 6) | (r << 4) | (r << 2) | r;
						int bb = (b << 6) | (b << 4) | (b << 2) | b;
						clut[n++] = BBBB(0, rr,gg,bb);
					}
				}
			}
		}
	} else {				// TOWNSや88VA系の G3R3B2
		for (g = 0; g < 8; ++g) {
			for (r = 0; r < 8; ++r) {
				for (b = 0; b < 4; ++b) {
					int gg = (g << 5) | (g << 2) | (g >> 1);
					int rr = (r << 5) | (r << 2) | (r >> 1);
					int bb = (b << 6) | (b << 4) | (b << 2) | b;
					clut[n++] = BBBB(0, rr,gg,bb);
				}
			}
		}
	}
}


int	 beta_getHdr(const void *dmy_data, int *dmy_w_p, int *dmy_h_p, int *dmy_bpp_p, int *dmy_clutNum_p)
{
	return 0;
}


int  beta_getClut(const void *dmy_data, void *dmy_clut, int dmy_clutNum)
{
	return 0;
}


int  beta_read(const void *dmy_data, void *dmy_dst, int dmy_wb, int dmy_h, int dmy_dbpp, void *dmy_clut, int dmy_dir)
{
	return 0;
}


#if 1	// bpp変換コピー clut対応

static inline int beta_getPix32sub(int dbpp, int r, int g, int b)
{
	if (dbpp == 1) {
		return ((g|r|b) != 0);
	} else if (dbpp == 2) {
		return (g * 9 + r * 5 + b * 2) >> (4+8-2);
	} else if (dbpp == 3) {
		return ((g >= 0x80) << 2) | ((r >= 0x80) << 1) | (b >= 0x80);
	} else if (dbpp == 4) {
		return ((g >= 0x80) << 2) | ((r >= 0x80) << 1) | (b >= 0x80);
	} else if (dbpp == 5) {
		return (g * 9 + r * 5 + b * 2) >> (4+8-5);
	} else if (dbpp == 6) {
		return ((g>>2) & 0x30) | ((r >> 4) & 0x0C) | ((b >> 6) & 3);
	} else if (dbpp == 7) {
		return ((g>>1) & 0x70) | ((r >> 4) & 0x0C) | ((b >> 6) & 3);
	} else {
		return (g & 0xE0) | ((r >> 3) & 0x1C) | ((b >> 6) & 3);
	}
}


static inline int beta_getPix32(const uint8_t *s, int x, int bpp, int bo, int dbpp)
{
	uint8_t a,r,g,b;
	int c;
	if (bpp <= 1) {
		s += x >> 3;
		x = (x & 7);
		if (bo == 0)
			x ^= 7;		/* x = 7 - x; */
		c = (*s >> x) & 1;
		if (dbpp > 8)
			return beta_clut[c];
		return c;
	} else if (bpp <= 2) {
		s += x >> 2;
		x  = (x & 3) << 1;
		if (bo == 0)
			x ^= 6;		/* x = 6 - x; */
		c = (*s >> x) & 3;
		if (dbpp > 8)
			return beta_clut[c];
		return c;
	} else if (bpp <= 4) {
		s += x >> 1;
		x  = (x & 1) << 2;
		if (bo == 0)
			x ^= 4;		/* x = 4 - x; */
		c = (*s >> x) & 15;
		if (dbpp > 8)
			return beta_clut[c];
		return c;
	} else if (bpp <= 8) {
		if (dbpp > 8)
			return beta_clut[s[x]];
		return s[x];
	} else if (bpp <= 12) {
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		a = (c & 0xF000)>> 8;a |= a >> 4;
		r = (c & 0x0F00)>> 4;r |= r >> 4;
		g =  c & 0x00F0;     g |= g >> 4;
		b = (uint8_t)(c << 4); b |= b >> 4;
		if (dbpp > 8)
			return ARGB(a,r,g,b);
		return beta_getPix32sub(dbpp, r,g,b);
	} else if (bpp <= 15) {
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		a = (uint16_t)((int16_t)c >> 7) >> 8;
		r = (c & 0x7C00) >> 7;
		r |= r >> 5;
		g = (c & 0x03E0) >> 2;
		g |= g >> 5;
		b = (c & 0x001F) << 3;
		b |= b >> 5;
		if (dbpp > 8)
			return ARGB(a,r,g,b);
		return beta_getPix32sub(dbpp, r,g,b);
	} else if (bpp <= 16) {
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		r = (c & 0xF800) >> 8;
		r |= r >> 5;
		g = (c & 0x07E0) >> 3;
		r |= r >> 6;
		b = (c & 0x001F) << 3;
		b |= b >> 5;
		if (dbpp > 8)
			return ARGB(0xFF,r,g,b);
		return beta_getPix32sub(dbpp, r,g,b);
	} else if (bpp <= 24) {
		s += x * 3;
		if (dbpp > 8) {
			if (bo)	c = ARGB(0xFF,s[0],s[1],s[2]);	/* big */
			else	c = ARGB(0xFF,s[2],s[1],s[0]);	/* ltl */
			return c;
		}
		if (bo) r = s[0], g = s[1], b = s[2];
		else    r = s[2], g = s[1], b = s[0];
		return beta_getPix32sub(dbpp, r,g,b);
	} else {
		s += x * 4;
		if (dbpp > 8) {
			if (bo)	c = PEEKmD(s);					/* big */
			else	c = PEEKiD(s);					/* ltl */
			return c;
		}
		if (bo) r = s[0], g = s[1], b = s[2];
		else    r = s[2], g = s[1], b = s[0];
		return beta_getPix32sub(dbpp, r,g,b);
	}
}


static inline void beta_putPix32(uint8_t *d, int x, int c, int bpp, int bo)
{
	int   a;
	uint8_t r,g,b;

	if (bpp <= 1) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = ((r|g|b) != 0);
		d += x >> 3;
		x &= 7;
		if (bo == 0)
			x = 7-x;
		*d &= ~(1 << x);
		*d |= c << x;
		return;
	} else if (bpp <= 2) {
		//c = (g * 9 + r * 5 + b * 2) >> 10;
		d += x >> 2;
		x = (x&3) << 1;
		if (bo == 0)
			x = 6 - x;
		*d &= ~(3 << x);
		*d |= c << x;
		return;
	} else if (bpp <= 4) {
		//c = (g * 9 + r * 5 + b * 2) >> 8;
		if (bo == 0) {
			if (x & 1)	d[x>>1] |= c;
			else		d[x>>1] = (c << 4);
		} else {
			if (x & 1)	d[x>>1] |= (c << 4);
			else		d[x>>1] = c;
		}
		return;
	} else if (bpp <= 8) {
		//d[x] = (((g >> 5)<<5) | ((r >> 5)<<2) | (b >> 6));
		d[x] = c;
		return;
	} else if (bpp <= 12) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		a = ((uint32_t)c >> 24);
		a = (a + 15) >> 4; if (a > 15) a = 15;
		c = (a << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 15) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		a = ((uint32_t)c >> 24) ? 0x8000 : 0;
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 16) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 24) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		d += x*3;
		if (bo) {
			d[0] = r;
			d[1] = g;
			d[2] = b;
		} else {
			d[0] = b;
			d[1] = g;
			d[2] = r;
		}
		return;
	} else {
		d += x*4;
		if (bo)	POKEmD(d, c);
		else	POKEiD(d, c);
		return;
	}

}



#define beta_pixCpy(d,dbpp,s,sbpp,sw,y0,y1,yd,d_pat,s_pat,boI,boO) do {\
	int xx_,yy_;\
	for (yy_ = (y0); yy_ != (y1); yy_ += (yd)) {\
		for (xx_ = 0; xx_ < (sw); xx_++) {\
			int cc_ = beta_getPix32((s), xx_, (sbpp),(boI),(dbpp));\
			beta_putPix32((d), xx_, cc_, (dbpp),(boO));\
		}\
		(s) += (s_pat);\
		(d) += (d_pat);\
	}\
} while (0)



#else  	// clut 未対応

static inline int beta_getPix32(uint8_t *s, int x, int bpp, int bo)
{
	int c;
	if (bpp <= 1) {
		s += x >> 3;
		x = (x & 7);
		if (bo == 0)
			x ^= 7;		// x = 7 - x;
		c = (*s >> x) & 1;
		return beta_clut[c];
	} else if (bpp <= 2) {
		s += x >> 2;
		x  = (x & 3) << 1;
		if (bo == 0)
			x ^= 6;		// x = 6 - x;
		c = (*s >> x) & 3;
		return beta_clut[c];
	} else if (bpp <= 4) {
		s += x >> 1;
		x  = (x & 1) << 2;
		if (bo == 0)
			x ^= 4;		// x = 4 - x;
		c = (*s >> x) & 15;
		return beta_clut[c];
	} else if (bpp <= 8) {
		return beta_clut[s[x]];
	} else if (bpp <= 12) {
		uint8_t a,r,g,b;
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		a = (c & 0xF000)>> 8;a |= a >> 4;
		r = (c & 0x0F00)>> 4;r |= r >> 4;
		g =  c & 0x00F0;     g |= g >> 4;
		b = (uint8_t)(c << 4); b |= b >> 4;
		return ARGB(a,r,g,b);
	} else if (bpp <= 15) {
		uint8_t a,r,g,b;
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		a = (uint16_t)((int16_t)c >> 7) >> 8;
		r = (c & 0x7C00) >> 7;
		g = (c & 0x03E0) >> 2;
		b = (c & 0x001F) << 3;
		return ARGB(a,r,g,b);
	} else if (bpp <= 16) {
		uint8_t r,g,b;
		s += x * 2;
		if (bo) c = PEEKmW(s);
		else	c = PEEKiW(s);
		r = (c & 0xF800) >> 8;
		g = (c & 0x07E0) >> 3;
		b = (c & 0x001F) << 3;
		return ARGB(0,r,g,b);
	} else if (bpp <= 24) {
		s += x * 3;
		if (bo)	c = ARGB(0,s[0],s[1],s[2]);	// big
		else	c = ARGB(0,s[2],s[1],s[0]);	// ltl
		return c;
	} else {
		s += x * 4;
		if (bo)	c = PEEKmD(s);					// big
		else	c = PEEKiD(s);					// ltl
		return c;
	}
}



static inline void beta_putPix32(uint8_t *d, int x, int c, int bpp, int bo)
{
	int   a;
	uint8_t r,g,b;

	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	if (bpp <= 1) {
		d += x >> 3;
		c = ((r|g|b) != 0);
		x &= 7;
		if (bo == 0)
			x = 7-x;
		*d &= 1 << x;
		*d |= c << x;
		return;
	} else if (bpp <= 2) {
		c = (g * 9 + r * 5 + b * 2) >> 10;
		d += x >> 2;
		x = (x&3) << 1;
		if (bo == 0)
			x = 6 - x;
		*d &= 3 << x;
		*d |= c << x;
		return;
	} else if (bpp <= 4) {
		c = (g * 9 + r * 5 + b * 2) >> 8;
		if ((x & 1) ^ bo)	d[x>>1] |= c;
		else				d[x>>1] = (c << 4);
		return;
	} else if (bpp <= 8) {
		d[x] = (((g >> 5)<<5) | ((r >> 5)<<2) | (b >> 6));
		return;
	} else if (bpp <= 12) {
		a = ((uint32_t)c >> 24);
		a = (a + 15) >> 4; if (a > 15) a = 15;
		c = (a << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 15) {
		a = ((uint32_t)c >> 24) ? 0x8000 : 0;
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 16) {
		c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
		d += x*2;
		if (bo) POKEmW(d, c);
		else 	POKEiW(d, c);
		return;
	} else if (bpp <= 24) {
		d += x*3;
		if (bo) {
			d[0] = r;
			d[1] = g;
			d[2] = b;
		} else {
			d[0] = b;
			d[1] = g;
			d[2] = r;
		}
		return;
	} else {
		d += x*4;
		if (bo)	POKEmD(d, c);
		else	POKEiD(d, c);
		return;
	}

}



#define beta_pixCpy(d,dbpp,s,sbpp,sw,y0,y1,yd,d_pat,s_pat,boI,boO) do {\
	int xx_,yy_;\
	for (yy_ = (y0); yy_ != (y1); yy_ += (yd)) {\
		for (xx_ = 0; xx_ < (sw); xx_++) {\
			int cc_ = beta_getPix32((s), xx_, (sbpp),(boI));\
			beta_putPix32((d), xx_, cc_, (dbpp),(boO));\
		}\
		(s) += (s_pat);\
		(d) += (d_pat);\
	}\
} while (0)


#endif




int  beta_write(const void *beta_data, int w, int h, int dbpp, const void *src, int srcWb, int sbpp, void *clut, int dir)
{
	return beta_conv(beta_data, WID2BYT(w,dbpp), h, dbpp, src, srcWb, sbpp, clut, dir, 0, 0);
}


/** flags;bit 0:Y反転
 *	boI,boO : 入力&出力バイトオーダー
 *	 		0:リトルエンディアン(インテル) 1:ビッグエンディアン(モトローラ)
 *			1バイトのピクセルのときは,バイト内の詰め順.
 *		 	0:上詰め(bmpに同じ)		1:下詰め(towns-tiff/tim系)
 */
int  beta_conv(const void *beta_data, int dstWb, int h, int dbpp, const void *src, int srcWb, int sbpp, const void *clut, int flags, int boI, int boO)
{
	uint8_t *d  = (uint8_t*)beta_data;
	uint8_t *s  = (uint8_t*)src;
	int   dw  = BYT2WID(dstWb, dbpp);
	int			y0,y1,yd,sw, d_pat, s_pat;

	beta_clut = (uint32_t*)clut;
	if (beta_clut == NULL) {
		static uint32_t clut0[256];
		beta_clut = clut0;
		beta_genClut(beta_clut, dbpp);
	}
	srcWb = (srcWb) ? srcWb : WID2BYT(dw, sbpp);
	sw    = BYT2WID(srcWb, sbpp);
	if (dw > sw)
		dw = sw;
	if (dw < sw)
		sw = dw;
	s_pat = srcWb /*- WID2BYT(sw,sbpp)*/;
	d_pat = dstWb /*- WID2BYT(dw,dbpp)*/;
	y0   = 0, y1 = h, yd = +1;
	if (flags & 1) {	// y反転
		y0 = h-1, y1 = -1, yd = -1;
		d     = d + y0 * dstWb;
		d_pat = d_pat - dstWb * 2;
	}
	beta_pixCpy(d,dbpp, s, sbpp, sw,y0,y1,yd, d_pat, s_pat, boI,boO);
	return dstWb * h;
}

