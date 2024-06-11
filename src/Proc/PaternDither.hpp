/**
 *  @file   PaternDither.h
 *  @brief  パターンディザを施す.
 *  @author Masashi KITAMURA
 */
#ifndef PATERNDITHER_HPP_INCLUDED__
#define PATERNDITHER_HPP_INCLUDED__

#include <stddef.h>
#include <string.h>
#include <memory.h>


class PaternDither {
    enum { R  = 0, G  = 1, B  = 2, A  = 3, };
	typedef int diff_t;

public:
    PaternDither() : difBuf_(NULL) {}
    ~PaternDither() { free(difBuf_); }

	enum {
		F_ERRDIF	= 0x0010,   // Use ErrorDifusion.
		F_REV	    = 0x0040,   // Reverse G-plane dither pattern.
		F_MONO  	= 0x0080,   // Monotone
		F_COL_JP    = 0x0100,   // For JP-RETRO-PC GRB clut (4),8,16,256 (mono 2,3,4,8)
		F_COL_WIN   = 0x0200,   // For Windows 16,256 clut
		F_COL_XTERM = 0x0400,   // For Xterm-256 clut
		F_COL_SP    = 0x0800,   // Otameshi(G6R6B6C40)

		F_HALF_VAL    = 0x08000,  // Use PatternDither and ErrorDifusion.
		F_EX27TO16    = 0x10000,
		F_OPT_EX1     = 0x20000,  // (with JP for jp16 mono tone 3,4)
		F_RGB_2BIT_Y  = 0x40000,
		F_RGB_2BIT_X  = 0x80000,
		F_RGB_2BIT_XY = 0xC0000,

    #if 0
		F_MONO_B    = 0x1000,   // Mono blue.
		F_MONO_R    = 0x2000,   // Mono red.
		F_MONO_CY   = 0x3000,   // Mono cyan.
		F_MONO_G    = 0x4000,   // Mono green.
		F_MONO_SKY  = 0x5000,   // Mono sky blue.
		F_MONO_Y    = 0x6000,   // Mono sky yellow.
		F_MONO_MSK  = 0x7000,   // Mono color mask
    #endif
	};

    /** 画像にパターン・ディザを施す。パターンはBayerのみ.
     * @param flags  フラグ.
     *          - bit1,0: 0=ディザ無 1=2x2 2=4x4 3=8x8
     */
    bool conv(
            unsigned*       dst,        	///< 出力バッファ.
            const unsigned* src,        	///< 入力バッファ.
            unsigned        w,          	///< 横幅.
            unsigned        h,          	///< 縦幅.
            unsigned        bpp,            ///< 出力予定 bpp
            unsigned        colNum,         ///< 色数.
            int             ditBpp,     	///< bpp ディザをかけた後の色のビット数.
            unsigned		typeAndFlags,	///< bit0-1:0=none,1=2x2,2=4x4,3=8x8 上位bit: F_...
            unsigned		monoCol = 0
    ) {
        delete difBuf_;
        difBuf_ = NULL;
        if (typeAndFlags & F_ERRDIF) {
            difBuf_ = (diff_t*)calloc(1, ((4 * w * 3) + 4) * sizeof(diff_t));
            if (difBuf_ == NULL)
                return false;
        }
        if (colNum == 0) {
            colNum = 1 << bpp;
            if (colNum == 0)
                colNum = 0xffffffff;
        }
        if (typeAndFlags & F_MONO) {
			if (bpp == 4 && (typeAndFlags & (F_COL_JP|F_COL_WIN))) {
				typeAndFlags |= F_OPT_EX1;
			}
		} else {
			if (bpp == 4 && (typeAndFlags & (F_COL_JP|F_COL_WIN))) {
				if (colNum >= 16)
					typeAndFlags |= F_EX27TO16;
			} else if ((bpp == 2 || colNum == 4) && (typeAndFlags & (F_COL_JP|F_COL_SP|F_COL_WIN|F_COL_XTERM))) {
				if ((typeAndFlags & F_RGB_2BIT_XY) == 0)
					typeAndFlags |= F_RGB_2BIT_Y;
			} else {
				typeAndFlags &= ~F_RGB_2BIT_XY;
			}
		}
		if ((typeAndFlags & 3) && (typeAndFlags & F_ERRDIF))
			typeAndFlags |= F_HALF_VAL;
        unsigned const* toneSizes = getToneSize(bpp, colNum, typeAndFlags, ditBpp);
        ditBpp  = getDitBpp(ditBpp, bpp, typeAndFlags);
        if (typeAndFlags & F_MONO)
            return convMono(dst, src, w, h, ditBpp, typeAndFlags, toneSizes[0], monoCol);
        else
            return convCol(dst, src, w, h, ditBpp, typeAndFlags, toneSizes);
    }

