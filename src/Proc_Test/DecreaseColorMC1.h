/**
 *  @file   DecreaseColorMC1.h
 *  @brief  メディアンカットな減色. YUV版
 *  @author Masashi KITAMURA
 *  @note
 *      http://www.h6.dion.ne.jp/~j_moto/html/program_myfo01.html
 *      のソースを改造したもの....した後に、yuv版のちゃんとした
 *      program_myfo03.htmlがあったことに気づくon_
 *
 */

#ifndef DECREASECOLORMC1_H
#define DECREASECOLORMC1_H

#pragma once

#include <assert.h>
#include <algorithm>


/// clut番号を連番でなく、スタックに溜めたものを出し入れして割り当てる.
#define DECREASECOLORMC_USE_MUBERSTK



/// メディアンカットな減色変換. 32ビット色(α無視)を8ビット色に変換.
template<typename DMY=void>
class DecreaseColor {
public:
    DecreaseColor();
    ~DecreaseColor();

    /// 変換.
    bool conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize);

    /// on で YUVでなく RGBで処理する. ※ GはY, RはU, BはVを使用.
    void setModeRGB(bool sw) { modeRGB_ = sw; }


private:
    bool     init(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned dstClut[], unsigned clutNum);

    int      convMain(int no, int num);
    unsigned calcMid(unsigned minVal, unsigned maxVal, unsigned val, unsigned total) const;
    void     makeClut();
    unsigned ayuv_to_argb(unsigned a, unsigned y0, unsigned u0, unsigned v0);
    void     makeDstPic();

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
  #ifdef DECREASECOLORMC_USE_MUBERSTK
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
    typedef unsigned char pixDat_int_t;

    /// 処理中のピクセルデータ. rgbモードでは、b,r,g順.
    struct pixDat_t {
        pixDat_int_t    v;
        pixDat_int_t    u;
        pixDat_int_t    y;
        unsigned char   no;                 ///< 処理のグループ番号(ほぼ色番号となる..が未使用色があるとづれる)
        unsigned char   idx;                ///< 色番号(未使用色をつめる処理のために用意)
    };

    /// clutを生成するのに使う.
    struct clutDat_t {
        double      y;                      ///< yの合計.
        double      u;                      ///< uの合計.
        double      v;                      ///< vの合計.
        double      num;                    ///< ピクセル合計.
    };

    unsigned*       clut_;                  ///< 減食した結果のclut.
    unsigned char*  dstPic_;                ///< 減色結果を入れるバッファ. NULLだと出力せず(clutのみの取得時).
    pixDat_t*       pixDat_;                ///< 処理中のピクセルデータ.
    const unsigned* srcPic_;                ///< 元のピクセルデータ.
    unsigned        w_;                     ///< 横幅
    unsigned        h_;                     ///< 縦幅
    unsigned        size_;                  ///< ピクセル合計
    unsigned        clutTop_;               ///< clutの開始番号.
    unsigned        clutSize_;              ///< clut数.

    bool            modeRGB_;               ///< YUVでなくGRBを用いる
};



template<typename DMY>
DecreaseColor<DMY>::DecreaseColor() {
    clut_           = 0;
    dstPic_         = 0;
    pixDat_         = 0;
    srcPic_         = 0;
    w_              = 0;
    h_              = 0;
    size_           = 0;
    clutTop_        = 0;
    clutSize_       = 0;
    modeRGB_        = 0;
}



template<typename DMY>
DecreaseColor<DMY>::~DecreaseColor() {
    if (pixDat_) {
        delete pixDat_;
        pixDat_ = 0;
    }
}



/// 変換.
///
template<typename DMY>
bool DecreaseColor<DMY>::conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize)
{
    // 初期化
    if (init(dstPic, srcPic, w, h, dstClut, clutSize) == false)
        return false;

    // メディアンカットな減色処理
    convMain(0, clutSize);

    // clutを作成する
    makeClut();

    // clutを用いて、24bit色を256色(clutSize色)に減色.
    makeDstPic();

    // 開放
    delete pixDat_;
    pixDat_ = 0;
    return true;
}



