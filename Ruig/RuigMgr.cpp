/**
 *	@file RuigMgr.cpp
 *	@brief	画像類似順並べ処理
 *  @author Masashi Kitamura (tenka@6809.net)
 */
#include "RuigMgr.hpp"
#include <math.h>

//#define USE_NATIVE_MINOR_NO

// -----------------------------------------------------------------------------------------------
#ifdef NDEBUG
#define DBGPRINTF(...)
#else
#define DBGPRINTF(...)      printf(__VA_ARGS__)
#endif

// 整数値中のonビットの数を数える
#if defined(__GNUC__) && (defined _WIN64 || defined __x86_64__)
#define FORCE_INLINE    __attribute__((always_inline))
static FORCE_INLINE uint32_t bit_popcount16(uint32_t x) { return __builtin_popcount((uint16_t)(x)); }
static FORCE_INLINE uint32_t bit_popcount32(uint32_t x) { return __builtin_popcount(x); }
static FORCE_INLINE uint32_t bit_popcount64(uint64_t x) { return __builtin_popcountl(x); }
#elif defined(_MSC_VER) && _MSC_VER >= 1600
#define FORCE_INLINE    __forceinline
static FORCE_INLINE uint32_t bit_popcount16(uint16_t bits) { return __popcnt16(bits); }
static FORCE_INLINE uint32_t bit_popcount32(uint32_t bits) { return __popcnt(bits); }
#ifdef _WIN64
static FORCE_INLINE uint32_t bit_popcount64(uint64_t bits) { return __popcnt64(bits); }
#else
static FORCE_INLINE uint32_t bit_popcount64(uint64_t bits) { return __popcnt((uint32_t)(bits>>32)) + __popcnt((uint32_t)bits); }
#endif
#else
#define FORCE_INLINE    inline
static FORCE_INLINE uint32_t bit_popcount16(uint32_t bits) {
    bits = (bits & 0x5555U) + ((bits >> 1) & 0x5555U);
    bits = (bits & 0x3333U) + ((bits >> 2) & 0x3333U);
    bits = (bits & 0x0f0fU) + ((bits >> 4) & 0x0f0fU);
    return (uint8_t) (bits) +(uint8_t) (bits >> 8);
}
static FORCE_INLINE uint32_t bit_popcount32(uint32_t bits) {
    bits = (bits & 0x55555555UL) + ((bits >> 1) & 0x55555555UL);
    bits = (bits & 0x33333333UL) + ((bits >> 2) & 0x33333333UL);
    bits = (bits & 0x0f0f0f0fUL) + ((bits >> 4) & 0x0f0f0f0fUL);
    bits = (bits & 0x00ff00ffUL) + ((bits >> 8) & 0x00ff00ffUL);
    return (uint16_t) (bits) +(uint16_t) (bits >> 16);
}
static FORCE_INLINE uint32_t bit_popcount64(uint64_t bits) {
    bits = (bits & 0x5555555555555555UL) + ((bits >> 1) & 0x5555555555555555UL);
    bits = (bits & 0x3333333333333333UL) + ((bits >> 2) & 0x3333333333333333UL);
    bits = (bits & 0x0f0f0f0f0f0f0f0fUL) + ((bits >> 4) & 0x0f0f0f0f0f0f0f0fUL);
    bits = (bits & 0x00ff00ff00ff00ffUL) + ((bits >> 8) & 0x00ff00ff00ff00ffUL);
    bits = (bits & 0x0000ffff0000ffffUL) + ((bits >> 16) & 0x0000ffff0000ffffUL);
    return (uint32_t) (bits) +(uint32_t) (bits >> 32);
}
#endif

/** 四捨五入
 */
static FORCE_INLINE double round(double x)
{
    return (x >= 0) ? floor(x + 0.5) : ceil(x - 0.5);
}

/// 近似評価値の判定に使う閾値等
enum CmpVal {
    CMPVAL_SAME = 6,
    CMPVAL_NEAR = 20, //16,
    CMPVAL_MID  = 24,
    CMPVAL_LARGE= 120,
};


// -----------------------------------------------------------------------------------------------

int g_debug_flag = 0;

/** デフォルト・コンストラクタの下請け
 */
void RuigFactor::defaultConstruct()
{
 #ifndef NDEBUG
    strcpy(id_, "ruigdat");
    namePtr_ = "(no data)";
 #endif

    parent_     = NULL;

    majorNo_    = 0;
    minorNo_    = 0;
    subNo_      = 0;

    width_      = 0;
    height_     = 0;
    lsize_      = 0;
    fsize_      = 0;
    origWidth_  = 0;
    origHeight_ = 0;

    mono_       = false;

    asp_        = 0;
    aspW_       = 0;
    aspH_       = 0;
    ptnY4x4_    = 0;
    packKey_.key_ = 0;
    ptnY8x8_    = 0;
    ptnY8x8sub_ = 0;
    ptnU8x8_    = 0;
    ptnV8x8_    = 0;
    ptnY16x16_[0][0] = 0;
    ptnY16x16_[0][1] = 0;
    ptnY16x16_[1][0] = 0;
    ptnY16x16_[1][1] = 0;

 #if 1
    memset(pix8x8_, 0, sizeof pix8x8_);
 #elif 1
    memset(pixC8x4x4_, 0, sizeof pixC8x4x4_);
 #else
    pixC4x4x4_[0] = 0;
    pixC4x4x4_[1] = 0;
    pixC4x4x4_[2] = 0;
 #endif

    thresholdY_ = 0;
    thresholdU_ = 0;
    thresholdV_ = 0;

    sameCount_  = 0;
    nearCount_  = 0;
    midCount_   = 0;
}


/** デフォルト・コンストラクタ
 */
RuigFactor::RuigFactor()
{
    defaultConstruct();
}