    bool convCol(
            unsigned*       dst,        	///< 出力バッファ.
            const unsigned* src,        	///< 入力バッファ.
            unsigned        w,          	///< 横幅.
            unsigned        h,          	///< 縦幅.
            unsigned        ditBpp,     	///< bpp ディザをかけた後の色のビット数.
            unsigned		typeAndFlags,	///< bit0-1:0=none,1=2x2,2=4x4,3=8x8 上位bit: F_...
			unsigned const* toneSizes
    ) {
        ditBpp = clamp(ditBpp, 3, 24/* 3*6 */);

        //r_bit,g_bit,b_bit は有効ビット数. 1～6. G,R,Bの順に多く割り当てる.
        int b_bit = ditBpp / 3;
        int g_bit = b_bit;
        int r_bit = g_bit;
        if ((ditBpp % 3) >= 1)
            ++g_bit;
        if ((ditBpp % 3) == 2)
            ++r_bit;

        // パターンディザを施すためのテーブルを作成.
        memset(it_, 0, sizeof it_);
        memset(dt_, 0, sizeof dt_);
        unsigned char   (*dt)[8][8][256] = dt_;
        makePtnDitherTable(it_[R], dt[R], R, toneSizes[0], r_bit, typeAndFlags & ~F_REV);
        makePtnDitherTable(it_[G], dt[G], G, toneSizes[1], g_bit, typeAndFlags         );
        makePtnDitherTable(it_[B], dt[B], B, toneSizes[2], b_bit, typeAndFlags & ~F_REV);

        diff_t* difBuf = &difBuf_[0];
        if (difBuf)
            memset(difBuf, 0, 4 * w * 3 * sizeof(diff_t));

        int  add      = 1;
        for (unsigned y = 0; y < h; ++y) {
            unsigned dy = y & 3;
            for (unsigned x = 0; x < w; ++x) {
                int rd = 0, gd = 0, bd = 0;
                if (difBuf) {
				    rd = int(difBuf[(dy * w + x) * 3 + R] >> 4);
				    gd = int(difBuf[(dy * w + x) * 3 + G] >> 4);
				    bd = int(difBuf[(dy * w + x) * 3 + B] >> 4);
					if (typeAndFlags & F_RGB_2BIT_XY) {
                        rd /= 3;
                        gd /= 3;
                        bd /= 3;
					}
                    if (typeAndFlags & F_HALF_VAL) {
                        rd >>= 1;
                        gd >>= 1;
                        bd >>= 1;
                    }
                }
                unsigned c = src[y*w+x];
                unsigned a = argb_a(c);

                int r0 = argb_r(c);
                int g0 = argb_g(c);
                int b0 = argb_b(c);

				int	r1 = int(r0 + rd);
				int	g1 = int(g0 + gd);
				int	b1 = int(b0 + bd);

				unsigned r2 = clamp(r1, 0, 255);
				unsigned g2 = clamp(g1, 0, 255);
				unsigned b2 = clamp(b1, 0, 255);

                unsigned u = x & 7;
                unsigned v = y & 7;

                unsigned char r3 = dt[R][v][u][r2];
                unsigned char g3 = dt[G][v][u][g2];
                unsigned char b3 = dt[B][v][u][b2];

				if (typeAndFlags & (F_EX27TO16|F_RGB_2BIT_XY)) {
					if (typeAndFlags & F_EX27TO16) {
						if (   (0 < r3 && r3 < 0xff)
							|| (0 < g3 && g3 < 0xff)
							|| (0 < b3 && b3 < 0xff) )
						{
							if (r3 == 0xff || g3 == 0xff || b3 == 0xff) {
								r3 = (r3 == 0xff) ? 0xff : 0x00;
								g3 = (g3 == 0xff) ? 0xff : 0x00;
								b3 = (b3 == 0xff) ? 0xff : 0x00;
							}
						}
					} else if (typeAndFlags & F_RGB_2BIT_XY) {
						//unsigned m = (x + y) % 3;
						unsigned m;
						if ((typeAndFlags & F_RGB_2BIT_XY) == F_RGB_2BIT_XY)
							m = (x + y) % 3;
						else if (typeAndFlags & F_RGB_2BIT_Y)
							m = x % 3;
						else
							m = y % 3;
						if (m == 0)      g3 = 0, b3 = 0;
						else if (m == 1) b3 = 0, r3 = 0;
						else             r3 = 0, g3 = 0;
					}
				}

                dst[y*w+x] = argb(a,r3,g3,b3);

                //if (typeAndFlags & F_ERRDIF)
                {
			        if (difBuf)
					    setDif(difBuf, w, h, x, y, add, r1 - r3, g1 - g3, b1 - b3);
                }
            }
            if (difBuf)
                memset(&difBuf[(dy * w)*3], 0, w * 3 * sizeof(diff_t));
        }
        return true;
    }

