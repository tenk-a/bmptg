/**
 *  @file   GrayClut.hpp
 *  @brief  灰色clut画関係の変換
 *  @author Masashi KITAMURA
 */

#ifndef GRAYCLUT_HPP
#define GRAYCLUT_HPP

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>


/// clutに従って32ビット色画を8ビット色画に変換. デフォルトclutの作成.
template<class A=int>
class GrayClut {
public:
    /// グレイ画像か?
    static bool isGrey(const unsigned* pSrc, unsigned w, unsigned h);
    static bool isGreyRGB(const unsigned* pSrc, unsigned w, unsigned h);

    /// グレイに近い画像か?
    static bool isNearGrey(const unsigned* pSrc, unsigned w, unsigned h, int threshold=12);

    /// グレイ(モノクロ)パレットの作成. 必ず256色分のメモリを用意のこと. bpp=3,4,6,8
    static void getFixedGreyClut(unsigned *clut, unsigned clutSize, unsigned clutBpp=8);

    /// 入力がグレイ画像前提で 8bit画像化.
    static bool fromGreyToBpp8(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h);

    /// モノクロの32ビット色画を前提にbpp(1,2,4)ビット色画に変換.
    static bool fromGreyToBppN(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, int bpp);

    /// モノクロの32ビット色画を前提に1ビット色画に変換. clutも作成.
    static bool fromGreyToBpp1Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut);

    /// モノクロの32ビット色画を前提に2ビット色画に変換. clutも作成.
    static bool fromGreyToBpp2Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut);

    /// モノクロの32ビット色画を前提に4ビット色画に変換. clutも作成.
    static bool fromGreyToBpp4Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut);
};


/// グレイ画像か?
template<class A>
bool GrayClut<A>::isGrey(const unsigned* pSrc, unsigned w, unsigned h)
{
    unsigned a = ARGB_A(pSrc[0]);
    for (unsigned j = 0; j < w * h; ++j) {
        unsigned c = pSrc[j];
        unsigned r = ARGB_R(c);
        if (r != ARGB_G(c) || r != ARGB_B(c) || a != ARGB_A(c))
            return false;
    }
    return true;
}

/// RGB部分がグレイ画像か?
template<class A>
bool GrayClut<A>::isGreyRGB(const unsigned* pSrc, unsigned w, unsigned h)
{
    for (unsigned j = 0; j < w * h; ++j) {
        unsigned c = pSrc[j];
        unsigned r = ARGB_R(c);
        if (r != ARGB_G(c) || r != ARGB_B(c))
            return false;
    }
    return true;
}

/// グレイ画像に近いか?
template<class A>
bool GrayClut<A>::isNearGrey(const unsigned* pSrc, unsigned w, unsigned h, int threshold)
{
    size_t  wh     = w * h;
    size_t  comp   = 0;
    size_t  near1  = 0;
    size_t  near2  = 0;
    size_t  ufar   = 0;
    size_t  vfar   = 0;
    int     a      = ARGB_A(pSrc[0]);
    for (size_t j = 0; j < wh; ++j) {
        unsigned c = pSrc[j];
        int      r = ARGB_R(c);
        int      g = ARGB_G(c);
        int      b = ARGB_B(c);
        //int    y = ( 58661*g +  29891*r + 11448*b) * (2048>>5) / ( 255 * (100000>>5) );
        int      u = (-33126*g -  16874*r + 50000*b) * (1024>>5) / ( 255 * (100000>>5) ); // + 512;
        int      v = (-41869*g +  50000*r -  8131*b) * (1024>>5) / ( 255 * (100000>>5) ); // + 512;
        int au = abs(u);
        int av = abs(v);
        if (au == 0 && av == 0) {
            ++comp;
        } else if (au <= 64 && av <= 64) {
            ++near1;
        } else if (au <= 128 && av <= 128) {
            ++near2;
        } else {
            if (au > 128)
                ++ufar;
            if (av > 128)
                ++vfar;
        }
    }
 #if 0
    printf(" @chk comp:%%%5.2f", double(comp)*100/wh);
    printf(" n1:%%%5.2f", double(near1)*100/wh);
    printf(" n2:%%%5.2f", double(near2)*100/wh);
    printf(" uf=%%%5.2f(%6d)", double(ufar)*100/wh, ufar);
    printf(" vf=%%%5.2f(%6d)", double(vfar)*100/wh, vfar);
 #endif
    return (100 * comp / wh >= 80) /* && (1000 * near2 / wh == 0)*/ && (100000 * ufar / wh == 0) && (100000 * vfar / wh == 0);
}


/** グレイ(モノクロ)パレットの作成.
 * bpp : 3,4,6,8
 */