/** コンストラクタ
 */
RuigFactor::RuigFactor(NamedImgPtr imgPtr, uint32_t fsize)
{
    defaultConstruct();
    if (!imgPtr)
        return;
    name_ = imgPtr->name();
 #ifndef NDEBUG
    namePtr_ = name_.c_str();
 #endif
    fsize_ = fsize;
    BppCnvImg const& img = imgPtr->img();
    init(img.image(), img.width(), img.height());
    origWidth_  = imgPtr->width();
    origHeight_ = imgPtr->height();
}

/** 初期化.
 *  画像から 近似チェック用の各種情報を生成
 */
void RuigFactor::init(uint8_t const* src, unsigned srcW, unsigned srcH)
{
    enum { SSZ = 4 };
    enum { MSZ = 8 };
    enum { LSZ = 16 };
    assert(src);
    assert(srcW >= LSZ);
    assert(srcH >= LSZ);
    typedef pix_t   Pix3[3];
    //Pix3          pixM[MSZ][MSZ] = { 0 };
    Pix3            pixL[LSZ][LSZ] = { 0 };
    unsigned        wid[LSZ];
    unsigned        hig[LSZ];

    // アスペクト比関係
    width_  = srcW;
    height_ = srcH;
    aspW_   = int(round(ASPECT_K * double(srcH) / double(srcW)));
    aspH_   = int(round(ASPECT_K * double(srcW) / double(srcH)));
    if (srcW >= srcH) {
        lsize_ = width_;
        asp_   = -int(aspH_);   // 横幅のほうが長いほど 値は小さくなる
    } else {
        lsize_ = height_;
        asp_   = int(aspW_);    // 縦幅のほうが長いほど 値は大きくなる
    }

    DBGPRINTF("%s %d*%d asp=%d lsize_=%d\n", name_.c_str(), width_, height_, asp_, lsize_);

    // 平均を取るための１範囲の縦横ピクセル数を調整
    unsigned        x1 = 0;
    unsigned        y1 = 0;
    for (unsigned i = 0; i < LSZ; ++i) {
        unsigned    x2 = (i + 1) * srcW / LSZ;
        unsigned    y2 = (i + 1) * srcH / LSZ;
        wid[i] = x2 - x1;
        hig[i] = y2 - y1;
        x1 = x2;
        y1 = y2;
    }

    // 画像を 16x16 縮小する.
 #if defined _WIN64 || defined __x86_64__
    typedef double      total_t;
 #else
    typedef uint64_t    total_t;
 #endif
    total_t     totalMidY = 0;
    total_t     totalMidU = 0;
    total_t     totalMidV = 0;
    uint8_t const* s = src;
    enum { K = 64 };
    for (unsigned dy = 0; dy < LSZ; ++dy) {
        for (unsigned dx = 0; dx < LSZ; ++dx) {
            total_t ti = 0, tu = 0, tv = 0;
            unsigned w = wid[dx];
            unsigned h = hig[dy];
            for (unsigned y = 0; y < h; ++y) {
                for (unsigned x = 0; x < w; ++x) {
                    int r = s[0];
                    int g = s[1];
                    int b = s[2];
                    s += 3;
                    uint32_t    i = K * (38444U * g + 19589U * r +  7503U * b             ) >> 16;
                    uint32_t    u = K * (-21709 * g + -11059 * r +  32768 * b +  32768*255) >> 16;
                    uint32_t    v = K * (-27439 * g +  32768 * r +  -5329 * b +  32768*255) >> 16;
                    ti += i;
                    tu += u;
                    tv += v;
                }
            }
            Pix3&    p  = pixL[dy][dx];
            total_t  sz = K * 255 * uint64_t(w) * h;
            p[0] = ti = PIX_MAX * ti / sz;
            p[1] = tu = PIX_MAX * tu / sz;
            p[2] = tv = PIX_MAX * tv / sz;
            totalMidY += ti;
            totalMidU += tu;
            totalMidV += tv;
        }
    }
    pix_t midY = totalMidY / (LSZ * LSZ);
    pix_t midU = totalMidU / (LSZ * LSZ);
    pix_t midV = totalMidV / (LSZ * LSZ);
    thresholdY_ = midY;
    thresholdU_ = midU;
    thresholdV_ = midV;
    packKey_.thresholdY_ = midY * 255 / PIX_MAX;
    packKey_.thresholdU_ = midU * 255 / PIX_MAX;
    packKey_.thresholdV_ = midV * 255 / PIX_MAX;

    DBGPRINTF("\tmy=%u mu=%u mv=%u (%u %u %u)\n", thresholdY_, thresholdU_, thresholdV_, packKey_.thresholdY_, packKey_.thresholdU_, packKey_.thresholdV_);

    // 16x16 を元に 8x8 縮小をつくり、類似チェック用bitパターンを作る
    uint8_t (*pixM)[8][3] = pix8x8_;
    {
        uint64_t    ptnY = 0;
        uint64_t    ptnU = 0;
        uint64_t    ptnV = 0;
        uint64_t    m    = uint64_t(1) << (MSZ*MSZ - 1);
        for (unsigned dy = 0; dy < LSZ; dy += 2) {
            for (unsigned dx = 0; dx < LSZ; dx += 2) {
                pix_t i  = (pixL[dy][dx][0] + pixL[dy][dx + 1][0] + pixL[dy + 1][dx][0] + pixL[dy + 1][dx + 1][0]) / 4;
                pix_t u  = (pixL[dy][dx][1] + pixL[dy][dx + 1][1] + pixL[dy + 1][dx][1] + pixL[dy + 1][dx + 1][1]) / 4;
                pix_t v  = (pixL[dy][dx][2] + pixL[dy][dx + 1][2] + pixL[dy + 1][dx][2] + pixL[dy + 1][dx + 1][2]) / 4;
                if (i >= midY)
                    ptnY |= m;
                if (u >= midU)
                    ptnU |= m;
                if (v >= midV)
                    ptnV |= m;
                m >>= 1;
                uint8_t* p = pixM[dy >> 1][dx >> 1];
                p[0] = i * 255 / PIX_MAX;
                p[1] = u * 255 / PIX_MAX;
                p[2] = v * 255 / PIX_MAX;
            }
        }
        ptnY8x8_ = ptnY;
        ptnU8x8_ = ptnU;
        ptnV8x8_ = ptnV;
        mono_    = ((ptnU == ~uint64_t(0)) & (ptnV == ~uint64_t(0)));
        DBGPRINTF("\tptn8: %016llx %016llx %016llx\n", ptnY8x8_, ptnU8x8_, ptnV8x8_);
    }

    // 8x8 を元に 4x4 縮小をつくり、類似チェック用bitパターンと 16階調パターンを作る.
    {
     #if 0
        uint64_t  pixY = 0;
        uint64_t  pixU = 0;
        uint64_t  pixV = 0;
        unsigned  sh   = 60;
     #endif
        unsigned    thrY = midY * 4 * 255 / PIX_MAX;
        unsigned    ptnY = 0;
        unsigned    m    = 1 << (SSZ*SSZ - 1);
        for (unsigned dy = 0; dy < MSZ; dy += 2) {
            for (unsigned dx = 0; dx < MSZ; dx += 2) {
                pix_t i4 = (pixM[dy][dx][0] + pixM[dy][dx + 1][0] + pixM[dy + 1][dx][0] + pixM[dy + 1][dx + 1][0]);
                pix_t u4 = (pixM[dy][dx][1] + pixM[dy][dx + 1][1] + pixM[dy + 1][dx][1] + pixM[dy + 1][dx + 1][1]);
                pix_t v4 = (pixM[dy][dx][2] + pixM[dy][dx + 1][2] + pixM[dy + 1][dx][2] + pixM[dy + 1][dx + 1][2]);
                if (i4 >= thrY)
                    ptnY |= m;
                m >>= 1;
             #if 0
                pixC8x4x4_[0][dy>>1][dx>>1] = i4 * 255 / (4 * PIX_MAX);
                pixC8x4x4_[1][dy>>1][dx>>1] = u4 * 255 / (4 * PIX_MAX);
                pixC8x4x4_[2][dy>>1][dx>>1] = v4 * 255 / (4 * PIX_MAX);
             #elif 0
                int   t = int(i4 * 15 / (4 * PIX_MAX));
                t       = (t < 0) ? 0 : (t > 15) ? 15 : t;
                pixY   |= uint64_t(t) << sh;
                t       = int(u4 * 15 / (4*PIX_MAX));
                t       = (t < 0) ? 0 : (t > 15) ? 15 : t;
                pixU   |= uint64_t(t) << sh;
                t       = int(v4 * 15 / (4*PIX_MAX));
                t       = (t < 0) ? 0 : (t > 15) ? 15 : t;
                pixV   |= uint64_t(t) << sh;
                sh -= 4;
             #endif
            }
        }
        ptnY4x4_        = ptnY;
     #if 0
        pixC4x4x4_[0]   = pixY;
        pixC4x4x4_[1]   = pixU;
        pixC4x4x4_[2]   = pixV;
        DBGPRINTF("\t4x4: %04x %016llx %016llx %016llx\n", ptnY, pixY, pixU, pixV);
     #endif
    }

    // 16x16 を 8x8 ４つとして、それぞれの 比較用パターンを作成する.
    {
        pix_t           threshold = midY;
        RuigPtn8x8_t    ptn[2][2] = {0};
        uint64_t        m         = uint64_t(1) << (MSZ*MSZ - 1);
        for (unsigned dy = 0; dy < MSZ; ++dy) {
            for (unsigned dx = 0; dx < MSZ; ++dx) {
                //pix_t threshold = pixM[dy][dx][0];    // 8x8の1点は 16x16の4点の平均値->閾値扱い
                for (unsigned ey = 0; ey < 2; ++ey) {
                    for (unsigned ex = 0; ex < 2; ++ex) {
                        pix_t pix = pixL[dy*2+ey][dx*2+ex][0];
                        if (pix >= threshold) {
                            ptn[ey][ex] |= m;
                        }
                    }
                }
                m >>= 1;
            }
        }
        ptnY16x16_[0][0] = ptn[0][0];
        ptnY16x16_[0][1] = ptn[0][1];
        ptnY16x16_[1][0] = ptn[1][0];
        ptnY16x16_[1][1] = ptn[1][1];
        DBGPRINTF("\tptn16:%016llx %016llx\n", ptnY16x16_[0][0], ptnY16x16_[0][1]);
        DBGPRINTF("\t      %016llx %016llx\n", ptnY16x16_[1][0], ptnY16x16_[1][1]);
    }

    // 8x8 の補助パターンの作成
    {
        // 明るい方と暗い方別々に、輝度の平均を求める.
        uint64_t subMidVal[2]   = { 0 };
        uint64_t subMidCnt[2]   = { 0 };
        uint64_t ptnY           = ptnY8x8_;
        uint64_t m              = uint64_t(1) << (MSZ*MSZ - 1);
        for (unsigned dy = 0; dy < MSZ; ++dy) {
            for (unsigned dx = 0; dx < MSZ; ++dx) {
                pix_t d = pixM[dy][dx][0];
                if (ptnY & m) {
                    subMidVal[1] += d;
                    ++subMidCnt[1];
                } else {
                    subMidVal[0] += d;
                    ++subMidCnt[0];
                }
                m >>= 1;
            }
        }
        if (subMidCnt[0] > 0)
            subMidVal[0] /= subMidCnt[0];
        if (subMidCnt[1] > 0)
            subMidVal[1] /= subMidCnt[1];

        // ptnY8x8_ のon/off に対応する部分の明るい/暗い別の、明暗判定を行う.
        uint64_t    ptnS = 0;
        m                = 1LL << (MSZ*MSZ - 1);
        for (unsigned dy = 0; dy < MSZ; ++dy) {
            for (unsigned dx = 0; dx < MSZ; ++dx) {
                if (pixM[dy][dx][0] >= subMidVal[(ptnY & m) != 0]) {
                    ptnS |= m;
                }
                m >>= 1;
            }
        }
        ptnY8x8sub_ = ptnS;
        DBGPRINTF("\tsub8: %016llx\n", ptnS);
    }
}