    bool convMono(
            unsigned*       dst,        	///< 出力バッファ.
            const unsigned* src,        	///< 入力バッファ.
            unsigned        w,          	///< 横幅.
            unsigned        h,          	///< 縦幅.
            unsigned        ditBpp,     	///< bpp ディザをかけた後の色のビット数.
            unsigned		typeAndFlags,	///< bit0-1:0=none,1=2x2,2=4x4,3=8x8 上位bit: F_...
			unsigned        toneSize,       ///< 階調数.
            unsigned        monoCol         ///< 単色の色.
    ) {
        bool colConv = ((typeAndFlags & (F_COL_JP|F_COL_SP|F_COL_WIN|F_COL_XTERM)) && (toneSize <= 4));
        if (ditBpp <= 0)
            ditBpp = 15;
        ditBpp /= 3;
        ditBpp = clamp(ditBpp, 1, 8);
        // パターンディザを施すためのテーブルを作成.
        memset(it_, 0, sizeof it_);
        memset(dt_, 0, sizeof dt_);
        unsigned char   (*dt)[8][8][256] = dt_;
            ditBpp = 8;
        unsigned ditBppTmp = colConv ? 8 : ditBpp;
        makePtnDitherTable(it_[G], dt[G], R, toneSize, ditBppTmp, typeAndFlags);

        makePtnDitherTable(it_[R], dt[R], R, toneSize, ditBppTmp, typeAndFlags);
        makePtnDitherTable(it_[B], dt[B], R, toneSize, ditBppTmp, typeAndFlags);

        diff_t* difBuf = difBuf_;
        if (difBuf)
            memset(difBuf, 0, 4 * w * 3 * sizeof(diff_t));
        bool half = (typeAndFlags & F_HALF_VAL);
        int  add = 1;
        for (unsigned y = 0; y < h; ++y) {
            unsigned dy = y & 3;
            for (unsigned x = 0; x < w; ++x) {
                int gd = 0;
                if (difBuf) {
                    gd = int(difBuf[(dy * w + x) * 3 + G] >> 4);
                    if (half)
                        gd >>= 1;
                }
                unsigned c = src[y*w+x];
                unsigned a = argb_a(c);

                int g0 = argb_g(c);
				int	g1 = int(g0 + gd);
				unsigned g2 = clamp(g1, 0, 255);

                unsigned u = x & 7;
                unsigned v = y & 7;

                unsigned char g3 = dt[G][v][u][g2];

                dst[y*w+x] = argb(a,g3,g3,g3);

			    if (difBuf) {
					int dg = g1 - g3;
					setDif(difBuf, w, h, x, y, add, dg, dg, dg);
				}
            }
            if (difBuf)
                memset(&difBuf[(dy * w)*3], 0, w * 3 * sizeof(diff_t));
        }
        if (colConv || monoCol)
        	monoColConv(dst, w, h, toneSize, typeAndFlags, monoCol);
        return true;
    }

