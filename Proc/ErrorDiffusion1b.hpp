#ifndef ERRORDIFFUSION1B_HPP
#define ERRORDIFFUSION1B_HPP

#include <stdlib.h>

#undef ERRORDIFFUSION1B_SET_DIF
#define ERRORDIFFUSION1B_SET_DIF(buf,w,h,i,x,y,d) do {	\
	if (0 <= (y) && (y) < int(h)) {	\
		if (0 <= (x) && (x) < int(w))	\
			(buf)[(y)*(w)+(x)].pl[i].dif += (d);	\
	}	\
} while(0)

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

	struct ToneTbl {
		unsigned num_;
		int      tbl_[64 + 4];
	public:
		void init(unsigned n) {
			memset((void*)this, 0, sizeof *this);
			if (n <  2) n = 2;
			if (n > 64) n = 64;
			num_ = n;
			unsigned d = 0xffff / (n-1);
			for (unsigned i = 0; i < n - 1; ++i)
				tbl_[i] = i * d;
			tbl_[n-1] = 0xffff;
			tbl_[n]   = 0xffff;
		}

		unsigned toTone(int val, int dt, int& lum, int& dif) const {
			unsigned	bgn=0, end;
			unsigned	low = 0;
			unsigned    hi  = num_;
			while (low < hi) {
				bgn = low;
				end = hi;
				unsigned mid = (low + hi - 1) / 2;
				if (val <= tbl_[mid]) {
					hi = mid;
				} else { //if (tbl[mid] < key)
					bgn = low;
					end = hi;
					low = mid + 1;
				}
			}
			int fst = tbl_[bgn];
			int lst = tbl_[bgn+1];
			int len = lst - fst;
			int thr = fst + (len * dt >> 8);
			int cur = bgn + (val >= thr);
			lum = tbl_[cur];
			dif = val - lum;
			return bgn;
		}
	};