template<class A>
void GrayClut<A>::getFixedGreyClut(unsigned *clut, unsigned clutSize, unsigned bpp)
{
    assert(clutSize > 0 && bpp > 0 && bpp <= 8);
    if (clutSize > 256)
        clutSize = 256;
    unsigned size = 1 << bpp;
    if (size > clutSize)
        size   = clutSize;
    int maxVal = size - 1;
    unsigned i;
    for (i = 0; i < size; ++i) {
        unsigned c = 255 * i / maxVal;
        clut[i] = (0xFF<<24)|(c << 16)|(c << 8)|c;
    }
    for (; i < clutSize; ++i) {
        clut[i] = 0;
    }
}


/// モノクロの32ビット色画を前提に8ビット色画に変換.
template<class A>
bool GrayClut<A>::fromGreyToBpp8(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h)
{
    for (unsigned j = 0; j < w * h; ++j)
        pDst[j] = (uint8_t)pSrc[j];
    return true;
}


/// モノクロの32ビット色画を前提にbpp(1,2,4,8)ビット色画に変換.
template<class A>
bool GrayClut<A>::fromGreyToBppN(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, int bpp)
{
    int ma = (1 << bpp) - 1;
    for (unsigned j = 0; j < w * h; ++j)
        pDst[j] = (uint8_t)pSrc[j] * ma / 255;
    return true;
}


/// モノクロの32ビット色画を前提に1ビット色画に変換. clutも作成.
template<class A>
bool GrayClut<A>::fromGreyToBpp1Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut)
{
    int c;
    unsigned wh = w * h;
    // まず bpp8 にする.
    uint64_t total = 0;
    for (unsigned j = 0; j < wh; ++j) {
        pDst[j] = c = (uint8_t)pSrc[j];
        total += c;
    }
    int thr = total / wh;
    uint64_t u_total = 0;
    uint64_t l_total = 0;
    uint32_t u_count = 0;
    uint32_t l_count = 0;
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr) {
            l_total += c;
            ++l_count;
            pDst[j]  = 0;
        } else {
            u_total += c;
            ++u_count;
            pDst[j]  = 1;
        }
    }
    int l = l_total / l_count;
    if (l > 255) l = 255;
    int u = u_total / u_count;
    if (u > 255) u = 255;
    clut[0] = (0xFF<<24)|(l<<16)|(l<<8)|l;
    clut[1] = (0xFF<<24)|(u<<16)|(u<<8)|u;
    return true;
}


/// モノクロの32ビット色画を前提に2ビット色画に変換. clutも作成.
template<class A>
bool GrayClut<A>::fromGreyToBpp2Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut)
{
    int c;
    unsigned wh = w * h;
    // まず bpp8 にする.
    uint64_t total0 = 0;
    for (unsigned j = 0; j < wh; ++j) {
        pDst[j] = c = (uint8_t)pSrc[j];
        total0 += c;
    }

    int thr0 = total0 / wh;
    uint64_t total1[2] = {0};
    uint32_t count1[2] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            total1[0] += c;
            ++count1[0];
        } else {
            total1[1] += c;
            ++count1[1];
        }
    }
    int thr1[2];
    thr1[0] = count1[0] ? total1[0] / count1[0] : 0;
    thr1[1] = count1[1] ? total1[1] / count1[1] : 0;
    uint64_t total2[4] = {0};
    uint32_t count2[4] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            if (c < thr1[0]) {
                total2[0] += c;
                ++count2[0];
                pDst[j] = 0;
            } else {
                total2[1] += c;
                ++count2[1];
                pDst[j] = 1;
            }
        } else {
            if (c < thr1[1]) {
                total2[2] += c;
                ++count2[2];
                pDst[j] = 2;
            } else {
                total2[3] += c;
                ++count2[3];
                pDst[j] = 3;
            }
        }
    }

    for (int i = 0; i < 4; ++i) {
        int c = count2[i] ? total2[i] / count2[i] : 0;
        if (c > 255) c = 255;
        clut[i] = (0xFF<<24)|(c<<16)|(c<<8)|c;
    }
    return true;
}