/// 今回の初期化.
///
template<typename DMY>
bool DecreaseColor<DMY>::init(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize)
{
    // とりあえず、パラメータは一通り控えておく.
    srcPic_   = srcPic;
    dstPic_   = dstPic;
    clut_     = dstClut;
    clutSize_ = clutSize;
    w_        = w;
    h_        = h;
    size_     = w * h;

    // 引数チェック
    if (srcPic == 0)
        return false;
    if (size_ == 0)
        return false;
    if (dstClut == 0)
        return false;
    if (clutSize < 2 || clutSize > 256)
        return false;

    // 作業メモリ確保
    pixDat_  = new pixDat_t[ size_ ];
    if (pixDat_ == 0)
        return false;

    // ピクセルrgbをyuvに変換して作業メモリを準備.
    for (unsigned i = 0; i < size_; ++i) {
        unsigned c      = srcPic[i];
        int     g       = argb_g(c);
        int     r       = argb_r(c);
        int     b       = argb_b(c);
        pixDat_[i].no   = clutTop_;
        if (modeRGB_) {
            pixDat_[i].y    = g;
            pixDat_[i].u    = r;
            pixDat_[i].v    = b;
        } else {
            int y,u,v;
            pixDat_[i].y    = y = ( 58661*g +  29891*r + 11448*b) / 100000;
            pixDat_[i].u    = u = (-33126*g -  16874*r + 50000*b) / 100000 + 128;
            pixDat_[i].v    = v = (-41869*g +  50000*r -  8131*b) / 100000 + 128;

            //x printf("%3d %3d,%3d,%3d  %2x,%2x,%2x\n", i, y,u,v, r,g,b);
            assert(0 <= y && y <= 255);
            assert(0 <= u && u <= 255);
            assert(0 <= v && v <= 255);
        }
    }

    return true;
}



/// 中間値を求める
///
template<typename DMY> inline
unsigned DecreaseColor<DMY>::calcMid(unsigned minVal, unsigned maxVal, unsigned val, unsigned total) const
{
    // ピクセル数を反映
    unsigned mid = val / total;
    mid += (mid <= minVal);
    return mid;

    // // 単純に割るだけの場合.
    // return (minVal + maxVal + 1) / 2;
}



/// メディアンカットな減色のメイン処理. 再帰実行.
/// @param  no      先頭番号
/// @param  num     処理する番号の範囲/数.
///
template<typename DMY>
int DecreaseColor<DMY>::convMain(int no, int num)
{
    //x printf("%d:%d\n", no, num);
    // 呼ばれた段階で pixDat_[].no には no が設定されている。
    // ので、num が2以上ないと分割の意味がないので、未満ならかえる。
    if (num < 2)
        return num;

    // まず対象となるピクセルを１つ探す
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

    // とりあえずの、y,u,v範囲を設定. ※ rgbモードではg,r,bに相当.
    unsigned    minY = pixDat->y;
    unsigned    maxY = minY;

    unsigned    minU = pixDat->u;
    unsigned    maxU = minU;

    unsigned    minV = pixDat->v;
    unsigned    maxV = minV;

    unsigned    sumY = minY;
    unsigned    sumU = minU;
    unsigned    sumV = minV;

    unsigned    sum  = 1;
    do {
        ++pixDat;
        if (no == pixDat->no) {
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

    int lenY = (int)(maxY - minY);
    int lenU = (int)(maxU - minU);
    int lenV = (int)(maxV - minV);

    // 距離がないってことはすでに１色のみなんで、分割できないので帰る.
    if (sum <= 1 || (lenY == 0 && lenU == 0 && lenV == 0))
        return num;     // 今回の処理をする段階でno番がnum個設定されてるので、そのままnumを返す.

    int mode;
    // 一番、幅がある部分を選択する
    // 同じ値の場合は Y,U,Vの順に選択される
    if (lenY >= lenU)
        mode = (lenY >= lenV) ? 2 : 0;
    else
        mode = (lenU >= lenV) ? 1 : 0;

    unsigned no1  = no;
    unsigned num1 = num / 2;
  #ifdef DECREASECOLORMC_USE_MUBERSTK
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
                pixDat->no = (pixDat->v < mid) ? (++c1, no1) : (++c2, no2);         // 暗い方から処理
            }
        }
        break;

    case 1: // U    (r)
        mid = calcMid(minU, maxU, sumU, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->u < mid) ? (++c1, no1) : (++c2, no2);
            }
        }
        break;

    case 2: // Y    (g)
        mid = calcMid(minY, maxY, sumY, sum);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            if (pixDat->no == no) {
                pixDat->no = (pixDat->y < mid) ? (++c1, no1) : (++c2, no2);
            }
        }
        break;

    default:
        assert(0);
    }

    // 分割したけど、１ピクセルもなかった場合.
    if (c1 == 0) {
      #ifdef DECREASECOLORMC_USE_MUBERSTK
        numberStk_.push(no1);   // 番号を未使用にする.
      #endif
        num1 = 0;
    }

  #ifdef DECREASECOLORMC_USE_MUBERSTK
    if (c2 == 0) {
        numberStk_.push(no2);
    }
  #endif

    // 再帰
    if (num1 > 1) {
        num1 = convMain(no1, num1);
    }

    unsigned num2 = num - num1;
    if (num <= 1 || c2 == 0)
        num2 = 0;

  #if defined(DECREASECOLORMC_USE_MUBERSTK) == 0
    if (no2 + num2 > no + num)
        num2 = no + num - no2;
  #endif

    if (num2 > 1) {
        num2 = convMain(no2, num2);
    }

    return num1 + num2;
}