/** デバッグ用のログ表示
 */
void RuigFactor::debugPrintLog() const
{
    DBGPRINTF("%-20s\t%-4d*%4d lsz=%6d fsz=%6d asp=%5d own=%d\n",name_.c_str(), origWidth_, origHeight_, lsize_, fsize_, asp_, isParentFactor());
    DBGPRINTF("\ti4:%04x i:%016llx s:%016llx\n", ptnY4x4_, ptnY8x8_, ptnY8x8sub_);
    DBGPRINTF("\tu:%016llx v:%016llx\n", ptnU8x8_, ptnV8x8_);
    DBGPRINTF("\ti16[0] %016llx %016llx\n", ptnY16x16_[0][0], ptnY16x16_[0][1]);
    DBGPRINTF("\ti16[1] %016llx %016llx\n", ptnY16x16_[1][0], ptnY16x16_[1][1]);
    //DBGPRINTF("\tc4 %016llx %016llx %016llx\n", pixC4x4x4_[0], pixC4x4x4_[1], pixC4x4x4_[2]);
    DBGPRINTF("\tthr y=%4d u=%4d v=%4d\n", thresholdY_, thresholdU_, thresholdV_);
    DBGPRINTF("\tsame %d+%d=%d\n", sameCount_, nearCount_, sameCount_+nearCount_);
}

