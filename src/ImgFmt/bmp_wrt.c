/**
 *	@file	bmp_wrt.c
 *	@brief	BMP �摜�f�[�^�o��
 *	@author	Masashi Kitamura
 *	@date	199?
 *	@note
 */

#include "bmp_wrt.h"
#include <stddef.h>
#include <string.h>


/*--------------------------------------------------------------------------*/
/* �R���p�C�����̒��날�킹. */


#if (defined _MSC_VER) || (defined __BORLANDC__ && __BORLANDC__ <= 0x0551)
typedef	unsigned char  uint8_t;
typedef	unsigned short uint16_t;
typedef	unsigned       uint32_t;
#else
#include <stdint.h>
#endif


#if !defined(inline) && !(defined __GNUC__) && (__STDC_VERSION__ < 199901L)
#define	inline			__inline
#endif



/*--------------------------------------------------------------------------*/

#define	BBBB(a,b,c,d)	((((uint8_t)(a))<<24)+(((uint8_t)(b))<<16)+(((uint8_t)(c))<<8)+((uint8_t)(d)))

/* �o�C�g�P�ʂ̃A�h���X����l�����o���������A�N�Z�X�p�}�N�� */
#define	PEEKB(a)		(*(const uint8_t *)(a))

#if defined _M_IX86 || defined _X86_ || defined _M_AMD64 || defined __amd64__	// X86 �́A�A���C�����g���C�ɂ���K�v���Ȃ��̂Œ��ڃA�N�Z�X
#define	PEEKiW(a)		(*(const uint16_t *)(a))
#define	PEEKiD(a)		(*(const uint32_t *)(a))
#define	POKEiD(a,b)		(*(uint32_t *)(a) = (b))
#else
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define GLB(a)			((uint8_t)(a))					///< a�� int16�^�Ƃ��ĉ��ʃo�C�g�̒l���擾
#define GHB(a)			((uint8_t)(((uint16_t)(a))>>8))		///< a�� int16�^�Ƃ��ď�ʃo�C�g�̒l���擾
#define GLW(a)			((uint16_t)(a))					///< int32�^�Ƃ��Ă� a �̉�16�r�b�g�̒l���擾
#define GHW(a)			((uint16_t)(((uint32_t)(a))>>16))	///< int32�^�Ƃ��Ă� a �̏�16�r�b�g�̒l���擾
#define POKEB(a,b)		(*(uint8_t *)(a) = (b))
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#endif

#define	BPP2BYT(bpp)	(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)
#define WID2BYT(w,bpp)	(((w) * "\1\4\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)
#define WID2BYT4(w,bpp)	((WID2BYT(w,bpp) + 3) & ~3)
#define	BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 1) ? ((w)<<1) : ((w)<<3))

#undef DBG_F
#define	DBG_F(x)




/*--------------------------------------------------------------------------*/
/* �o�͂̂��߂̏������ݏ���(�G���f�B�A���΍�)								*/


#if defined _M_IX86 || defined _X86_ || defined _M_AMD64 || defined __amd64__	// X86 �́A�A���C�����g���C�ɂ���K�v���Ȃ��̂Œ��ڃA�N�Z�X
// �ėp�ł͂Ȃ��A���̃t�@�C���݂̂ŗL���ȃ}�N���B
// ���� a �͕K��uint8_t*�^�ϐ��ւ̃|�C���^�ł��邱��!
// mput_b3()�� c �͕���p�̂Ȃ��l�ł��邱�ƁI

#define mput_b1(a, c)		(*((*a)++) = (c))
#define mput_b2(a, c)		(*(uint16_t*)(*a) = (c),  (*a) += 2)
// #define mput_b3(a, c)	(*(uint16_t*)(*a) = (c),  (*a)[2] = (c)>>16, (*a) += 3)
#define mput_b4(a, c)		(*(uint32_t*)(*a) = (c),  (*a) += 4)
#define mput_cpy(d,s,l,n)	memcpy((d),(s),(l)*(n))

//#define mput_cpy1(d,s,l)	memcpy(d, s, (l))
//#define mput_cpy2(d,s,l)	MEMCPY2(d, s, (l)*2)
//#define mput_cpy3(d,s,l)	memcpy(d, s, (l)*3)
//#define mput_cpy4(d,s,l)	MEMCPY4(d, s, (l)*4)

#else

static inline void mput_b1(uint8_t **a, int c)
{
	// 1�o�C�g��������������
	uint8_t *d = (uint8_t*)*a;
	//DBG_F(("b1:%x  %02x\n", d, c));
	*d++ = (uint8_t)c;
	*a   = (uint8_t*)d;
}

