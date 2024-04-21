/**
 *  @file   DecreaseColorIfWithin256.h
 *  @brief  32ビット色画の色数が256色(指定色数)に収まるなら、256色画に変換.
 *  @author Masashi KITAMURA
 */

#ifndef DECREASECOLORIFWITHIN256_H
#define DECREASECOLORIFWITHIN256_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>


/** 32ビット色画の色数が256色(指定色数)に収まるなら、256色画に変換. 収まらない場合は変換しない.
 */
template<class DMY=void>
class DecreaseColorIfWithin256 {
public:
    DecreaseColorIfWithin256() {}
    DecreaseColorIfWithin256(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false) {
        conv(pDst,pSrc,w,h,pClut,clutSize, transf);
    }

    static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false);

    static unsigned countColor(const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false);

private:
    template<typename T> static unsigned binary_find_tbl_n(T* tbl, unsigned num, const T& key);
    template<typename T> static unsigned binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key);
};



/// 色数をカウントする. チェックのために1色余分に必要で、clutのメモリは clutSize+1 確保しておくこと.
template<class DMY>
unsigned DecreaseColorIfWithin256<DMY>::countColor(const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf)
{
    assert(clutSize > 0);
    pClut[0] = 0;
    unsigned num = unsigned(transf);
    for (unsigned i = 0; i < w*h && num <= clutSize; ++i) {
        unsigned c = pSrc[i];
        binary_insert_tbl_n(pClut, num, c);
    }
    return num;
}



/** 32ビット色画の色数が256色(指定色数)に収まるなら、256色画に変換. 収まらない場合は変換しない.
 */
template<class DMY>
bool DecreaseColorIfWithin256<DMY>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf)
{
    /// 実際の色数が、clutSize以内に収まるかをチェック.
    unsigned clut[256+1];       // 一個余分に確保.
    unsigned num = 0;
    if (clutSize > 256)
        clutSize = 256;

    num = countColor(pSrc, w, h, clut, clutSize, transf);
    if (num > clutSize)
        return false;

    // clut内に収まる色数だった場合.
    std::memcpy(pClut    , clut, num*sizeof(pClut[0]));
    if (num < clutSize)
        std::memset(pClut+num, 0, (clutSize-num)*sizeof(pClut[0]));
    for (unsigned i = 0; i < w*h; ++i)
        pDst[i] = binary_find_tbl_n(&clut[0], num, pSrc[i]);
    return true;
}



/** 整列されたテーブル中でkeyがある位置(番号:0 ～ num-1)を返す. なければ、numを返す.
 */
template<class DMY>
template<typename T>
unsigned DecreaseColorIfWithin256<DMY>::binary_find_tbl_n(T* tbl, unsigned num, const T& key)
{
    unsigned    low = 0;
    unsigned    hi  = num;
    while (low < hi) {
        unsigned    mid = (low + hi - 1) / 2;
        if (key < tbl[mid]) {
            hi = mid;
        } else if (tbl[mid] < key) {
            low = mid + 1;
        } else {
            return mid;
        }
    }
    return num;
}



/** テーブルpTblに値keyを追加. 範囲チェックは予め行っていること前提.
 *  @return テーブル中のkeyの位置.
 */
template<class DMY>
template<typename T>
unsigned DecreaseColorIfWithin256<DMY>::binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key) {
    unsigned    hi  = rNum;
    unsigned    low = 0;
    unsigned    mid = 0;
    while (low < hi) {
        mid = (low + hi - 1) / 2;
        if (key < pTbl[mid]) {
            hi = mid;
        } else if (pTbl[mid] < key) {
            ++mid;
            low = mid;
        } else {
            return mid; /* 同じものがみつかったので追加しない */
        }
    }

    // 新規登録
    ++rNum;

    // 登録箇所のメモリを空ける
    for (hi = rNum; --hi > mid;) {
        pTbl[hi] = pTbl[hi-1];
    }

    // 登録
    pTbl[mid] = key;
    return mid;
}


#endif  // DECREASECOLORIFWITHIN256_H
