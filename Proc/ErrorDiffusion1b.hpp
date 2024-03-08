#ifndef ERRORDIFFUSION1B_HPP
#define ERRORDIFFUSION1B_HPP

#include <stdlib.h>

class ErrorDiffusion1b {
	typedef unsigned short  lum_t;
	typedef int    			dif_t;
	typedef unsigned char	dst_t;
	struct Plane {
		dif_t	dif;
		lum_t	lum;
		dst_t	dst;
		dst_t	sub;
	};
	struct Pix {
		Plane	pl[3];
	};

public:
	ErrorDiffusion1b() : buf_(NULL) {}
	~ErrorDiffusion1b() { free(buf_); }

	bool conv(
			unsigned*		dst,		///< 出力バッファ
			const unsigned*	src,		///< 入力バッファ
			unsigned		w,			///< 横幅
			unsigned		h			///< 縦幅
			//int 			ditBpp,		///< bpp 出力の色のビット数.
			//int 			flgs
	) {
		enum { R  = 0, G  = 1, B  = 2, A  = 3, };
		buf_ = (Pix*)realloc(buf_, w * h * sizeof(Pix));
		if (buf_ == NULL)
			return false;
		memset(buf_, 0, w * h * 4 * sizeof(short));
		for (unsigned y = 0; y < h; ++y) {
			for (unsigned x = 0; x < w; ++x) {
				unsigned ofs = y * w + x;
				unsigned c = src[ofs];
				Pix&     pix = buf_[ofs];
				unsigned b = argb_b(c);
				unsigned g = argb_g(c);
				unsigned r = argb_r(c);
				unsigned a = argb_a(c);
				pix.pl[R].lum = (r << 8) | r;
				pix.pl[G].lum = (g << 8) | g;
				pix.pl[B].lum = (b << 8) | b;
				pix.pl[0].sub = a;
			}
		}

		for (unsigned i = 0; i < 3; ++i) {
			for (unsigned y = 0; y < h; ++y) {
				bool rev = (y & 1) ^ (i & 1);
//				bool rev = false;
				if (!rev) {
					for (int x = 0; unsigned(x) < w; ++x) {
						Plane&  pli   = buf_[y * w + x].pl[i];
						dif_t   lum  = pli.lum;
						dif_t   dif  = pli.dif;
						dif_t   dl   = lum + dif;
//						dif_t   dl   = lum;
						if (dl >= 0x8000) {
							dl = dl - 65535/*0xffff*/;
							if (dl > 0) dl = 0;
							pli.dst = 0xff;
						} else {
							//dl = dl;
							if (dl < 0) dl = 0;
							pli.dst = 0;
						}
						if ((x + 1) < w) {
							buf_[(y + 0) * w + (x + 1)].pl[i].dif += dl * 9 / 32;
							if ((x + 2) < w)
								buf_[(y + 0) * w + (x + 2)].pl[i].dif += dl * 1 / 32;
						}
						if ((y + 1) < h) {
							if ((x - 1) >= 0)
								buf_[(y + 1) * w + (x - 1)].pl[i].dif += dl * 6 / 32;
							//if (1)
								buf_[(y + 1) * w + (x + 0)].pl[i].dif += dl * 9 / 32;
							if ((x + 1) < w)
								buf_[(y + 1) * w + (x + 1)].pl[i].dif += dl * 6 / 32;
							if ((y + 2) < h)
								buf_[(y + 2) * w + (x + 0)].pl[i].dif += dl * 1 / 32;
						}
					}
				} else {
					for (int x = int(w); --x >= 0;) {
						Plane&  pli   = buf_[y * w + x].pl[i];
						dif_t   lum  = pli.lum;
						dif_t   dif  = pli.dif;
						dif_t   dl   = lum + dif;
//						dif_t   dl   = lum;
						if (dl >= 0x8000) {
							dl = dl - 65535/*0xffff*/;
							//if (dl > 0) dl = 0;
							pli.dst = 0xff;
						} else {
							//dl = dl;
							//if (dl < 0) dl = 0;
							pli.dst = 0;
						}
						if ((x - 1) >= 0) {
							buf_[(y + 0) * w + (x - 1)].pl[i].dif += dl * 9 / 32;
							if ((x - 2) >= 0)
								buf_[(y + 0) * w + (x - 2)].pl[i].dif += dl * 1 / 32;
						}
						if ((y + 1) < h) {
							if ((x + 1) < w)
								buf_[(y + 1) * w + (x + 1)].pl[i].dif += dl * 6 / 32;
							//if (1)
								buf_[(y + 1) * w + (x + 0)].pl[i].dif += dl * 9 / 32;
							if ((x - 1) >= 0)
								buf_[(y + 1) * w + (x - 1)].pl[i].dif += dl * 6 / 32;
							if ((y + 2) < h)
								buf_[(y + 2) * w + (x + 0)].pl[i].dif += dl * 1 / 32;
						}
					}
				}
			}
		}
		for (unsigned y = 0; y < h; ++y) {
			for (int x = 0; unsigned(x) < w; ++x) {
				Plane* pl = buf_[y * w + x].pl;
				dst[y * w + x] = argb(pl[0].sub, pl[R].dst, pl[G].dst, pl[B].dst);
			}
		}
		return true;
	}

private:
	static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
		return (a << 24) | (r << 16) | (g << 8) | (b);
	}

	static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
	static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
	static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
	static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

private:
	Pix* buf_;
};

#endif
