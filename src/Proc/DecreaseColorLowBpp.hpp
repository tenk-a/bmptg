/**
 *  @file   DecreaseColorLowBpp.hpp
 *  @brief  32ビット色画の色数が256色(指定色数)に収まるなら、256色画に変換.
 *  @author Masashi KITAMURA
 */

#ifndef DECREASECOLORLOWBPP_HPP
#define DECREASECOLORLOWBPP_HPP

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>
#include "pix32_kyuv.h"


/** 32ビット色画の色数が256色(指定色数)に収まるなら、256色画に変換. 収まらない場合は変換しない.
 */
template<class DMY=void>
class DecreaseColorLowBpp {
    DecreaseColorLowBpp() {}
public:
    static bool convPix32ToBpp1(uint8_t* pDst, const uint32_t* pSrc, unsigned w, unsigned h, unsigned* clut);
    static bool convPix32ToBpp2(uint8_t* pDst, const uint32_t* pSrc, unsigned w, unsigned h, unsigned* clut);
};


/// 32ビット色画を1ビット色画に変換. clut作成.
template<class A>
bool DecreaseColorLowBpp<A>::convPix32ToBpp1(uint8_t* pDst, const uint32_t* pSrc, unsigned w, unsigned h, unsigned* clut)
{
    int             wh   = w * h;
    pix32_kyuv_t*   kyuv = new pix32_kyuv_t[ wh ];
    if (!kyuv)
        return false;
    pix32_kyuvFromRgb(kyuv, w, h, 0, 0, pSrc, w, h);

    uint64_t  total0 = 0;
    for (unsigned j = 0; j < wh; ++j) {
        total0 += kyuv[j].at[0];
    }
    int thr0 = total0 / wh;
    uint64_t total1[2][4] = {0};
    uint32_t count1[2]    = {0};
    for (unsigned j = 0; j < wh; ++j) {
        int c = kyuv[j].at[0];
        if (c < thr0) {
            for (int i = 0; i < 4; ++i)
                total1[0][i] += kyuv[j].at[i];
            ++count1[0];
            pDst[j] = 0;
        } else {
            for (int i = 0; i < 4; ++i)
                total1[1][i] += kyuv[j].at[i];
            ++count1[1];
            pDst[j] = 1;
        }
    }

    pix32_kyuv_t    kyuvClut = {0};
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            kyuvClut.at[i] = count1[j] ? total1[j][i] / count1[j] : 0;
        }
        PIX32_RGB_FROM_KYUV(clut[j], kyuvClut.at[0], kyuvClut.at[1], kyuvClut.at[2], kyuvClut.at[3], int);
    }

    delete[] kyuv;

    return true;
}


/// 32ビット色画を2ビット色画に変換. clut作成.
template<class A>
bool DecreaseColorLowBpp<A>::convPix32ToBpp2(uint8_t* pDst, const uint32_t* pSrc, unsigned w, unsigned h, unsigned* clut)
{
    int             wh   = w * h;
    pix32_kyuv_t*   kyuv = new pix32_kyuv_t[ wh ];
    if (!kyuv)
        return false;
    pix32_kyuvFromRgb(kyuv, w, h, 0, 0, pSrc, w, h);

    uint64_t  total0 = 0;
    for (unsigned j = 0; j < wh; ++j) {
        total0 += kyuv[j].at[0];
    }
    int thr0 = total0 / wh;
    uint64_t total1[2] = {0};
    uint32_t count1[2] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        int c = kyuv[j].at[0];
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
    uint64_t total2[4][4] = {{0}};
    uint32_t count2[4] = {0};
    for (unsigned j = 0; j < wh; ++j) {
        int c = kyuv[j].at[0];
        if (c < thr0) {
            if (c < thr1[0]) {
                for (int i = 0; i < 4; ++i)
                    total2[0][i] += kyuv[j].at[i];
                ++count2[0];
                pDst[j] = 0;
            } else {
                for (int i = 0; i < 4; ++i)
                    total2[1][i] += kyuv[j].at[i];
                ++count2[1];
                pDst[j] = 1;
            }
        } else {
            if (c < thr1[1]) {
                for (int i = 0; i < 4; ++i)
                    total2[2][i] += kyuv[j].at[i];
                ++count2[2];
                pDst[j] = 2;
            } else {
                for (int i = 0; i < 4; ++i)
                    total2[3][i] += kyuv[j].at[i];
                ++count2[3];
                pDst[j] = 3;
            }
        }
    }

    pix32_kyuv_t    kyuvClut = {0};
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            kyuvClut.at[i] = count2[j] ? total2[j][i] / count2[j] : 0;
        }
        PIX32_RGB_FROM_KYUV(clut[j], kyuvClut.at[0], kyuvClut.at[1], kyuvClut.at[2], kyuvClut.at[3], int);
    }

    delete[] kyuv;

    return true;
}


#endif  // DECREASECOLORIFWITHIN256_H