/// clutを生成.
///
template<typename DMY>
void DecreaseColor<DMY>::makeClut()
{
    clutDat_t       clutDat[256];
    unsigned        clut[256];
    unsigned char   clutNos[256];
    std::memset(clutNos,    0, sizeof clutNos);
    std::memset(clutDat, 0x00, sizeof(clutDat_t) * 256);
    std::memset(clut ,   0x00, sizeof(unsigned)  * 256);
    std::memset(clut_,   0xff, sizeof(unsigned)  * clutSize_);

    pixDat_t    *pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        int        no = pixDat->no;
        clutDat_t* d  = &clutDat[no];
        d->y         += pixDat->y;
        d->u         += pixDat->u;
        d->v         += pixDat->v;
        ++d->num;
        ++pixDat;
    }

    // yuv->rgb変換. ※ clut番号を詰める.
    unsigned idx = 0;
    for (unsigned i = 0; i < 256; ++i) {
        clutDat_t*  d   = &clutDat[i];
        double      num = d->num;
        clut[idx]       = 0;
        if (num > 0) {
            clut[idx]   = ayuv_to_argb(0xff, unsigned(d->y/num), unsigned(d->u/num), unsigned(d->v/num));
            clutNos[i]  = idx;
            ++idx;
        }
    }

    pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        int        no = pixDat->no;
        pixDat->idx   = clutNos[ no ];
        ++pixDat;
    }

    // clutSize_個のclutを作成する
    assert(clutSize_ >= 0 && clutSize_ <= 256);
    std::memcpy(clut_, clut, sizeof(unsigned) * clutSize_);
}



/// (a,y,u,v) 1色を argbに変換.
///
template<typename DMY>
unsigned DecreaseColor<DMY>::ayuv_to_argb(unsigned a, unsigned y0, unsigned u0, unsigned v0)
{
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



/// 減色したピクセルデータを生成
///
template<typename DMY>
void DecreaseColor<DMY>::makeDstPic() {
    unsigned char   *dstPic = dstPic_;
    if (dstPic == 0)
        return;     // clut作成だけのときはnullだから、かえる

    pixDat_t    *pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        //x *dstPic++  = pixDat->no;    // ごく、単純にメディアンカットしたときの番号をそのままピクセル値とする.
        *dstPic++  = pixDat->idx;
        ++pixDat;
    }
}


#endif
