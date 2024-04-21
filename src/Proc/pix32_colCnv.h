/**
 *  @file   pix32_colConv.h
 *  @brief  32ビット色画像の、色関係の変換.
 *  @author Masashi KITAMURA
 */
#ifndef PIX32_COLCNV_H
#define PIX32_COLCNV_H



#ifndef PIX32_ARGB  // a,r,g,b結合＆分解マクロ.

/// 8bit値のa,r,g,bを繋げて 32bit の色値にする.
#define PIX32_ARGB(a,r,g,b)     ((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)       ((unsigned char)(argb))         ///< argb値中の blueの値を取得.
#define PIX32_GET_G(argb)       ((unsigned char)((argb)>>8))    ///< argb値中の greenの値を取得.
#define PIX32_GET_R(argb)       ((unsigned char)((argb)>>16))   ///< argb値中の greenの値を取得.
#define PIX32_GET_A(argb)       (((unsigned)(argb))>>24)        ///< argb値中の alphaの値を取得.

#endif

#ifndef PIX32_CLAMP
#define PIX32_CLAMP(c,mi,ma)    (((c) < (mi)) ? (mi) : ((ma) < (c)) ? (ma) : (c))
#endif

#include "pix32_kyuv.h"


/// pix32_swapARGB での変換指定番号.
enum PIX32_SWAPARGB {
    // 現在の画像がARGB順として、並びを変えたい場合に指定.
    PIX32_SWAP_TO_ARGB =  0,
    PIX32_SWAP_TO_RAGB =  1,
    PIX32_SWAP_TO_AGRB =  2,
    PIX32_SWAP_TO_GARB =  3,
    PIX32_SWAP_TO_RGAB =  4,
    PIX32_SWAP_TO_GRAB =  5,
    PIX32_SWAP_TO_ARBG =  6,
    PIX32_SWAP_TO_RABG =  7,
    PIX32_SWAP_TO_ABRG =  8,
    PIX32_SWAP_TO_BARG =  9,
    PIX32_SWAP_TO_RBAG = 10,
    PIX32_SWAP_TO_BRAG = 11,
    PIX32_SWAP_TO_AGBR = 12,
    PIX32_SWAP_TO_GABR = 13,
    PIX32_SWAP_TO_ABGR = 14,
    PIX32_SWAP_TO_BAGR = 15,
    PIX32_SWAP_TO_GBAR = 16,
    PIX32_SWAP_TO_BGAR = 17,
    PIX32_SWAP_TO_RGBA = 18,
    PIX32_SWAP_TO_GRBA = 19,
    PIX32_SWAP_TO_RBGA = 20,
    PIX32_SWAP_TO_BRGA = 21,
    PIX32_SWAP_TO_GBRA = 22,
    PIX32_SWAP_TO_BGRA = 23,

    // 現在の画像のA,R,G,Bの順を指定して、変換の結果 ARGB順にしたい場合.
    // ※ バグってるかも...
    PIX32_SWAP_FROM_ARGB =  0,
    PIX32_SWAP_FROM_RAGB =  6,
    PIX32_SWAP_FROM_AGRB =  2,
    PIX32_SWAP_FROM_GARB =  8,
    PIX32_SWAP_FROM_RGAB = 12,
    PIX32_SWAP_FROM_GRAB = 14,
    PIX32_SWAP_FROM_ARBG =  1,
    PIX32_SWAP_FROM_RABG =  7,
    PIX32_SWAP_FROM_ABRG =  3,
    PIX32_SWAP_FROM_BARG =  9,
    PIX32_SWAP_FROM_RBAG = 13,
    PIX32_SWAP_FROM_BRAG = 15,
    PIX32_SWAP_FROM_AGBR =  4,
    PIX32_SWAP_FROM_GABR = 10,
    PIX32_SWAP_FROM_ABGR =  5,
    PIX32_SWAP_FROM_BAGR = 11,
    PIX32_SWAP_FROM_GBAR = 16,
    PIX32_SWAP_FROM_BGAR = 17,
    PIX32_SWAP_FROM_RGBA = 18,
    PIX32_SWAP_FROM_GRBA = 20,
    PIX32_SWAP_FROM_RBGA = 19,
    PIX32_SWAP_FROM_BRGA = 21,
    PIX32_SWAP_FROM_GBRA = 22,
    PIX32_SWAP_FROM_BGRA = 23,
};



