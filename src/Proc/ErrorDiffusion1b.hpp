#ifndef ERRORDIFFUSION1B_HPP
#define ERRORDIFFUSION1B_HPP

#include <stdlib.h>
#include <assert.h>

#undef ERRORDIFFUSION1B_SET_DIF
#ifdef NDEBUG
#define ERRORDIFFUSION1B_SET_DIF(buf,w,h,i,x,y,d) do {  \
    if (0 <= (y) && (y) < int(h)) { \
        if (0 <= (x) && (x) < int(w)) {   \
            Plane& pp = (buf)[(y)*(w)+(x)].pl[i]; \
            pp.dif += (d);    \
        } \
    }   \
} while(0)
#else
#define ERRORDIFFUSION1B_SET_DIF(buf,w,h,i,x,y,d)   setDif(buf, w, h, i, x, y, d)
#endif

class ErrorDiffusion1b {
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
                static const int xtbl[6] = { 0x0000,0x5f5f,0x8787,0xafaf,0xd7d7,0xffff, };
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

        unsigned toLumDif(int val, int dt, int& lum, int& dif) const {
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
            unsigned cur = mid;
            int fst = tbl_[cur];
            int lst = tbl_[cur+1];
            int len = lst - fst;
            int thr = fst + (len * dt >> 8);
            cur += (val >= thr);
            lum = tbl_[cur];
            dif = val - lum;
            return cur;
        }
    };

