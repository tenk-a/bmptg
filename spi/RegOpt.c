/*
	���W�X�g���o�R��spi�̃I�v�V�����v�f�𐧌䂷�郋�[�`��
	Tenk's SusiePlugIn Registory Option
	2000-10-09		by tenk*
		 10-17
	200?			���܂���
	2007-06			������3�o�C�g���܂鎞�̃p�e�B���O�����̏C��.
 */

#include <windows.h>
#include <stdlib.h>
#include "regopt.h"
//#include "dbg.h"


/* ----------------------------------------------------------------------- */

#if 0	/* �ʂɃw�b�_���g���ꍇ */
#include "def.h"
#else	/* ���̃t�@�C���݂̂ŕ���ꍇ */
typedef unsigned char  Uint8;
typedef unsigned short Uint16;
typedef unsigned	   Uint32;
#define inline			__inline
#define CPU_X86 		/* ���g���G���f�B�A�������o�C�g�����̃A�h���X�̃A���C�����g�𖳎��ł���Ƃ��ɒ�` */

#define BB(a,b) 		((((Uint8)(a))<<8)+(Uint8)(b))
#define WW(a,b) 		((((Uint16)(a))<<16)+(Uint16)(b))
#define BBBB(a,b,c,d)	((((Uint8)(a))<<24)+(((Uint8)(b))<<16)+(((Uint8)(c))<<8)+((Uint8)(d)))

#define GLB(a)			((unsigned char)(a))
#define GHB(a)			GLB(((unsigned short)(a))>>8)
#define GLLB(a) 		GLB(a)
#define GLHB(a) 		GHB(a)
#define GHLB(a) 		GLB(((unsigned long)(a))>>16)
#define GHHB(a) 		GLB(((unsigned long)(a))>>24)
#define GLW(a)			((unsigned short)(a))
#define GHW(a)			GLW(((unsigned long)(a))>>16)

/* �o�C�g�P�ʂ̃A�h���X����l�����o���������A�N�Z�X�p�}�N�� */
#define POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#ifdef CPU_X86
#define POKEiW(a,b) 	(*(unsigned short *)(a) = (b))
#define POKEiD(a,b) 	(*(unsigned 	  *)(a) = (b))
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEiW(a,GLW(b)))
#else
#define POKEiW(a,b) 	(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define POKEiD(a,b) 	(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a), GLLB(b)))
#endif

#define POKEmW(a,b) 	(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define POKEmD(a,b) 	(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))
#define POKEmB3(a,b)	(POKEB((a)+0, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a)+2, GLLB(b)))

#ifdef BIG_ENDIAN
#define POKEW(a,b)		POKEmW(a,b)
#define POKEB3(a,b) 	POKEmB3(a,b)
#define POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define POKEW(a,b)		POKEiW(a,b)
#define POKEB3(a,b) 	POKEiB3(a,b)
#define POKED(a,b)		POKEiD(a,b)
#endif

#define MEMSETD(d, s, sz)		do {int   *d__ = (int  *)(d); int c__ = (sz); do { *d__++ = (int)  (s); } while(--c__); } while (0)

#define GR_ARGB(a,r,g,b)		((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define GR_RGB(r,g,b)			((((unsigned char)(0))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))

#define GR_RGB_B(rgb)			((unsigned char)(rgb))
#define GR_RGB_G(rgb)			((unsigned char)((rgb)>>8))
#define GR_RGB_R(rgb)			((unsigned char)((rgb)>>16))
#define GR_RGB_A(rgb)			((unsigned char)((rgb)>>24))

#define GR_BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))
#define GR_WID2BYT(w,bpp)	(((bpp) > 24) ? (w)<<2 : ((bpp) > 16) ? (w)*3 : ((bpp) > 8) ? (w)<<1 : ((bpp) > 4) ? (w) : ((bpp) > 1) ? ((w+1)>>1) : ((w+7)>>3))
#define GR_WID2BYT4(w,bpp)	((GR_WID2BYT(w,bpp) + 3) & ~3)

#endif


#ifdef _WINDOWS
#include <windows.h>
#define MALLOC(sz)			LocalAlloc(LMEM_FIXED, (sz))
#define FREE(p) 			LocalFree(p)
#define MEM_CPY(d,s,sz) 	CopyMemory((d),(s),(sz))
#define MEM_CLR(m,sz)		ZeroMemory((m), (sz))
#else
#include <stdlib.h> 	/* NULL, malloc, free ���g�p */
#define MALLOC(sz)			malloc(sz)
#define FREE(p) 			free(p)
#define MEM_CPY(d,s,sz) 	memcpy((d),(s),(sz))
#define MEM_CLR(m,sz)		memset((m),0,(sz))
#endif