/** 色のargbの順を並び替える.
 */
inline void pix32_swapARGB(unsigned *pix, unsigned w, unsigned h, PIX32_SWAPARGB rotNo)
{
    enum { B=0,G=1,R=2,A=3};
    static const char tbl[][4] = {
      // b g r a        //      mem     little endian値(プログラム中はこちら)
        {B,G,R,A,},     // 0    bgra    argb
        {B,G,A,R,},     // 1    bgar    ragb
        {B,R,G,A,},     // 2    brga    agrb
        {B,R,A,G,},     // 3    brag    garb
        {B,A,G,R,},     // 4    bagr    rgab
        {B,A,R,G,},     // 5    barg    grab
        {G,B,R,A,},     // 6    gbra    arbg
        {G,B,A,R,},     // 7    gbar    rabg
        {G,R,B,A,},     // 8    grba    abrg
        {G,R,A,B,},     // 9    grab    barg
        {G,A,B,R,},     //10    gabr    rbag
        {G,A,R,B,},     //11    garb    brag
        {R,B,G,A,},     //12    rbga    agbr
        {R,B,A,G,},     //13    rbag    gabr
        {R,G,B,A,},     //14    rgba    abgr
        {R,G,A,B,},     //15    rgab    bagr
        {R,A,B,G,},     //16    rabg    gbar
        {R,A,G,B,},     //17    ragb    bgar
        {A,B,G,R,},     //18    abgr    rgba
        {A,B,R,G,},     //19    abrg    grba
        {A,G,B,R,},     //20    agbr    rbga
        {A,G,R,B,},     //21    agrb    brga
        {A,R,B,G,},     //22    arbg    gbra
        {A,R,G,B,},     //23    argb    bgra
        // ↑のテーブル内のA,R,G,Bは意味としては数値のほうがよいが,
    };
    unsigned char   *p = (unsigned char*)pix;
    const char      *t = tbl[rotNo];
    unsigned        n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w*h; ++n) {
        unsigned char b = p[B];
        unsigned char g = p[G];
        unsigned char r = p[R];
        unsigned char a = p[A];
        p[(int)t[B]] = b;
        p[(int)t[G]] = g;
        p[(int)t[R]] = r;
        p[(int)t[A]] = a;
        p += 4;
    }
}




/** 画像の有効ビット数を変更する.(r,g,b格ビットの下位を0クリアする)
 */
inline void pix32_bppChg(unsigned *pix, unsigned w, unsigned h, unsigned colBit, bool zeroHosei=true)
{
    static const unsigned colBitTbl[] = {
        0x000000,                           // 0
        0x008000, 0x808000, 0x808080,       // 1, 2, 3
        0x80c080, 0xc0c080, 0xc0c0c0,       // 4, 5, 6
        0xc0e0c0, 0xe0e0c0, 0xe0e0e0,       // 7, 8, 9
        0xe0f0e0, 0xf0f0e0, 0xf0f0f0,       //10,11,12
        0xf0f8f0, 0xf8f8f0, 0xf8f8f8,       //13,14,15
        0xf8fcf8, 0xfcfcf8, 0xfcfcfc,       //16,17,18
        0xfcfefc, 0xfefefc, 0xfefefe,       //19,20,21
        0xfefffe, 0xfffffe, 0xffffff,       //22,23,24
    };

    static const unsigned colZeroTbl[] = {
        0x000000,                           // 0
        0x000000, 0x000000, 0x000000,       // 1, 2, 3
        0x000000, 0x000000, 0x000000,       // 4, 5, 6
        0x000000, 0x000000, 0x000000,       // 7, 8, 9
        0x001000, 0x100000, 0x000010,       //10,11,12
        0x000800, 0x080000, 0x000008,       //13,14,15
        0x000400, 0x040000, 0x000004,       //16,17,18
        0x000200, 0x020000, 0x000002,       //19,20,21
        0x000100, 0x010000, 0x000001,       //22,23,24
    };

    unsigned    colBitMsk   = colBitTbl[colBit];
    unsigned    colZeroPtn  = colZeroTbl[colBit];
    unsigned    *p          = (unsigned*)pix;
    unsigned    n;

    if (colBit > 24)
        colBit = 24;
    for (n = 0; n < w * h; ++n) {
        unsigned c = p[n];
        unsigned a = c & 0xFF000000;
        c &= 0xFFFFFF;
        if (c) {
            c = c & colBitMsk;
            if (c == 0 && a && zeroHosei) { // 0の補正をするとき.
                // 色0で、αがあるとき、0でない一番暗い(青)色を設定.
                c = colZeroPtn;
            }
        }
        p[n] = c | a;
    }
}





