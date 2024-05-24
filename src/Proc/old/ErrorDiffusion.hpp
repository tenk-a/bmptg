#ifndef ERRORDIFFUSION_HPP
#define ERRORDIFFUSION_HPP

#include <stdlib.h>

#undef ERRORDIFFUSION_SET_DIF
#ifdef NDEBUG
#define ERRORDIFFUSION_SET_DIF(buf,w,h,i,x,y,d) do {  \
    if (0 <= (y) && (y) < int(h)) { \
        if (0 <= (x) && (x) < int(w)) {   \
            Plane& pp = (buf)[(y)*(w)+(x)].pl[i]; \
            pp.dif += (d);    \
        } \
    }   \
} while(0)
#else
#define ERRORDIFFUSION_SET_DIF(buf,w,h,i,x,y,d)   setDif(buf, w, h, i, x, y, d)
#endif


class ErrorDiffusion {
    typedef unsigned short  lum_t;
    typedef int             dif_t;
    typedef unsigned char   dst_t;

    struct Plane {
        dif_t   dif;
        lum_t   lum;
        dst_t   dst;
        dst_t   sub;
        dst_t	idx;
    };
    struct Pix {
        Plane   pl[3];
    };

    struct ToneTbl {
        unsigned num_;
        int      tbl_[64 + 4];
    public:
		void init(unsigned const tbl[64], unsigned n) {
			assert(2 <= n && n <= 64);
            memset((void*)this, 0, sizeof *this);
            num_ = n;
            for (unsigned i = 0; i < n; ++i)
            	tbl_[i] = int(tbl[i]);
            tbl_[n] = 0xffff;
		}

        void init(unsigned n, bool xterm) {
            memset((void*)this, 0, sizeof *this);
            if (n <  2) n = 2;
            if (n > 64) n = 64;
            num_ = n;
            if (xterm && n == 6) {
                static const int xtbl[6] = { 0x0000,     0x5f5f,0x8787,0xafaf,0xd7d7,0xffff, };
                for (unsigned i = 0; i < 6; ++i)
                    tbl_[i] = xtbl[i];
            } else {
                unsigned d = 0xffff / (n-1);
                for (unsigned i = 0; i < n - 1; ++i)
                    tbl_[i] = i * d;
            }
            tbl_[n-1] = 0xffff;
            tbl_[n]   = 0xffff;
        }

        unsigned toLumIdx(int val) const {
            unsigned    low = 0;
            unsigned    hi  = num_;
            unsigned    mid = 0;
            while (low < hi) {
                mid = (low + hi) / 2;
                if (val < tbl_[mid]) {
                    hi = mid;
                } else {
                    low = mid + 1;
                }
            }
			return mid;
		}

        unsigned valToIdx(int val, int dt) const {
            unsigned idx = toLumIdx(val);
            int fst = tbl_[idx];
            int lst = tbl_[idx+1];
            int len = lst - fst;
            int thr = fst + (len * dt >> 8);
            idx += (val >= thr);
			return idx;
        }

        unsigned idxValToLum(int idx, int val, int& dif) const {
            unsigned lum = tbl_[idx];
            dif = val - lum;
            return lum;
        }
    };

	struct Rgb333ToIdx16 {
		Rgb333ToIdx16() {
			for (unsigned g = 0; g < 3; ++g) {
				for (unsigned r = 0; r < 3; ++r) {
					for (unsigned b = 0; b < 3; ++b) {
						if (  (g == r && b == 0)
							||(b == g && r == 0)
							||(r == b && g == 0)
						) {
							tbl_[g][r][b] = (g << 16) | (r << 8) | b;
						} else {
							unsigned g2 = g > 0 ? 2 : 0;
							unsigned r2 = r > 0 ? 2 : 0;
							unsigned b2 = b > 0 ? 2 : 0;
							tbl_[g][r][b] = (g2 << 16) | (r2 << 8) | b2;
						}
					}
				}
			}
		}
		void adjust(uint8_t& ri, uint8_t& gi, uint8_t& bi) {
			unsigned t = tbl_[ri][gi][bi];
			ri = uint8_t(t >> 16);
			gi = uint8_t(t >>  8);
			bi = uint8_t(t >>  0);
		}
	public:
		uint32_t	tbl_[3][3][3];
	};

public:
    ErrorDiffusion() : buf_(NULL)
    {}
    ~ErrorDiffusion() { free(buf_); }

	enum {
		F_XTERM 	= 0x10,
		F_MONO  	= 0x100,
		F_ERRDIF	= 0x1000,
		F_444TO16	= 0x1000000,
	};