/* ----------------------------------------------------------------------- */

#define ER_OK			  0  /* ����I��			   */
#define ER_NO_FUNCTION	 -1  /* ���̋@�\�̓C���v�������g����Ă��Ȃ� */
#define ER_ABORT		  1  /* �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~���� */
#define ER_NOT_SUPPROT	  2  /* ���m�̃t�H�[�}�b�g	   */
#define ER_OUT_OF_ORDER   3  /* �f�[�^�����Ă���	   */
#define ER_NOT_ENOUGH_MEM 4  /* �������[���m�ۏo���Ȃ� */
#define ER_MEMORY		  5  /* �������[�G���[�iLock�o���Ȃ��A���j*/
#define ER_FILE_READ	  6  /* �t�@�C�����[�h�G���[   */
/*#define ER_RESERVE	  7*//* (�\��)				   */
#define ER_ETC			  8  /* �����G���[			   */



/* ----------------------------------------------------------------------- */

int RegOpt_Get(regopt_t *ro)
{
	HKEY hKey;
	DWORD dwType = REG_BINARY;
	DWORD dwByte = sizeof(regopt_t);
	int n;

  #if 1
	n = RegOpenKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, KEY_ALL_ACCESS, &hKey);
  #else
	n = RegCreateKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, "",
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwPosition);
  #endif
	if (n != ERROR_SUCCESS)
		return 0;
	MEM_CLR(ro, sizeof(*ro));
	n = RegQueryValueEx(hKey, "ORDER", NULL, &dwType, (BYTE *)ro, &dwByte);
	RegCloseKey(hKey);
	if (n != ERROR_SUCCESS && n != ERROR_MORE_DATA)
		return 0;
	return ro->enable;
}


int  RegOpt_Put(regopt_s_t *rs, int w, int h, int bpp, const DWORD *clut, int clutNum, const char *fname)
{
	HKEY hKey;
	DWORD dwPosition;

	rs->w		= w;
	rs->h		= h;
	rs->bpp 	= (short)bpp;
	rs->clutNum = (short)clutNum;

	RegCreateKeyEx(HKEY_CURRENT_USER, REGOPT_FILE_NAME, 0, "",
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwPosition);
	rs->time = 0;//timeGetTime();
	RegSetValueEx(hKey, "GINFO", 0, REG_BINARY, (CONST BYTE *)rs, sizeof(*rs));
	if (bpp <= 8) {
		DWORD clut0[256];
		if (clutNum < 256) {
			MEM_CLR(clut0, 256*sizeof(clut[0]));
			clut	= (DWORD*)MEM_CPY(clut0, clut, clutNum*sizeof(clut[0]));
			clutNum = 256;
		}
		RegSetValueEx(hKey, "CLUT", 0,	REG_BINARY, (CONST BYTE *)clut, sizeof(clut[0])*clutNum);
	}
	if (fname == NULL)
		fname = "";
	RegSetValueEx(hKey, "FNAME", 0,  REG_SZ, (CONST BYTE *)fname, strlen(fname));
	RegCloseKey(hKey);
	return 1;
}


#if 0
static int RegOpt_gridRect(int gw, int gh, int *x_p, int *y_p, int *w_p, int *h_p)
{
	/* �O���b�h�T�C�Y gw*gh �Ƃ��āA�O���b�h�ɍ����悤�ɋ�`�T�C�Y�𒲐� */
	int x0,y0,x1,y1;

	if (gw > 1) {
		x1 = ((*x_p + *w_p + gw - 1) / gw) * gw;
		x0 = (*x_p / gw) * gw;
		*x_p = x0;
		*w_p = x1 - x0;
	}
	if (gh > 1) {
		y1 = ((*y_p + *h_p + gh - 1) / gh) * gh;
		y0 = (*y_p / gh) * gh;
		*y_p = y0;
		*h_p = y1 - y0;
	}

	return 0;
}
#endif