	static int getDitBpp(int ditBpp, unsigned bpp, unsigned typeAndFlags) {
        if (ditBpp <= 0) {              // デフォルトの色のビット数を出力に合わせて選ぶ.
         #if 1
            return 15;
         #else
            if (typeAndFlags & F_MONO) {
                ditBpp  = (bpp <=  8) ? bpp*3
                        : (bpp <  12) ? 3*3
                        : (bpp <= 12) ? 4*3
                        : (bpp <= 15) ? 5*3
                        : (bpp <= 18) ? 6*3
                        :               8*3;
            } else if (typeAndFlags & (F_COL_JP|F_COL_SP|F_COL_WIN|F_COL_XTERM)) {  // 固定clutの場合.
                 ditBpp = (bpp <   3) ? 15
                        : (bpp <=  3) ?  3
                        : (bpp <=  6) ?  6
                        : (bpp <=  8) ? 15
                        : (bpp <=  9) ?  9
                        : (bpp <= 12) ? 12
                        : (bpp <= 15) ? 15
                        : (bpp == 16) ? 16
                        : (bpp <= 18) ? 18
                        :               24;
            } else {
                ditBpp  = (bpp <   3) ? 15
                        : (bpp <=  9) ? bpp
                        : (bpp <= 12) ? 12
                        : (bpp <= 15) ? 15
                        : (bpp == 16) ? 16
                        : (bpp <= 18) ? 18
                        :               24;
            }
         #endif
        }
        return ditBpp;
	}

private:
	static unsigned const* getToneSize(unsigned bpp, unsigned colNum, unsigned typeAndFlags, unsigned ditBpp) {
        // RGBを何階調にするか.
        static unsigned const toneSizeTbl[][3] = {
            // r   g   b
            {  8,  8,  4, },    // 0 :  8bit r3g3b2   256色.
            {  7,  9,  4, },    // 1 :  8bit          252色. win clut
            {  2,  2,  2, },    // 2 :  3bit r1g1b1     8色.
            {  3,  3,  3, },    // 3 :  4bit           27色.
            {  4,  4,  4, },    // 4 :  6bit r2g2b2    64色.
            {  5,  5,  5, },    // 5 :  7bit          125色.
            {  6,  6,  6, },    // 6 :  8bit          216色. sp
            {  7,  7,  7, },    // 7 :  9bit          343色. (xterm)
            {  8,  8,  8, },    // 8 :  9bit r3g3b3   512色.
            { 16, 16, 16, },    // 9 : 12bit r4g4b4  4096色.
            { 32, 32, 32, },    //10 : 15bit r5g5b5 32768色.
            { 32, 64, 32, },    //11 : 16bit r5g6b5 65536色.
            { 64, 64, 64, },    //12 : 18bit r6g6b6 262144色.
            {128,128,128, },    //13 : 21bit r7g7b7
            {256,256,256, },    //14 : 21bit r7g7b7

        };

        int idx = 0;
        if (typeAndFlags & F_MONO) {
            idx = (bpp <=  1 || colNum <= 2) ? 2
                : (             colNum <= 3) ? 3
                : (bpp <=  2 || colNum <= 4) ? 4
                : (             colNum <= 5) ? 5
                : (             colNum <= 6) ? 6
                : (             colNum <= 7) ? 7
                : (bpp <=  3 || colNum <= 8) ? 8
                : (bpp <=  4) ? 9
                : (bpp <=  5) ? 10
                : (bpp <=  6) ? 12
                : (bpp <=  7) ? 13
                :               14;
		} else if (typeAndFlags & (F_COL_JP|F_COL_SP|F_COL_WIN|F_COL_XTERM)) {
            idx = (bpp <=  5 || colNum <=  32) ? 2
                : (bpp <=  6 || colNum <=  64) ? 4
                : (bpp <=  7 || colNum <= 128) ? 5
                : (bpp <=  8) ? 0
                : (bpp <= 11) ? 8
                : (bpp <= 12) ? 9
                : (bpp <= 15) ? 10
                : (bpp <= 18) ? 12
                : (bpp <= 21) ? 13
                :               14;
            if (idx == 0) {	// 256 色.
			    if (typeAndFlags & F_COL_WIN) {
				    idx = 1;
			    } else if (typeAndFlags & F_COL_SP) {	// G6R6B6C40
				    idx = 6;
                } else if (typeAndFlags & F_COL_XTERM) {
				    idx = 7;
			    }
            } else if ((bpp == 4 || bpp == 5) && (colNum >= 16 && colNum < 27)) {
			    if (typeAndFlags & (F_COL_JP|F_EX27TO16)) {
				    idx = 3;
			    }
		    }
        } else {
            if (bpp <= 8)
                ditBpp = (ditBpp > 0 && ditBpp <= 24) ? ditBpp : 15;
            else if (ditBpp > bpp)
                ditBpp = bpp;
            idx = (ditBpp <=  3) ? 2
                : (ditBpp <=  6) ? 4
                : (ditBpp <=  9) ? 8
                : (ditBpp <= 12) ? 9
                : (ditBpp <= 15) ? 10
                : (ditBpp <= 18) ? 12
                : (ditBpp <= 21) ? 13
                :                  14;
        }
	 #if 1	// zantei.
	 	if (idx > 12)
	 		idx = 12;
	 #endif
        return toneSizeTbl[idx];
	}

private:
    /// パターンテーブルの作成.
    void makePtnDitherTable(
    		  unsigned char it[256],
    		  unsigned char dt[8][8][256],
              unsigned      no,
    		  unsigned 		toneNum,
    		  unsigned 		c_bit,
    		  unsigned 		typeAndFlags)
    {
        static const signed char dmPtn[4][8][8] = {
            //パターンディザ無し.
            {
                {0}
            }, {    //Bayer 2x2
                {  -96,  32, -96,  32, -96,  32, -96,  32,},
                {   96, -32,  96, -32,  96, -32,  96, -32,},
                {  -96,  32, -96,  32, -96,  32, -96,  32,},
                {   96, -32,  96, -32,  96, -32,  96, -32,},
                {  -96,  32, -96,  32, -96,  32, -96,  32,},
                {   96, -32,  96, -32,  96, -32,  96, -32,},
                {  -96,  32, -96,  32, -96,  32, -96,  32,},
                {   96, -32,  96, -32,  96, -32,  96, -32,},
            }, {    //Bayer 4x4
                { -120,   8, -88,  40,-120,   8, -88,  40,},
                {   72, -56, 104, -24,  72, -56, 104, -24,},
                {  -72,  56,-104,  24, -72,  56,-104,  24,},
                {  120,  -8,  88, -40, 120,  -8,  88, -40,},
                { -120,   8, -88,  40,-120,   8, -88,  40,},
                {   72, -56, 104, -24,  72, -56, 104, -24,},
                {  -72,  56,-104,  24, -72,  56,-104,  24,},
                {  120,  -8,  88, -40, 120,  -8,  88, -40,},
            }, {    //Bayer 8x8
                { -126,   2, -94,  34,-118,  10, -86,  42,},
                {   66, -62,  98, -30,  74, -54, 106, -22,},
                {  -78,  50,-110,  18, -70,  58,-102,  26,},
                {  114, -14,  82, -46, 122,  -6,  90, -38,},
                { -114,  14, -82,  46,-122,   6, -90,  38,},
                {   78, -50, 110, -18,  70, -58, 102, -26,},
                {  -66,  62, -98,  30, -74,  54,-106,  18,},
                {  126,  -2,  94, -34, 118, -10,  86, -42,},
            },
        };

        unsigned        typ    = typeAndFlags & 3;
        unsigned        subIdx = 0;
		// xterm256 clut 向け補正.
		bool xterm256 = false;
		bool win256   = false;
        //if (typeAndFlags & F_MONO)
        //    xterm256 = win256 = 0;
		if (typeAndFlags & F_COL_XTERM) {
			xterm256 = true;
            if (toneNum == 3)
                subIdx = 2;
            else if (toneNum == 4)
                subIdx = 1;
			else
                toneNum  = 7;
        } else if (typeAndFlags & F_COL_WIN) {
			if (toneNum > 3) {
				win256  = true;
                if ((no ==G && toneNum == 4) || (no == R && toneNum == 4))
                    subIdx = 1;
                else
                    toneNum = (no == G) ? 9 : (no == R) ? 7 : 4;
			}
        }

		// n階調 調整.
		assert(1 < toneNum && toneNum <= 256);
		unsigned toneNumM1 = toneNum - 1;
		unsigned idx = 0;
		for (unsigned i = 0; i < toneNum; ++i) {
			unsigned last = (i+1) * 256 / toneNum;
			unsigned cc;
            if (xterm256) {
                static unsigned char const tone[3][7] = {
                    { 0x00,0x00,0x5f,0x87,0xaf,0xd7,0xff }, // 7
                    { 0x00,0x5f,0xaf,0xff }, // 4
                    { 0x00,0x87,0xff }, // 3
				};
                cc = tone[subIdx][i];
            } else if (win256) {
                if (subIdx == 1) {
                    static unsigned char const tone[3][4] = {
					    { 0x00, 0x55, 0xaa, 0xff, },
					    { 0x00, 0x5f, 0x9f, 0xff, },
					    { 0x00, 0x55, 0xAA, 0xFF, },
				    };
                    cc = tone[no][i];
                } else {
                    static unsigned char const tone[3][9] = {
					    { 0x00, 0x2a, 0x55, 0x7f, 0xaa, 0xd4, 0xff, },				// 7
					    { 0x00, 0x1f, 0x3f, 0x5f, 0x7f, 0x9f, 0xbf, 0xdf, 0xff, },	// 9
					    { 0x00, 0x55, 0xAA, 0xFF, },								// 4
				    };
                    cc = tone[no][i];
                }
            } else {
                cc = i * 255 / toneNumM1;
            }
			while (idx < last)
				it[idx++] = cc;
		}

		// 色ビット数の調整.
        static const unsigned msk0[] = {0x80,0xC0,0xE0,0xF0, 0xF8,0xFC,0xFE,0xFF};
        unsigned              msk    = msk0[c_bit-1];
        for (unsigned i = 0; i < 256; ++i) {
            unsigned  k = it[i];
            k &= msk;
            if (c_bit >= 8)
            	;
            if (c_bit > 3)
            	k = k | (k >> c_bit);
            else if (c_bit == 3)
                k = k | (k >> 3) | (k >> (2*3));
            else if (c_bit == 2)
                k = k | (k >> 2) | (k >> (2*2)) | (k >> (3*2));
            else
            	k = k ? 0xff : 0x00;
            it[i] = k;
            //x DBG_PRINTF(("%02x", k)); if (i % 16 == 15) DBG_PRINTF(("\n")); else DBG_PRINTF((" "));
        }

		bool	revf = (typeAndFlags & F_REV) != 0;
		bool	half = (typeAndFlags & F_HALF_VAL) != 0;

        // パターンディザを反映したテーブルの作成.
        const signed char (*pPtn)[8] = dmPtn[typ];
        for (int y = 0; y < 8; y++) {
            for (int x  = 0; x < 8; x++) {
                int yy  = (revf == 0) ? y : 7 - y;
                int dm  = pPtn[yy][x];
				if (half)
					dm >>= 1;
                dm      = dm / int(toneNum); // dm >>= c_bit;
                for (int i = 0; i < 256; i++) {
                    int c       = i+dm;
                    c           = clamp(c, 0, 0xff);
                    dt[y][x][i] = it[c];
                }
            }
        }
    }