static inline void mput_b2(uint8_t **a, int c)
{
	// 2�o�C�g��������������
	uint8_t *d = (uint8_t*)*a;
	//DBG_F(("b2:%x  %04x\n", d, c));
	*d++ = (uint8_t)c;
	*d++ = (uint8_t)(c>>8);
	*a   = (uint8_t*)d;
}

/*
static inline void mput_b3(uint8_t **a, int c)
{
	// 3�o�C�g��������������
	uint8_t *d = (uint8_t*)*a;
	//DBG_F(("b3:%x  %06x\n", d, c));
	*d++ = (uint8_t)c;
	*d++ = (uint8_t)(c>>8);
	*d++ = (uint8_t)(c>>16);
	*a   = (uint8_t*)d;
}
*/

static inline void mput_b4(uint8_t **a, int c)
{
	// 4�o�C�g��������������
	uint8_t *d = (uint8_t*)*a;
	//DBG_F(("b4:%x  %08x\n", d, c));
	*d++ = (uint8_t)c;
	*d++ = (uint8_t)(c>>8);
	*d++ = (uint8_t)(c>>16);
	*d++ = (uint8_t)(c>>24);
	*a   = (uint8_t*)d;
}

static inline void mput_cpy(void *dst, const void *src, int len, int n)
{
  #if 1
	memcpy(dst, src, len * n);
  #else
	uint8_t *d = (uint8_t*)dst;
	if (n == 4) {
		uint32_t *s = (uint32_t*)src;
		do {
			mput_b4(&d, *s++);
		} while (--len);
	} else if (n == 2) {
		uint16_t *s = (uint16_t*)src;
		do {
			mput_b2(&d, *s++);
		} while (--len);
	} else {
		memcpy(d, src, len * n);
	}
  #endif
}

#endif



/*--------------------------------------------------------------------------*/
static int bmp_putPixs(uint8_t *dst, int dst_w, int h, int dstBpp, const uint8_t *src, int src_wb, int srcBpp, uint32_t *clut, int dir);
static void bmp_getDfltClut(int bpp, uint32_t *clut);


int  bmp_write(void *bmp_data, int w, int h, int bpp, const void *src, int src_wb, int srcBpp, const void *clut0, int dir)
{
	return bmp_writeEx(bmp_data, w, h, bpp, src, src_wb, srcBpp, clut0, 1<<srcBpp, dir, 0,0);
}