/** 画像に α情報があるか? (全てα=0か全てα=0xffならα情報無し)
 */
inline int pix32_isUseAlpha(const unsigned *p, unsigned w, unsigned h)
{
    int             chk;
    const unsigned* e  = p + w * h;
    assert(p != 0 && w != 0 && h != 0);
    chk = (unsigned char)(*p++ >> 24);
    if (0 < chk && chk < 255)
        return true;
    while (p < e) {
        int a   = (unsigned char)(*p++ >> 24);
        if (a != chk)
            return true;
    }
    return false;
}




/** α値が全て0か?
 */
inline int pix32_isAlphaAllZero(const unsigned *pix, unsigned w, unsigned h)
{
    unsigned        c = 0;
    const unsigned* e = pix + w * h;
    assert(pix != 0 && w != 0 && h != 0);
    while (pix < e)
        c |= *pix++;
    return ((c >> 24) == 0);
}




/** 入力のαを 0クリア.
 */
inline void pix32_clearAlpha(unsigned *pix, unsigned w, unsigned h, unsigned a)
{
    unsigned char *px = (unsigned char*)pix;
    unsigned n;
    assert(pix != 0 && w != 0 && h != 0);
    px += 3;
    for (n = 0; n < w * h; ++n) {
        *px = a;
        px += 4;
    }
}



/** 抜き色のα情報を 0に.
 */
inline void pix32_clearAlphaOfColKey(unsigned* pix, unsigned w, unsigned h, unsigned colKey)
{
    unsigned n;
    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = pix[n];
        if ((c & 0xFFFFFF) == colKey) {
            pix[n] &= 0xFFFFFF;
        }
    }
}

/** α=0のRGBを0クリア.
 */
inline void pix32_clearColorIfAlpha0(unsigned *pix, unsigned w, unsigned h)
{
    unsigned    *px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px;
        if ((c >> 24) == 0)
            c = 0;
        *px++ = c;
    }
}


/** 入力のαを反転する必要があるとき.
 */
inline void pix32_revAlpha(unsigned* pix, unsigned w, unsigned h)
{
    unsigned char *px = (unsigned char*)pix;
    unsigned n;
    assert(pix != 0 && w != 0 && h != 0);
  #if defined BIG_ENDIAN == 0
    px += 3;
  #endif
    for (n = 0; n < w * h; n++) {
        *px = 0xFF - *px;
        px += 4;
    }
}




/** 抜き色はα=0に、それ以外はα=0xFFにする.
 */
inline void pix32_genColKeyToAlpha(unsigned *pix, unsigned w, unsigned h, unsigned colKey)
{
    unsigned    *px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px & 0xFFFFFF;
        if (c != colKey)
            c |= 0xFF000000;
        *px++ = c;
    }
}


#if 0
/** RGBよりモノクロ値を求め、それをαとする.
 */
inline void pix32_monoToAlpha(unsigned *pix, unsigned w, unsigned h)
{
    unsigned*   px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px;
        unsigned r = (unsigned char)(c >> 16);
        unsigned g = (unsigned char)(c >>  8);
        unsigned b = (unsigned char)(c);
        // unsigned a = ( 587*g +  299*r + 114*b) / 1000;
        // *px++ = (a << 24) | (c & 0xFFFFFF);
        unsigned a = (256*38444U * g + 256*19589U * r +  256*7503U * b);
        *px++ = (a & 0xFF000000) | (c & 0xFFFFFF);
    }
}
#endif


/** RGBよりモノクロ値を求め、それをαとする.
 */
inline void pix32_monoToAlpha(unsigned *pix, unsigned w, unsigned h, double ratio, int ofs)
{
    unsigned*   px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px;
        int      r = (unsigned char)(c >> 16);
        int      g = (unsigned char)(c >>  8);
        int      b = (unsigned char)(c);
        r = (int)(r * ratio + ofs);
        g = (int)(g * ratio + ofs);
        b = (int)(b * ratio + ofs);
        //int   a = ( 587*g +  299*r + 114*b) / 1000;
        int     a = (38444 * g + 19589 * r +  7503 * b) >> 16;
        if (a < 0) a = 0; else if (a > 255) a = 255;
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;
        *px++ = PIX32_ARGB(a,r,g,b);
    }
}



