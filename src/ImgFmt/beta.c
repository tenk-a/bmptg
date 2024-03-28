/**
 *	@file	beta.c
 *	@brief	生バイナリの生成。パレット無し. 読み込み系は未対応
 *	@author	Masashi Kitamura
 */


#include "beta.h"
#include "def.h"
#include <stddef.h>
#include <assert.h>


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

//#define	BB(a,b)		((((unsigned char)(a))<<8)|(unsigned char)(b))
//#define	WW(a,b)		((((unsigned short)(a))<<16)|(unsigned short)(b))
#define	BBBB(a,b,c,d)	((((uint8_t)(a))<<24)|(((uint8_t)(b))<<16)|(((uint8_t)(c))<<8)|((uint8_t)(d)))

#define	GLB(a)			((unsigned char)(a))
#define	GHB(a)			GLB(((unsigned short)(a))>>8)
#define	GLW(a)			((unsigned short)(a))
#define	GHW(a)			GLW(((unsigned long)(a))>>16)

/* バイト単位のアドレスから値を取り出すメモリアクセス用マクロ */
#define	PEEKB(a)		(*(const unsigned char  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#if defined _M_IX86 || defined _X86_ || defined _M_AMD64 || defined __amd64__	// X86 は、アライメントを気にする必要がないので直接アクセス
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

static uint32_t *beta_s_clut;



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


// ----------------------------------------------------------------------------

enum {
	BPPIDX_01,
	BPPIDX_02,
	//BPPIDX_03,
	BPPIDX_04,
	BPPIDX_08,
	BPPIDX_12,
	BPPIDX_13,	// gray:8 + alpha:8
	BPPIDX_15,
	BPPIDX_16,
	BPPIDX_24,
	BPPIDX_32,
	BPPIDX_NUM,
};

static inline int beta_bpp2idx(int bpp) {
	return  (bpp <=  1) ?  BPPIDX_01
	      : (bpp <=  2) ?  BPPIDX_02
	      //: (bpp <=  3) ?  BPPIDX_03
	      : (bpp <=  4) ?  BPPIDX_04
	      : (bpp <=  8) ?  BPPIDX_08
	      : (bpp <= 12) ?  BPPIDX_12
	      : (bpp <= 13) ?  BPPIDX_13
	      : (bpp <= 15) ?  BPPIDX_15
	      : (bpp <= 16) ?  BPPIDX_16
	      : (bpp <= 24) ?  BPPIDX_24
	      :                BPPIDX_32;
}


// ------------------------------------

static uint32_t beta_rgb2idx_bpp01(int r, int g, int b) {
	return ((g|r|b) != 0);
}

static uint32_t beta_rgb2idx_bpp02(int r, int g, int b) {
	return (g * 9 + r * 5 + b * 2) >> (4+8-2);
}

static uint32_t beta_rgb2idx_bpp04(int r, int g, int b) {
	return ((g >= 0x80) << 2) | ((r >= 0x80) << 1) | (b >= 0x80);
}

static uint32_t beta_rgb2idx_bpp08(int r, int g, int b) {
	return (g & 0xE0) | ((r >> 3) & 0x1C) | ((b >> 6) & 3);
}


typedef uint32_t (*beta_getIdx_sub_t)(int r, int g, int b);
static beta_getIdx_sub_t const s_beta_getIdx_sub_tbl[] = {
	beta_rgb2idx_bpp01,
	beta_rgb2idx_bpp02,
	beta_rgb2idx_bpp04,
	beta_rgb2idx_bpp08,
};

beta_getIdx_sub_t	beta_getIdx_sub	= beta_rgb2idx_bpp08;

static void beta_set_rgb2idx(int bpp) {
	int     idx = beta_bpp2idx(bpp);
	assert(idx <= BPPIDX_08);
	beta_getIdx_sub = s_beta_getIdx_sub_tbl[idx];

}


// ------------------------------------

static uint32_t beta_getRgb_bpp01m(const uint8_t *s, int x) {
	int c;
	s += x >> 3;
	x = (x & 7);
	//if (bo == 0)
	//	x ^= 7;		/* x = 7 - x; */
	c = (*s >> x) & 1;
	return beta_s_clut[c];
}

static uint32_t beta_getRgb_bpp01i(const uint8_t *s, int x) {
	int c;
	s += x >> 3;
	x = (x & 7);
	//if (bo == 0)
		x ^= 7;		/* x = 7 - x; */
	c = (*s >> x) & 1;
	return beta_s_clut[c];
}

