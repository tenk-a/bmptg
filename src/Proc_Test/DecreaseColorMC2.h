/**
 *  @file   DecreaseColorMC2.h
 *  @brief  ���f�B�A���J�b�g�Ȍ��F. YUV��
 *  @author Masashi KITAMURA
 *  @note
 *      http://www.h6.dion.ne.jp/~j_moto/html/program_myfo01.html
 *      �̃\�[�X��������������....������ɁAyuv�ł̂����Ƃ���
 *      program_myfo03.html�����������ƂɋC�Â�on_
 *
 */

#ifndef DECREASECOLORMC2_H
#define DECREASECOLORMC2_H

#pragma once

#include <assert.h>
#include <algorithm>


/// clut�ԍ���A�ԂłȂ��A�X�^�b�N�ɗ��߂����̂��o�����ꂵ�Ċ��蓖�Ă�.
#define DECREASECOLORMC_USE_MUBERSTK



/// ���f�B�A���J�b�g�Ȍ��F�ϊ�.
template<typename DMY=unsigned>
class DecreaseColor {
public:
    DecreaseColor();
    ~DecreaseColor();

    /// �ϊ�.
    bool conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize);

    /// ���Ɋւ���ݒ�. ���f�[�^�ɁA���������Ȃ��ꍇ��alpNum=1(�����F�̂�)�Ƃ��Ĉ���.
    void setAlphaParam(int alpNum, unsigned alpBit=0, unsigned minA=0, unsigned maxA=255);

    /// ���Ɋւ���ݒ�. ���f�[�^�ɔ��������Ȃ��Ƃ��ł� �K��alpNum���͂���悤�Ƃ���.
    void forceSetAlphaEnv(int alpNum, unsigned alpBit=0, unsigned minA=0, unsigned maxA=255);

    /// ���Ԓl�����߂�̂Ɏg���p�����[�^. �b��.
    void setCalcMidParam(unsigned calcMidMode=1, float ratioLenY=1.2f, float ratioLenU=1.f, float ratioLenV=1.f);

    /// on �� YUV�łȂ� RGB�ŏ�������.
    void setModeRGB(bool sw);

private:
    typedef unsigned int    pix32_t;
    typedef unsigned char   pix8_t;
    typedef unsigned short  pix16_t;

    bool init(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned dstClut[], unsigned clutNum, int* pAlpNum);
    int checkAlp();
    int convMain(int no, int num);
    unsigned calcMid(unsigned minVal, unsigned maxVal, unsigned val, unsigned total) const;
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

    /// �������̃s�N�Z���f�[�^. rgb���[�h�ł́Ab,r,g,a��.
    struct pixDat_t {
        pixDat_int_t    v;
        pixDat_int_t    u;
        pixDat_int_t    y;
        unsigned char   a;
        unsigned char   no;                 ///< �����̃O���[�v�ԍ�(�قڐF�ԍ��ƂȂ�..�����g�p�F������ƂÂ��)
        unsigned char   idx;                ///< �F�ԍ�(���g�p�F���߂鏈���̂��߂ɗp��)
    };

    /// clut�𐶐�����̂Ɏg��.
    struct clutDat_t {
        double      a;                      ///< a�̍��v.
        double      y;                      ///< y�̍��v.
        double      u;                      ///< u�̍��v.
        double      v;                      ///< v�̍��v.
        double      num;                    ///< �s�N�Z�����v.
    };

    unsigned*       clut_;                  ///< ���H�������ʂ�clut.
    pix8_t*         dstPic_;                ///< ���F���ʂ�����o�b�t�@. NULL���Əo�͂���(clut�݂̂̎擾��).
    pixDat_t*       pixDat_;                ///< �������̃s�N�Z���f�[�^.
    const pix32_t*  srcPic_;                ///< ���̃s�N�Z���f�[�^.
    unsigned        w_;                     ///< ����
    unsigned        h_;                     ///< �c��
    unsigned        size_;                  ///< �s�N�Z�����v
    unsigned        clutTop_;               ///< clut�̊J�n�ԍ�.
    unsigned        clutSize_;              ///< clut��.

    unsigned        ratioLenA_;             ///< �����������邩�ǂ����ɗp����B0 or 1�Ƃ��Ďg�p�B
    bool            forceAlpNum_;           ///< �����I��alpNum����K�������F�Ƃ��Ĉ���(���� �g�p���Ȃ��ԍ��ɂȂ�)
    int             alpNum_;                ///< ���tcolor�ɂ���F��
    unsigned        minA_;                  ///< ����ȉ��̂Ƃ��́A����(��=0)�ɂ���
    unsigned        maxA_;                  ///< ����ȏ�̂Ƃ��́A�s����(��=0xff)�ɂ���

    // ���Ԓl�����߂�̂ɗp����p�����[�^
    float           ratioLenY_;             ///< Y,U,V�̑I���ɂ����āA�\�� Y��ratioLenY_�{���Ă����r.
    float           ratioLenU_;             ///< Y,U,V�̑I���ɂ����āA�\�� U��ratioLenU_�{���Ă����r.
    float           ratioLenV_;             ///< Y,U,V�̑I���ɂ����āA�\�� V��ratioLenV_�{���Ă����r.
    unsigned        calcMidMode_;           ///< 0���ƒP����(min+max)/2  1���ƃs�N�Z�����f.
    //x unsigned    midDif_;                ///< max-min>=2*dif�𖞂������ɁA�s�N�Z�����𔽉f���Ē����l���擾.
    //x float       ratioMidSiz_;           ///< ���̊�����S�̂̃s�N�Z�����Ɋ|�������ʂ����A�����ꍇ�Ƀs�N�Z�����𔽉f���Ē����l���擾.

    bool            modeRGB_;               ///< YUV�łȂ�GRB��p����

    unsigned        transparentNo_;
    unsigned        alpTopNo_;
    unsigned        alpBit_;
    unsigned        normNo_;
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

    ratioLenA_   = 0;       // �������F�����̑Ώۂɂ��邩�ǂ�����ݒ肷��̂ɗp���Ă���.
    setAlphaParam(-1);
}



