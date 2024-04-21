/**
 *  @file   DecreaseColorMC.h
 *  @brief  メディアンカットな減色. YUV版
 *  @author Masashi KITAMURA
 *  @note
 *      http://www.h6.dion.ne.jp/~j_moto/html/program_myfo01.html
 *      のソースを改造したもの....した後に、yuv版のちゃんとした
 *      program_myfo03.htmlがあったことに気づくon_
 *
 */

#ifndef DECREASECOLORMC_H
#define DECREASECOLORMC_H

#pragma once

#include <assert.h>
#include <cstring>
#include <algorithm>

/// clut番号を連番でなく、スタックに溜めたものを出し入れして割り当てる.
#define DECREASECOLORMC_USE_NUBERSTK



/// メディアンカットな減色変換.
template<typename DMY=unsigned>
class DecreaseColorMC_T {
public:
    DecreaseColorMC_T();
    ~DecreaseColorMC_T();

    /// 変換.
    bool conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize);

    /// αに関する設定. 実データに、半透明がない場合はalpNum=1(抜き色のみ)として扱う.
    void setAlphaParam(int alpNum, unsigned alpBit=0, unsigned minA=0, unsigned maxA=255);

    /// αに関する設定. 実データに半透明がないときでも 必ずalpNum分はそれようとする.
    void forceSetAlphaEnv(int alpNum, unsigned alpBit=0, unsigned minA=0, unsigned maxA=255);

    /// 中間値を求めるのに使うパラメータ. 暫定.
    void setCalcMidParam(unsigned calcMidMode=1, float ratioLenY=1.2f, float ratioLenU=1.f, float ratioLenV=1.f);

    /// on で YUVでなく RGBで処理する.
    void setModeRGB(bool sw);

private:
    typedef unsigned int    pix32_t;
    typedef unsigned char   pix8_t;
    typedef unsigned short  pix16_t;
 #if defined _MSC_VER || defined __DMC__ || defined __WATCOMC__ || __BORLANDC__
    typedef unsigned __int64 ulonglong_t;
 #else
    typedef unsigned long long ulonglong_t;
 #endif

    bool init(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned dstClut[], unsigned clutNum, int* pAlpNum);
    int checkAlp();
    int convMain(int no, int num);
    unsigned calcMid(unsigned minVal, unsigned maxVal, ulonglong_t val, ulonglong_t total) const;
    void makeClut(int alpNum);
    unsigned ayuv_to_argb(unsigned a, unsigned y0, unsigned u0, unsigned v0);
    void makeDstPic();

    static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
        return (a << 24) | (r << 16) | (g << 8) | (b);
    }

    static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
    static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
    static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
    static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

    static int clamp(int val, int mi, int ma) {
        if (val < mi)
            return mi;
        else if (ma < val)
            return ma;
        return val;
    }


private:
  #ifdef DECREASECOLORMC_USE_NUBERSTK
    class NumberStk {
    public:
        NumberStk() {
            for (int i = 0; i < 256; ++i)
                no_[i] = i;
            ind_ = 0;
        }

        int pop() {
            int n = -1;
            if (ind_ < 256)
                n =  no_[ ind_++ ];
            return n;
        }

        void push(int no) {
            if (ind_ > 0) {
                no_[--ind_] = no;
            }
        }

    private:
        unsigned char   no_[256];
        int             ind_;
    };

    NumberStk   numberStk_;
 #endif