/** モノクロに変換.
 */
inline void pix32_toMono(unsigned *pix, unsigned w, unsigned h)
{
    unsigned*   px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px;
        unsigned a = c >> 24;
        unsigned r = (unsigned char)(c >> 16);
        unsigned g = (unsigned char)(c >>  8);
        unsigned b = (unsigned char)(c);
        //c = ( 587*g +  299*r + 114*b) / 1000;
        c = (38444 * g + 19589 * r +  7503 * b) >> 16;
        *px++ = PIX32_ARGB(a,c,c,c);
    }
}



/** R,G,B,Aいずれかのチャンネルを用いてモノクロに変換.
 */
inline void pix32_chARGBtoMono(unsigned *pix, unsigned w, unsigned h, int ch)
{
    unsigned*   px = (unsigned*)pix;
    unsigned    n;
    unsigned    sh;

    assert(0 <= ch && ch < 4);
    assert(pix != 0 && w != 0 && h != 0);
    sh = ch * 8;
    for (n = 0; n < w * h; ++n) {
        unsigned c = (unsigned char)(*px >> sh);
        *px++ = PIX32_ARGB(0xff,c,c,c);
    }
}



/** 色colの各ARGB値を、ピクセルの各ARGBの各々に乗ずる.
 */
inline void pix32_colMul(unsigned *pix, unsigned w, unsigned h, unsigned col)
{
    unsigned        *px = (unsigned*)pix;
    unsigned char   sa = col >> 24;
    unsigned char   sr = (unsigned char)(col >> 16);
    unsigned char   sg = (unsigned char)(col >>  8);
    unsigned char   sb = (unsigned char)(col);
    unsigned        n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned        d  = *px;
        unsigned char   da = d >> 24;
        unsigned char   dr = (unsigned char)(d >> 16);
        unsigned char   dg = (unsigned char)(d >>  8);
        unsigned char   db = (unsigned char)(d);
        da = (da * sa) / 255;
        dr = (dr * sr) / 255;
        dg = (dg * sg) / 255;
        db = (db * sb) / 255;
        d  = (da << 24) | (dr << 16) | (dg << 8) | db;
        *px++ = d;
    }
}



/** R,G,B,A 各々を二乗.
 */
inline void pix32_colChSquare(unsigned *pix, unsigned w, unsigned h)
{
    unsigned*   px = (unsigned*)pix;
    unsigned    n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = *px;
        unsigned a = c >> 24;
        unsigned r = (unsigned char)(c >> 16);
        unsigned g = (unsigned char)(c >>  8);
        unsigned b = (unsigned char)(c);
        a       = a * a / 0xff;
        r       = r * r / 0xff;
        g       = g * g / 0xff;
        b       = b * b / 0xff;
        *px++   = PIX32_ARGB(a,r,g,b);
    }
}



/** ratio=0.0～1.0 の率で、画素(rgb)の各値を変更.
 */
inline void pix32_changeRgbTone(unsigned *pix, unsigned w, unsigned h, double ratio) {
    unsigned size = w * h;
    unsigned n;
    for (n = 0; n < size; ++n) {
        unsigned c  = pix[n];
        int      r  = (int)(PIX32_GET_R(c) * ratio);
        int      g  = (int)(PIX32_GET_G(c) * ratio);
        int      b  = (int)(PIX32_GET_B(c) * ratio);
        unsigned a  = PIX32_GET_A(c);
        if (a) {
            r       = PIX32_CLAMP(r, 0, 255);
            g       = PIX32_CLAMP(g, 0, 255);
            b       = PIX32_CLAMP(b, 0, 255);
            pix[n]  = PIX32_ARGB(a,r,g,b);
        }
    }
}



/** ratio=0.0～1.0 の率で、画素(yuv)の輝度(y)のみを変更.
 */