/// モノクロの32ビット色画を前提に4ビット色画に変換. clutも作成.
template<class A>
bool GrayClut<A>::fromGreyToBpp4Clut(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* clut)
{
    int c;
    unsigned wh = w * h;
    // まず bpp8 にする.
    uint64_t total0 = 0;
    for (unsigned j = 0; j < wh; ++j) {
        pDst[j] = c = (uint8_t)pSrc[j];
        total0 += c;
    }

    int thr0 = total0 / wh;
    uint64_t total1[2] = {0};
    uint32_t count1[2] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            total1[0] += c;
            ++count1[0];
        } else {
            total1[1] += c;
            ++count1[1];
        }
    }
    int thr1[2];
    thr1[0] = count1[0] ? total1[0] / count1[0] : 0;
    thr1[1] = count1[1] ? total1[1] / count1[1] : 0;
    uint64_t total2[4] = {0};
    uint32_t count2[4] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            if (c < thr1[0]) {
                total2[0] += c;
                ++count2[0];
            } else {
                total2[1] += c;
                ++count2[1];
            }
        } else {
            if (c < thr1[1]) {
                total2[2] += c;
                ++count2[2];
            } else {
                total2[3] += c;
                ++count2[3];
            }
        }
    }

    int thr2[4];
    thr2[0] = count2[0] ? total2[0] / count2[0] : 0;
    thr2[1] = count2[1] ? total2[1] / count2[1] : 0;
    thr2[2] = count2[2] ? total2[2] / count2[2] : 0;
    thr2[3] = count2[3] ? total2[3] / count2[3] : 0;
    uint64_t total3[8] = {0};
    uint32_t count3[8] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            if (c < thr1[0]) {
                if (c < thr2[0]) {
                    total3[0] += c;
                    ++count3[0];
                } else {
                    total3[1] += c;
                    ++count3[1];
                }
            } else {
                if (c < thr2[1]) {
                    total3[2] += c;
                    ++count3[2];
                } else {
                    total3[3] += c;
                    ++count3[3];
                }
            }
        } else {
            if (c < thr1[1]) {
                if (c < thr2[2]) {
                    total3[4] += c;
                    ++count3[4];
                } else {
                    total3[5] += c;
                    ++count3[5];
                }
            } else {
                if (c < thr2[3]) {
                    total3[6] += c;
                    ++count3[6];
                } else {
                    total3[7] += c;
                    ++count3[7];
                }
            }
        }
    }


    int thr3[8];
    thr3[0] = count3[0] ? total3[0] / count3[0] : 0;
    thr3[1] = count3[1] ? total3[1] / count3[1] : 0;
    thr3[2] = count3[2] ? total3[2] / count3[2] : 0;
    thr3[3] = count3[3] ? total3[3] / count3[3] : 0;
    thr3[4] = count3[4] ? total3[4] / count3[4] : 0;
    thr3[5] = count3[5] ? total3[5] / count3[5] : 0;
    thr3[6] = count3[6] ? total3[6] / count3[6] : 0;
    thr3[7] = count3[7] ? total3[7] / count3[7] : 0;
    uint64_t total4[16] = {0};
    uint32_t count4[16] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        c = pDst[j];
        if (c < thr0) {
            if (c < thr1[0]) {
                if (c < thr2[0]) {
                    if (c < thr3[0]) {
                        total4[0] += c;
                        ++count4[0];
                        pDst[j] = 0;
                    } else {
                        total4[1] += c;
                        ++count4[1];
                        pDst[j] = 1;
                    }
                } else {
                    if (c < thr3[1]) {
                        total4[2] += c;
                        ++count4[2];
                        pDst[j] = 2;
                    } else {
                        total4[3] += c;
                        ++count4[3];
                        pDst[j] = 3;
                    }
                }
            } else {
                if (c < thr2[1]) {
                    if (c < thr3[2]) {
                        total4[4] += c;
                        ++count4[4];
                        pDst[j] = 4;
                    } else {
                        total4[5] += c;
                        ++count4[5];
                        pDst[j] = 5;
                    }
                } else {
                    if (c < thr3[3]) {
                        total4[6] += c;
                        ++count4[6];
                        pDst[j] = 6;
                    } else {
                        total4[7] += c;
                        ++count4[7];
                        pDst[j] = 7;
                    }
                }
            }
        } else {
            if (c < thr1[1]) {
                if (c < thr2[2]) {
                    if (c < thr3[4]) {
                        total4[8] += c;
                        ++count4[8];
                        pDst[j] = 8;
                    } else {
                        total4[9] += c;
                        ++count4[9];
                        pDst[j] = 9;
                    }
                } else {
                    if (c < thr3[5]) {
                        total4[10] += c;
                        ++count4[10];
                        pDst[j] = 10;
                    } else {
                        total4[11] += c;
                        ++count4[11];
                        pDst[j] = 11;
                    }
                }
            } else {
                if (c < thr2[3]) {
                    if (c < thr3[6]) {
                        total4[12] += c;
                        ++count4[12];
                        pDst[j] = 12;
                    } else {
                        total4[13] += c;
                        ++count4[13];
                        pDst[j] = 13;
                    }
                } else {
                    if (c < thr3[7]) {
                        total4[14] += c;
                        ++count4[14];
                        pDst[j] = 14;
                    } else {
                        total4[15] += c;
                        ++count4[15];
                        pDst[j] = 15;
                    }
                }
            }
        }
    }

    for (int i = 0; i < 16; ++i) {
        int c = count4[i] ? total4[i] / count4[i] : 0;
        if (c > 255) c = 255;
        clut[i] = (0xFF<<24)|(c<<16)|(c<<8)|c;
    }

    return true;
}


#endif