/** 最初の画像登録時のソート用比較
 */
inline bool RuigFactor::isLess4FirstSort(RuigFactor const& r) const {
    int dif = asp_ - r.asp_;
    if (abs(dif) >= ASPECT_K / 10)
        return dif < 0;
    if (ptnY4x4_ != r.ptnY4x4_)
        return ptnY4x4_ < r.ptnY4x4_;
    if (ptnY8x8_ != r.ptnY8x8_)
        return ptnY8x8_ < r.ptnY8x8_;
    if (packKey_.key_ != r.packKey_.key_)
        return packKey_.key_ < r.packKey_.key_;
 #if 0
    if (pixC4x4x4_[3] != r.pixC4x4x4_[3])
        return pixC4x4x4_[3] < r.pixC4x4x4_[3];
    if (pixC4x4x4_[2] != r.pixC4x4x4_[2])
        return pixC4x4x4_[2] < r.pixC4x4x4_[2];
    if (pixC4x4x4_[0] != r.pixC4x4x4_[0])
        return pixC4x4x4_[0] < r.pixC4x4x4_[0];
 #endif
    size_t   lsz = origWidth_   * origHeight_;
    size_t   rsz = r.origWidth_ * r.origHeight_;
    if (lsz != rsz)
        return lsz > rsz;
    if (origWidth_ != r.origWidth_)
        return origWidth_ > r.origWidth_;
    return fsize_ > r.fsize_;
}

/** 指定 Factorとの似て具合の評価値計算
 */