static uint32_t beta_getIdx_bpp01m(const uint8_t *s, int x) {
	int c;
	s += x >> 3;
	x = (x & 7);
	//if (bo == 0)
	//	x ^= 7;		/* x = 7 - x; */
	c = (*s >> x) & 1;
	return c;
}

static uint32_t beta_getIdx_bpp01i(const uint8_t *s, int x) {
	int c;
	s += x >> 3;
	x = (x & 7);
	//if (bo == 0)
		x ^= 7;		/* x = 7 - x; */
	c = (*s >> x) & 1;
	return c;
}

static uint32_t beta_getRgb_bpp02m(const uint8_t *s, int x) {
	int c;
	s += x >> 2;
	x  = (x & 3) << 1;
	//if (bo == 0)
	//	x ^= 6;		/* x = 6 - x; */
	c = (*s >> x) & 3;
	return beta_s_clut[c];
}

static uint32_t beta_getRgb_bpp02i(const uint8_t *s, int x) {
	int c;
	s += x >> 2;
	x  = (x & 3) << 1;
	//if (bo == 0)
		x ^= 6;		/* x = 6 - x; */
	c = (*s >> x) & 3;
	return beta_s_clut[c];
}

static uint32_t beta_getIdx_bpp02m(const uint8_t *s, int x) {
	int c;
	s += x >> 2;
	x  = (x & 3) << 1;
	//if (bo == 0)
	//	x ^= 6;		/* x = 6 - x; */
	c = (*s >> x) & 3;
	return c;
}

static uint32_t beta_getIdx_bpp02i(const uint8_t *s, int x) {
	int c;
	s += x >> 2;
	x  = (x & 3) << 1;
	//if (bo == 0)
		x ^= 6;		/* x = 6 - x; */
	c = (*s >> x) & 3;
	return c;
}

static uint32_t beta_getRgb_bpp04m(const uint8_t *s, int x) {
	int c;
	s += x >> 1;
	x  = (x & 1) << 2;
	//if (bo == 0)
	//	x ^= 4;		/* x = 4 - x; */
	c = (*s >> x) & 15;
	return beta_s_clut[c];
}

static uint32_t beta_getRgb_bpp04i(const uint8_t *s, int x) {
	int c;
	s += x >> 1;
	x  = (x & 1) << 2;
	//if (bo == 0)
		x ^= 4;		/* x = 4 - x; */
	c = (*s >> x) & 15;
	return beta_s_clut[c];
}

static uint32_t beta_getIdx_bpp04m(const uint8_t *s, int x) {
	int c;
	s += x >> 1;
	x  = (x & 1) << 2;
	//if (bo == 0)
	//	x ^= 4;		/* x = 4 - x; */
	c = (*s >> x) & 15;
	return c;
}

static uint32_t beta_getIdx_bpp04i(const uint8_t *s, int x) {
	int c;
	s += x >> 1;
	x  = (x & 1) << 2;
	//if (bo == 0)
		x ^= 4;		/* x = 4 - x; */
	c = (*s >> x) & 15;
	return c;
}

static uint32_t beta_getRgb_bpp08im(const uint8_t *s, int x) {
	return beta_s_clut[s[x]];
}

static uint32_t beta_getIdx_bpp08im(const uint8_t *s, int x) {
	return s[x];
}


static uint32_t beta_getRgb_bpp12m(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	a = (c & 0xF000)>> 8 ; a |= a >> 4;
	r = (c & 0x0F00)>> 4 ; r |= r >> 4;
	g =  c & 0x00F0      ; g |= g >> 4;
	b = (uint8_t)(c << 4); b |= b >> 4;
	return ARGB(a,r,g,b);
}

static uint32_t beta_getRgb_bpp12i(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	a = (c & 0xF000)>> 8 ; a |= a >> 4;
	r = (c & 0x0F00)>> 4 ; r |= r >> 4;
	g =  c & 0x00F0      ; g |= g >> 4;
	b = (uint8_t)(c << 4); b |= b >> 4;
	return ARGB(a,r,g,b);
}