private:
 #if 0
    typedef unsigned char pixDat_int_t;
    enum { YUV_MUL = 1 };
 #else
    typedef unsigned short  pixDat_int_t;
    enum { YUV_MUL = 16 };
 #endif

    /// 処理中のピクセルデータ. rgbモードでは、b,r,g,a順.
    struct pixDat_t {
        pixDat_int_t    v;
        pixDat_int_t    u;
        pixDat_int_t    y;
        unsigned char   a;
        unsigned char   no;                 ///< 処理のグループ番号(ほぼ色番号となる..が未使用色があるとづれる)
        unsigned char   idx;                ///< 色番号(未使用色をつめる処理のために用意)
    };

    /// clutを生成するのに使う.
    struct clutDat_t {
        double      a;                      ///< aの合計.
        double      y;                      ///< yの合計.
        double      u;                      ///< uの合計.
        double      v;                      ///< vの合計.
        double      num;                    ///< ピクセル合計.
    };

    unsigned*       clut_;                  ///< 減食した結果のclut.
    pix8_t*         dstPic_;                ///< 減色結果を入れるバッファ. NULLだと出力せず(clutのみの取得時).
    pixDat_t*       pixDat_;                ///< 処理中のピクセルデータ.
    const pix32_t*  srcPic_;                ///< 元のピクセルデータ.
    unsigned        w_;                     ///< 横幅.
    unsigned        h_;                     ///< 縦幅.
    unsigned        size_;                  ///< ピクセル合計.
    unsigned        clutTop_;               ///< clutの開始番号.
    unsigned        clutSize_;              ///< clut数.

    unsigned        ratioLenA_;             ///< αを処理するかどうかに用いる。0 or 1として使用.
    bool            forceAlpNum_;           ///< 強制的にalpNum分を必ず透明色として扱う(結果 使用しない番号になる)
    int             alpNum_;                ///< α付colorにする色数.
    unsigned        minA_;                  ///< これ以下のときは、透明(α=0)にする.
    unsigned        maxA_;                  ///< これ以上のときは、不透明(α=0xff)にする.

    // 中間値を求めるのに用いるパラメータ.
    double          ratioLenY_;             ///< Y,U,Vの選択において、予め YをratioLenY_倍してから比較.
    double          ratioLenU_;             ///< Y,U,Vの選択において、予め UをratioLenU_倍してから比較.
    double          ratioLenV_;             ///< Y,U,Vの選択において、予め VをratioLenV_倍してから比較.
    unsigned        calcMidMode_;           ///< 0だと単純に(min+max)/2  1だとピクセル反映.
    //x unsigned    midDif_;                ///< max-min>=2*difを満たす時に、ピクセル数を反映して中央値を取得.
    //x float       ratioMidSiz_;           ///< この割合を全体のピクセル数に掛けた結果よりも、多い場合にピクセル数を反映して中央値を取得.

    bool            modeRGB_;               ///< YUVでなくGRBを用いる.

    unsigned        transparentNo_;
    unsigned        alpTopNo_;
    unsigned        alpBit_;
    unsigned        normNo_;
};



template<typename DMY>
DecreaseColorMC_T<DMY>::DecreaseColorMC_T() {
    clut_           = 0;
    dstPic_         = 0;
    pixDat_         = 0;
    srcPic_         = 0;
    w_              = 0;
    h_              = 0;
    size_           = 0;
    clutTop_        = 0;
    clutSize_       = 0;
    ratioLenA_      = 0;
    forceAlpNum_    = 0;
    alpNum_         = 0;
    minA_           = 0;
    maxA_           = 0;

    transparentNo_  = 0;
    alpTopNo_       = 0;
    alpBit_         = 0;
    normNo_         = 0;

    modeRGB_        = 0;

    calcMidMode_    = 0;
    ratioLenY_      = 1;
    ratioLenU_      = 1;
    ratioLenV_      = 1;
    //x midDif_     = 0;
    //x ratioMidSiz_= 0;
    setCalcMidParam();

    ratioLenA_   = 0;       // αを減色処理の対象にするかどうかを設定するのに用いている.
    setAlphaParam(-1);
}



template<typename DMY>
DecreaseColorMC_T<DMY>::~DecreaseColorMC_T() {
    if (pixDat_) {
        delete pixDat_;
        pixDat_ = 0;
    }
}