public:
	ErrorDiffusion1b() : buf_(NULL) {}
	~ErrorDiffusion1b() { free(buf_); }

	struct Opt {
		unsigned char	type;	// 0=�f�B�U�� 1=2x2 2=4x4 3=8x8.
		bool	 		alpha;	// 1:���v���[�����f�B�U���� 0:���Ȃ�.
		bool			errDif;	// �덷�g�U 1:���� 0:���Ȃ�.
		bool     		edRev;	// �덷�g�U�ŋ����񔽓] 1:����. 0:���Ȃ�.
		bool     		ditRev;	// AG��RB�ƂŃ}�g���N�X���t�� 1:���� 0:���Ȃ�.
		//Opt() : type(0), alpha(false), ditRev(false), edRev(false) {}
	};

	bool conv(
			unsigned*		dst,		///< �o�̓o�b�t�@
			const unsigned*	src,		///< ���̓o�b�t�@
			unsigned		w,			///< ����
			unsigned		h,			///< �c��
			unsigned		ditTyp,
			const unsigned*	tones		///< �K����. tones[3]
			//int 			flgs
	) {
		enum { R  = 0, G  = 1, B  = 2, A  = 3, };
		Pix* buf = (Pix*)realloc(buf_, w * h * sizeof(Pix));
		buf_ = buf;
		if (buf == NULL)
			return false;
		memset(buf, 0, w * h * sizeof(Pix));

		for (unsigned i = 0; i < 3; ++i)
			toneTbl_[i].init(tones[i]);

		for (unsigned y = 0; y < h; ++y) {
			for (unsigned x = 0; x < w; ++x) {
				unsigned ofs = y * w + x;
				unsigned c = src[ofs];
				Pix&     pix = buf[ofs];
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

	 #if 1 //defined(OTAMESHI_DIT)
		ditTyp = clamp(ditTyp, 0, 2);
		static const signed char dmPtn[3][4][4] = {
			//�p�^�[���f�B�U����
			{
				{ 128, 128, 128, 128,},
				{ 128, 128, 128, 128,},
				{ 128, 128, 128, 128,},
				{ 128, 128, 128, 128,},
			}, {	//Bayer 2x2 
				{ 0*64+32, 2*64+32, 0*64+32, 2*64+32,},
				{ 3*64+32, 1*64+32, 3*64+32, 1*64+32,},
				{ 0*64+32, 2*64+32, 0*64+32, 2*64+32,},
				{ 3*64+32, 1*64+32, 3*64+32, 1*64+32,},
			}, {	//Bayer 4x4
				{  0*16+8, 8*16+8,  2*16+8, 10*16+8,},
				{ 12*16+8, 4*16+8, 14*16+8,  6*16+8,},
				{  3*16+8,11*16+8,  1*16+8,  9*16+8,},
				{ 15*16+8, 7*16+8, 13*16+8,  5*16+8,},
			},
		};
	 #endif

		for (unsigned i = 0; i < 3; ++i) {
			ToneTbl& toneTbl = toneTbl_[i];
			for (unsigned y = 0; y < h; ++y) {
				int  add = 1;
				int  xb  = 0;
				int  xe  = w;
				if ((y & 1) ^ (i & 1)) {
					add = -1;
					xb  = w - 1;
					xe  = -1;
				}
				for (int x = xb; x != xe; x += add) {
					Plane&  pli  = buf[y * w + x].pl[i];
					dif_t   lum  = pli.lum;
					dif_t   dif  = pli.dif;
					dif_t   dl   = lum + dif;
				 #if 1 //defined(OTAMESHI_DIT)
					int 	dt = dmPtn[ditTyp][y & 3][x & 3];
				 #endif
					toneTbl.toTone(dl, dt, lum, dl);
					pli.dst = lum >> 8;
				 #if 0
				 #elif 1	// Floyd-Steinberg
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dl * 7 / 16);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, dl * 3 / 16);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, dl * 5 / 16);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, dl * 1 / 16);
				 #elif 0	// my3: Floyd-Steinberg mod. ref. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
					unsigned idx = clamp((f ? -dl : dl) >> 13, 0, 7);
					static const int ktbl[8][4] = {
						{ 256 - 3*28 - 5*28 - 1*28, 3*28, 5*28, 1*28 },
						//{ 256 - 3*26 - 5*26 - 1*26, 3*26, 5*26, 1*26 },
						{ 256 - 3*24 - 5*24 - 1*24, 3*24, 5*24, 1*24 },
						{ 256 - 3*20 - 5*20 - 1*20, 3*20, 5*20, 1*20 },
						{ 256 - 3*16 - 5*16 - 1*16, 3*16, 5*16, 1*16 },
						{ 256 - 3*12 - 5*12 - 1*12, 3*12, 5*12, 1*12 },
						{ 256 - 3* 8 - 5* 8 - 1* 8, 3* 8, 5* 8, 1* 8 },
						{ 256 - 3* 4 - 5* 4 - 1* 4, 3* 4, 5* 4, 1* 4 },
						//{ 256 - 3* 2 - 5* 2 - 1* 2, 3* 2, 5* 0, 1* 2 },
						{ 256 - 3* 0 - 5* 0 - 1* 0, 3* 0, 5* 0, 1* 0 },
					};
					int const* k = ktbl[idx];
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, k[0] * dl >> 8);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, k[1] * dl >> 8);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, k[2] * dl >> 8);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, k[3] * dl >> 8);
				 #elif 1	// see. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
					unsigned idx = clamp((f ? -dl : dl) >> (5+8), 0, 7);
					//unsigned idx = clamp((f ? -dl : dl) >> (4+8), 0, 7);
					static const int ktbl[8][4] = {
						{  9, 21, 35, 7 },	// [0]
						{ 18, 18, 30, 6 },	// [1]
						{ 27, 15, 25, 5 },	// [2]
						{ 36, 12, 20, 4 },	// [3]
						{ 45,  9, 15, 3 },	// [4]
						{ 54,  6, 10, 2 },	// [5]
						{ 63,  3,  5, 1 },	// [6]
						{ 72,  0,  0, 0 },	// [7]
					};
					int const* k = ktbl[idx];
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dl * k[0] / 72);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, dl * k[1] / 72);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, dl * k[2] / 72);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, dl * k[3] / 72);
				 #elif 1	// my1
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dl * 18 / 64);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+2*add, y+0, dl *  2 / 64);

					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1, y+1, dl * 12 / 64);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0, y+1, dl * 18 / 64);
					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1, y+1, dl * 12 / 64);

					ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0, y+2, dl *  2 / 64);
				 #endif
				}
			}
		}
		for (unsigned y = 0; y < h; ++y) {
			for (int x = 0; unsigned(x) < w; ++x) {
				Plane* pl = buf[y * w + x].pl;
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

	static int clamp(int val, int mi, int ma) {
		if (val < mi)
			return mi;
		else if (ma < val)
			return ma;
		return val;
	}

private:
	Pix* 	buf_;
	ToneTbl	toneTbl_[3];
};

#undef ERRORDIFFUSION1B_SET_DIF

#endif