static uint32_t beta_getIdx_bpp12m(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	a = (c & 0xF000)>> 8;a |= a >> 4;
	r = (c & 0x0F00)>> 4;r |= r >> 4;
	g =  c & 0x00F0;     g |= g >> 4;
	b = (uint8_t)(c << 4); b |= b >> 4;
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getIdx_bpp12i(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	a = (c & 0xF000)>> 8;a |= a >> 4;
	r = (c & 0x0F00)>> 4;r |= r >> 4;
	g =  c & 0x00F0;     g |= g >> 4;
	b = (uint8_t)(c << 4); b |= b >> 4;
	return beta_getIdx_sub(r,g,b);
}


static uint32_t beta_getRgb_bpp13m(const uint8_t *s, int x) {
	uint8_t a,g;
	int     c;
	s += x * 2;
	c = PEEKmW(s);
	a = (uint8_t)(c >> 8);
	g = (uint8_t)c;
	return ARGB(a,g,g,g);
}

static uint32_t beta_getRgb_bpp13i(const uint8_t *s, int x) {
	uint8_t a,g;
	int     c;
	s += x * 2;
	c = PEEKiW(s);
	a = (uint8_t)(c >> 8);
	g = (uint8_t)c;
	return ARGB(a,g,g,g);
}

static uint32_t beta_getIdx_bpp13m(const uint8_t *s, int x) {
	uint8_t a,g;
	int     c;
	s += x * 2;
	c = PEEKmW(s);
	a = (uint8_t)(c >> 8);
	g = (uint8_t)c;
	return g;
}

static uint32_t beta_getIdx_bpp13i(const uint8_t *s, int x) {
	uint8_t a,g;
	int     c;
	s += x * 2;
	c = PEEKiW(s);
	a = (uint8_t)(c >> 8);
	g = (uint8_t)c;
	return g;
}


static uint32_t beta_getRgb_bpp15m(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	a = (uint16_t)((int16_t)c >> 7) >> 8;
	r = (c & 0x7C00) >> 7;
	r |= r >> 5;
	g = (c & 0x03E0) >> 2;
	g |= g >> 5;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return ARGB(a,r,g,b);
}

static uint32_t beta_getRgb_bpp15i(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	a = (uint16_t)((int16_t)c >> 7) >> 8;
	r = (c & 0x7C00) >> 7;
	r |= r >> 5;
	g = (c & 0x03E0) >> 2;
	g |= g >> 5;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return ARGB(a,r,g,b);
}

static uint32_t beta_getIdx_bpp15m(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	a = (uint16_t)((int16_t)c >> 7) >> 8;
	r = (c & 0x7C00) >> 7;
	r |= r >> 5;
	g = (c & 0x03E0) >> 2;
	g |= g >> 5;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getIdx_bpp15i(const uint8_t *s, int x) {
	uint8_t a,r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	a = (uint16_t)((int16_t)c >> 7) >> 8;
	r = (c & 0x7C00) >> 7;
	r |= r >> 5;
	g = (c & 0x03E0) >> 2;
	g |= g >> 5;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getRgb_bpp16m(const uint8_t *s, int x) {
	uint8_t r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	r = (c & 0xF800) >> 8;
	r |= r >> 5;
	g = (c & 0x07E0) >> 3;
	r |= r >> 6;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return ARGB(0xFF,r,g,b);
}

static uint32_t beta_getRgb_bpp16i(const uint8_t *s, int x) {
	uint8_t r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	r = (c & 0xF800) >> 8;
	r |= r >> 5;
	g = (c & 0x07E0) >> 3;
	r |= r >> 6;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return ARGB(0xFF,r,g,b);
}

static uint32_t beta_getIdx_bpp16m(const uint8_t *s, int x) {
	uint8_t r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKmW(s);
	r = (c & 0xF800) >> 8;
	r |= r >> 5;
	g = (c & 0x07E0) >> 3;
	r |= r >> 6;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getIdx_bpp16i(const uint8_t *s, int x) {
	uint8_t r,g,b;
	int     c;
	s += x * 2;
	//if (bo) c = PEEKmW(s);
	//else	c = PEEKiW(s);
	c = PEEKiW(s);
	r = (c & 0xF800) >> 8;
	r |= r >> 5;
	g = (c & 0x07E0) >> 3;
	r |= r >> 6;
	b = (c & 0x001F) << 3;
	b |= b >> 5;
	return beta_getIdx_sub(r,g,b);
}


static uint32_t beta_getRgb_bpp24m(const uint8_t *s, int x) {
	uint32_t c;
	s += x * 3;
	//if (bo)	c = ARGB(0xFF,s[0],s[1],s[2]);	/* big */
	//else	c = ARGB(0xFF,s[2],s[1],s[0]);	/* ltl */
	c = ARGB(0xFF,s[0],s[1],s[2]);	/* big */
	return c;
}

static uint32_t beta_getRgb_bpp24i(const uint8_t *s, int x) {
	uint32_t c;
	s += x * 3;
	//if (bo)	c = ARGB(0xFF,s[0],s[1],s[2]);	/* big */
	//else	c = ARGB(0xFF,s[2],s[1],s[0]);	/* ltl */
	c = ARGB(0xFF,s[2],s[1],s[0]);	/* ltl */
	return c;
}

static uint32_t beta_getIdx_bpp24m(const uint8_t *s, int x) {
	uint8_t r,g,b;
	s += x * 3;
	//if (bo) r = s[0], g = s[1], b = s[2];
	//else    r = s[2], g = s[1], b = s[0];
	 r = s[0], g = s[1], b = s[2];
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getIdx_bpp24i(const uint8_t *s, int x) {
	uint8_t r,g,b;
	s += x * 3;
	//if (bo) r = s[0], g = s[1], b = s[2];
	//else    r = s[2], g = s[1], b = s[0];
	r = s[2], g = s[1], b = s[0];
	return beta_getIdx_sub(r,g,b);
}


static uint32_t beta_getRgb_bpp32m(const uint8_t *s, int x) {
	uint32_t  c;
	s += x * 4;
	//if (bo)	c = PEEKmD(s);		/* big */
	//else  	c = PEEKiD(s);		/* ltl */
	c = PEEKmD(s);					/* big */
	return c;
}

static uint32_t beta_getRgb_bpp32i(const uint8_t *s, int x) {
	uint32_t  c;
	s += x * 4;
	//if (bo)	c = PEEKmD(s);		/* big */
	//else  	c = PEEKiD(s);		/* ltl */
	c = PEEKiD(s);					/* ltl */
	return c;
}

static uint32_t beta_getIdx_bpp32m(const uint8_t *s, int x) {
	uint8_t r,g,b;
	s += x * 4;
	//if (bo) r = s[0], g = s[1], b = s[2];
	//else    r = s[2], g = s[1], b = s[0];
	r = s[0], g = s[1], b = s[2];
	return beta_getIdx_sub(r,g,b);
}

static uint32_t beta_getIdx_bpp32i(const uint8_t *s, int x) {
	uint8_t r,g,b;
	s += x * 4;
	//if (bo) r = s[0], g = s[1], b = s[2];
	//else    r = s[2], g = s[1], b = s[0];
	r = s[2], g = s[1], b = s[0];
	return beta_getIdx_sub(r,g,b);
}


typedef uint32_t (*beta_getPix_t)(const uint8_t *s, int x);

static beta_getPix_t const s_beta_getPix_tbl[2][2][BPPIDX_NUM] = {
	{
		{
			beta_getIdx_bpp01i,
			beta_getIdx_bpp02i,
			beta_getIdx_bpp04i,
			beta_getIdx_bpp08im,
			beta_getIdx_bpp12i,
			beta_getIdx_bpp13i,
			beta_getIdx_bpp15i,
			beta_getIdx_bpp16i,
			beta_getIdx_bpp24i,
			beta_getIdx_bpp32i,
		}, {
			beta_getIdx_bpp01m,
			beta_getIdx_bpp02m,
			beta_getIdx_bpp04m,
			beta_getIdx_bpp08im,
			beta_getIdx_bpp12m,
			beta_getIdx_bpp13m,
			beta_getIdx_bpp15m,
			beta_getIdx_bpp16m,
			beta_getIdx_bpp24m,
			beta_getIdx_bpp32m,
		}
	}, {
		{
			beta_getRgb_bpp01i,
			beta_getRgb_bpp02i,
			beta_getRgb_bpp04i,
			beta_getRgb_bpp08im,
			beta_getRgb_bpp12i,
			beta_getRgb_bpp13i,
			beta_getRgb_bpp15i,
			beta_getRgb_bpp16i,
			beta_getRgb_bpp24i,
			beta_getRgb_bpp32i,
		}, {
			beta_getRgb_bpp01m,
			beta_getRgb_bpp02m,
			beta_getRgb_bpp04m,
			beta_getRgb_bpp08im,
			beta_getRgb_bpp12m,
			beta_getRgb_bpp13m,
			beta_getRgb_bpp15m,
			beta_getRgb_bpp16m,
			beta_getRgb_bpp24m,
			beta_getRgb_bpp32m,
		}
	}
};
static beta_getPix_t	s_beta_getPix	= beta_getIdx_bpp08im;

static inline void beta_set_getPix(int srcBpp, int dstBpp, int boI) {
	int srcIdx = beta_bpp2idx(srcBpp);
	assert(srcIdx <= BPPIDX_NUM);
	boI &= 1;
	if (dstBpp <= 8) {	// index
		if (srcBpp > 8) {
			beta_set_rgb2idx(dstBpp);
		}
		s_beta_getPix = s_beta_getPix_tbl[0][boI][srcIdx];
	} else {
		s_beta_getPix = s_beta_getPix_tbl[1][boI][srcIdx];
	}
}


// ------------------------------------

static void beta_putPix_bpp01m(uint8_t *d, int x, int c) {
	c = c & 1;
	d += x >> 3;
	x &= 7;
	//if (bo == 0)
	//	x = 7-x;
	*d &= ~(1 << x);
	*d |= c << x;
}

static void beta_putPix_bpp01i(uint8_t *d, int x, int c) {
	c = c & 1;
	d += x >> 3;
	x &= 7;
	//if (bo == 0)
		x = 7-x;
	*d &= ~(1 << x);
	*d |= c << x;
}


static void beta_putPix_bpp02m(uint8_t *d, int x, int c) {
	c &= 3;
	d += x >> 2;
	x = (x&3) << 1;
	//if (bo == 0)
	//	x = 6 - x;
	*d &= ~(3 << x);
	*d |= c << x;
}

static void beta_putPix_bpp02i(uint8_t *d, int x, int c) {
	c &= 3;
	d += x >> 2;
	x = (x&3) << 1;
	//if (bo == 0)
		x = 6 - x;
	*d &= ~(3 << x);
	*d |= c << x;
}


static void beta_putPix_bpp04m(uint8_t *d, int x, int c) {
	c &= 0x0f;
	//if (bo == 0) {
	//	if (x & 1)	d[x>>1] |= c;
	//	else		d[x>>1] = (c << 4);
	//} else {
		if (x & 1)	d[x>>1] |= (c << 4);
		else		d[x>>1] = c;
	//}
}

static void beta_putPix_bpp04i(uint8_t *d, int x, int c) {
	c &= 0x0f;
	//if (bo == 0) {
		if (x & 1)	d[x>>1] |= c;
		else		d[x>>1] = (c << 4);
	//} else {
	//	if (x & 1)	d[x>>1] |= (c << 4);
	//	else		d[x>>1] = c;
	//}
}


static void beta_putPix_bpp08im(uint8_t *d, int x, int c) {
	d[x] = c;
}


static void beta_putPix_bpp12m(uint8_t *d, int x, int c) {
	int     a;
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24);
	a = (a + 15) >> 4; if (a > 15) a = 15;
	c = (a << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEmW(d, c);
}

static void beta_putPix_bpp12i(uint8_t *d, int x, int c) {
	int     a;
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24);
	a = (a + 15) >> 4; if (a > 15) a = 15;
	c = (a << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEiW(d, c);
}


static void beta_putPix_bpp13m(uint8_t *d, int x, int c) {
	int     a;
	uint8_t g;
	//r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	//b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24);
	c = (a << 8) | g;
	d += x*2;
	POKEmW(d, c);
}

static void beta_putPix_bpp13i(uint8_t *d, int x, int c) {
	int     a;
	uint8_t g;
	//r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	//b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24);
	c = (a << 8) | g;
	d += x*2;
	POKEiW(d, c);
}


static void beta_putPix_bpp15m(uint8_t *d, int x, int c) {
	int     a;
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24) ? 0x8000 : 0;
	c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEmW(d, c);
}

static void beta_putPix_bpp15i(uint8_t *d, int x, int c) {
	int     a;
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	a = ((uint32_t)c >> 24) ? 0x8000 : 0;
	c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEiW(d, c);
}


static void beta_putPix_bpp16m(uint8_t *d, int x, int c) {
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEmW(d, c);
	return;
}

static void beta_putPix_bpp16i(uint8_t *d, int x, int c) {
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
	d += x*2;
	//if (bo) POKEmW(d, c);
	//else    POKEiW(d, c);
	POKEiW(d, c);
	return;
}


static void beta_putPix_bpp24m(uint8_t *d, int x, int c) {
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	d += x*3;
	//if (bo) {
		d[0] = r;
		d[1] = g;
		d[2] = b;
	//} else {
	//	d[0] = b;
	//	d[1] = g;
	//	d[2] = r;
	//}
}

static void beta_putPix_bpp24i(uint8_t *d, int x, int c) {
	uint8_t r,g,b;
	r = (uint8_t)(c >> 16);
	g = (uint8_t)(c >>  8);
	b = (uint8_t)(c >>  0);
	d += x*3;
	//if (bo) {
	//	d[0] = r;
	//	d[1] = g;
	//	d[2] = b;
	//} else {
		d[0] = b;
		d[1] = g;
		d[2] = r;
	//}
}


static void beta_putPix_bpp32m(uint8_t *d, int x, int c) {
	d += x*4;
	//if (bo)	POKEmD(d, c);
	//else  	POKEiD(d, c);
	POKEmD(d, c);
}

static void beta_putPix_bpp32i(uint8_t *d, int x, int c) {
	d += x*4;
	//if (bo)	POKEmD(d, c);
	//else  	POKEiD(d, c);
	POKEiD(d, c);
}


typedef void (*beta_putPix_t)(uint8_t *d, int x, int c);

static beta_putPix_t const s_beta_putPix_tbl[2][BPPIDX_NUM] = {
	{
		beta_putPix_bpp01i,
		beta_putPix_bpp02i,
		beta_putPix_bpp04i,
		beta_putPix_bpp08im,
		beta_putPix_bpp12i,
		beta_putPix_bpp13i,
		beta_putPix_bpp15i,
		beta_putPix_bpp16i,
		beta_putPix_bpp24i,
		beta_putPix_bpp32i,
	}, {
		beta_putPix_bpp01m,
		beta_putPix_bpp02m,
		beta_putPix_bpp04m,
		beta_putPix_bpp08im,
		beta_putPix_bpp12m,
		beta_putPix_bpp13m,
		beta_putPix_bpp15m,
		beta_putPix_bpp16m,
		beta_putPix_bpp24m,
		beta_putPix_bpp32m,
	}
};
static beta_putPix_t 	s_beta_putPix = beta_putPix_bpp08im;

static inline void beta_set_putPix(int dstBpp, int boO)
{
	int    dstIdx = beta_bpp2idx(dstBpp);
	boO &= 1;
	s_beta_putPix =	s_beta_putPix_tbl[boO][dstIdx];
}

// ------------------------------------

static inline void beta_pixCpy(uint8_t *d, int dstBpp, uint8_t const* s, int srcBpp, int sw
							   , int y0, int y1, int yd, int d_pat, int s_pat, int boI, int boO)
{
	int x, y;
	beta_set_getPix(srcBpp, dstBpp, boI);
	beta_set_putPix(dstBpp, boO);
	for (y = y0; y != y1; y += yd) {
		for (x = 0; x < sw; x++) {
			uint32_t   c = s_beta_getPix(s, x);
			s_beta_putPix(d, x, c);
		}
		s += s_pat;
		d += d_pat;
	}
}



// ----------------------------------------------------------------------------

/** flags;bit 0:Y反転
 *	boI,boO : 入力&出力バイトオーダー
 *	 		0:リトルエンディアン(インテル) 1:ビッグエンディアン(モトローラ)
 *			1バイトのピクセルのときは,バイト内の詰め順.
 *		 	0:上詰め(bmpに同じ)		1:下詰め(towns-tiff系)
 */
int  beta_conv(const void *beta_data, int dstWb, int h, int dbpp, const void *src, int srcWb, int sbpp, const void *clut, int flags, int boI, int boO)
{
	uint8_t *d  = (uint8_t*)beta_data;
	uint8_t *s  = (uint8_t*)src;
	int   dw  = BYT2WID(dstWb, dbpp);
	int			y0,y1,yd,sw, d_pat, s_pat;

	beta_s_clut = (uint32_t*)clut;
	if (beta_s_clut == NULL) {
		static uint32_t clut0[256];
		beta_s_clut = clut0;
		beta_genClut(beta_s_clut, dbpp);
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



int  beta_write(const void *beta_data, int w, int h, int dbpp, const void *src, int srcWb, int sbpp, void *clut, int dir)
{
	return beta_conv(beta_data, WID2BYT(w,dbpp), h, dbpp, src, srcWb, sbpp, clut, dir, 0, 0);
}
