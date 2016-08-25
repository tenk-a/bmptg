/**
 *	@file	gr.c
 *	@brief	グラフィック描画
 *	@author tenk* (Masashi KITAMURA)
 */
#include <windows.h>
#include <stdio.h>
#include "gr.h"
//#include "dbg.h"

#define inline		__inline
#define GR_BPP		32




/*--------------------------------------------------------------------------*/
#if GR_BPP == 32	//
 // 描画バッファを32bit色にするとき
 typedef unsigned			gr_pix_t;
 #define GR_RGB2PIX(c)		(c)
#else
 // 描画バッファを16bit色にするとき
 typedef unsigned short 	gr_pix_t;
 #define GR_RGB2PIX(c)		(((((c)>>16)&0xF8)<<7)|((((c)>>8)&0xF8)<<2)|(((c)>>3)&0x1f))
#endif


typedef struct gr_t {		// グラフィック関係の変数。
	BYTE		*pix;		// 描画バッファの表示基準の先頭アドレス
	int 		lPitch; 	// 描画バッファの次の行までのバイト数
	BYTE		*pix_mem;	// 描画バッファのmallocしたときのアドレス
	HWND		hwnd;		// カレントウィンドウ
	BITMAPINFO	bmpInfo;	// 仮想フレームバッファ表示に使う
	HBITMAP 	hBitmap;
	int 		wx,wy,ww,wh;
} gr_t;


static gr_t 	 gr_inst;
static gr_t 	 *gr		= NULL;
static gr_inf_t  gr_inf_inst;
extern gr_inf_t  *gr_inf	= &gr_inf_inst;




/*--------------------------------------------------------------------------*/

/** グラフィック処理の初期化
 */
int  __fastcall gr_init(HWND hwnd, int w, int h, int md)
{
	RECT r;

	gr = &gr_inst;
	memset(gr, 0, sizeof(*gr));

	gr->hwnd	 = hwnd;
	gr_inf->mode = md;
	gr_inf->w	 = w;
	gr_inf->h	 = h;

	// 表示範囲を取得
	GetClientRect(hwnd, &r);
	gr_inf->fs_w  = r.right  - r.left;
	gr_inf->fs_h  = r.bottom - r.top;
	if (gr_inf->fs_w == 0 || gr_inf->fs_h == 0)
		return 0;
	//if (gr_inf->w < 0 || gr_inf->h < 0) {
	//	gr_inf->w = gr_inf->fs_w;
	//	gr_inf->h = gr_inf->fs_h;
	//}
	gr_inf->x0 = r.left;	//	(gr_inf->fs_w - gr_inf->w) >> 1;
	gr_inf->y0 = r.top; 	//	(gr_inf->fs_h - gr_inf->h) >> 1;

	GetWindowRect(hwnd, &r);
	gr->wx = r.left;
	gr->wy = r.top;
	gr->ww = r.right  - r.left;
	gr->wh = r.bottom - r.top;

	// 描画バッファの設定. 16 or 32ビットカラー。
	gr->lPitch	= -gr_inf->w * sizeof(gr_pix_t);

	// 描画バッファを表示するときに使う
	memset(&gr->bmpInfo, 0, sizeof(gr->bmpInfo));
	gr->bmpInfo.bmiHeader.biSize		  = 0x28/*sizeof(gr->bmpInfo)*/;
	gr->bmpInfo.bmiHeader.biWidth		  = gr_inf->w;
	gr->bmpInfo.bmiHeader.biHeight		  = gr_inf->h;
	gr->bmpInfo.bmiHeader.biPlanes		  = 1;
	gr->bmpInfo.bmiHeader.biBitCount	  = 8*sizeof(gr_pix_t);   // 16 or 32
	gr->bmpInfo.bmiHeader.biCompression   = BI_RGB;
	gr->bmpInfo.bmiHeader.biSizeImage	  = 0;
	gr->bmpInfo.bmiHeader.biXPelsPerMeter = 0;
	gr->bmpInfo.bmiHeader.biYPelsPerMeter = 0;
	gr->bmpInfo.bmiHeader.biClrUsed 	  = 0;
	gr->bmpInfo.bmiHeader.biClrImportant  = 0;
	//
	{
		//BITMAPINFOHEADER bi;
		BITMAPINFOHEADER *lpbi;
		HDC 	hdc;
		HANDLE	hdib;

		hdib	= GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) );
		lpbi	= (BITMAPINFOHEADER*)GlobalLock(hdib);
		*lpbi	= gr->bmpInfo.bmiHeader;
		hdc 	= GetDC(hwnd);
		gr->hBitmap = CreateDIBSection(hdc, (CONST BITMAPINFO *)lpbi, DIB_RGB_COLORS, (void*)&gr->pix_mem, NULL, 0);
		ReleaseDC(hwnd, hdc);
		GlobalUnlock(hdib);
		GlobalFree(hdib);
	}
	gr->pix 	= gr->pix_mem + (gr_inf->h - 1) * gr_inf->w * sizeof(gr_pix_t);

	return 1;
}