template<typename DMY>
DecreaseColor<DMY>::~DecreaseColor() {
    if (pixDat_) {
        delete pixDat_;
        pixDat_ = 0;
    }
}



/// �ϊ�.
template<typename DMY>
bool DecreaseColor<DMY>::conv(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize) {
    // ������
    int alpNum = 0;
    if (init(dstPic, srcPic, w, h, dstClut, clutSize, &alpNum) == false)
        return false;

    if (alpBit_ > 0) {      // AnIm �ȃs�N�Z���ɂ���̂ŃC���f�b�N�X��m�r�b�g���ɂ���.
        assert(alpBit_ < 8);
        unsigned cnum = 1 << (8 - alpBit_);
        clutSize      = (clutSize < cnum) ? clutSize : cnum;
        alpNum        = (alpNum > 0);   // �C���f�b�N�X���̓��Ȃ��A���ǁA�����F�w��͔��f�B
    }

    // ���v���[���̏���.
    if (alpNum > 1) {
        ratioLenA_ = 1;
        convMain(alpTopNo_, alpNum-1);              // �����F�͏����ςƂ��ĂP���炷
    }

    // ���f�B�A���J�b�g�Ȍ��F����
    ratioLenA_ = 0;
    convMain(normNo_, clutSize-alpNum);

    // clut���쐬����
    makeClut(alpNum);

    // clut��p���āA24bit�F��256�F(clutSize�F)�Ɍ��F.
    makeDstPic();

    // �J��
    delete pixDat_;
    pixDat_ = 0;
    return true;
}



/** ���Ɋւ���ݒ�. ���f�[�^�ɁA���������Ȃ��ꍇ��alpNum=1(�����F�̂�)�Ƃ��Ĉ���.
 *  @param  alpNum      �������F�Ɏg���F��.
 *  @param  minA        ����ȉ��͓����F�Ƃ���
 *  @param  maxA        ����ȏ�͕s�����Ƃ���
 */
template<typename DMY>
void DecreaseColor<DMY>::setAlphaParam(int alpNum, unsigned alpBit, unsigned minA, unsigned maxA) {
    forceAlpNum_ = false;
  #if 1 // �ڐ�̌덷�o�O��΍�
    if (minA < 1)
        minA = 1;
    if (maxA > 255-1)
        maxA = 255-1;
  #endif

    alpNum_  = alpNum;
    minA_    = minA;
    maxA_    = maxA;

    alpBit_  = alpBit;
    if (alpBit > 0)     // ���r�b�g���m�ۂ���ꍇ�́A�C���f�b�N�X���̓��Ȃ�(�������A�����F�w��͔��f)
        alpNum_ = (alpNum != 0);
    // invariant();
}