int  bmp_writeEx(void *bmp_data, int w, int h, int dstBpp, const void *src, int src_wb, int srcBpp, const void *clut0, int clutNum, int dir, int rslX, int rslY)
{
	enum {BMP_TOP_HDR_SZ = 2+12};
	uint32_t 	clut1[256];
	uint8_t 	*bm	 	= (uint8_t*)bmp_data;
	uint8_t 	*d     	= bm;
	uint32_t 	*clut 	= (uint32_t*)clut0;
	//int   	cm  	= (dir & 0x80) ? 8 : 0;	// ���k���[�h�͖��Ή�
	//int   	dn  	= BPP2BYT(dstBpp);
	int   		sn  	= BPP2BYT(srcBpp);
	int   		c;
	int			i;
	int			n;
	int			hdrSz;
	int   		clutAlpMode = (dir >> 6) & 1;

	dir = (dir & 1) ^ 1;
	src_wb = (src_wb) ? src_wb : (w * sn + 3) & ~3;

	//DBG_F(("dstBpp = %d dir=%x dr=%x\n",dstBpp,dir,dr));

	/* �w�b�_�쐬 */
	mput_b1(&d, 'B');
	mput_b1(&d, 'M');
	c = BMP_TOP_HDR_SZ + 40;
	mput_b4(&d, c);							/* 2/2  �t�@�C���T�C�Y */
	mput_b4(&d, 0);							/* 6/6  �\�� */
	if (dstBpp <= 8)
		c = BMP_TOP_HDR_SZ + 40 + (1<<dstBpp)*4;
	else
		c = BMP_TOP_HDR_SZ + 40;
	mput_b4(&d, c);							/*  a/10 �s�N�Z���E�f�[�^�ւ̃I�t�Z�b�g */
	mput_b4(&d, 40);						/*  e/14 �w�b�_�T�C�Y */
	mput_b4(&d, w);							/* 12/18 width */
	mput_b4(&d, h);							/* 16/22 height */
	mput_b2(&d, 1);							/* 1a/26 �v���[����. must be 1 */
	mput_b2(&d, dstBpp);					/* 1c/28 bits per pixel */
	mput_b4(&d, 0);							/* 1e/30 ���k���[�h. �����k�Ȃ�0 */
	mput_b4(&d, 0);							/* 22/34 ���k�f�[�^�T�C�Y. �����k�Ȃ�0 */
	mput_b4(&d, rslX);						/* 26/38 x resolution */
	mput_b4(&d, rslY);						/* 2a/42 y resolution */
	mput_b4(&d, 0);							/* 2e/46 clut�̐�. 2,4,8�r�b�g�F�ł͂��̒l�Ɋ֌W�Ȃ�1<<n�F����. ���� 0�ł悢 */
	mput_b4(&d, 0);							/* 32/50 �d�v�ȐF�̐��c�c���ʖ�������f�[�^. ���� 0�ł悢. */
											/* 36/54 */
	/* �p���b�g���� --------------- */
	if (dstBpp <= 8) {
		if (srcBpp > 8 || clut == NULL) {
			clut = clut1;
			clutNum = 1<<dstBpp;
			bmp_getDfltClut(dstBpp, clut);
		}
		for (i = 0; i < (1<<dstBpp); i++) {
			c = (i < clutNum) ? clut[i] : 0;
			if (clutAlpMode == 0)
				c &= 0xFFFFFF;
			mput_b4(&d, c);
		}
	}

	//�����k
	hdrSz = (int)(d - bm);
	bmp_putPixs(d, w, h, dstBpp, (uint8_t*)src, src_wb, srcBpp, clut, dir);
	d += WID2BYT4(w, dstBpp) * h;
	n = (int)(d - bm);
  #if 0
	if (cm && (dstBpp == 4 || dstBpp == 8)) {
		m = malloc(WID2BYT4(w,dstBpp) * 2 * h + 2*h + 16);
		if (m) {
			if (dstBpp == 4)
				i = bmp_encodeRle4(m, bm+hdrSz, w, h);
			else
				i = bmp_encodeRle8(m, bm+hdrSz, w, h);
			if (hdrSz+i < n) {	// ���k����Ă���
				memcpy(bm+hdrSz, m, i);
				POKEiD(bm+0x1e, 1);			/*1e ���k���[�h. �����k�Ȃ�0 */
				POKEiD(bm+0x22, i);			/*22 ���k�f�[�^�T�C�Y. �����k�Ȃ�0 */
				n = hdrSz + i;
			}
			free(m);
		}
	}
  #endif
	POKEiD(bm+2, n);
	return n;
}



static void bmp_getDfltClut(int bpp, uint32_t *clut)
{
	int i,c; //r,g,b;

	if (clut == NULL)
		return;
	if (bpp == 1) {			// ����
		clut[0] = 0;
		clut[1] = 0xFFFFFF;
	} else if (bpp == 4) {	// �P�F�Z�W
		for (i = 0; i < 16; i++) {
			c = (i << 4) | i;
			clut[i] = BBBB(0, c,c,c);
		}
	} else {				// TOWNS��88VA�n�� G3R3B2
		for (i = 0; i < 256; i++)
			clut[i] = BBBB(0, (i&0x1C)<<3, (i&0xE0), (i&3)<<6);
	}
}


#if 0

static inline int	 GetPx1(uint8_t **sp, int srcBpp, uint32_t *clut)
{
	uint8_t *s = *sp;
	int c,r,g,b,a;

	if (srcBpp <= 8) {
		c = clut[*s++];
	} else if (srcBpp <= 15) {
		c  = PEEKW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		a  = (c & 0x8000) ? 0x80 : 0;
		c = BBBB(a, r, g, b);
		s += 2;
	} else if (srcBpp <= 16) {
		c  = PEEKW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		c = BBBB(0, r, g, b);
		s += 2;
	} else if (srcBpp <= 24) {
		c = BBBB(0, s[2], s[1], s[0]);
		s += 3;
	} else {
		c = PEEKD(s);
		s += 4;
	}
	*sp = s;
	return c;
}


static inline void	 PutPx1(uint8_t **dp, int c, int dstBpp)
{
	uint8_t *d = *dp;
	int   r, g, b, a;

	if (dstBpp == 8) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		mput_b1(&d, ((g >> 5)<<5) | ((r >> 5)<<2) | (b >> 6));
	} else if (dstBpp == 15) {
		a = (c >> 24) ? 0x8000 : 0;
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		mput_b2(&d, c);
	} else if (dstBpp == 16) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		mput_b2(&d, c);
	} else if (dstBpp == 24) {
		mput_b3(&d, c);
	} else {
		mput_b4(&d, c);
	}
	*dp = d;
}