inline void pix32_changeTone(unsigned *pix, unsigned w, unsigned h, double ratio) {
    unsigned size = w * h;
    unsigned n;
    for (n = 0; n < size; ++n) {
        unsigned c  = pix[n];
        int      r  = PIX32_GET_R(c);
        int      g  = PIX32_GET_G(c);
        int      b  = PIX32_GET_B(c);
        unsigned a  = PIX32_GET_A(c);
        if (a) {
            double   y  = PIX32_RGB_TO_KYUV_Y(r, g, b);
            double   u  = PIX32_RGB_TO_KYUV_U(r, g, b);
            double   v  = PIX32_RGB_TO_KYUV_V(r, g, b);
            y *= ratio;
            y = PIX32_CLAMP(y, 0, PIX32_KYUV_MUL_K*255);
            PIX32_RGB_FROM_KYUV(pix[n], y, u, v, a, double);
        }
    }
}



/** ピクセルの色colの各ARGB値を、ピクセルの各ARGBの各々に乗ずる.
 */
inline void pix32_argbScale(unsigned *pix, unsigned w, unsigned h, double argb[4])
{
    unsigned        *px = (unsigned*)pix;
    unsigned        n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned    d  = *px;
        int         da = d >> 24;
        int         dr = (unsigned char)(d >> 16);
        int         dg = (unsigned char)(d >>  8);
        int         db = (unsigned char)(d);
        da = (int)(da * argb[0]);   if (da > 255) da = 255; else if (da < 0) da = 0;
        dr = (int)(dr * argb[1]);   if (dr > 255) dr = 255; else if (dr < 0) dr = 0;
        dg = (int)(dg * argb[2]);   if (dg > 255) dg = 255; else if (dg < 0) dg = 0;
        db = (int)(db * argb[3]);   if (db > 255) db = 255; else if (db < 0) db = 0;
        d  = (da << 24) | (dr << 16) | (dg << 8) | db;
        *px++ = d;
    }
}



/** ピクセルの色colの各ARGB値を、ピクセルの各ARGBの各々に乗ずる.
 */
inline void pix32_ayuvScale(unsigned *pix, unsigned w, unsigned h, double ratio[4])
{
    unsigned size = w * h;
    unsigned n;
    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < size; ++n) {
        unsigned c  = pix[n];
        int      r  = PIX32_GET_R(c);
        int      g  = PIX32_GET_G(c);
        int      b  = PIX32_GET_B(c);
        int      a  = (int)(PIX32_GET_A(c) * ratio[0]);
        double   y  = PIX32_RGB_TO_KYUV_Y(r, g, b);
        double   u  = PIX32_RGB_TO_KYUV_U(r, g, b);
        double   v  = PIX32_RGB_TO_KYUV_V(r, g, b);
        y *= ratio[1];
        u *= ratio[2];
        v *= ratio[3];
        a = PIX32_CLAMP(a, 0, 255);
        y = PIX32_CLAMP(y, 0, PIX32_KYUV_MUL_K*255);
        u = PIX32_CLAMP(u, 0, PIX32_KYUV_MUL_K*255);
        v = PIX32_CLAMP(v, 0, PIX32_KYUV_MUL_K*255);
        PIX32_RGB_FROM_KYUV(pix[n], y, u, v, a, double);
    }
}



/** α情報をモノクロ画像化.
 */
inline void pix32_alphaToRGB(unsigned *pix, unsigned w, unsigned h)
{
    unsigned *p = (unsigned*)pix;
    unsigned n;

    assert(pix != 0 && w != 0 && h != 0);
    for (n = 0; n < w * h; ++n) {
        unsigned c = p[n];
        c = c >> 24;
        p[n] = (0xFF << 24) | (c << 16) | (c << 8) | c;
    }
}



/** α=0は色000に、α1～254 (minA,maxA] のピクセルは引数の色rgbとαブレンドしてα=0xffの画像に変換.
 *  (淵半透明有りのバストアップ画像を、透明不透明のみ画像に変換する用)
 */
inline void pix32_alphaBlendByColor(unsigned* pix, unsigned w, unsigned h, unsigned rgb, unsigned nukiumeRgb=0, unsigned minA=0, unsigned maxA=255)
{
    unsigned *p = (unsigned*)pix;
    unsigned n;
    nukiumeRgb    &= 0xFFFFFF;

    for (n = 0; n < w * h; ++n) {
        unsigned c = p[n];
        unsigned a = PIX32_GET_A(c);
        if (a <= minA) {
            c = nukiumeRgb;
        } else if (a < maxA) {
            unsigned r = PIX32_GET_R(c) * a + PIX32_GET_R(rgb) * (0xFF-a);
            unsigned g = PIX32_GET_G(c) * a + PIX32_GET_G(rgb) * (0xFF-a);
            unsigned b = PIX32_GET_B(c) * a + PIX32_GET_B(rgb) * (0xFF-a);
            r /= 255;
            g /= 255;
            b /= 255;
            c = PIX32_ARGB(0xFF, r, g, b);
        } else {
            c |= PIX32_ARGB(0xff,0,0,0);
        }
        p[n] = c;
    }
}






