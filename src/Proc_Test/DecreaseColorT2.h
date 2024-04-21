/**
 *  @file   DecreaseColorT2.h
 *  @brief  ���F����. �p�x�J�E���g���đ������̏�. ���F�Ή���.
 *  @author Masashi KITAMURA
 */

#ifndef DECREASECOLORT2_H
#define DECREASECOLORT2_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <functional>
#if defined _WIN32 == 0
#include <stdint.h>
#endif



/// ���F����. 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A=void>
class DecreaseColor {
public:
    DecreaseColor() {; }
    DecreaseColor(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize) {
        conv(pSrc,w,h,pDst,clut,clutSize);
    }

    static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut, unsigned clutSize);

private:
    static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
        return (a << 24) | (r << 16) | (g << 8) | (b);
    }

    static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
    static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
    static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
    static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

private:
    struct Hst {
      #if defined _WIN32                // vc��stdint.h���Ȃ��̂ł��̑΍�.
        typedef unsigned __int64 sum_t; // win�n�R���p�C����vc�݊��ŌÂ�����__int64������.
      #else
        typedef uint64_t         sum_t;
      #endif
        sum_t           g;
        sum_t           r;
        sum_t           b;
        std::size_t     num;
        short           no;
        short           a;

        bool operator<(const Hst& r) const {
            return this->num > r.num;
        }
    };
};



/// 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A>
bool DecreaseColor<A>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize)
{
    assert(clutSize <= 256);
    enum { MIN_A = 2 };
    enum { BI=4, N=1<<BI, SH=8-BI };
    enum { NUM = 1 + N*N*N };
    assert( NUM >= 256 );
    Hst* pHst = new Hst[ NUM ];
    std::memset(pHst, 0, sizeof(Hst) * NUM);
    int top = 0;
    for (unsigned i = 0; i < w * h; ++i) {
        unsigned c = pSrc[i];
        int      a = argb_a(c);
        unsigned g = argb_g(c);
        unsigned r = argb_r(c);
        unsigned b = argb_b(c);
        unsigned no;
        if (a < MIN_A) {
            a   = 0;
            no  = 0;
            top = 1;
        } else {
            a   = 0xff;
            no  = 1 + (g>>SH) * N*N + (r>>SH) * N + (b>>SH);
        }
        Hst* h = &pHst[no];
        ++h->num;
        h->no = no;
        h->a  = a;
        h->g += g;
        h->r += r;
        h->b += b;
    }

    // �������m����clutSize��I�o(�擪�Ɉړ�).
    std::nth_element( &pHst[top], &pHst[clutSize], &pHst[NUM], std::less<Hst>() );

    // �������̂���F���擾.
    std::memset(pClut, 0, clutSize*sizeof(pClut[0]));
    unsigned n = 0;
    for (unsigned j = 0; j < clutSize; ++j) {
        const Hst*  h   = &pHst[j];
        std::size_t num = h->num;
        if (num == 0)
            continue;   // break;
        int      a    = h->a;
        unsigned g    = unsigned(h->g / num);
        unsigned r    = unsigned(h->r / num);
        unsigned b    = unsigned(h->b / num);
        pClut[n] = argb(a,r,g,b);
        ++n;
    }
    // ���ۂɎg�p����Ă���clut�������߂�.
    if (n < 2)  // �������Œ�2�F�͎g�����Ƃ�.
        n = 2;
    clutSize = n;

    // clut���Â����̏��ɕ��ג���.
    std::sort( &pClut[top], &pClut[clutSize] );

    // �����clut�ŁA���ׂẴs�N�Z����clutSize�F��.
    // �F�I���̏����Ƃ��āA
    //   - a,r,g,b�̒l���߂����m(�F���m�̍������������m)
    //   - 1�F����r,g,b�̋P�x�̑召�֌W���A�߂����m
    // ���`�F�b�N.
    std::memset(pHst, 0, sizeof(Hst) * clutSize);
    for (unsigned j = 0; j < w * h; ++j) {
        unsigned c  = pSrc[j];
        int      a  = argb_a(c);
        int      g  = argb_g(c);
        int      r  = argb_r(c);
        int      b  = argb_b(c);
        int      o  = g - r;
        int      p  = r - b;
        int      q  = b - g;

        unsigned ii = 0;
        if (a >= MIN_A) {   // �����ȊO�̃s�N�Z���������Ƃ�.
            a = 0xff;
            unsigned k = 0xFFFFFFFF;
            for (unsigned i = 0; i < clutSize; ++i) {
                unsigned ic = pClut[i];
                if (int(argb_a(ic)) >= MIN_A) {
                    int         ig = argb_g(ic);
                    int         ir = argb_r(ic);
                    int         ib = argb_b(ic);
                    int         io = ig - ir;
                    int         ip = ir - ib;
                    int         iq = ib - ig;

                    int         gg = ig - g;
                    int         rr = ir - r;
                    int         bb = ib - b;
                    int         oo = io - o;
                    int         pp = ip - p;
                    int         qq = iq - q;
                    unsigned    ik = /* a*a+ */ gg*gg + rr*rr + bb*bb + oo*oo + pp*pp + qq*qq;
                    if (ik < k) {
                        k  = ik;
                        ii = i;
                    }
                }
            }
        } else {
            a = g = r = b = 0;
        }
        pDst[j] = ii;

        // �g�p����F�l�̍��v.
        Hst* t = &pHst[ ii ];
        ++t->num;
        t->a  = a;
        t->g += g;
        t->r += r;
        t->b += b;
    }

    // clut�̊e�F�����ۂ̃s�N�Z���̐F�̕��ςɂ���.
    for (unsigned i = 0; i < clutSize; ++i) {
        const Hst*  t    = &pHst[i];
        unsigned    n    = t->num;
        unsigned    g=0, r=0, b=0, a=t->a;
        if (n) {
            g = unsigned(t->g / n);
            r = unsigned(t->r / n);
            b = unsigned(t->b / n);
        }
        pClut[i] = argb(a,r,g,b);
    }

    delete pHst;
    return true;
}


#endif