void __fastcall gr_term(void)
{
	// グラフィック処理の終了
	if (gr) {
		if (gr->hBitmap)
			DeleteObject(gr->hBitmap);
		memset(gr, 0, sizeof(*gr));
		memset(gr_inf, 0, sizeof(*gr_inf));
	}
	gr = NULL;
}


void __fastcall gr_update(void)
{
	// 画面の更新
	HDC hdc;
	HBITMAP hb;

	if (gr == NULL)
		return;
	hdc  = GetDC(gr->hwnd);
	hb = CreateCompatibleBitmap(hdc, gr_inf->w, gr_inf->h);
	if (hb) {
		HDC hdc2;
		hdc2 = CreateCompatibleDC(hdc);
		SelectObject(hdc2, hb);
		SetDIBitsToDevice(hdc2, 0, 0, gr_inf->w, gr_inf->h, 0, 0,
			0, gr_inf->h, gr->pix_mem, &gr->bmpInfo, DIB_RGB_COLORS);
		BitBlt(hdc, gr_inf->x0,gr_inf->y0, gr_inf->w, gr_inf->h, hdc2, 0,0, SRCCOPY);
		DeleteObject(hdc2);
		DeleteObject(hb);
	} else {
		SetDIBitsToDevice(hdc, gr_inf->x0, gr_inf->y0, gr_inf->w, gr_inf->h, 0, 0,
			0, gr_inf->h, gr->pix_mem, &gr->bmpInfo, DIB_RGB_COLORS);
	}

	ReleaseDC(gr->hwnd,hdc);
}




/** 描画バッファクリア
 */
void __fastcall gr_cls(int c)
{
	if (gr) {
		unsigned *p = (unsigned *)gr->pix_mem;
		int l = gr_inf->w * gr_inf->h * sizeof(gr_pix_t)/sizeof(unsigned);
		do {
			*p++ = c;
		} while (--l);
	}
}



/** 窓移動時の再設定.
 */
void __fastcall gr_wm_move(HWND hwnd)
{
	RECT r, r0;

	if (gr == NULL || gr_inf->w == 0 || gr_inf->h == 0)
		return;

	GetClientRect(hwnd, &r);
	gr_inf->x0 = r.left;
	gr_inf->y0 = r.top;
	/*if (r.right-r.left != gr_inf->w || r.bottom-r.top != gr_inf->h)*/ {
		GetWindowRect(hwnd, &r0);
		gr->wx = r0.left;
		gr->wy = r0.top;
		r0.right  = r0.left + gr->ww;
		r0.bottom = r0.top	+ gr->wh;
		MoveWindow(hwnd, r0.left, r0.top, r0.right-r0.left, r0.bottom-r0.top, TRUE);
	}
}



/*-------------------------------------------*/

#define PX_SET_ALP(d,c, alp)	do {\
		int cc_=c;\
		int 		na_ = 0xff - alp, a_ = alp;\
		unsigned	r_,g_,b_;\
		r_	= (cc_ & 0xFF0000) * na_;\
		g_	= (cc_ & 0x00FF00) * na_;\
		b_	= (cc_ & 0x0000FF) * na_;\
		cc_ = *(gr_pix_t*)(d);\
		r_	= r_ + (cc_ & 0xFF0000) * a_;\
		g_	= g_ + (cc_ & 0x00FF00) * a_;\
		b_	= b_ + (cc_ & 0x0000FF) * a_;\
		r_	= (r_ >> 8) & 0xFF0000;\
		g_	= (g_ >> 8) & 0x00FF00;\
		b_	= (b_ >> 8) & 0x0000FF;\
		*(gr_pix_t*)(d) = (gr_pix_t)(r_|g_|b_);\
	} while(0)



/** 点を描画.
 */