 #undef ERRORDIFFUSION_SET_DIF
 #ifdef NDEBUG
	#define ERRORDIFFUSION_SET_DIF(difBuf,w,h,x,y,rd,gd,bd) do {  \
	    if (0 <= (y) && (y) < int(h)) { \
	        if (0 <= (x) && (x) < int(w)) {   \
                unsigned dy = (y) & 3; \
	            diff_t* dSt = (difBuf) + ((dy * (w)+(x)) * 3); \
	            dSt[0] += (rd) >> (8-4);    \
	            dSt[1] += (gd) >> (8-4);    \
	            dSt[2] += (bd) >> (8-4);    \
	        } \
	    }   \
	} while(0)
 #else
	#define ERRORDIFFUSION_SET_DIF(difBuf,w,h,x,y,rd,gd,bd)   setDif1(difBuf, w, h, x, y, rd,gd,bd)
	static void setDif1(diff_t* difBuf, int w, int h, int x, int y, int rd, int gd, int bd) {
	    if (0 <= (y) && (y) < int(h)) {
	        if (0 <= (x) && (x) < int(w)) {
                unsigned dy = (y) & 3;
	            diff_t* dSt = (difBuf) + ((dy * (w)+(x)) * 3);
	            dSt[0] += (rd) >> (8-4);
	            dSt[1] += (gd) >> (8-4);
	            dSt[2] += (bd) >> (8-4);
	        }
	    }
	}
 #endif

	static void setDif(diff_t* difBuf, unsigned w, unsigned h,
			unsigned x, unsigned y, int add,
			int rd, int gd, int bd)
	{
     #if 0    // Floyd-Steinberg
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+1*add, y+0, rd*7*16, gd*7*16, bd*7*16);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x-1*add, y+1, rd*3*16, gd*3*16, bd*3*16);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+0    , y+1, rd*5*16, gd*5*16, bd*5*16);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+1*add, y+1, rd*1*16, gd*1*16, bd*1*16);
     #elif 1    // my3: Floyd-Steinberg mod. ref. http://www.st.nanzan-u.ac.jp/info/gr-thesis/2014/11se309.pdf
        static const int ktbl[8][4] = {
            { 256 - 3*28 - 5*28 - 1*28, 3*28, 5*28, 1*28 },
            { 256 - 3*24 - 5*24 - 1*24, 3*24, 5*24, 1*24 },
            { 256 - 3*20 - 5*20 - 1*20, 3*20, 5*20, 1*20 },
            { 256 - 3*16 - 5*16 - 1*16, 3*16, 5*16, 1*16 },
            { 256 - 3*12 - 5*12 - 1*12, 3*12, 5*12, 1*12 },
            { 256 - 3* 8 - 5* 8 - 1* 8, 3* 8, 5* 8, 1* 8 },
            { 256 - 3* 4 - 5* 4 - 1* 4, 3* 4, 5* 4, 1* 4 },
            { 256 - 3* 0 - 5* 0 - 1* 0, 3* 0, 5* 0, 1* 0 },
        };

        unsigned   ri = clamp((rd<0 ? -rd : rd) >> 5, 0, 7);
        int const* rk = ktbl[ri];
        unsigned   gi = clamp((gd<0 ? -gd : gd) >> 5, 0, 7);
        int const* gk = ktbl[gi];
        unsigned   bi = clamp((bd<0 ? -bd : bd) >> 5, 0, 7);
        int const* bk = ktbl[bi];

        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+1*add, y+0, rk[0] * rd, gk[0] * gd, bk[0] * bd);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x-1*add, y+1, rk[1] * rd, gk[1] * gd, bk[1] * bd);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+0    , y+1, rk[2] * rd, gk[2] * gd, bk[2] * bd);
        ERRORDIFFUSION_SET_DIF(difBuf,w,h, x+1*add, y+1, rk[3] * rd, gk[3] * gd, bk[3] * bd);

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

	void monoColConv(unsigned* dst, unsigned w, unsigned h, unsigned toneSize, unsigned typeAndFlags, unsigned col) {
        if ((typeAndFlags & (F_COL_JP|F_OPT_EX1)) == F_COL_JP) {
			if (toneSize == 3) {
				unsigned colors[3] = { 0x000000, 0x0000FF, 0x00FFFF, };
                unsigned toneSizeM1 = toneSize - 1;
		        for (unsigned n = 0; n < w * h; ++n) {
			        auto     d   = dst[n];
			        unsigned c   = d & 0xff;
			        unsigned idx = toneSizeM1 * c / 0xfe;
			        dst[n] = (d & 0xff000000) | colors[idx];
		        }
				return;
			} else if (toneSize == 4) {
				unsigned colors[4] = { 0x000000, 0x0000FF, 0x00FFFF, 0xffffff, };
                unsigned toneSizeM1 = toneSize - 1;
		        for (unsigned n = 0; n < w * h; ++n) {
			        auto     d   = dst[n];
			        unsigned c   = d & 0xff;
			        unsigned idx = toneSizeM1 * c / 0xff;
			        dst[n] = (d & 0xff000000) | colors[idx];
		        }
				return;
			}
		}
		if (col == 0)
			col = 0xffffff;
		unsigned colors[256] = {0};
		unsigned br = argb_r(col);
		unsigned bg = argb_g(col);
		unsigned bb = argb_b(col);
		for (unsigned i = 0; i < 256; ++i) {
			unsigned r = it_[R][i];
			unsigned g = it_[G][i];
			unsigned b = it_[B][i];
			r = r * br / 255;
			g = g * bg / 255;
			b = b * bb / 255;
			colors[i] = argb(0x00, r, g, b);
		}
		for (unsigned n = 0; n < w * h; ++n) {
			auto     d   = dst[n];
			unsigned idx = d & 0xff;
			dst[n] = (d & 0xff000000) | colors[idx];
		}
	}

    #if 0
	static void replaceMonoCol(unsigned* dst, unsigned w, unsigned h, unsigned toneSize, unsigned const* colors) {
		unsigned toneSizeM1 = toneSize - 1;
		for (unsigned n = 0; n < w * h; ++n) {
			auto     d   = dst[n];
			unsigned c   = d & 0xff;
			unsigned idx = toneSizeM1 * c / 0xff;
			dst[n] = (d & 0xff000000) | colors[idx];
		}
	}
    #endif

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
	diff_t*         difBuf_;
    unsigned char	dt_[3][8][8][256];
    unsigned char	it_[3][256];
};

#endif