static inline int RegOpt_Pix1(regopt_t *ro, DWORD c, DWORD c2)
{
	int a,r,g,b;
	int a0,r2,g2,b2;

	a = GR_RGB_A(c);
	r = GR_RGB_R(c) * ro->tone / 1000;
	g = GR_RGB_G(c) * ro->tone / 1000;
	b = GR_RGB_B(c) * ro->tone / 1000;
	a0 = a;
	if (ro->alpHalf&1)
		a = a * 255 / 128;
	a = a * ro->alpVal / 255;	// ���������[�g�𒲐��������ꍇ�p
	r2 = GR_RGB_R(c2);
	g2 = GR_RGB_G(c2);
	b2 = GR_RGB_B(c2);

	if (ro->alpMode == 0 || a0 > 0) {	//�����ꗥ���f����ꍇ�A�܂��́A�����������Ȃ��
		if (ro->alpType == 1) {
			r = (r * a + r2 * (255 - a)) / 255; if (r > 255) r = 255; else if (r < 0) r = 0;
			g = (g * a + g2 * (255 - a)) / 255; if (g > 255) g = 255; else if (g < 0) g = 0;
			b = (b * a + b2 * (255 - a)) / 255; if (b > 255) b = 255; else if (b < 0) b = 0;
		} else if (ro->alpType == 2) {
			r = r * a / 255 + r2; if (r > 255) r = 255;
			g = g * a / 255 + g2; if (g > 255) g = 255;
			b = b * a / 255 + b2; if (b > 255) b = 255;
		} else if (ro->alpType == 3) {
			r = r2 - r * a / 255; if (r < 0) r = 0;
			g = g2 - g * a / 255; if (g < 0) g = 0;
			b = b2 - b * a / 255; if (b < 0) b = 0;
		}
	}
	return GR_ARGB(0,r,g,b);
}