/// 変換.
template<typename DMY>
bool DecreaseColorMC_T<DMY>::conv(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize) {
    // 初期化.
    int alpNum = 0;
    if (init(dstPic, srcPic, w, h, dstClut, clutSize, &alpNum) == false)
        return false;

    if (alpBit_ > 0) {      // AnIm なピクセルにするのでインデックスをmビット分にする.
        assert(alpBit_ < 8);
        unsigned cnum = 1 << (8 - alpBit_);
        clutSize      = (clutSize < cnum) ? clutSize : cnum;
        alpNum        = (alpNum > 0);   // インデックス中はαなし、けど、抜き色指定は反映.
    }

    // αプレーンの処理.
    if (alpNum > 1) {
        ratioLenA_ = 1;
        convMain(alpTopNo_, alpNum-1);              // 透明色は処理済として１つずらす.
    }

    // メディアンカットな減色処理.
    ratioLenA_ = 0;
    convMain(normNo_, clutSize-alpNum);

    // clutを作成する.
    makeClut(alpNum);

    // clutを用いて、24bit色を256色(clutSize色)に減色.
    makeDstPic();

    // 開放.
    delete pixDat_;
    pixDat_ = 0;
    return true;
}



/** αに関する設定. 実データに、半透明がない場合はalpNum=1(抜き色のみ)として扱う.
 *  @param  alpNum      半透明色に使う色数.
 *  @param  minA        これ以下は透明色とする.
 *  @param  maxA        これ以上は不透明とする.
 */
template<typename DMY>
void DecreaseColorMC_T<DMY>::setAlphaParam(int alpNum, unsigned alpBit, unsigned minA, unsigned maxA) {
    forceAlpNum_ = false;
  #if 1 // 目先の誤差バグを対策.
    if (minA < 1)
        minA = 1;
    if (maxA > 255-1)
        maxA = 255-1;
  #endif

    alpNum_  = alpNum;
    minA_    = minA;
    maxA_    = maxA;

    alpBit_  = alpBit;
    if (alpBit > 0)     // αビットを確保する場合は、インデックス中はαなし(ただし、抜き色指定は反映)
        alpNum_ = (alpNum != 0);
    // invariant();
}



/** αに関する設定. 実データに半透明がないときでも 必ずalpNum分はそれようとする.
 *  @param  alpNum      半透明色に使う色数.
 *  @param  minA        これ以下は透明色とする
 *  @param  maxA        これ以上は不透明とする
 */
template<typename DMY>
void DecreaseColorMC_T<DMY>::forceSetAlphaEnv(int alpNum, unsigned alpBit, unsigned minA, unsigned maxA) {
    setAlphaParam(alpNum,alpBit,minA,maxA);
    forceAlpNum_ = true;
}



/** 中間値を求めるのに使うパラメータ. 暫定仕様.
 *  適当な計算なので、意味はわかりにくい...試行錯誤する用.
 */
template<typename DMY>
void DecreaseColorMC_T<DMY>::setCalcMidParam(unsigned calcMidMode, float ratioLenY, float ratioLenU, float ratioLenV)
{
    calcMidMode_ = calcMidMode;             // 0だと単純に(min+max)/2  1だとピクセル反映.
    ratioLenY_   = ratioLenY;               // Y,U,Vの選択において、予め YをratioLenY_倍する.
    ratioLenU_   = ratioLenU;
    ratioLenV_   = ratioLenV;
    //x midDif_      = midDif;              // max-min>=2*difを満たす時に、ピクセル数を反映して中央値を取得.
    //x ratioMidSiz_ = ratioMidSiz;         // この割合を全体のピクセル数に掛けた結果よりも、多い場合にピクセル数を反映して中央値を取得.
}



/** on で YUVでなく RGBで処理する. ※ GはY, RはU, BはVを使用.
 */
template<typename DMY>
void DecreaseColorMC_T<DMY>::setModeRGB(bool sw) {
    modeRGB_ = sw;
    if (sw) // RGBのときは、頻度を使うモードにし、Y(G)を1倍にしておく.
        setCalcMidParam(1, 1.0f, 1.0f, 1.0f);
    else
        setCalcMidParam();
}