static inline void pset(int x, int y, int c)
{
	if ((unsigned)x < (unsigned)gr_inf->w && (unsigned)y < (unsigned)gr_inf->h) {		// ※(unsigned)することで負の座標も範囲外にしている
		unsigned char *p = gr->pix + y * gr->lPitch + x * sizeof(gr_pix_t); 			//(x,y)座標のアドレスを設定
		*(gr_pix_t*)p = c;
	}
}



/** 描画範囲の調整.
 */
static inline void drawRectInChk(int *ax, int *ay, int *aw, int *ah, int *au, int *av)
{
	int 	da_x1 = 0, da_y1 = 0, da_x2 = gr_inf->w-1, da_y2 = gr_inf->h-1;
	int 	x,y, x2,y2;

	x	=  (*ax > da_x1) ? *ax : da_x1;
	y	=  (*ay > da_y1) ? *ay : da_y1;
	x2	=  (*ax+*aw < da_x2) ? *ax+*aw : da_x2;
	y2	=  (*ay+*ah < da_y2) ? *ay+*ah : da_y2;
	*au += x - *ax;
	*av += y - *ay;
	*ax = x;
	*ay = y;
	*aw = x2 - x;
	*ah = y2 - y;
}



/** (x,y) にピクセル cを表示.
 */
void __fastcall gr_pset(int x, int y, int c)
{
	//DBG_F(("gr_pset(%3d,%3d,%06x)\n",x,y,c));
	if (gr)
		pset(x,y, GR_RGB2PIX(c));
}



/** 線を描画.
 */
void __fastcall gr_line(int x0, int y0, int x1, int y1, int c)
{
	int w = 1 + x1 - x0, h = 1 + y1 - y0;
	int x,y,t,d;

	if (gr == NULL)
		return;
	//DBG_F(("gr_line(%3d,%3d,%3d,%3d, %06x)\n",x0,y0,x1,y1,c));
	c  = GR_RGB2PIX(c);
	if (abs(w) >= abs(h)) {
		if (w < 0) {
			w = -w;
			t = x0, x0 = x1, x1 = t;
			t = y0, y0 = y1, y1 = t;
			h = y1 - y0;
		}
		y = y0<<16;
		d = (w) ? ((h << 16) / w) : 0;
		for (x = x0; x <= x1; x++) {
			pset(x, y>>16, c);
			y += d;
		}
	} else {
		if (h < 0) {
			h = -h;
			t = x0, x0 = x1, x1 = t;
			t = y0, y0 = y1, y1 = t;
			w = x1 - x0;
		}
		x = x0 << 16;
		d = (h) ? ((w << 16) / h) : 0;
		for (y = y0; y <= y1; y++) {
			pset(x>>16, y, c);
			x += d;
		}
	}
}



/** 線で箱描画.
 */
void __fastcall gr_boxLine(int x0, int y0, int w, int h, int c)
{
	int x,y;

	if (gr == NULL)
		return;
	//DBG_F(("gr_boxLine(%3d,%3d,%3d,%3d, %06x)\n",x,y,w,h,c));
	c  = GR_RGB2PIX(c);
	for (x = x0; x < x0 + w; x++) {
		pset(x, y0	  , c);
		pset(x, y0+h-1, c);
	}
	for (y = y0+1; y < y0 + h-1; y++) {
		pset(x0,	 y, c);
		pset(x0+w-1, y, c);
	}
}



/** 塗りつぶした箱を描画.
 */
void __fastcall gr_boxFill(int x0, int y0, int w, int h, int c)
{
	int u = 0, v = 0;
	int x,y, d, a = (unsigned char)(c >> 24);

	if (gr == NULL)
		return;
	//DBG_F(("gr_boxFill(%3d,%3d,%3d,%3d, %06x)\n",x,y,w,h,c));
	c = GR_RGB2PIX(c);
	drawRectInChk(&x0, &y0, &w, &h, &u, &v);		// 表示範囲に矩形をレサイズ
	if (w > 0 && h > 0) {
		gr_pix_t *p = (gr_pix_t*)(gr->pix + y0 * gr->lPitch + x0 * sizeof(gr_pix_t));
		d = gr->lPitch - w*sizeof(gr_pix_t);
		y = h;
		if (a == 0) {
			do {
				x = w;
				do {
					*p++ = c;
				} while (--x);
				p = (gr_pix_t*)((char*)p + d);
			} while (--y);
	  #if 0
		} else {
			do {
				x = w;
				do {
					PX_SET_ALP(p, c, a);
					p++;
				} while (--x);
				p = (gr_pix_t*)((char*)p + d);
			} while (--y);
	  #endif
		}
	}
}


