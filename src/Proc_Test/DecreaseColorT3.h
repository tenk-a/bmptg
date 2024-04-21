/**
 *  @file   DecreaseColorT3.h
 *  @brief  ���F����. �p�x�J�E���g���đ������̏�. ��(����)�Ή���.
 *  @author Masashi KITAMURA
 */

#ifndef DECREASECOLORT3_H
#define DECREASECOLORT3_H

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

    static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h,unsigned* clut, unsigned clutSize);

private:
    static unsigned conv1(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                        , unsigned* pClut, unsigned clutSize, int minA, int maxA, int idx);

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
      #if defined _WIN32                // vc��stdint.h���Ȃ��̂ł��̑΍�.
        typedef unsigned __int64 sum_t; // win�n�R���p�C����vc�݊��ŌÂ�����__int64������.
      #else
        typedef uint64_t         sum_t;
      #endif
        sum_t           g;
        sum_t           r;
        sum_t           b;
        sum_t           a;
        std::size_t     num;
        short           no;

        bool operator<(const Hst& r) const {
            return this->num > r.num;
        }
    };

};



/// 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A>
bool DecreaseColor<A>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize)
{
    // �s�N�Z����clut������. (�����摜��Ԃɂ���)
    std::memset(pDst, 0, w*h);
    std::memset(pClut, 0, 256*sizeof(pClut[0]));

    // ����,������,�s�����̑��݃`�F�b�N
    enum { MIN_A = ALP_D, MAX_A = 255-ALP_D };
    bool tranf = false;
    bool alpf  = false;
    bool nalpf = false;
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

    if (nalpf == 0) {
        // �s�������Ȃ��ꍇ�́A���ׂẴp���b�g�𔼓�������.
        return conv1(pDst,pSrc,w,h,pClut,clutSize, MIN_A, 255, tranf) != 0;
    } else if (alpf == false || clutSize < 40) {
        // �s�����Ɣ����F�݂̂̂Ƃ�
        return conv1(pDst,pSrc,w,h,pClut,clutSize, MAX_A+1, 255, tranf) != 0;
    } else {
        // �s�����Ɣ�����������Ƃ� (�o�X�g�A�b�v��/����������z��)
        unsigned idx = clutSize / 8;    // �K���ɔ������������߂�.(256�F����32�F)
        if (idx < 20)
            idx = 20;
        idx = conv1(pDst,pSrc,w,h,pClut,idx, MIN_A, MAX_A, tranf);
        return conv1(pDst,pSrc,w,h,pClut,clutSize, MAX_A+1, 255, idx) != 0;
    }
}



/// 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A>
unsigned DecreaseColor<A>::conv1(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                                    , unsigned* pClut, unsigned clutSize, int minA, int maxA, int idx)
{
    assert(clutSize <= 256 && clutSize >= idx+2);
    enum { BI=4, N=1<<BI, SH=8-BI };
    enum { NUM = 1 + N*N*N };
    assert( NUM >= 256 );
    Hst* pHst = new Hst[ NUM ];
    std::memset(pHst, 0, sizeof(Hst) * NUM);

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
    // �������m����clutSize��I�o(�擪�Ɉړ�).
    std::nth_element( &pHst[0], &pHst[colNum], &pHst[NUM], std::less<Hst>() );

    std::memset(pClut+idx, 0, colNum*sizeof(pClut[0]));

    // �������̂���F���擾.
    unsigned n = 0;
    for (unsigned j = 0; j < colNum; ++j) {
        const Hst*  h = &pHst[j];
        std::size_t num = h->num;
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
    // ���ۂɎg�p����Ă���clut�������߂�.
    if (n < 2)  // �������Œ�2�F�͎g�����Ƃ�.
        n = 2;
    clutSize = n + idx;

    // �Â����̏��ɕ��ג���.
    std::sort( &pClut[idx], &pClut[clutSize] );

    // �����clut�ŁA���ׂẴs�N�Z����clutSize�F��.
    // �߂��F�̏����Ƃ��āA
    //   - a,r,g,b�̒l���߂����m(�F���m�̍������������m)
    //   - 1�F����r,g,b�̋P�x�̑召�֌W���A�߂����m
    // ���`�F�b�N.
    std::memset(pHst, 0, sizeof(Hst) * clutSize);
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

            // �g�p����F�l�̍��v������.
            Hst* t = &pHst[ ii ];
            ++t->num;
            t->a += a;
            t->g += g;
            t->r += r;
            t->b += b;
        }
    }

    // clut�̊e�F�����ۂ̃s�N�Z���̐F�̕��ςɂ���.
    for (unsigned i = idx; i < clutSize; ++i) {
        const Hst*  t    = &pHst[i];
        unsigned    n    = t->num;
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

    delete pHst;
    return clutSize;
}

#endif
