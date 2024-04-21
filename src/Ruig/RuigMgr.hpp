/**
 *  @file RuigMgr.hpp
 *  @brief  画像類似順並べ処理
 *  @author Masashi Kitamura (tenka@6809.net)
 */
#ifndef RUIGMGR_HPP
#define RUIGMGR_HPP

#pragma once

#include <stdint.h>
#include <deque>
#include <set>
#include <map>
#include <list>
#include <vector>
#include "NamedImg.hpp"
#include "../misc/intrusive_ptr_ref_count.hpp"

class RuigFactor;
class RuigMgr;

typedef std::vector<RuigFactor*>        RuigFactors;
typedef intrusive_ptr<RuigFactor>       RuigFactorPtr;
typedef uint64_t                        RuigPtn8x8_t;
typedef uint16_t                        RuigPtn4x4_t;
typedef uint64_t                        RuigC4x4x4_t;   // tColor4bit 4x4


/// 1画像情報 (同一画像をひとまとめにできる版)
class RuigFactor
    : public intrusive_ptr_ref_count<RuigFactor>
    //, public tiny_twlink<RuigFactor, sizeof(intrusive_ptr_ref_count<RuigFactor>) >
{
    friend RuigMgr;
public:
    RuigFactor();
    RuigFactor(NamedImgPtr img, uint32_t fsize=0);

    uint32_t            majorNo() const { return majorNo_; }
    uint32_t            minorNo() const { return minorNo_; }
    uint32_t            subNo()   const { return subNo_; }

    uint32_t            origWidth()  const { return origWidth_; }
    uint32_t            origHeight() const { return origHeight_; }
    //uint32_t          width()  const { return width_; }
    //uint32_t          height() const { return height_; }
    std::string const&  name()   const { return name_; }
    int                 aspect() const { return asp_; }

    RuigFactor const*   parent() const { return parent_; }
    bool                isParentFactor() const { return this == parent_; }

    bool     isLess4FirstSort(RuigFactor const& r) const;
    uint32_t calcCmpVal(RuigFactor const& r) const;
    void     addLink(RuigFactor* factor, uint32_t cmpValue);
    void     checkAndGenSameLinks(RuigFactors& sortFactors, uint32_t& majorNo);
    bool     hasNearSame() const { return minorNo_ > 0 || subNo_ > 0; }

    // にてそうなので後で詳細チェックすべき一覧
    //RuigFactors const& sameLinks() const { return sameLinks_; }
    //RuigFactors const& nearLinks() const { return nearLinks_; }

    void debugPrintLog() const;

private:
    typedef std::multimap<uint32_t, RuigFactor*> ValueLinks;
    typedef std::map<RuigFactor*, size_t>   FactorVals;
    struct NearSameLinksCmp;

    void defaultConstruct();
    bool init(uint8_t const* src, unsigned srcW, unsigned srcH, unsigned bpp);

    void convValueLinks(FactorVals& factorVals);
    void genNearSameLinks(RuigFactors& nearSameLinks);
    bool hasNearLinks();
    void setSameLinks(RuigFactors const& nearSameLinks, size_t idx, uint32_t majorNo, uint32_t minorNo, RuigFactors& result);

private:
    enum    { ASPECT_K = 1000 };
    enum    { PIX_MAX  = 1023 };
    typedef uint32_t    pix_t;

    /// 画像の色(yuv)平均値を元にした整数値
    union PackKey {
        uint32_t    key_;
        struct {
            uint8_t thresholdY_;
            uint8_t thresholdU_;
            uint8_t thresholdV_;
        };
    };

private:
 #ifndef NDEBUG
    char            id_[8];
    char const*     namePtr_;
 #endif
    std::string     name_;          ///< ファイル名
    RuigFactor*     parent_;        ///< same 時の代表へのポインタ. 同じモノが無ければ自分自身を指す.

    uint32_t        majorNo_;       ///< mgrで登録されるときの番号 1～
    uint16_t        minorNo_;       ///< 近いが同型ではないものに振られる番号. 0:近同無 1～:近同有
    uint16_t        subNo_;         ///< 同型内での番号 0:同型無 1～:同型有

    uint32_t        width_;         ///< 横幅
    uint32_t        height_;        ///< 縦幅
    uint32_t        lsize_;         ///< 横幅か縦幅の長いほうのサイズ
    uint32_t        fsize_;         ///< ファイルサイズ

    uint32_t        origWidth_;     ///< 元画像の横幅
    uint32_t        origHeight_;    ///< 元画像の縦幅

private:
    bool            mono_;          ///< モノクロかどうか
    int             asp_;           ///< 正なら aspW_ の値、負なら絶対値がaspH_の値.
    int             aspW_;          ///< 横幅を分母にした1000分率によるアスペクト比
    int             aspH_;          ///< 縦幅を分母にした1000分率によるアスペクト比
    RuigPtn4x4_t    ptnY4x4_;       ///< 4x4 縮小した輝度の2値画像
    PackKey         packKey_;       ///< 画像の色(YUV)平均値を用いた比較用整数値
    RuigPtn8x8_t    ptnY8x8_;       ///< 8x8 縮小した輝度の2値画像
    RuigPtn8x8_t    ptnY8x8sub_;    ///< 8x8 縮小し 二値化後、off側とon側別で輝度平均作って上下で二値化したもの
    RuigPtn8x8_t    ptnU8x8_;       ///< 8x8 縮小したUの2値画像
    RuigPtn8x8_t    ptnV8x8_;       ///< 8x8 縮小したVの2値画像
    RuigPtn8x8_t    ptnY16x16_[2][2]; ///< 16x16 縮小した輝度の2値画像

    //RuigC4x4x4_t  pixC4x4x4_[3];  // 0:y 1:u 2:v
    //uint8_t       pixC8x4x4_[3][4][4];

    pix_t           thresholdY_;    ///< Yを二値化する場合の閾値
    pix_t           thresholdU_;    ///< Uを二値化する場合の閾値
    pix_t           thresholdV_;    ///< Vを二値化する場合の閾値

private:
    uint32_t        sameCount_;     ///< 同一視画の数
    uint32_t        nearCount_;     ///< 近似画の数
    uint32_t        midCount_;      ///< 中程度に近そうな画の数
    //RuigFactors   sameLinks_;     ///< 代表Factorの場合: ほぼ同じとみなした一覧.
    //RuigFactors   nearLinks_;     ///< 代表Factorの場合: 部分的に違う場合もあるがだいたい同じとみなした一覧.
    ValueLinks      valueLinks_;    ///< 同じか近いと思われる画像の評価値とポインタ. 相互に設定される.

private:
    uint8_t         pix8x8_[8][8][3];
};


/// 画像群をまとめて管理する
class RuigMgr {
public:
    RuigMgr();

    void reserve(size_t capa);

    bool insert(NamedImgPtr img, uint32_t fsize=0);

    bool run();

    size_t size() const { return factorBodys_.size(); }
    size_t countNearSameFactors();

    template<class T>
    bool for_each(T& func) {
        RuigFactor const* const* factors = &allFactors_[0];
        size_t  size  = allFactors_.size();
        for (size_t i = 0; i < size; ++i) {
            if (func(factors[i]))
                return true;
        }
        return false;
    }

private:
    bool initAllFactors();
    void checkNearSameLinks();
    void sortFactors();

private:
    struct FirstSortLess {
        bool operator()(RuigFactorPtr const& l, RuigFactorPtr const& r) const {
            return l->isLess4FirstSort(*r);
        }
    };

    typedef std::multiset< RuigFactorPtr, FirstSortLess >  RuigFactorBodys;    // 全部の実体用

private:
    RuigFactorBodys     factorBodys_;       ///< 全factorの実体.
    RuigFactors         allFactors_;        ///< 全factorソート済の一覧(舐める用)
};

#endif
