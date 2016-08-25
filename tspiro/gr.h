#ifndef GR_H
#define GR_H
#ifdef __cplusplus
extern "C" {
#endif

#define GR_RGB(r,g,b)		(((unsigned char)(r)<<16)|((unsigned char)(g)<<8)|(unsigned char)(b))

typedef struct gr_inf_t {
	int 	w, h;			// 描画画面サイズ
	int 	x0,y0;			// 描画画面の表示画面での開始位置.
	int 	fs_w, fs_h; 	// 表示画面サイズ
	int 	mode;			// 0=センター表示  1=画面サイズに合わせる（拡大)
} gr_inf_t;

extern gr_inf_t *gr_inf;	// 画面情報
#define gr_width()		(gr_inf->w)
#define gr_height() 	(gr_inf->h)

#if defined(_WINDOWS_)
int __fastcall	gr_init(HWND hwnd, int w, int h, int md);
#endif
void __fastcall gr_term(void);
void __fastcall gr_update(void);
void __fastcall gr_cls(int c);
void __fastcall gr_wm_move(HWND hwnd);
void __fastcall gr_pset(int x, int y, int c);
void __fastcall gr_line(int x0, int y0, int x1, int y1, int c);
void __fastcall gr_boxLine(int x0, int y0, int w, int h, int c);
void __fastcall gr_boxFill(int x0, int y0, int w, int h, int c);

#ifdef __cplusplus
};
#endif
#endif
