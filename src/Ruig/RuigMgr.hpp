/**
 *  @file RuigMgr.hpp
 *  @brief  �摜�ގ������׏���
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


/// 1�摜��� (����摜���ЂƂ܂Ƃ߂ɂł����)
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

    // �ɂĂ����Ȃ̂Ō�ŏڍ׃`�F�b�N���ׂ��ꗗ
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

    /// �摜�̐F(yuv)���ϒl�����ɂ��������l
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
    std::string     name_;          ///< �t�@�C����
    RuigFactor*     parent_;        ///< same ���̑�\�ւ̃|�C���^. �������m��������Ύ������g���w��.

    uint32_t        majorNo_;       ///< mgr�œo�^�����Ƃ��̔ԍ� 1�`
    uint16_t        minorNo_;       ///< �߂������^�ł͂Ȃ����̂ɐU����ԍ�. 0:�ߓ��� 1�`:�ߓ��L
    uint16_t        subNo_;         ///< ���^���ł̔ԍ� 0:���^�� 1�`:���^�L

    uint32_t        width_;         ///< ����
    uint32_t        height_;        ///< �c��
    uint32_t        lsize_;         ///< �������c���̒����ق��̃T�C�Y
    uint32_t        fsize_;         ///< �t�@�C���T�C�Y

    uint32_t        origWidth_;     ///< ���摜�̉���
    uint32_t        origHeight_;    ///< ���摜�̏c��

private:
    bool            mono_;          ///< ���m�N�����ǂ���
    int             asp_;           ///< ���Ȃ� aspW_ �̒l�A���Ȃ��Βl��aspH_�̒l.
    int             aspW_;          ///< �����𕪕�ɂ���1000�����ɂ��A�X�y�N�g��
    int             aspH_;          ///< �c���𕪕�ɂ���1000�����ɂ��A�X�y�N�g��
    RuigPtn4x4_t    ptnY4x4_;       ///< 4x4 �k�������P�x��2�l�摜
    PackKey         packKey_;       ///< �摜�̐F(YUV)���ϒl��p������r�p�����l
    RuigPtn8x8_t    ptnY8x8_;       ///< 8x8 �k�������P�x��2�l�摜
    RuigPtn8x8_t    ptnY8x8sub_;    ///< 8x8 �k���� ��l����Aoff����on���ʂŋP�x���ύ���ď㉺�œ�l����������
    RuigPtn8x8_t    ptnU8x8_;       ///< 8x8 �k������U��2�l�摜
    RuigPtn8x8_t    ptnV8x8_;       ///< 8x8 �k������V��2�l�摜
    RuigPtn8x8_t    ptnY16x16_[2][2]; ///< 16x16 �k�������P�x��2�l�摜

    //RuigC4x4x4_t  pixC4x4x4_[3];  // 0:y 1:u 2:v
    //uint8_t       pixC8x4x4_[3][4][4];

    pix_t           thresholdY_;    ///< Y���l������ꍇ��臒l
    pix_t           thresholdU_;    ///< U���l������ꍇ��臒l
    pix_t           thresholdV_;    ///< V���l������ꍇ��臒l

private:
    uint32_t        sameCount_;     ///< ���ꎋ��̐�
    uint32_t        nearCount_;     ///< �ߎ���̐�
    uint32_t        midCount_;      ///< �����x�ɋ߂����ȉ�̐�
    //RuigFactors   sameLinks_;     ///< ��\Factor�̏ꍇ: �قړ����Ƃ݂Ȃ����ꗗ.
    //RuigFactors   nearLinks_;     ///< ��\Factor�̏ꍇ: �����I�ɈႤ�ꍇ�����邪�������������Ƃ݂Ȃ����ꗗ.
    ValueLinks      valueLinks_;    ///< �������߂��Ǝv����摜�̕]���l�ƃ|�C���^. ���݂ɐݒ肳���.

private:
    uint8_t         pix8x8_[8][8][3];
};


/// �摜�Q���܂Ƃ߂ĊǗ�����
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

    typedef std::multiset< RuigFactorPtr, FirstSortLess >  RuigFactorBodys;    // �S���̎��̗p

private:
    RuigFactorBodys     factorBodys_;       ///< �Sfactor�̎���.
    RuigFactors         allFactors_;        ///< �Sfactor�\�[�g�ς̈ꗗ(�r�߂�p)
};

#endif