/// 今回の初期化.
template<typename DMY>
bool DecreaseColorMC_T<DMY>::init(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize, int* pAlpNum) {
    // とりあえず、パラメータは一通り控えておく.
    srcPic_   = srcPic;
    dstPic_   = dstPic;
    clut_     = dstClut;
    clutSize_ = clutSize;
    w_        = w;
    h_        = h;
    size_     = w * h;

    // 引数チェック.
    if (srcPic == 0)
        return false;
    if (size_ == 0)
        return false;
    if (dstClut == 0)
        return false;
    if (clutSize < 2 || clutSize > 256)
        return false;

    // 作業メモリ確保.
    pixDat_  = new pixDat_t[ size_ ];
    if (pixDat_ == 0)
        return false;

    // ピクセルrgbをyuvに変換して作業メモリを準備.
    for (unsigned i = 0; i < size_; ++i) {
        pix32_t c       = srcPic[i];
        int     a       = argb_a(c);
        int     g       = argb_g(c);
        int     r       = argb_r(c);
        int     b       = argb_b(c);
        pixDat_[i].no   = clutTop_;
        pixDat_[i].a    = a;
        if (modeRGB_) {
            pixDat_[i].y    = YUV_MUL * g;
            pixDat_[i].u    = YUV_MUL * r;
            pixDat_[i].v    = YUV_MUL * b;
        } else {
            pixDat_[i].y    = YUV_MUL*(38444U*g + 19589U*r + 7503U*b) / 65536;
            pixDat_[i].u    = YUV_MUL*(-21709*g + -11059*r + 32768*b) / 65536 + YUV_MUL*128;
            pixDat_[i].v    = YUV_MUL*(-27439*g +  32768*r + -5329*b) / 65536 + YUV_MUL*128;
        }
    }

    *pAlpNum = checkAlp();

    return true;
}



/// αの状態をチェック＆反映.
///
template<typename DMY>
int DecreaseColorMC_T<DMY>::checkAlp()
{
    using namespace std;
    pixDat_t*   pixEnd  = &pixDat_[size_];
    pixDat_t*   pixDat;

    int         minA    = (int)minA_;
    int         maxA    = (int)maxA_;

    // αは何色用いるか、を予め設定.
    int  alpNum   = alpNum_;
    if (alpNum < 0) {   // 適当にα数を設定.
        if (clutSize_ < 32) {
            alpNum = 1;
        } else {
            alpNum = clutSize_ / 8;
            if (alpNum < 16)
                alpNum = 16;
        }
    }

    //x if (alpNum > 0)
    {
        unsigned alpChk[256];
        memset(alpChk, 0, sizeof alpChk);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            unsigned a = pixDat->a;
            ++alpChk[a];
        }

        // αチェック.
        unsigned    tra     = 0;
        unsigned    blend   = 0;
        unsigned    beta    = 0;
        for (int i = 0; i < 256; ++i) {
            unsigned c = alpChk[i];
            if (i <= minA)
                tra  += c;
            else if (i < maxA)
                blend += c;
            else
                beta  += c;
        }

        //x alpMode_     = (blend) ? ALPMODE_BLEND : (tra) ? ALPMODE_TRANSPARENT : ALPMODE_NONE;
        // 強制的にαclutを用意するのでなければ実データの状態を反映.
        if (forceAlpNum_ == false && alpNum > 0) {
            if (blend == 0) {
                //x if (tra == 0)   // 透明半透明が実データになければ、終わり.
                //x     return 0;
                alpNum = (tra != 0);        // 実データに半透明がないなら、抜き色だけ.
            }
        }
    }

  #ifdef DECREASECOLORMC_USE_NUBERSTK
    if (alpNum > 0) {
        transparentNo_ = numberStk_.pop();
        alpTopNo_      = transparentNo_;
        if (alpNum > 1)
            alpTopNo_ = numberStk_.pop();   // 初期値の設定で、必ず 1を返してるの前提.
    }
    normNo_         = numberStk_.pop();
  #else
    transparentNo_  = clutTop_;
    alpTopNo_       = clutTop_ + (alpNo > 1);
    normNo_         = clutTop_ + alpNum;
  #endif

    for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
        int a  =  pixDat->a;
        if (a  <= minA && alpNum > 0) {             // 透明.
            pixDat->no = transparentNo_;
            pixDat->a  = 0;

        } else if (a < maxA && alpNum > 1) {        // 半透明.
            pixDat->no = alpTopNo_;

        } else {                                    // 不透明.
            pixDat->no = normNo_;
            if (alpBit_ == 0)                       // αビットの指定のない時は不透明として0xffに設定.
                pixDat->a  = 0xff;
        }
    }

    return alpNum;
}