public:
    ErrorDiffusion1b() : buf_(NULL) {}
    ~ErrorDiffusion1b() { free(buf_); }

 #if 0
    struct Opt {
        unsigned char   type;   // 0=ディザ無 1=2x2 2=4x4 3=8x8.
        bool            mono;
        bool            errDif; // 誤差拡散 1:する 0:しない.
        bool            edRev;  // 誤差拡散で偶数列反転 1:する. 0:しない.
        bool            ditRev; // AGとRBとでマトリクスを逆に 1:する 0:しない.
        //Opt() : type(0), alpha(false), ditRev(false), edRev(false) {}
    };
 #endif

    bool conv(
            unsigned*       dst,        ///< 出力バッファ.
            const unsigned* src,        ///< 入力バッファ.
            unsigned        w,          ///< 横幅.
            unsigned        h,          ///< 縦幅.
            unsigned        ditTyp,
            unsigned        dpp,
            unsigned const  toneSizes[3], ///< 階調数. tones[3]
            unsigned const  tones[3][64]  ///< 階調.
            //int           flgs
    ) {
        enum { R  = 0, G  = 1, B  = 2, A  = 3, };
        Pix* buf = (Pix*)realloc(buf_, w * h * sizeof(Pix));
        buf_ = buf;
        if (buf == NULL)
            return false;
        memset(buf, 0, w * h * sizeof(Pix));
        bool errDif = (ditTyp & 0x1000) == 0;   // noErrDif フラグが立っていなければ誤差拡散する.
        bool mono   = (ditTyp & 0x100) != 0;
        bool xterm  = (ditTyp & 0x10) != 0;
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

     #if 1 //defined(OTAMESHI_DIT)
        ditTyp &= 3;
        //ditTyp = clamp(ditTyp, 0, 3);
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
     #endif

        unsigned plane = mono ? 1 : 3;
        for (unsigned i = 0; i < plane; ++i) {
            ToneTbl& toneTbl = toneTbl_[i];
            for (unsigned y = 0; y < h; ++y) {
                int  add = 1;
                int  xb  = 0;
                int  xe  = w;
                //bool rev = ((y & 1) ^ (i & 1));
                bool rev = (y & 1);
                if (rev) {
                    add = -1;
                    xb  = w - 1;
                    xe  = -1;
                }
                for (int x = xb; x != xe; x += add) {
                    Plane&  pli = buf[y * w + x].pl[i];
                    dif_t   lum = pli.lum;
                    dif_t   dif = pli.dif;
                 #if 1 //defined(OTAMESHI_DIT)
                    int     dt  = dmPtn[ditTyp][y & 7][x & 7];
                 #endif
                    pli.idx = toneTbl.toLumDif(lum + dif, dt, lum, dif);
                    pli.dst = lum >> 8;
                    if (errDif) {
                     #if 0
                     #elif 0    // Floyd-Steinberg
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dif * 7 / 16);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, dif * 3 / 16);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, dif * 5 / 16);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, dif * 1 / 16);
                     #elif 1    // my3: Floyd-Steinberg mod. ref. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
                        unsigned idx = clamp((dif<0 ? -dif : dif) >> 13, 0, 7);
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
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, k[0] * dif >> 8);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, k[1] * dif >> 8);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, k[2] * dif >> 8);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, k[3] * dif >> 8);
                     #elif 1    // see. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
                        unsigned idx = clamp((f ? -dif : dif) >> (5+8), 0, 7);
                        //unsigned idx = clamp((f ? -dif : dif) >> (4+8), 0, 7);
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
                        int const* k = ktbl[idx];
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dif * k[0] / 72);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1*add, y+1, dif * k[1] / 72);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0    , y+1, dif * k[2] / 72);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+1, dif * k[3] / 72);
                     #elif 1    // my1
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1*add, y+0, dif * 18 / 64);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+2*add, y+0, dif *  2 / 64);

                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x-1, y+1, dif * 12 / 64);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0, y+1, dif * 18 / 64);
                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+1, y+1, dif * 12 / 64);

                        ERRORDIFFUSION1B_SET_DIF(buf,w,h,i, x+0, y+2, dif *  2 / 64);
                     #endif
                    }
                }
            }
        }
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
        return true;
    }


    bool convDigital8(
            unsigned*       dst,        ///< 出力バッファ.
            const unsigned* src,        ///< 入力バッファ.
            unsigned        w,          ///< 横幅.
            unsigned        h,          ///< 縦幅.
            unsigned        ditTyp,
            unsigned        dpp,
            unsigned		colNum
    ) {
		if (colNum == 4) {
	      #if 1
	        static const unsigned clut[] = { 0xFF000000, 0xFF0000ff, 0xFF00ffff, 0xFFffffff, };	// 黒 青 水 白.
	        //enum { K=0, B=0xFFFF*1/10, S=0xFFFF*(1+6)/10, W=0xFFFF };   // G6R3B1 黒青水白.
	        //enum { K=0, B=0xFFFF*2/16, S=0xFFFF*(2+9)/16, W=0xFFFF };   // G9R5B2 黒青水白.
	        enum { K=0, B=0xFFFF*3/16, S=0xFFFF*(3+8)/16, W=0xFFFF };   // G8R5B2 黒青水白.
	      #elif 0
	        static const unsigned clut[] = { 0xFF000000, 0xFF0000ff, 0xFFff00ff, 0xFFffffff, };	// 黒 青 紫 白.
	        enum { K=0, B=0xFFFF*1/10, S=0xFFFF*(1+3)/10, W=0xFFFF };   // G6R3B1 黒青紫白.
	      #else
	        // static const unsigned clut[] = { 0xFF000000, 0xFF0000ff, 0xFFff00ff, 0xFFff0000, };
	        // enum { K=0, B=0xFFFF*2/16, S=0xFFFF*(2+5)/16, W=0xFFFF };   // G9R5B2 黒青紫赤.
	      #endif
			static unsigned const toneTbl[3][64] = {
	            { K, B, S, W },
	            { K, B, S, W },
	            { K, B, S, W },
			};
	        static unsigned const toneNums[3] = { 4, 4, 4, };

	        conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, dpp, toneNums, toneTbl);

	        enum { d = 255 / 4 };
	        enum { KB = K >> 8, BB = B >> 8, SB = S >> 8, WB = W >> 8 };
	        uint32_t* p = (uint32_t*)dst;
			for (size_t y = 0; y < h; ++y) {
				for (size_t x = 0; x < w; ++x) {
					unsigned c = p[ y * w + x ];
	                c &= 0xff;
					unsigned i = (c < BB) ? 0 : (c < SB) ? 1 : (c < WB) ? 2 : 3;
				 #if 1
	                static int s_i = 0;
	                if (i == 1)
	                    ++s_i;
	                if (i == 2)
	                    ++s_i;
	                if (i == 3)
	                    ++s_i;
	             #endif
	                c = clut[i];
					p[ y * w + x ] = c;
				}
			}
		} else {
	        static const unsigned clut[] = { 0xFF000000, 0xFFFF00ff, 0xFFffffff, };	// 黒 青 白.
	        enum { K=0, B=0xFFFF*8/16, W=0xFFFF };
			static unsigned const toneTbl[3][64] = {
	            { K, B, W },
	            { K, B, W },
	            { K, B, W },
			};
	        static unsigned const toneNums[3] = { 3, 3, 3, };

	        conv((uint32_t*)dst, (uint32_t const*)src, w, h, ditTyp, dpp, toneNums, toneTbl);

	        enum { KB = K >> 8, BB = B >> 8, WB = W >> 8 };
	        uint32_t* p = (uint32_t*)dst;
			for (size_t y = 0; y < h; ++y) {
				for (size_t x = 0; x < w; ++x) {
					unsigned c = p[ y * w + x ];
	                c &= 0xff;
					unsigned i = (c < BB) ? 0 : (c < WB) ? 1 : 2;
				 #if 1
	                static int s_i = 0;
	                if (i == 1)
	                    ++s_i;
	                if (i == 2)
	                    ++s_i;
	                if (i == 3)
	                    ++s_i;
	             #endif
	                c = clut[i];
					p[ y * w + x ] = c;
				}
			}
		}
		return true;
	}

private:

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

private:
    Pix*    buf_;
    ToneTbl toneTbl_[3];
};

#undef ERRORDIFFUSION1B_SET_DIF

#endif