/** ���Ɋւ���ݒ�. ���f�[�^�ɔ��������Ȃ��Ƃ��ł� �K��alpNum���͂���悤�Ƃ���.
 *  @param  alpNum      �������F�Ɏg���F��.
 *  @param  minA        ����ȉ��͓����F�Ƃ���
 *  @param  maxA        ����ȏ�͕s�����Ƃ���
 */
template<typename DMY>
void DecreaseColor<DMY>::forceSetAlphaEnv(int alpNum, unsigned alpBit, unsigned minA, unsigned maxA) {
    setAlphaParam(alpNum,alpBit,minA,maxA);
    forceAlpNum_ = true;
}



/** ���Ԓl�����߂�̂Ɏg���p�����[�^. �b��d�l.
 *  �K���Ȍv�Z�Ȃ̂ŁA�Ӗ��͂킩��ɂ���...���s���낷��p.
 */
template<typename DMY>
void DecreaseColor<DMY>::setCalcMidParam(unsigned calcMidMode, float ratioLenY, float ratioLenU, float ratioLenV)
{
    calcMidMode_ = calcMidMode;             // 0���ƒP����(min+max)/2  1���ƃs�N�Z�����f.
    ratioLenY_   = ratioLenY;               // Y,U,V�̑I���ɂ����āA�\�� Y��ratioLenY_�{����
    ratioLenU_   = ratioLenU;
    ratioLenV_   = ratioLenV;
    //x midDif_      = midDif;              // max-min>=2*dif�𖞂������ɁA�s�N�Z�����𔽉f���Ē����l���擾.
    //x ratioMidSiz_ = ratioMidSiz;         // ���̊�����S�̂̃s�N�Z�����Ɋ|�������ʂ����A�����ꍇ�Ƀs�N�Z�����𔽉f���Ē����l���擾.
}



/** on �� YUV�łȂ� RGB�ŏ�������. �� G��Y, R��U, B��V���g�p.
 */
template<typename DMY>
void DecreaseColor<DMY>::setModeRGB(bool sw) {
    modeRGB_ = sw;
    if (sw) // RGB�̂Ƃ��́A�p�x���g�����[�h�ɂ��AY(G)��1�{�ɂ��Ă���.
        setCalcMidParam(1, 1.0f, 1.0f, 1.0f);
    else
        setCalcMidParam();
}