/// 中間値を求める.
///
template<typename DMY> inline
unsigned DecreaseColorMC_T<DMY>::calcMid(unsigned minVal, unsigned maxVal, ulonglong_t val, ulonglong_t total) const {
    if (calcMidMode_ == 0) {    // 単純に割る..
        return (minVal + maxVal + 1) / 2;
    } else {    // ピクセル数を反映..
        unsigned mid = unsigned(val / total);
        mid += (mid <= minVal);
        return mid;
    }
}



/// メディアンカットな減色のメイン処理. 再帰実行.
/// @param  no      先頭番号.
/// @param  num     処理する番号の範囲/数.
///
template<typename DMY>
int DecreaseColorMC_T<DMY>::convMain(int no, int num) {
    //x printf("%d:%d\n", no, num);
    // 呼ばれた段階で pixDat_[].no には no が設定されている.
    // ので、num が2以上ないと分割の意味がないので、未満ならかえる.
    if (num < 2)
        return num;

    // まず対象となるピクセルを１つ探す.
    pixDat_t*   pixEnd = &pixDat_[size_];
    pixDat_t*   pixDat = &pixDat_[0];
    if (no != pixDat->no) {
        do {
            ++pixDat;
        } while (pixDat != pixEnd && no != pixDat->no);
    }

    // 対象となるピクセルが無かった...
    if (pixDat == pixEnd) {
        return 0;
    }

    // とりあえずの、a,y,u,v範囲を設定. ※ rgbモードではa,g,r,bに相当.
    unsigned    minA = pixDat->a;
    unsigned    maxA = minA;
    unsigned    minY = pixDat->y;
    unsigned    maxY = minY;
    unsigned    minU = pixDat->u;
    unsigned    maxU = minU;
    unsigned    minV = pixDat->v;
    unsigned    maxV = minV;
    ulonglong_t sumA = minA;
    ulonglong_t sumY = minY;
    ulonglong_t sumU = minU;
    ulonglong_t sumV = minV;

    ulonglong_t sum  = 1;
    do {
        ++pixDat;
        if (no == pixDat->no) {
            unsigned    a = pixDat->a;
            if (a < minA)   minA = a;
            if (a > maxA)   maxA = a;
            sumA += a;

            unsigned    y = pixDat->y;      // (g)
            if (y < minY)   minY = y;
            if (y > maxY)   maxY = y;
            sumY += y;

            unsigned    u = pixDat->u;      // (r)
            if (u < minU)   minU = u;
            if (u > maxU)   maxU = u;
            sumU += u;

            unsigned    v = pixDat->v;      // (b)
            if (v < minV)   minV = v;
            if (v > maxV)   maxV = v;
            sumV += v;

            ++sum;
        }
    } while (pixDat != pixEnd);

    int lenA = (maxA - minA) * ratioLenA_;      // αを処理するかどうかは、ratioLenA_が1(以上)でする、0でしない.
    int lenY = (int)((maxY - minY) * ratioLenY_ / YUV_MUL);
    int lenU = (int)((maxU - minU) * ratioLenU_ / YUV_MUL);
    int lenV = (int)((maxV - minV) * ratioLenV_ / YUV_MUL);

    // 距離がないってことはすでに１色のみなんで、分割できないので帰る.
    if (sum <= 1 || (lenA == 0 && lenY == 0 && lenU == 0 && lenV == 0))
        return num;     // 今回の処理をする段階でno番がnum個設定されてるので、そのままnumを返す.

    int mode;
    // 一番、幅がある部分を選択する.
    // 同じ値の場合は A,Y,U,Vの順に選択される.
    if (ratioLenA_ && lenA >= lenY) {
        if (lenA >= lenU)
            mode = (lenA >= lenV) ? 3 : 0;
        else
            mode = (lenU >= lenV) ? 1 : 0;
    } else {
        if (lenY >= lenU)
            mode = (lenY >= lenV) ? 2 : 0;
        else
            mode = (lenU >= lenV) ? 1 : 0;
    }

    unsigned no1  = no;
    unsigned num1 = num / 2;
  #ifdef DECREASECOLORMC_USE_NUBERSTK
    unsigned no2  = numberStk_.pop();
  #else
    unsigned no2  = no  + num1;
  #endif

    unsigned c1=0,c2=0;
    unsigned mid;
    switch (mode)  {
    case 0: // V    (b)
        mid = calcMid(minV, maxV, sumV, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->v < mid) ? (++c1, no1) : (++c2, no2);         // 暗い方から処理.
                //pixDat->no = (pixDat->v >= mid) ? (++c1, no1) : (++c2, no2);      // 明るい方から処理.
            }
        }
        break;

    case 1: // U    (r)
        mid = calcMid(minU, maxU, sumU, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->u < mid) ? (++c1, no1) : (++c2, no2);
                //pixDat->no = (pixDat->u >= mid) ? (++c1, no1) : (++c2, no2);
            }
        }
        break;

    case 2: // Y    (g)
        mid = calcMid(minY, maxY, sumY, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->y < mid) ? (++c1, no1) : (++c2, no2);
                //pixDat->no = (pixDat->y >= mid) ? (++c1, no1) : (++c2, no2);
            }
        }
        break;

    case 3: // A    (a)
        mid = calcMid(minA, maxA, sumA, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->a < mid) ? (++c1, no1) : (++c2, no2);
                //x pixDat->no = (pixDat->a > mid) ? (++c1, no1) : (++c2, no2); // こっちだと誤変換が発生する...
            }
        }
        break;

    default:
        assert(0);
    }

    // 分割したけど、１ピクセルもなかった場合.
    if (c1 == 0) {
      #ifdef DECREASECOLORMC_USE_NUBERSTK
        numberStk_.push(no1);   // 番号を未使用にする.
      #endif
        num1 = 0;
    }

  #ifdef DECREASECOLORMC_USE_NUBERSTK
    if (c2 == 0) {
        numberStk_.push(no2);
    }
  #endif

    // 再帰.
    if (num1 > 1) {
        num1 = convMain(no1, num1);
    }

    unsigned num2 = num - num1;
    if (num <= 1 || c2 == 0)
        num2 = 0;

  #if defined(DECREASECOLORMC_USE_NUBERSTK) == 0
    if (no2 + num2 > no + num)
        num2 = no + num - no2;
  #endif

    if (num2 > 1) {
        num2 = convMain(no2, num2);
    }

    return num1 + num2;
}