    bool conv(
            unsigned*       dst,        	///< 出力バッファ.
            const unsigned* src,        	///< 入力バッファ.
            unsigned        w,          	///< 横幅.
            unsigned        h,    			///< 縦幅.
            unsigned        ditTyp,
            unsigned const  toneSizes[3],	///< 階調数. tones[3]
            unsigned const  tones[3][64],	///< 階調.
            uint32_t const* monoClut = NULL
    ) {
        enum { R  = 0, G  = 1, B  = 2, A  = 3, };
        Pix* buf = (Pix*)realloc(buf_, w * h * sizeof(Pix));
        buf_ = buf;
        if (buf == NULL)
            return false;
        memset(buf, 0, w * h * sizeof(Pix));

        bool rgb444toC16 = (ditTyp & F_444TO16);
        bool errDif = (ditTyp & F_ERRDIF) == 0;   // noErrDif フラグが立っていなければ誤差拡散する.
        bool mono   = (ditTyp & F_MONO) != 0;
        bool xterm  = (ditTyp & F_XTERM) != 0;
		if (tones) {
	        for (unsigned i = 0; i < 3; ++i)
    	        toneTbl_[i].init(tones[i], toneSizes[i]);
		} else {
	        for (unsigned i = 0; i < 3; ++i)
    	        toneTbl_[i].init(toneSizes[i], xterm);
    	}

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

        ditTyp &= 3;

        unsigned plane = mono ? 1 : 3;
        for (unsigned y = 0; y < h; ++y) {
            int  add = 1;
            int  xb  = 0;
            int  xe  = w;
          #if 0
            //bool rev = ((y & 1) ^ (i & 1));
            bool rev = (y & 1);
            if (rev) {
                add = -1;
                xb  = w - 1;
                xe  = -1;
            }
          #endif
			unsigned char colidx[3] = {0};
            unsigned vals[3] = {0};
            for (int x = xb; x != xe; x += add) {
				Pix& pi = buf[y * w + x];
		        for (unsigned i = 0; i < plane; ++i) {
                    int     dt  = getDmPtn(ditTyp, x & 7, y & 7);
		            ToneTbl& toneTbl = toneTbl_[i];
                    Plane&  pli = pi.pl[i];
                    dif_t   lum = pli.lum;
                    dif_t   dif = pli.dif;
                    dif_t   val = lum + dif;
                    vals[i]     = val;
                    unsigned idx= toneTbl.valToIdx(val, dt);
                    colidx[i]   = (idx < toneSizes[i]) ? idx : toneSizes[i] - 1;
                }
                if (rgb444toC16) {	// Digital RGB333 to 16 color
                    rgb333toIdx16_.adjust(colidx[0],colidx[1],colidx[2]);
                }
		        for (unsigned i = 0; i < plane; ++i) {
		            ToneTbl& toneTbl = toneTbl_[i];
                    Plane&  pli = pi.pl[i];
                    unsigned ci = colidx[i];
                    pli.idx     = ci;
                    int     dif;
					dif_t   lum = toneTbl.idxValToLum(ci, vals[i], dif);
                    pli.dst     = lum >> 8;
                    if (errDif) {
                     #if 0    // Floyd-Steinberg
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+0, dif * 7 / 16);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x-1*add, y+1, dif * 3 / 16);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+0    , y+1, dif * 5 / 16);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+1, dif * 1 / 16);
                     #elif 1    // my3: Floyd-Steinberg mod. ref. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
                        unsigned kidx = clamp((dif<0 ? -dif : dif) >> 13, 0, 7);
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
                        int const* k = ktbl[kidx];
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+0, k[0] * dif >> 8);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x-1*add, y+1, k[1] * dif >> 8);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+0    , y+1, k[2] * dif >> 8);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+1, k[3] * dif >> 8);
                     #elif 1    // see. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
                        unsigned kidx = clamp((f ? -dif : dif) >> (5+8), 0, 7);
                        //unsigned kidx = clamp((f ? -dif : dif) >> (4+8), 0, 7);
                        static const int ktbl[8][4] = {
                            {  9, 21, 35, 7 },  // [0]
                            { 18, 18, 30, 6 },  // [1]
                            { 27, 15, 25, 5 },  // [2]
                            { 36, 12, 20, 4 },  // [3]
                            { 45,  9, 15, 3 },  // [4]
                            { 54,  6, 10, 2 },  // [5]
                            { 63,  3,  5, 1 },  // [6]
                            { 72,  0,  0, 0 },  // [7]
                        };
                        int const* k = ktbl[kidx];
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+0, dif * k[0] / 72);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x-1*add, y+1, dif * k[1] / 72);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+0    , y+1, dif * k[2] / 72);
                        ERRORDIFFUSION_SET_DIF(buf,w,h,i, x+1*add, y+1, dif * k[3] / 72);
                     #endif
                    }
                }
            }
        }

        //pixBufToPix32(dst, buf, w, h);
		if (mono == false) {
	        for (unsigned y = 0; y < h; ++y) {
	            for (int x = 0; unsigned(x) < w; ++x) {
	                Plane* pl = buf[y * w + x].pl;
	                uint8_t r = pl[R].dst;
	                uint8_t g = pl[G].dst;
	                uint8_t b = pl[B].dst;
	                dst[y * w + x] = argb(pl[0].sub, r, g, b);
	            }
	        }
		} else if (monoClut) {
	        for (unsigned y = 0; y < h; ++y) {
	            for (int x = 0; unsigned(x) < w; ++x) {
	                Plane* pl = buf[y * w + x].pl;
	                uint8_t i = pl[R].idx;
	                dst[y * w + x] = monoClut[i];
	            }
	        }
		} else {
	        for (unsigned y = 0; y < h; ++y) {
	            for (int x = 0; unsigned(x) < w; ++x) {
	                Plane* pl = buf[y * w + x].pl;
	                uint8_t i = pl[R].dst;
	                dst[y * w + x] = argb(pl[0].sub, i, i, i);
	            }
	        }
		}
        return true;
    }


    bool convDigital8(
            unsigned*       dst,        ///< 出力バッファ.
            const unsigned* src,        ///< 入力バッファ.
            unsigned        w,          ///< 横幅.
            unsigned        h,          ///< 縦幅.
            unsigned        ditTyp,
            unsigned		colNum,
            uint32_t		monoCol = 0xFFffffff
    ) {
        unsigned monoColIdx = ((monoCol >> (16+7)) & 1) << 1
                            | ((monoCol >> ( 8+7)) & 1) << 2
                            | ((monoCol >> (   7)) & 1) ;
        bool mono   = (ditTyp & 0x100) != 0;
		if (mono || colNum < 4) {
			if (colNum == 2) {
				unsigned toneSize[3] = { 2, 2, 2, };
				uint32_t clut[2] = { 0, monoCol };
				return conv(dst, src, w, h, ditTyp, toneSize, NULL, clut);
			} else if (colNum == 4 && (monoColIdx == 7 || monoColIdx == 0 || monoColIdx == 5 || monoColIdx == 1)) {
		      #if 1
		        static const uint32_t clut[] = { 0xFF000000, 0xFF0000ff, 0xFF00ffff, 0xFFffffff, };	// 黒 青 水 白.
		        //enum { K=0, B=0xFFFF*1/10, S=0xFFFF*(1+6)/10, W=0xFFFF };   // G6R3B1 黒青水白.
		        //enum { K=0, B=0xFFFF*2/16, S=0xFFFF*(2+9)/16, W=0xFFFF };   // G9R5B2 黒青水白.
		        enum { K=0, B=0xFFFF*3/16, S=0xFFFF*(3+8)/16, W=0xFFFF };   // G8R5B2 黒青水白.
		      #endif
				static unsigned const toneTbl[3][64] = {
		            { K, B, S, W },
		            { K, B, S, W },
		            { K, B, S, W },
				};
		        static unsigned const toneNums[3] = { 4, 4, 4, };
		        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, toneNums, toneTbl, clut);
			} else /*if (colNum == 3)*/ {
				uint32_t W = 0xFFffffff;
    			if (monoColIdx == 7) {
	    		    monoCol = 0xff00ffff;	// 水色.
	    		} else if (monoColIdx == 0) {
	    		    monoCol = 0xff0000ff;	// 青.
	    			W = 0xFF00FFFF;			// 水色.
	    		}
                const uint32_t clut[] = { 0xFF000000, 0xFF000000|monoCol, W, };	// 黒 (色) 白.
                unsigned B = (uint8_t(monoCol >> 8) * 6 + uint8_t(monoCol >> 16) * 3 + uint8_t(monoCol) * 1) / 10u;
                B = (B << 8) | B;
				unsigned const toneTbl[3][64] = {
		            { 0, B, 0xFFFF },
		            { 0, B, 0xFFFF },
		            { 0, B, 0xFFFF },
				};
		        static unsigned const toneNums[3] = { 3, 3, 3, };
		        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, toneNums, toneTbl, clut);
			}
		} else {
	        static unsigned const toneNums[3] = { 2, 2, 2, };
	        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, toneNums, NULL);
		}
        return true;
	}

    bool convDigital16(
            unsigned*       dst,        ///< 出力バッファ.
            const unsigned* src,        ///< 入力バッファ.
            unsigned        w,          ///< 横幅.
            unsigned        h,          ///< 縦幅.
            unsigned        ditTyp,
            unsigned		colNum,
            uint32_t		monoCol = 0xffffff
    ) {
        static unsigned const monoColBase[] = {
            0x000000, 0x0000ff, 0xff0000, 0xff00ff,
            0x00ff00, 0x00ffff, 0xffff00, 0xffffff,
        };
        static unsigned const monoCols4[][4] = {
            { 0x000000, 0x888888, 0xcccccc, 0xffffff },
            { 0x000000, 0x000088, 0x0000ff, 0xffffff },
            { 0x000000, 0x880000, 0xff0000, 0xffffff },
            { 0x000000, 0x880088, 0xff00ff, 0xffffff },
            { 0x000000, 0x008800, 0x00ff00, 0xffffff },
            { 0x000000, 0x008888, 0x00ffff, 0xffffff },
            { 0x000000, 0x888800, 0xffff00, 0xffffff },
            { 0x000000, 0x888888, 0xcccccc, 0xffffff },
        };
        if (colNum == 0)
            colNum = 256;
        unsigned monoColIdx = (((monoCol >> (16+7)) & 1) << 1)
                            | (((monoCol >> ( 8+7)) & 1) << 2)
                            |  ((monoCol >> (   7)) & 1);
        monoCol   = monoColBase[monoColIdx];
        bool mono = (ditTyp & 0x100) != 0;
		if (mono || colNum < 4) {
		    enum { K=0, /* B=0xFFFF*8/16, */ W=0xFFFF };
            unsigned C = (uint8_t(monoCol >> 8) * 6 + uint8_t(monoCol >> 16) * 3 + uint8_t(monoCol) * 1) / 10u;
            unsigned B = C >> 1;
            C = (C << 8) | C;
            B = (B << 8) | B;
			if (colNum == 2) {
				unsigned toneNums[3] = { 2, 2, 2, };
				uint32_t clut[2] = { 0, monoCol };
				return conv(dst, src, w, h, ditTyp, toneNums, NULL, clut);
			} else if (colNum == 3) {
			    //if (monoColIdx == 0 || monoColIdx == 7) { B = 0x8888; }
				unsigned const toneTbl[3][64] = {
		            { 0, 0x8888/*B*/, 0xffff },
		            { 0, 0x8888/*B*/, 0xffff },
		            { 0, 0x8888/*B*/, 0xffff },
				};
		        static unsigned const toneNums[3] = { 3, 3, 3, };
		        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, toneNums, toneTbl, monoCols4[monoColIdx]);
			} else {	// colNum4
			    if (monoColIdx == 0 || monoColIdx == 7) {
				    C = 0xcccc;
				    B = 0x8888;
			    }
				static unsigned const toneTbl[3][64] = {
		            { 0, B, C, 0xFFFF },
		            { 0, B, C, 0xFFFF },
		            { 0, B, C, 0xFFFF },
				};
		        static unsigned const toneNums[3] = { 4, 4, 4, };
		        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, toneNums, toneTbl, monoCols4[monoColIdx]);
			}
		} else {
	        static unsigned const toneNums[3] = { 3, 3, 3, };
	        return conv((uint32_t*)dst, (uint32_t const*)src, w, h, F_444TO16 | ditTyp, toneNums, NULL);
		}
        return true;
	}