uint32_t RuigFactor::calcCmpVal(RuigFactor const& rhs) const
{
    enum { ML_SAME  =  4, ML_NEAR = 10, ML_MID=24, ML_LARGE=40 };
    enum { ASP_THR = ASPECT_K / 4 };
    int difAspH = abs(aspH_ - rhs.aspH_);
    int difAspW = abs(aspW_ - rhs.aspW_);
    if (difAspH>= ASP_THR || difAspW >= ASP_THR)
        return 10000;

    uint32_t difPtnY = 0;
    int difAsp = 0; //std::max(difAspW, difAspH) / (ASP_THR/3);
    difPtnY += difAsp;

    DBGPRINTF("cmp %s, %s\n", name().c_str(), rhs.name().c_str());

    uint32_t li8 = 0;
    uint32_t luv = 0;
    // 8x8 輝度の違うピクセル数を求める.
    uint64_t mi8 = ptnY8x8_ ^ rhs.ptnY8x8_;
    DBGPRINTF("\t%s\t%016llx\n", "mi8", mi8);
    uint64_t mu8 = ptnU8x8_ ^ rhs.ptnU8x8_;
    DBGPRINTF("\t%s\t%016llx\n", "mu8", mu8);
    uint64_t mv8 = ptnV8x8_ ^ rhs.ptnV8x8_;
    DBGPRINTF("\t%s\t%016llx\n", "mv8", mv8);
    if (mi8) {
        li8 = bit_popcount64(mi8);
        DBGPRINTF("\tptnY8x8:%016llx cnt=%d\n", mi8, li8);
        //if (li8 > ML_LARGE)
        if (li8 > ML_MID)
            return 10000;
    }
    uint64_t muv = mu8 | mv8;
    muv = muv & ~mi8;
    if (muv) {
        luv = bit_popcount64(muv);
        DBGPRINTF("\tptnUV8x8:%016llx cnt=%d\n", muv, luv);
    }

 #if 1
    uint32_t li8s = 0;
    {
        // 8x8 輝度の補助情報の違うピクセル数を反映.
        uint64_t mi8s = ptnY8x8sub_ ^ rhs.ptnY8x8sub_;
        DBGPRINTF("\t%s\t%016llx\n", "mi8s", mi8s);
        mi8s    &= ~mi8;
        if (mi8s) {
            li8s = bit_popcount64(mi8s);
            DBGPRINTF("\tptnYs8x8:%016llx cnt=%d\n", mi8s, li8s);
            if (/*li8 +*/ li8s >= ML_LARGE)
                return 10000;
        }
    }
 #endif

    uint32_t li16 = 0;
    {
        // 16x16 輝度の違うピクセル数を反映.
        uint64_t m00 = ptnY16x16_[0][0] ^ rhs.ptnY16x16_[0][0];
        DBGPRINTF("\t%s\t%016llx\n", "m00", m00);
        uint64_t m01 = ptnY16x16_[0][1] ^ rhs.ptnY16x16_[0][1];
        DBGPRINTF("\t%s\t%016llx\n", "m01", m01);
        uint64_t m10 = ptnY16x16_[1][0] ^ rhs.ptnY16x16_[1][0];
        DBGPRINTF("\t%s\t%016llx\n", "m10", m10);
        uint64_t m11 = ptnY16x16_[1][1] ^ rhs.ptnY16x16_[1][1];
        DBGPRINTF("\t%s\t%016llx\n", "m11", m11);
        uint64_t mi16= (m00 | m01 | m10 | m11);
        DBGPRINTF("\t%s\t%016llx\n", "mi16", mi16);
      #if 0 // 2x2 中 1しか違わない場合は、同じとみなす
        uint64_t em0 = m00 ^ m01;
        uint64_t em1 = m10 ^ m11;
        uint64_t em  = em0 ^ em1;
        mi16 = mi16 & ~em;
      #endif
        DBGPRINTF("\t%s\t%016llx\n", "mi16e", mi16);
        mi16         = mi16 & ~mi8; // 8x8 で違う部分はすでにカウント済とする.
        if (mi16) {
            li16 = bit_popcount64(mi16);
            DBGPRINTF("\tptnY16x16:%016llx cnt=%d\n", mi16, li16);
        }
        if (/*li8 + */ li16 >= ML_LARGE)
            return 10000;
    }

    // y,u,v 閾値(平均値) の、相手との差を求める
    enum { THR_SAME = 4 };
    enum { THR_NEAR = 10 };
    int difThrY = packKey_.thresholdY_ - rhs.packKey_.thresholdY_;
    int difThrU = packKey_.thresholdU_ - rhs.packKey_.thresholdU_;
    int difThrV = packKey_.thresholdV_ - rhs.packKey_.thresholdV_;
    DBGPRINTF("\tdifThr %d,%d,%d\n", difThrY, difThrU, difThrV);
    difThrY = abs(difThrY);
    difThrU = abs(difThrU);
    difThrV = abs(difThrV);

    // 両方モノクロの場合は uvのダミーを設定.
    if (unsigned(mono_) & unsigned(rhs.mono_)) {
        luv = li8;
        difThrU = difThrV = difThrY / 4;
        DBGPRINTF("\tmonomono %d,%d,%d\n",luv, difThrU, difThrV);
    }
    bool skip = false;
    int rc = 0;
    if (li8 <= ML_NEAR) {
        if (difThrU <= THR_NEAR && difThrV <= THR_NEAR) { // u,v
            bool issameuv = (difThrU <= THR_SAME && difThrV <= THR_SAME);
            if (luv <= ML_NEAR) {                         // u,v はほぼ一緒とみなす
                if (issameuv && luv <= ML_SAME && difThrY <= THR_SAME) {
                    rc = difAsp/2 + (li8+luv) / 2 + (difThrY+difThrU+difThrV) / (3*8) + li16/4 + li8s/4;
                    DBGPRINTF("\tyuv-same %d\n", rc);
                    //skip = (rc < 12);
                } else if (luv <= ML_NEAR && li16 <= ML_NEAR && li8s <= ML_NEAR) {
                    rc = difAsp + li8 + luv + (difThrY+difThrU+difThrV) / (3*4) + li16/4 + li8s/4;
                    DBGPRINTF("\tuv-same %d\n", rc);
                    //skip = (rc < 12);
                } else {
                    rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV) / 8 + li16/4 + li8s/4;
                    DBGPRINTF("\tuv-same %d\n", rc);
                }
            } else if (difThrY <= THR_SAME) {
                if (issameuv && li8s <= ML_NEAR || li16 <= ML_NEAR) {  // 大筋似ているけれど細部が違うパターン?
                    rc = difAsp + li8 + luv / 3 + (difThrY + difThrU + difThrV) / 8 + li16/3 + li8s/3;
                    DBGPRINTF("\ty-same %d\n", rc);
                } else {
                    rc = difAsp + li8 / 2 + luv + (difThrY + difThrU + difThrV) / 3 + li16/2 + li8s/2;
                    DBGPRINTF("\ty-near %d\n", rc);
                }
            } else {
                rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV)/2 + li16/2 + li8s/2;
                DBGPRINTF("\tnear %d\n", rc);
            }
        } else if (luv <= ML_NEAR && li16 <= ML_NEAR && li8s <= ML_NEAR) {
            if (luv <= ML_SAME && li16 <= ML_SAME && li8s <= ML_SAME) {
                //skip = true;
                rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV) / 3 + li16/4 + li8s/4;
                DBGPRINTF("\ti-same %d\n", rc);
            } else {
                rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV) / 3 + li16/2 + li8s/2;
                DBGPRINTF("\ti-near %d\n", rc);
            }
        } else if (unsigned(mono_) | unsigned(rhs.mono_)) {               // 片方がモノクロの時
            if (difThrY <= THR_SAME) {
                rc = difAsp + li8*4/3 + li16 / 4 + li8s/4;
                DBGPRINTF("\tkatamono-same %d\n", rc);
            } else {
                rc = difAsp + li8*2 + difThrY/8 + li16 / 2 + li8s/2;
                DBGPRINTF("\tkatamono %d\n", rc);
            }
        } else {
            rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV)/2 + li16/2 + li8s/2;
            DBGPRINTF("\tnear %d\n", rc);
        }
    } else if (li8 <= ML_MID) {
        rc = difAsp + li8 + luv + (difThrY + difThrU + difThrV)*2/3 + li16*2/3 + li8s*2/3;
        DBGPRINTF("\tmid %d\n", rc);
    } else {
        rc = difAsp*2 + li8 + luv + (difThrY + difThrU + difThrV) + li16 + li8s;
        DBGPRINTF("\tetc %d\n", rc);
    }
 #if 1
    // あまり同じではないけれど似てるかもしれない範囲にあるなら、8x8画像のyuv比較
    if (CMPVAL_SAME < rc && rc <= CMPVAL_LARGE && !skip) {
        uint32_t        isum = 0, usum = 0, vsum = 0;
        uint32_t        icnt = 0, uvcnt = 0, iuvcnt = 0;
        uint32_t        icnt2 = 0, uvcnt2 = 0, iuvcnt2 = 0;
        enum { K2 = 3};
        uint8_t const*  pl = &pix8x8_[0][0][0];
        uint8_t const*  pr = &rhs.pix8x8_[0][0][0];
        for (unsigned n = 0; n < 64; ++n) {
            unsigned idif = abs(pl[0] - pr[0]);
            unsigned udif = abs(pl[1] - pr[1]);
            unsigned vdif = abs(pl[2] - pr[2]);
            isum += idif;
            usum += udif;
            vsum += vdif;
            if (idif <= K2) {
                ++icnt2;
                if (idif <= 1)
                    ++icnt;
            }
            if (udif <= K2 && vdif <= K2) {
                ++uvcnt2;
                if (idif <= K2)
                    ++iuvcnt2;
                if (udif <= 1 && vdif <= 1) {
                    ++uvcnt;
                    if (idif <= 1)
                        ++iuvcnt;
                }
            }
            pl += 3;
            pr += 3;
        }
        DBGPRINTF("\tpix8x8 dif-sum y:%d,u:%d,v:%d\n", isum, usum, vsum);
        DBGPRINTF("\tpix8x8 dif-cnt %d,%d,%d  %d,%d,%d\n", icnt, uvcnt, iuvcnt, icnt2, uvcnt2, iuvcnt2);
        if (mono_ ^ rhs.mono_) {
            if (icnt2 >= 32) {
                if (icnt >= 43) {
                    if (rc > CMPVAL_SAME)
                        rc = CMPVAL_SAME;
                    DBGPRINTF("\tMono-same rc=%d\n", rc);
                } else {
                    if (rc > CMPVAL_NEAR)
                        rc = CMPVAL_NEAR;
                    DBGPRINTF("\tMono-near rc=%d\n", rc);
                }
            } else if (icnt < 14) {
                if (rc < CMPVAL_NEAR+1)
                    rc = CMPVAL_NEAR+1;
                DBGPRINTF("\tMono-far rc=%d\n", rc);
            }
        } else {
            if (iuvcnt >= 14) {
                if (iuvcnt >= 43 || (rc <= CMPVAL_NEAR && iuvcnt >= 32)) {
                    if (rc > CMPVAL_SAME)
                        rc = CMPVAL_SAME;
                    DBGPRINTF("\tIUV-same rc=%d\n", rc);
                } else if (iuvcnt2 >= 21) {
                    if (rc > CMPVAL_NEAR)
                        rc = CMPVAL_NEAR;
                    DBGPRINTF("\tIUV-near rc=%d\n", rc);
                } else {
                    DBGPRINTF("\tIUV-mid  rc=%d\n", rc);
                }
            } else if (li8 <= ML_NEAR && uvcnt >= 36) {
                if (li8 <= ML_SAME && iuvcnt >= 43) {
                    if (rc > CMPVAL_SAME)
                        rc = CMPVAL_SAME;
                    DBGPRINTF("\tUV-same  rc=%d\n", rc);
                } else if (uvcnt < 14) {
                    if (rc < CMPVAL_NEAR+1)
                        rc = CMPVAL_NEAR+1;
                    DBGPRINTF("\tUV-far  rc=%d\n", rc);
                }
            } else {
                if (rc < CMPVAL_NEAR+1)
                    rc = CMPVAL_NEAR+1;
                DBGPRINTF("\tIUV-far  rc=%d\n", rc);
            }
        }
    }
 #endif
    return rc;
}