#endif


static inline uint8_t *bmp_putPix32(uint8_t *d, int c, int bpp)
{
	int r,g,b,a;

	if (bpp == 15) {	// 15�̓�1bit����Ƃ��Ĉ���
		a = (c >> 24) ? 0x8000 : 0;
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		*(uint16_t*)d = c;
		return d + 2;
	} else if (bpp <= 16) {
		r = (uint8_t)(c >> 16);
		g = (uint8_t)(c >>  8);
		b = (uint8_t)(c >>  0);
		c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		*(uint16_t*)d = c;
		return d + 2;
	} else if (bpp <= 24) {
		d[0] = (uint8_t)c;
		d[1] = (uint8_t)(c>>8);
		d[2] = (uint8_t)(c>>16);
		return d + 3;
	} else {
		*(uint32_t*)d = c;
		return d + 4;
	}
}



static inline int bmp_getPix32(const uint8_t *s, int x, int bpp, uint32_t *clut, int ofs)
{
	int c,r,g,b,a;

	if (bpp <= 1) {
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		return clut[c + ofs];
	} else if (bpp <= 2) {
		int n = (x & 3)<<1;
		c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
		return clut[c];
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;	//c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
		return clut[c + ofs];
	} else if (bpp <= 8) {
		return clut[s[x] + ofs];
	} else if (bpp <= 16) {
		s += x * 2;
		c  = (s[1]<<8) | s[0];
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		a  = (c & 0x8000) ? 0x80 : 0;
		return  BBBB(a, r,g,b);
	} else if (bpp <= 24) {
		s += x * 3;
		return BBBB(0, s[2], s[1], s[0]);
	} else {
		s += x * 4;
		return BBBB(s[3], s[2], s[1], s[0]);
	}
}



/// 256�F�̃s�N�Z���ɂ��Ď擾�B���F�� 256�F���� G3R3B2 �`���ւ̊ȈՕϊ�.
static inline int bmp_getPix8(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
	int c,r,g,b;

	dmy_clut;

	if (bpp <= 1) {
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		return c + ofs;
	} else if (bpp <= 2) {
		int n = (x & 3)<<1;
		c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
		return c + ofs;
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;	//c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
		return c + ofs;
	} else if (bpp <= 8) {
		return s[x] + ofs;
	} else if (bpp <= 16) {
		s += x * 2;
		c  = s[1]<<8 | s[0];
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);
	} else if (bpp <= 24) {
		s += x * 3;
		return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
	} else {
		s += x * 4;
		return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
	}
}


// 16�F�s�N�Z���ɂ��Ď擾�B���F�� 16�F���� ���m�N���ւ̊ȈՕϊ�.
static inline int bmp_getPix42(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
	int c,r,g,b, m = (1<<bpp)-1, j=12-bpp;

	dmy_clut;

	if (bpp <= 1) {
		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);
		return (c + ofs) & m;
	} else if (bpp <= 2) {
		int n = (x & 3)<<1;
		c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
		return (c + ofs) & m;
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2)));
		return (c + ofs) & m;
	} else if (bpp <= 8) {
		return (s[x] + ofs) & m;
	} else if (bpp <= 16) {
		s += x * 2;
		c  = PEEKiW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		return (g * 9 + r * 5 + b * 2) >> j;
	} else if (bpp <= 24) {
		s += x * 3;
		return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> j;
	} else {
		s += x * 4;
		return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> j;
	}
}



static int bmp_getPix1(const uint8_t *s, int x, int bpp, uint32_t *dmy_clut, int ofs)
{
	int c, n;

	dmy_clut;

	if (bpp <= 1) {
		n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);
		//c = (x & 1) ? s[x>>1] & 0x0f : (uint8_t)s[x>>1] >> 4;
		return (c+ofs)&1;
	} else if (bpp <= 2) {
		int n = (x & 3)<<1;
		c = (s[x>>2] & (0xC0 >> n)) >> (6-n);
		return (c + ofs)&1;
	} else if (bpp <= 4) {
		c = s[x>>1] >> ((((x&1)^1)<<2));
		return (c + ofs)&1;
	} else if (bpp <= 8) {
		return (s[x] + ofs) & 1;
	} else if (bpp <= 16) {
		s += x * 2;
		return (PEEKiW(s) > 0);
	} else if (bpp <= 24) {
		s += x * 3;
		return BBBB(0,s[2],s[1],s[0]) > 0;
	} else {
		s += x * 4;
		return (PEEKiD(s) > 0);
	}
}


