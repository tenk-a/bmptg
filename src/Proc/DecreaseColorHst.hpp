/**
 *  @file   DecreaseColorHst.h
 *  @brief  減色処理. 頻度カウントして多いもの順. α(安易)対応版.
 *  @author Masashi KITAMURA
 */

#ifndef DECREASECOLORHST_H
#define DECREASECOLORHST_H

#pragma once

#include <stddef.h>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <functional>
#if defined _WIN32 == 0
#include <stdint.h>
#endif



/// 減色処理. 32ビット色画を8ビット色画に変換.
template<class A=void>
class DecreaseColorHst {
public:
    DecreaseColorHst() {; }
    DecreaseColorHst(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize, int alpNum=-1) {
        conv(pDst, pSrc,w,h,clut,clutSize,alpNum);
    }

    static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize, int alpNum=-1);

private:
    static unsigned conv1(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                , unsigned* pClut, unsigned clutSize, int idx, int minA, int maxA);

    static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
        return (a << 24) | (r << 16) | (g << 8) | (b);
    }

    static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
    static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
    static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
    static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

private:
    enum { ALP_D = 4 };

    struct Hst {
      #if defined _WIN32                // vcにstdint.hがないのでその対策.
        typedef unsigned __int64 sum_t; // win系コンパイラはvc互換で古くから__int64がある.
      #else
        typedef uint64_t         sum_t;
      #endif
        sum_t           g;
        sum_t           r;
        sum_t           b;
        sum_t           a;
        size_t          num;
        short           no;

        bool operator<(const Hst& r) const {
            return this->num > r.num;
        }
    };

};



/// 32ビット色画を8ビット色画に変換.
template<class A>
bool DecreaseColorHst<A>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, int alpNum)
{
    using namespace std;
    // ピクセル＆clut初期化. (透明画像状態にする)
    memset(pDst, 0, w*h);
    memset(pClut, 0, 256*sizeof(pClut[0]));

    // 透明,半透明,不透明の存在チェック.
    enum { MIN_A = ALP_D, MAX_A = 255-ALP_D };
    bool tranf = false;
    bool alpf  = false;
    bool nalpf = false;
    if (alpNum != 0) {
        for (unsigned i = 0; i < w*h; ++i) {
            unsigned a = argb_a( pSrc[i] );
            if (a < MIN_A) {
                tranf = 1;
            } else if ( a <= MAX_A ) {
                alpf  = 1;
            } else {
                nalpf = 1;
            }
        }
    }

    if (nalpf == 0) {
        // 不透明がない場合は、すべてのパレットを半透明扱い.
        return conv1(pDst,pSrc,w,h,pClut,clutSize, tranf, MIN_A, 255) != 0;
    } else if (alpf == false || clutSize < 32) {
        // 不透明と抜き色のみのとき.
        return conv1(pDst,pSrc,w,h,pClut,clutSize, tranf, MAX_A+1, 255) != 0;
    } else {
        // 不透明と半透明があるとき (バストアップ画/淵半透明を想定)
        unsigned idx = (unsigned)alpNum;
        if (alpNum < 0) {       // 適当に半透明数を決める指定だった.(256色時に32色).
            idx = clutSize / 8;
            if (idx < 16)
                idx = 16;
        }
        idx = conv1(pDst,pSrc,w,h,pClut,idx, tranf, MIN_A, MAX_A);
        return conv1(pDst,pSrc,w,h,pClut,clutSize, idx, MAX_A+1, 255) != 0;
    }
}