template<typename DMY>
void DecreaseColorMC_T<DMY>::makeClut(int alpNum) {
    using namespace std;
    clutDat_t       clutDat[ 256 ];
    unsigned        clut[256];
    unsigned char   clutNos[256];
    memset(clutNos, 0, sizeof clutNos);
    memset(clutDat, 0x00, sizeof(clutDat_t) * 256);
    memset(clut , 0x00, sizeof(unsigned) * 256);
    memset(clut_, 0xff, sizeof(unsigned) * clutSize_);

    pixDat_t    *pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        int        no = pixDat->no;
        clutDat_t* d  = &clutDat[no];
        d->a         += pixDat->a;
        d->y         += pixDat->y;
        d->u         += pixDat->u;
        d->v         += pixDat->v;
        ++d->num;
        ++pixDat;
    }

    unsigned idx = 0;
    for (unsigned i = 0; i < 256; ++i) {
        clutDat_t*  d   = &clutDat[i];
        double      num = d->num;
        clut[idx]       = 0;
        if (num > 0) {
            int a = unsigned(d->a / num);
            if (a <= 1)
                a = 0;
            if (i == 0 && a == 0) {
                clut[idx]    = 0;
            } else {
                if (a >= 254 || int(idx) >= alpNum || alpBit_)
                    a = 0xff;
                num *= YUV_MUL;
                clut[idx]    = ayuv_to_argb(a, unsigned(d->y/num), unsigned(d->u/num), unsigned(d->v/num));
            }
            clutNos[i] = idx;
            ++idx;
        }
    }

    pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        int        no = pixDat->no;
        pixDat->idx   = clutNos[ no ];
        ++pixDat;
    }

    // clutSize_個のclutを作成する.
    assert(clutSize_ >= 0 && clutSize_ <= 256);
    if (alpBit_ == 0) {     // 普通の減色.
        memcpy(clut_, clut, sizeof(unsigned) * clutSize_);
    } else {
        // αNiM なピクセル用のclutを作る.
        assert(0 < alpBit_ && alpBit_ < 8);
        unsigned    alpBit  = alpBit_;
        unsigned    indBit  = 8 - alpBit;
        unsigned    cnum    = 1 << indBit;
        unsigned    alpCnt  = 1 << alpBit;
        unsigned    n       = 0;

        for (unsigned a = 0; a < alpCnt; ++a) {
            unsigned    aa  = 0xFF * a / (alpCnt-1);
            for (unsigned i = 0; i < cnum; ++i) {
                if (n >= clutSize_)                 // 念のためチェック.
                    goto EXIT_LOOP;
                clut_[n] = argb(aa,0,0,0) | (clut[i] & 0xFFffFF);
                ++n;
            }
        }
      EXIT_LOOP:
        ;
    }
}