/// ����̏�����.
template<typename DMY>
bool DecreaseColor<DMY>::init(pix8_t* dstPic, const pix32_t* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize, int* pAlpNum) {
    // �Ƃ肠�����A�p�����[�^�͈�ʂ�T���Ă���.
    srcPic_   = srcPic;
    dstPic_   = dstPic;
    clut_     = dstClut;
    clutSize_ = clutSize;
    w_        = w;
    h_        = h;
    size_     = w * h;

    // �����`�F�b�N
    if (srcPic == 0)
        return false;
    if (size_ == 0)
        return false;
    if (dstClut == 0)
        return false;
    if (clutSize < 2 || clutSize > 256)
        return false;

    // ��ƃ������m��
    pixDat_  = new pixDat_t[ size_ ];
    if (pixDat_ == 0)
        return false;

    // �s�N�Z��rgb��yuv�ɕϊ����č�ƃ�����������.
    for (unsigned i = 0; i < size_; ++i) {
        pix32_t c       = srcPic[i];
        int     a       = argb_a(c);
        int     g       = argb_g(c);
        int     r       = argb_r(c);
        int     b       = argb_b(c);
        pixDat_[i].no   = clutTop_;
        pixDat_[i].a    = a;
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

    *pAlpNum = checkAlp();

    return true;
}



/// ���̏�Ԃ��`�F�b�N�����f
///
template<typename DMY>
int DecreaseColor<DMY>::checkAlp()
{

    pixDat_t*   pixEnd  = &pixDat_[size_];
    pixDat_t*   pixDat;

    int         minA    = (int)minA_;
    int         maxA    = (int)maxA_;

    // ���͉��F�p���邩�A��\�ߐݒ�
    int  alpNum   = alpNum_;
    if (alpNum < 0) {   // �K���Ƀ�����ݒ�.
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
        std::memset(alpChk, 0, sizeof alpChk);
        for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
            unsigned a = pixDat->a;
            ++alpChk[a];
        }

        // ���`�F�b�N.
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
        // �����I�Ƀ�clut��p�ӂ���̂łȂ���Ύ��f�[�^�̏�Ԃ𔽉f.
        if (forceAlpNum_ == false && alpNum > 0) {
            if (blend == 0) {
                //x if (tra == 0)   // ���������������f�[�^�ɂȂ���΁A�I���
                //x     return 0;
                alpNum = (tra != 0);        // ���f�[�^�ɔ��������Ȃ��Ȃ�A�����F����.
            }
        }
    }

  #ifdef DECREASECOLORMC_USE_MUBERSTK
    if (alpNum > 0) {
        transparentNo_ = numberStk_.pop();
        alpTopNo_      = transparentNo_;
        if (alpNum > 1)
            alpTopNo_ = numberStk_.pop();   // �����l�̐ݒ�ŁA�K�� 1��Ԃ��Ă�̑O��.
    }
    normNo_         = numberStk_.pop();
  #else
    transparentNo_  = clutTop_;
    alpTopNo_       = clutTop_ + (alpNo > 1);
    normNo_         = clutTop_ + alpNum;
  #endif

    for (pixDat = &pixDat_[0]; pixDat != pixEnd; ++pixDat) {
        int a  =  pixDat->a;
        if (a  <= minA && alpNum > 0) {             // ����
            pixDat->no = transparentNo_;
            pixDat->a  = 0;

        } else if (a < maxA && alpNum > 1) {        // ������
            pixDat->no = alpTopNo_;

        } else {                                    // �s����
            pixDat->no = normNo_;
            if (alpBit_ == 0)                       // ���r�b�g�̎w��̂Ȃ����͕s�����Ƃ���0xff�ɐݒ�.
                pixDat->a  = 0xff;
        }
    }

    return alpNum;
}



/// ���Ԓl�����߂�
///
template<typename DMY> inline
unsigned DecreaseColor<DMY>::calcMid(unsigned minVal, unsigned maxVal, unsigned val, unsigned total) const {
    if (calcMidMode_ == 0) {    // �P���Ɋ���
        return (minVal + maxVal + 1) / 2;

    } else {    // �s�N�Z�����𔽉f
        unsigned mid = val / total;
        mid += (mid <= minVal);
        return mid;
    }
}