/// 32ビット色画を8ビット色画に変換.
template<class A>
unsigned DecreaseColorHst<A>::conv1(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                                , unsigned* pClut, unsigned clutSize, int idx, int minA, int maxA)
{
    using namespace std;
    assert(clutSize <= 256 && clutSize >= idx+2);
    unsigned    BI=4;
  RETRY:
    unsigned    N=1<<BI, SH=8-BI;
    unsigned    NUM = 1 + N*N*N;
    assert( NUM >= 256 );
    Hst* pHst = new Hst[ NUM ];
    memset(pHst, 0, sizeof(Hst) * NUM);

    for (unsigned i = 0; i < w * h; ++i) {
        unsigned c = pSrc[i];
        int      a = argb_a(c);
        a          = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
        unsigned g = argb_g(c);
        unsigned r = argb_r(c);
        unsigned b = argb_b(c);
        if (minA <= a && a <= maxA) {
            unsigned no  = 1 + (g>>SH) * N*N + (r>>SH) * N + (b>>SH);
            Hst* h = &pHst[no];
            ++h->num;
            h->no = no;
            h->a += a;
            h->g += g;
            h->r += r;
            h->b += b;
        }
    }

    unsigned colNum = clutSize - idx;

 #ifdef __WATCOMC__ // for open watcom 1.9. nth_element がないので sort で代用.
    sort(&pHst[0], &pHst[NUM], less<Hst>() );
 #else
    // 多いモノからclutSize個を選出(先頭に移動).
    nth_element( &pHst[0], &pHst[colNum], &pHst[NUM], std::less<Hst>() );
 #endif

    memset(pClut+idx, 0, colNum*sizeof(pClut[0]));

    // 多いものから色を取得.
    unsigned n = 0;
    for (unsigned j = 0; j < colNum; ++j) {
        const Hst*  h   = &pHst[j];
        size_t      num = h->num;
        if (num == 0)
            continue;   // break;
        unsigned a    = unsigned(h->a / num);
        unsigned g    = unsigned(h->g / num);
        unsigned r    = unsigned(h->r / num);
        unsigned b    = unsigned(h->b / num);
        a   = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
        pClut[n+idx] = argb(a,r,g,b);
        ++n;
    }

    if (clutSize >= 128 && n <= clutSize-32 && (BI >= 4 && BI < 8) ) {
        // clutが多くあまってたら色深度を増やす(おそらく単色やツートンカラー系の画像だろう)
        ++BI;
        goto RETRY;
    }

    // 実際に使用されているclut数を求める.
    if (n < 2)  // ただし最低2色は使うことに.
        n = 2;
    clutSize = n + idx;

    // 暗いもの順に並べ直す.
    std::sort( &pClut[idx], &pClut[clutSize] );

    // 現状のclutで、すべてのピクセルをclutSize色化.
    // 近い色の条件として,
    //   - a,r,g,bの値が近いモノ(色同士の差が小さいモノ)
    //   - 1色内のr,g,bの輝度の大小関係が、近いモノ.
    // をチェック.
    memset(pHst, 0, sizeof(Hst) * clutSize);
    for (unsigned j = 0; j < w * h; ++j) {
        unsigned ii = 0;
        unsigned    c   = pSrc[j];
        int         a   = argb_a(c);
        int         g   = argb_g(c);
        int         r   = argb_r(c);
        int         b   = argb_b(c);
        int         o   = g - r;
        int         p   = r - b;
        int         q   = b - g;

        a = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
        if (minA <= a && a <= maxA) {
            unsigned k = 0xFFFFFFFF;
            for (unsigned i = idx; i < clutSize; ++i) {
                unsigned ic = pClut[i];
                int      ia = argb_a(ic);
                int      ig = argb_g(ic);
                int      ir = argb_r(ic);
                int      ib = argb_b(ic);
                int      io = ig - ir;
                int      ip = ir - ib;
                int      iq = ib - ig;

                int      aa = ia - a;
                int      gg = ig - g;
                int      rr = ir - r;
                int      bb = ib - b;
                int      oo = io - o;
                int      pp = ip - p;
                int      qq = iq - q;
                unsigned ik = 3*aa*aa + gg*gg + rr*rr + bb*bb + oo*oo + pp*pp + qq*qq;
                if (ik < k) {
                    k  = ik;
                    ii = i;
                }
            }
            pDst[j] = ii;

            // 使用数や色値の合計をする.
            Hst* t = &pHst[ ii ];
            ++t->num;
            t->a += a;
            t->g += g;
            t->r += r;
            t->b += b;
        }
    }

    // clutの各色を実際のピクセルの色の平均にする.
    for (unsigned i = idx; i < clutSize; ++i) {
        const Hst*  t    = &pHst[i];
        size_t      n    = t->num;
        unsigned    g=0, r=0, b=0, a=0;
        if (n) {
            a = unsigned(t->a / n);
            a = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
            g = unsigned(t->g / n);
            r = unsigned(t->r / n);
            b = unsigned(t->b / n);
        }
        pClut[i] = argb(a,r,g,b);
    }

    delete[] pHst;
    return clutSize;
}

#endif  // DECREASECOLORHST_H