/** srcBpp�F w*h�h�b�g��src �摜���AdstBpp�F w*h �� dst�摜�ɕϊ�����.
 * dir : bit0:�s�N�Z������0=�ォ�� 1=������   bit1:0=������ 1=�E����
 */
static int bmp_putPixs(uint8_t *dst, int dst_w, int h, int dstBpp, const uint8_t *src, int src_wb, int srcBpp, uint32_t *clut, int dir)
{
	enum {ofs=0};
	const uint8_t *s;
	uint8_t *d;
	int  c, dpat, spat, dst_wb;
	int  x,y, y0, y1, yd, w;

	DBG_F(("%x,%d,%d,%d,%x,%d,%d,%x,%d\n",dst,dst_w,h,dstBpp,src,src_wb,srcBpp,clut,dir));
	if (dst == NULL || src == NULL || dst_w == 0 || h == 0 || src_wb == 0) {
		return 0;
	}
	spat = src_wb;
	w = BYT2WID(src_wb, srcBpp);
	dst_wb = WID2BYT4(dst_w, dstBpp);
	dpat = dst_wb - WID2BYT(w, dstBpp);
	//	if (dpat < 0) {
	//		w    = BYT2WID(dst_wb, dstBpp);
	//		dpat = dst_wb - WID2BYT(w, dstBpp);
	//	}
	DBG_F(("spat=%d, dpat=%d, w=%d, dst_wb=%d, srcBpp=%d, dstBpp=%d\n", spat, dpat, w, dst_wb, srcBpp,dstBpp));

	// �摜�̌����̒���
	d   = (uint8_t *)dst;
	s   = (const uint8_t *)src;
	y0  = 0, y1 = h, yd = +1;
	if (dir & 1) {
		y0  = h-1, y1 = -1, yd = -1;
		d   = d + y0 * dst_wb;
		dpat = dpat - dst_wb * 2;
	}
	//DBG_F(("[%d, %d, %d, %d]\n", y0,y1,yd,dpat));

	// �摜�R�s�[
	if (dstBpp > 8) {	// �o�͂����F�̂Ƃ�
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix32(s, x, srcBpp, clut,ofs);
				d = bmp_putPix32(d, c, dstBpp);
			}
			d += dpat;
			s += spat;
		}
	} else if (dstBpp > 4) {	// �o�͂�256�F�̂Ƃ�
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix8(s, x, srcBpp, clut,ofs);
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	} else if (dstBpp > 2) {	// �o�͂� 16�F�̂Ƃ�
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = (uint8_t)(bmp_getPix42(s, x, srcBpp, clut,ofs) << 4);
				if (++x < w)
					c |= bmp_getPix42(s, x, srcBpp, clut,ofs) & 0x0F;
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	} else if (dstBpp > 1) {	// �o�͂� 4�F�̂Ƃ�
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix42(s, x, srcBpp, clut,ofs) << 6;
				if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs) << 4;
				if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs) << 2;
				if (++x < w) c |= bmp_getPix42(s, x, srcBpp, clut,ofs);
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	} else {	// �o�͂� 2�F�̂Ƃ�
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix1(s, x, srcBpp, clut,ofs) << 7;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 6;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 5;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 4;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 3;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 2;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 1;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs);
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	}
	return 1;
}



#if 0
static void bmp_encodeRle8(uint8_t *dst, const uint8_t *src, int w, int h)
{
	int   wb = WID2BYT4(w,8);
	uint8_t *d = dst, *s = src;
	uint8_t *e = s + wb * h;
}
#endif




//----------------------------------------------------------------------
// ���k�⏕
//----------------------------------------------------------------------

int bmp_chkDstBpp(int bpp)
{
	if (bpp <= 1)		return 1;
	else if (bpp <= 4)	return 4;
	else if (bpp <= 8)	return 8;
	else if (bpp <= 16)	return 16;
	else if (bpp <= 24)	return 24;
	else 				return 32;
}


/** �摜�t�@�C���f�[�^�����̂ɕK�v�ȃ������T�C�Y��Ԃ�
 */
int bmp_encodeWorkSize(int w, int h, int bpp)
{
	int wb = WID2BYT4(w, bpp);
	return 256+4*256 + 2 * wb * h;
}