/** 近同評価値リンクに登録
 */
void RuigFactor::addLink(RuigFactor* child, uint32_t cmpVal)
{
    DBGPRINTF("\tlink %s to %s\n", child->name_.c_str(), name_.c_str());

    if (cmpVal <= CMPVAL_SAME)
        ++sameCount_;
    else if (cmpVal <= CMPVAL_NEAR)
        ++nearCount_;
    else
        ++midCount_;
    valueLinks_.insert(ValueLinks::value_type(cmpVal, child));
}

/** なるべく似てる画像が近くに並ぶようにして、sortFactorsに登録
 */
void RuigFactor::checkAndGenSameLinks(RuigFactors& sortFactors, uint32_t& majorNo)
{
    if (parent_)    // すでに決定済みだった.
        return;
    if (valueLinks_.empty()) {  // 類似画像がないなら これ一つに決定.
        parent_  = this;
        majorNo_ = majorNo;
        minorNo_ = 0;
        subNo_   = 0;
        sortFactors.push_back(this);
        return;
    }

    RuigFactors nearSameLinks;
    genNearSameLinks(nearSameLinks);

    // 近同順の一覧の作成
    uint32_t minorNo = 0;
    for (size_t i = 0; i < nearSameLinks.size(); ++i) {
        RuigFactor* cur = nearSameLinks[i];
        if (!cur->parent_) {
            if (i && !cur->hasNearLinks()) {    // 登録済みに近いか？
              #ifndef USE_NATIVE_MINOR_NO
                if (minorNo == 1) {
                    RuigFactor* prev = sortFactors.back();
                    if (prev->subNo_ == 0) {
                        prev->minorNo_ = 0;
                    }
                }
              #endif
                ++majorNo;
                minorNo = 0;
            }
            cur->setSameLinks(nearSameLinks, i, majorNo, ++minorNo, sortFactors);
        }
    }
 #ifndef USE_NATIVE_MINOR_NO
    if (minorNo == 1) {
        RuigFactor* prev = sortFactors.back();
        if (prev->subNo_ == 0) {
            prev->minorNo_ = 0;
        }
    }
 #endif

 #if 0
    DBGPRINTF("***\n");
    for (size_t i = 0; i < nearSameLinks.size(); ++i) {
        RuigFactor* cur     = nearSameLinks[i];
        DBGPRINTF("%s %05d:%02d:%02d  cnt=%d\n", cur->name_.c_str(), cur->majorNo_, cur->minorNo_, cur->subNo_, int(cur->valueLinks_.size()));
        for (ValueLinks::iterator ite = cur->valueLinks_.begin(); ite != cur->valueLinks_.end(); ++ite) {
            RuigFactor* p = ite->second;
            DBGPRINTF("\t%s %05d:%02d:%02d cmpVal=%d\n", p->name_.c_str(), p->majorNo_, p->minorNo_, p->subNo_, ite->first);
        }
    }
 #endif
}