int RegOpt_GetPicture(regopt_t *ro, BYTE *dst, int w, int h, int dstBpp, DWORD *clut0, int clutNum, const BYTE* src, int srcWidByt, int srcBpp)
{
	DWORD clut[256];
	DWORD *bg, *bu = NULL;
	BYTE *p;
	int rw = ro->gridW, rh = ro->gridH, c1 = ro->bgCol1, c2 = ro->bgCol2;
	int x,y,i,n,c,ry;
	//int x0,y0,w0,h0;

	//DBG_F(("%d*%d %d %d\n", w, h, bpp, clutNum));

	// BG �쐬
	bg = (DWORD*)MALLOC(w * h * 4);
	if (bg == NULL)
		return ER_NOT_ENOUGH_MEM;
	if (ro->bgGridMode == 0) {
		MEMSETD(bg, 0, w * h);
	} else {
		MEMSETD(bg, ro->bgCol1, w * h);
		if (rw && rh) {
			for (y = 0; y < h; y++) {
				ry = h-1-y;
				for (x = 0; x < w; x++) {
					bg[ry*w+x] = (((x / rw) + (y / rh)) & 1) ? c1 : c2;
				}
			}
		}
	}

	if (srcBpp == 8) {	// �p���b�g�t���摜�̂Ƃ�
		if (clutNum == 0)
			return ER_ETC;

		// �p���b�g�Ƀ���񂪂��邩�A�`�F�b�N
		c = 0;
		for (i = 0; i < clutNum; i++) {
			c |= clut0[i];
		}

		// �p���b�g�����[�e�[�g���Ȃ���R�s�[
		MEM_CLR(clut, sizeof(clut));
		n = (clutNum <= 256) ? clutNum : 256;
		for (i = 0; i < n; i++) {
			clut[i] = clut0[(i+ro->clutOfs)%clutNum];
		}
		clutNum = 256;

		if (c & 0xFF000000) {	// ����������
			if (ro->alpRev&1) { // 0���s����, 0xFF�������Ƃ��Ĉ��������ꍇ�΍�
				for (i = 0; i < clutNum; i++) {
					clut[i] ^= 0xFF000000;
				}
			}
		} else {				// �������摜�������B
			// �ʏ�� 0xFF ���s�����` 0�������Ȃ̂�,����������
			for (i = 0; i < clutNum; i++) {
				clut[i] |= 0xFF000000;
			}
		}
		if (ro->keyMode == 2) { 		// �����F�Ƃ��Ĕԍ����g���ꍇ�A�����F�̓��ł����������ɂ���
			c1 = ro->clutKey;	//c1 = (ro->clutKey + ro->clutOfs) % clutNum;
			for (i = 0; i < clutNum; i++) {
				if (i == c1) {
					clut[i] &= 0xFFFFFF;
				}
			}
		} else if (ro->keyMode == 1) {		// ���F���_�C���N�g�J���[�̂Ƃ��A�����F�̓��ł����������ɂ���
			for (i = 0; i < clutNum; i++) {
				if ((clut[i] & 0xFFFFFF) == (ro->colKey&0xFFFFFF)) {
					clut[i] &= 0xFFFFFF;
				}
			}
		}

		// �������[�h�ɍ��킹�ď���
		switch (ro->keyMode) {
		case 1: // �_�C���N�g�J���[�Ŕ����w�肳��Ă���ꍇ
			c1 = ro->colKey;
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					c = src[y*srcWidByt+x];
					c = clut[c];
					if ((c&0xFFFFFF) != (c1 & 0xFFFFFF)) {
						bg[y*w+x] = RegOpt_Pix1(ro, c, bg[y*w+x]);
					}
				}
			}
			break;

		case 2: // clut�ԍ��Ŕ����F���w�肳��Ă���ꍇ
			c1 = ro->clutKey;
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					c = src[y*srcWidByt+x];
					if (c != c1) {
						bg[y*w+x] = RegOpt_Pix1(ro, clut[c], bg[y*w+x]);
					}
				}
			}
			break;

		default:	// ��������
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					c = src[y*srcWidByt+x];
					bg[y*w+x] = RegOpt_Pix1(ro, clut[c], bg[y*w+x]);
				}
			}
			break;
		}
	} else {	// �_�C���N�g�J���[�̂Ƃ�
		bu = (DWORD*)src;
		// ����񂪂��邩�A�`�F�b�N
		c = 0;
		for (i = 0; i < w*h; i++) {
			c |= bu[i];
		}
		if (c & 0xFF000000) {	// ������������A
			if (ro->alpRev&1) { // 0���s����, 0xFF�������Ƃ��Ĉ��������ꍇ�΍�
				for (i = 0; i < w*h; i++) {
					bu[i] ^= 0xFF000000;
				}
			}
		} else {
			for (i = 0; i < w*h; i++) {
				bu[i] |= 0xFF000000;
			}
		}

		c1 = ro->colKey;
		if (ro->keyMode == 0) {
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					bg[y*w+x] = RegOpt_Pix1(ro, bu[y*w+x], bg[y*w+x]);
				}
			}
		} else {
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					c = bu[y*w+x];
					if ((c&0xFFFFFF) != (c1&0xFFFFFF)) {
						bg[y*w+x] = RegOpt_Pix1(ro, c, bg[y*w+x]);
					}
				}
			}
		}
	}
	if (ro->bgGridMode && ro->gridFlg && rw && rh) {
		c1 = ro->gridCol;
		for (y = 0; y < h; y+=rh) {
			ry = h-1-y;
			for (x = 0; x < w; x++)
				bg[ry*w+x] = c1;
		}
		for (x = 0; x < w; x+=rw) {
			for (y = 0; y < h; y++) {
				bg[y*w+x] = c1;
			}
		}
	}

	if (dstBpp == 24) {
		// n = GR_WID2BYT4(w, 24) - w * 3;
		n = ((w * 3 + 3) & ~3) - w * 3;

		p = dst;
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				c = bg[y*w+x];
				POKEiB3(p, c);
				p += 3;
			}
			if (n & 2) {
				POKEW(p, 0);
				p+=2;
			}
			if (n & 1) {
				POKEB(p, 0);
				p++;
			}
		}
	} else {	// 32�r�b�g�F�ŕԂ���
		memcpy(dst, bg, w*h*4);
	}
	FREE(bg);
	return ER_OK;
}



#if 0
static int RegOpt_getNukiRect(void *pix0, int xsz, int ysz, int *x_p, int *y_p, int *w_p, int *h_p)
{
	/* �w�i�F�ȊO���g���Ă����`�����߂�. �����F�̓t���J���[ */
	int x,y,x0,y0,x1,y1;
	Uint32 *pix = pix0;

	if (pix == NULL || xsz == 0 || ysz == 0) {
		return -1;
	}

	x0 = y0 =  0x7fff;
	x1 = y1 = -0x7fff;

	for (y = 0; y < ysz; y++) {
		for (x = 0; x < xsz; x++) {
			if ((pix[y*xsz+x] >> 24) != 0) {
				if (x0 > x) x0 = x;
				if (y0 > y) y0 = y;
				if (x1 < x) x1 = x;
				if (y1 < y) y1 = y;
			}
		}
	}

	if (x0 == 0x7fff || x1 == -0x7fff) {	/* �w�i�F�݂̂̂Ƃ� */
		*w_p = 0;
		*h_p = 0;
		*x_p = 0;
		*y_p = 0;
	} else {
		*x_p = x0;
		*y_p = y0;
		*w_p = x1+1 - x0;
		*h_p = y1+1 - y0;
	}
	return 0;
}
#endif