/// ���f�B�A���J�b�g�Ȍ��F�̃��C������. �ċA���s.
/// @param  no      �擪�ԍ�
/// @param  num     ��������ԍ��͈̔�/��.
///
template<typename DMY>
int DecreaseColor<DMY>::convMain(int no, int num) {
    //x printf("%d:%d\n", no, num);
    // �Ă΂ꂽ�i�K�� pixDat_[].no �ɂ� no ���ݒ肳��Ă���B
    // �̂ŁAnum ��2�ȏ�Ȃ��ƕ����̈Ӗ����Ȃ��̂ŁA�����Ȃ炩����B
    if (num < 2)
        return num;

    // �܂��ΏۂƂȂ�s�N�Z�����P�T��
    pixDat_t*   pixEnd = &pixDat_[size_];
    pixDat_t*   pixDat = &pixDat_[0];
    if (no != pixDat->no) {
        do {
            ++pixDat;
        } while (pixDat != pixEnd && no != pixDat->no);
    }

    // �ΏۂƂȂ�s�N�Z������������...
    if (pixDat == pixEnd) {
        return 0;
    }

    // �Ƃ肠�����́Aa,y,u,v�͈͂�ݒ�. �� rgb���[�h�ł�a,g,r,b�ɑ���.
    unsigned    minA = pixDat->a;
    unsigned    maxA = minA;
    unsigned    minY = pixDat->y;
    unsigned    maxY = minY;
    unsigned    minU = pixDat->u;
    unsigned    maxU = minU;
    unsigned    minV = pixDat->v;
    unsigned    maxV = minV;
    unsigned    sumA = minA;
    unsigned    sumY = minY;
    unsigned    sumU = minU;
    unsigned    sumV = minV;

    unsigned    sum  = 1;
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

    int lenA = (maxA - minA) * ratioLenA_;      // �����������邩�ǂ����́AratioLenA_��1(�ȏ�)�ł���A0�ł��Ȃ�.
    int lenY = (int)((maxY - minY) * ratioLenY_);
    int lenU = (int)((maxU - minU) * ratioLenU_);
    int lenV = (int)((maxV - minV) * ratioLenV_);

    // �������Ȃ����Ă��Ƃ͂��łɂP�F�݂̂Ȃ�ŁA�����ł��Ȃ��̂ŋA��.
    if (sum <= 1 || (lenA == 0 && lenY == 0 && lenU == 0 && lenV == 0))
        return num;     // ����̏���������i�K��no�Ԃ�num�ݒ肳��Ă�̂ŁA���̂܂�num��Ԃ�.

    int mode;
    // ��ԁA�������镔����I������
    // �����l�̏ꍇ�� A,Y,U,V�̏��ɑI�������
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
                pixDat->no = (pixDat->v < mid) ? (++c1, no1) : (++c2, no2);         // �Â������珈��
                //pixDat->no = (pixDat->v >= mid) ? (++c1, no1) : (++c2, no2);      // ���邢�����珈��
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
                //x pixDat->no = (pixDat->a > mid) ? (++c1, no1) : (++c2, no2); // ���������ƌ�ϊ�����������...
            }
        }
        break;

    default:
        assert(0);
    }

    // �����������ǁA�P�s�N�Z�����Ȃ������ꍇ.
    if (c1 == 0) {
      #ifdef DECREASECOLORMC_USE_MUBERSTK
        numberStk_.push(no1);   // �ԍ��𖢎g�p�ɂ���.
      #endif
        num1 = 0;
    }

  #ifdef DECREASECOLORMC_USE_MUBERSTK
    if (c2 == 0) {
        numberStk_.push(no2);
    }
  #endif

    // �ċA
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



template<typename DMY>
void DecreaseColor<DMY>::makeClut(int alpNum) {
    clutDat_t       clutDat[ 256 ];
    unsigned        clut[256];
    unsigned char   clutNos[256];
    std::memset(clutNos, 0, sizeof clutNos);
    std::memset(clutDat, 0x00, sizeof(clutDat_t) * 256);
    std::memset(clut , 0x00, sizeof(unsigned) * 256);
    std::memset(clut_, 0xff, sizeof(unsigned) * clutSize_);

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

    // clutSize_��clut���쐬����
    assert(clutSize_ >= 0 && clutSize_ <= 256);
    if (alpBit_ == 0) {     // ���ʂ̌��F.
        std::memcpy(clut_, clut, sizeof(unsigned) * clutSize_);
    } else {
        // ��NiM �ȃs�N�Z���p��clut�����.
        assert(0 < alpBit_ && alpBit_ < 8);
        unsigned    alpBit  = alpBit_;
        unsigned    indBit  = 8 - alpBit;
        unsigned    cnum    = 1 << indBit;
        unsigned    alpCnt  = 1 << alpBit;
        unsigned    n       = 0;

        for (unsigned a = 0; a < alpCnt; ++a) {
            unsigned    aa  = 0xFF * a / (alpCnt-1);
            for (unsigned i = 0; i < cnum; ++i) {
                if (n >= clutSize_)                 // �O�̂��߃`�F�b�N.
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
unsigned DecreaseColor<DMY>::ayuv_to_argb(unsigned a, unsigned y0, unsigned u0, unsigned v0) {
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



/// ���F�����s�N�Z���f�[�^�𐶐�
///
template<typename DMY>
void DecreaseColor<DMY>::makeDstPic() {
    pix8_t      *dstPic = dstPic_;
    if (dstPic == 0)
        return;     // clut�쐬�����̂Ƃ���null������A������

    if (alpBit_ == 0) {
        pixDat_t    *pixDat = &pixDat_[0];
        for (unsigned i = 0; i < size_; ++i) {
            //x *dstPic++  = pixDat->no;    // �����A�P���Ƀ��f�B�A���J�b�g�����Ƃ��̔ԍ������̂܂܃s�N�Z���l�Ƃ���.
            *dstPic++  = pixDat->idx;
            ++pixDat;
        }
    } else {
        // �s�N�Z�����Ƀ��r�b�g���m�ۂ��Ă�ꍇ.
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




#endif
