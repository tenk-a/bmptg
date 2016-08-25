#ifndef GR_H
#define GR_H
#ifdef __cplusplus
extern "C" {
#endif

#define GR_RGB(r,g,b)		(((unsigned char)(r)<<16)|((unsigned char)(g)<<8)|(unsigned char)(b))

typedef struct gr_inf_t {
	int 	w, h;			// �`���ʃT�C�Y
	int 	x0,y0;			// �`���ʂ̕\����ʂł̊J�n�ʒu.
	int 	fs_w, fs_h; 	// �\����ʃT�C�Y
	int 	mode;			// 0=�Z���^�[�\��  1=��ʃT�C�Y�ɍ��킹��i�g��)
} gr_inf_t;

extern gr_inf_t *gr_inf;	// ��ʏ��
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