/// 近同画像一覧のソート用(面積大きいのを優先)
struct RuigFactor::NearSameLinksCmp {
    bool operator()(RuigFactor const* l, RuigFactor const* r) const {
        // カラー優先.
        if (!l->mono_ && r->mono_)
            return true;
        else if (l->mono_ && !r->mono_)
            return false;

        // 面積大きいのを優先
        size_t   lsz = l->origWidth_ * l->origHeight_;
        size_t   rsz = r->origWidth_ * r->origHeight_;
        if (lsz != rsz)
            return lsz > rsz;

        // 同一画像が多いものを優先
        if (l->sameCount_ != r->sameCount_)
            return l->sameCount_ > r->sameCount_;

        // 近い画像が多いものを優先
        size_t lv = l->sameCount_ + l->nearCount_;
        size_t rv = r->sameCount_ + r->nearCount_;
        if (lv != rv)
            return lv > rv;
        lv += l->midCount_;
        rv += r->midCount_;
        if (lv != rv)
            return lv > rv;

        // ファイルサイズが大きいものを優先
        // (jpgの高クロリティだとサイズが大きくなる傾向を考慮. 低クオリティの再jpg化による悪化物もあるので注意だけど)
        return l->fsize_ > r->fsize_;
    }
};

/** 各Factorが持つ近同リンクをひとまとめにし、画像サイズやリンク量を元に１つの一覧を作成する.
 */
void RuigFactor::genNearSameLinks(RuigFactors& nearSameLinks)
{
    FactorVals factorVals;
    convValueLinks(factorVals);

    // 近いモノが多い順位並べる
    typedef std::multiset<RuigFactor*, RuigFactor::NearSameLinksCmp> NaerSameLinkSet;
    NaerSameLinkSet links;
    for (FactorVals::iterator ite = factorVals.begin(); ite != factorVals.end(); ++ite) {
        links.insert(NaerSameLinkSet::value_type(ite->first));

        DBGPRINTF("%s %d\n", ite->first->name_.c_str(), ite->second);
    }
    factorVals.clear();

 #if 1
    size_t size = links.size();
    // 既出のFactorに近い物順に並べる
    int*   cmpVals = reinterpret_cast<int*>(alloca(size * sizeof(int)));
    if (!cmpVals) {
        std::vector<int> cmpValTmp(size);
        cmpVals = &cmpValTmp[0];
    }
    // memset(cmpVals, 0, size * sizeof cmpVals[0]);
    nearSameLinks.resize(size);
    size_t i = 0;
    for (NaerSameLinkSet::iterator ite = links.begin(); ite != links.end(); ++ite) {
        nearSameLinks[i++] = *ite;
    }
    DBGPRINTF("%s cmpval=%d\n", nearSameLinks[0]->name_.c_str(), 0);
    for (i = 1; i < size; ++i) {
        for (size_t j = i; j < size; ++j) {
            RuigFactor* cur     = nearSameLinks[j];
            int         fndVal  = 100000;
            for (size_t k = 0; k < i; ++k) {
                RuigFactor* tgt = nearSameLinks[k];
                for (ValueLinks::iterator ite = cur->valueLinks_.begin(); ite != cur->valueLinks_.end(); ++ite) {
                    if (ite->second == tgt && fndVal > ite->first) {
                        fndVal = ite->first;
                        break;
                    }
                }
            }
            cmpVals[j] = fndVal;
        }
        int     fndVal  = 100000;
        size_t  fndIdx  = i;
        for (size_t j = i; j < size; ++j) {
            if (fndVal > cmpVals[j]) {
                fndVal = cmpVals[j];
                fndIdx = j;
            }
        }
        if (i != fndIdx)
            std::swap(nearSameLinks[i], nearSameLinks[fndIdx]);
        DBGPRINTF("%s cmpval=%d\n", nearSameLinks[i]->name_.c_str(), fndVal);
    }
 #else
    nearSameLinks.resize(links.size());
    size_t i = 0;
    for (NaerSameLinkSet::iterator ite = links.begin(); ite != links.end(); ++ite) {
        nearSameLinks[i++] = *ite;
        DBGPRINTF("%s\n", (*ite)->name_.c_str());
    }
 #endif
}