template<typename DMY>
unsigned DecreaseColorMC_T<DMY>::ayuv_to_argb(unsigned a, unsigned y0, unsigned u0, unsigned v0) {
    unsigned r, g, b;
    if (modeRGB_) {
        g = y0;
        r = u0;
        b = v0;
    } else {
        int y = int(y0);
        int u = int(u0) - 128;
        int v = int(v0) - 128;
        g = (100000*y -  34414*u -  71414*v) / 100000;
        r = (100000*y            + 140200*v) / 100000;
        b = (100000*y + 177200*u           ) / 100000;

        g = clamp(g, 0, 255);
        r = clamp(r, 0, 255);
        b = clamp(b, 0, 255);
    }
    return argb(a,r,g,b);
}



/// 減色したピクセルデータを生成.
///
template<typename DMY>
void DecreaseColorMC_T<DMY>::makeDstPic() {
    pix8_t      *dstPic = dstPic_;
    if (dstPic == 0)
        return;     // clut作成だけのときはnullだから、かえる.

    if (alpBit_ == 0) {
        pixDat_t    *pixDat = &pixDat_[0];
        for (unsigned i = 0; i < size_; ++i) {
            //x *dstPic++  = pixDat->no;    // ごく、単純にメディアンカットしたときの番号をそのままピクセル値とする.
            *dstPic++  = pixDat->idx;
            ++pixDat;
        }
    } else {
        // ピクセル中にαビットを確保してる場合.
        unsigned    indBit  = 8 - alpBit_;
        unsigned    indNum  = 1 << indBit;
        unsigned    alpMsk  = ~(indNum - 1);
        pixDat_t    *pixDat = &pixDat_[0];
        for (unsigned i = 0; i < size_; ++i) {
            assert(pixDat->no < indNum);
            unsigned a  =  pixDat->a;
            a           =  a & alpMsk;
            //x *dstPic++  = a | (pixDat->no);
            *dstPic++  = a | (pixDat->idx);
            ++pixDat;
        }
    }
}




typedef DecreaseColorMC_T<>     DecreaseColorMC;

#endif