// ===========================================================================
// C++向.
// image(), witdh(),height(),bpp() をメンバーに持つ画像クラスを操作.

#if defined __cplusplus

template<class IMG>
void pix32_swapARGB(IMG& img, PIX32_SWAPARGB rotNo) {
    assert(img.bpp() == 32);
    pix32_swapARGB((unsigned*)img.image(), img.width(), img.height(), rotNo);
}

template<class IMG>
void pix32_bppChg(IMG& img, unsigned colBit, bool zeroHosei=true) {
    assert(img.bpp() == 32);
    pix32_bppChg((unsigned*)img.image(), img.width(), img.height(), colBit, zeroHosei);
}


template<class IMG>
int  pix32_isUseAlpha(IMG& img) {
    assert(img.bpp() == 32);
    return pix32_isUseAlpha((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
int  pix32_isAlphaAllZero(IMG& img) {
    assert(img.bpp() == 32);
    return pix32_isAlphaAllZero((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
void pix32_clearAlpha(IMG& img, unsigned a=0) {
    assert(img.bpp() == 32);
    pix32_clearAlpha((unsigned*)img.image(), img.width(), img.height(), a);
}


template<class IMG>
void pix32_clearAlphaOfColKey(IMG& img, unsigned colKey) {
    assert(img.bpp() == 32);
    pix32_clearAlphaOfColKey((unsigned*)img.image(), img.width(), img.height(), colKey);
}


template<class IMG>
void pix32_revAlpha(IMG& img) {
    assert(img.bpp() == 32);
    pix32_revAlpha((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
void pix32_genColKeyToAlpha(IMG& img, unsigned colKey) {
    assert(img.bpp() == 32);
    pix32_genColKeyToAlpha((unsigned*)img.image(), img.width(), img.height(), colKey);
}


template<class IMG>
void pix32_toMono(IMG& img) {
    assert(img.bpp() == 32);
    pix32_toMono((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
void pix32_chARGBtoMono(IMG& img, int ch) {
    assert(img.bpp() == 32);
    pix32_chARGBtoMono((unsigned*)img.image(), img.width(), img.height(), ch);
}


template<class IMG>
void pix32_changeRgbTone(IMG& img, double ratio) {
    assert(img.bpp() == 32);
    pix32_changeRgbTone((unsigned*)img.image(), img.width(), img.height(), ratio);
}


template<class IMG>
void pix32_changeTone(IMG& img, double ratio) {
    assert(img.bpp() == 32);
    pix32_changeTone((unsigned*)img.image(), img.width(), img.height(), ratio);
}


template<class IMG>
void pix32_argbScale(IMG& img, double ratio[4]) {
    assert(img.bpp() == 32);
    pix32_argbScale((unsigned*)img.image(), img.width(), img.height(), ratio);
}

template<class IMG>
void pix32_yuvScale(IMG& img, double ratio[4]) {
    assert(img.bpp() == 32);
    pix32_ayuvScale((unsigned*)img.image(), img.width(), img.height(), ratio);
}

template<class IMG>
void pix32_colMul(IMG& img, unsigned col) {
    assert(img.bpp() == 32);
    pix32_colMul((unsigned*)img.image(), img.width(), img.height(), col);
}


template<class IMG>
void pix32_colChSquare(IMG& img) {
    assert(img.bpp() == 32);
    pix32_colChSquare((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
void pix32_alphaToRGB(IMG& img) {
    assert(img.bpp() == 32);
    pix32_alphaToRGB((unsigned*)img.image(), img.width(), img.height());
}


template<class IMG>
void pix32_alphaBlendByColor(IMG& img, unsigned rgb, unsigned nukiumeRgb=0, unsigned minA=0, unsigned maxA=255) {
    assert(img.bpp() == 32);
    pix32_alphaBlendByColor((unsigned*)img.image(), img.width(), img.height(), rgb, nukiumeRgb, minA,maxA);
}


#endif  // __cplusplus

#endif  // PIX32_COLCNV_H