private:
#if 0
	void pixBufToPix32(uint32_t* dst, Pix* buf, size_t w, size_t h) {
        for (unsigned y = 0; y < h; ++y) {
            for (int x = 0; unsigned(x) < w; ++x) {
                Plane* pl = buf[y * w + x].pl;
                unsigned char r, g, b;
                r = pl[R].dst;
                if (!mono) {
                    g = pl[G].dst;
                    b = pl[B].dst;
                } else {
                    b = g = r;
                }
                dst[y * w + x] = argb(pl[0].sub, r, g, b);
            }
        }
	}
#endif

    static void setDif(Pix* buf, size_t w, size_t h, int i, int x, int y, int d) {
        if (0 <= (y) && (y) < int(h)) { 
            if (0 <= (x) && (x) < int(w)) {   
                Plane& pp = (buf)[(y)*(w)+(x)].pl[i]; 
                pp.dif += (d);    
            } 
        }   
    }

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


	static short getDmPtn(unsigned typ, unsigned x, unsigned y) {
        static const short dmPtn[4][8][8] = {
            {       // パターンディザ無し.
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
                { 128, 128, 128, 128, 128, 128, 128, 128, },
            }, {    //Bayer 2x2
                { 0*64+32, 2*64+32, 0*64+32, 2*64+32,  0*64+32, 2*64+32, 0*64+32, 2*64+32, },   // 1 5
                { 3*64+32, 1*64+32, 3*64+32, 1*64+32,  3*64+32, 1*64+32, 3*64+32, 1*64+32, },   // 7 3
                { 0*64+32, 2*64+32, 0*64+32, 2*64+32,  0*64+32, 2*64+32, 0*64+32, 2*64+32, },
                { 3*64+32, 1*64+32, 3*64+32, 1*64+32,  3*64+32, 1*64+32, 3*64+32, 1*64+32, },
                { 0*64+32, 2*64+32, 0*64+32, 2*64+32,  0*64+32, 2*64+32, 0*64+32, 2*64+32, },   // 1 5
                { 3*64+32, 1*64+32, 3*64+32, 1*64+32,  3*64+32, 1*64+32, 3*64+32, 1*64+32, },   // 7 3
                { 0*64+32, 2*64+32, 0*64+32, 2*64+32,  0*64+32, 2*64+32, 0*64+32, 2*64+32, },
                { 3*64+32, 1*64+32, 3*64+32, 1*64+32,  3*64+32, 1*64+32, 3*64+32, 1*64+32, },
            }, {    //Bayer 4x4
                {  0*16+8, 8*16+8,  2*16+8, 10*16+8,   0*16+8, 8*16+8,  2*16+8, 10*16+8, },
                { 12*16+8, 4*16+8, 14*16+8,  6*16+8,  12*16+8, 4*16+8, 14*16+8,  6*16+8, },
                {  3*16+8,11*16+8,  1*16+8,  9*16+8,   3*16+8,11*16+8,  1*16+8,  9*16+8, },
                { 15*16+8, 7*16+8, 13*16+8,  5*16+8,  15*16+8, 7*16+8, 13*16+8,  5*16+8, },
                {  0*16+8, 8*16+8,  2*16+8, 10*16+8,   0*16+8, 8*16+8,  2*16+8, 10*16+8, },
                { 12*16+8, 4*16+8, 14*16+8,  6*16+8,  12*16+8, 4*16+8, 14*16+8,  6*16+8, },
                {  3*16+8,11*16+8,  1*16+8,  9*16+8,   3*16+8,11*16+8,  1*16+8,  9*16+8, },
                { 15*16+8, 7*16+8, 13*16+8,  5*16+8,  15*16+8, 7*16+8, 13*16+8,  5*16+8, },
            }, {    //Bayer 8x8
                {   0*4+2, 48*4+2, 12*4+2, 60*4+2,  3*4+2, 51*4+2, 15*4+2, 63*4+2, },
                {  32*4+2, 16*4+2, 44*4+2, 28*4+2, 35*4+2, 19*4+2, 47*4+2, 31*4+2, },
                {   8*4+2, 56*4+2,  4*4+2, 52*4+2, 11*4+2, 59*4+2,  7*4+2, 55*4+2, },
                {  40*4+2, 24*4+2, 36*4+2, 20*4+2, 43*4+2, 27*4+2, 39*4+2, 23*4+2, },
                {   2*4+2, 50*4+2, 14*4+2, 62*4+2,  1*4+2, 49*4+2, 13*4+2, 61*4+2, },
                {  34*4+2, 18*4+2, 46*4+2, 30*4+2, 33*4+2, 17*4+2, 45*4+2, 29*4+2, },
                {  10*4+2, 58*4+2,  6*4+2, 54*4+2,  9*4+2, 57*4+2,  5*4+2, 53*4+2, },
                {  42*4+2, 26*4+2, 38*4+2, 22*4+2, 41*4+2, 25*4+2, 37*4+2, 21*4+2, },
            },
        };
		return dmPtn[typ][y][x];
	}

private:
    Pix*    		buf_;
    ToneTbl 		toneTbl_[3];
	Rgb333ToIdx16	rgb333toIdx16_;
};

#undef ERRORDIFFUSION_SET_DIF

#endif