/** 各Factorの近同リンクを (Factor*,評価値) な近同リンク一覧にリンクを辿りながら再帰登録
 */
void RuigFactor::convValueLinks(FactorVals& factorVals)
{
    size_t val = sameCount_ + nearCount_;
    factorVals.insert(FactorVals::value_type(this, val));
    for (ValueLinks::iterator ite = valueLinks_.begin(); ite != valueLinks_.end(); ++ite) {
        FactorVals::iterator fnd = factorVals.find(ite->second);
        if (fnd == factorVals.end())
            ite->second->convValueLinks(factorVals);
    }
}

/** 登録済みのリンクと近いかどうか
 */
bool RuigFactor::hasNearLinks()
{
    for (ValueLinks::iterator ite = valueLinks_.begin(); ite != valueLinks_.end(); ++ite) {
        RuigFactor* cur = ite->second;
        if (cur->parent_ && ite->first <= CMPVAL_NEAR)
            return true;    // 登録済みのものと近いものがあれば true
    }
    // 登録済みのもので近いものはなかった
    return false;
}

/** 統一された 近同リンク から、自身と ほぼ同じ もののみを集めて resultFactors に登録.
 */
void RuigFactor::setSameLinks(RuigFactors const& nearSameLinks, size_t idx, uint32_t majorNo, uint32_t minorNo, RuigFactors& resultFactors)
{
    int subNo   = 1;
    parent_     = this;
    majorNo_    = majorNo;
    minorNo_    = minorNo;
    subNo_      = subNo;
    //sameLinks_.push_back(this);
    resultFactors.push_back(this);
    for (ValueLinks::iterator ite = valueLinks_.begin(); ite != valueLinks_.end(); ++ite) {
        if (ite->first > CMPVAL_SAME)  // 同じとみなすには違いが大きいものはスキップ
            continue;
        RuigFactor* cur = ite->second;
        if (cur->parent_)               // すでに処理済みならスキップ
            continue;
        RuigFactors::const_iterator fnd = std::find(nearSameLinks.begin()+idx+1, nearSameLinks.end(), cur);
        if (fnd == nearSameLinks.end()) // 統一の一覧になければスキップ(処理済は検査対象にしてないので見つからない場合有り)
            continue;
        cur->parent_  = this;           // 親を設定.
        cur->majorNo_ = majorNo;
        cur->minorNo_ = minorNo;
        cur->subNo_   = ++subNo;
        //sameLinks_.push_back(cur);
        resultFactors.push_back(cur);
    }
    if (subNo == 1) {
        resultFactors.back()->subNo_ = 0;
    }
}



// ---------------------------------------------------------------------------------

/** デフォルトコンストラクタ
 */
RuigMgr::RuigMgr()
{
}

/** 画像(情報)登録
 */
bool RuigMgr::insert(NamedImgPtr img, uint32_t fsize)
{
    RuigFactorPtr   factor( new RuigFactor(img, fsize) );
    factorBodys_.insert(factor);
    return true;
}

/** 登録済みのものを近似順に並べる
 */
bool RuigMgr::run()
{
    if (initAllFactors() == false)
        return false;

    checkNearSameLinks();

    sortFactors();

    return true;
}

/** 総当りチェック用に tree構造なコンテナを vector に変換.
 */
bool RuigMgr::initAllFactors()
{
    size_t size = factorBodys_.size();
    if (!size)
        return false;
    allFactors_.resize(size);        ///< 全factorソート済の一覧(舐める用)
    if (allFactors_.size() < size)
        return false;
    size_t i = 0;
    for (RuigFactorBodys::iterator ite = factorBodys_.begin();
        ite != factorBodys_.end();
        ++ite)
    {
        allFactors_[i++] = &**ite;
    }
    return !allFactors_.empty();
}

/** 全てのFactorの 近同評価を行う
 */
void RuigMgr::checkNearSameLinks()
{
    RuigFactor** factors = &allFactors_[0];
    size_t       size    = allFactors_.size();
    for (size_t i = 0; i < size; ++i) {
        RuigFactor* ip =  factors[i];
        DBGPRINTF("%s %04dx%04d asp=%d\n", ip->name_.c_str(), ip->origWidth(), ip->origHeight(), ip->aspect());
        for (size_t k = i+1; k < size; ++k) {
            RuigFactor* kp =  factors[k];
            uint32_t val = ip->calcCmpVal(*kp);
            DBGPRINTF("\tcmpVal=%d\n", val);
            if (val <= CMPVAL_MID) {     // 近同だったら、それぞれに評価＆リンクを登録.
                ip->addLink(kp, val);
                kp->addLink(ip, val);
            }
        }
    }
}

/** Factorごとの近同評価を元に、近同順に allFactors_ を並べ直す.
 */
void RuigMgr::sortFactors()
{
    size_t       size    = allFactors_.size();
    RuigFactors  factors;
    factors.reserve(size);
    uint32_t     majorNo = 0;
    for (size_t i = 0; i < size; ++i) {
        RuigFactor* ip =  allFactors_[i];
        if (!ip->parent()) {
            ++majorNo;
            ip->checkAndGenSameLinks(factors, majorNo);
        }
    }
    // 結果を反映
    factors.swap(allFactors_);
}

/** 近同イメージがある画像の数を数える
 */
size_t RuigMgr::countNearSameFactors()
{
    size_t count = 0;
    for (size_t i = 0; i < allFactors_.size(); ++i)
        count += allFactors_[i]->hasNearSame();
    return count;
}
