/**
 *  @file   DecreaseColorMC1.h
 *  @brief  ���f�B�A���J�b�g�Ȍ��F. YUV��
 *  @author Masashi KITAMURA
 *  @note
 *      http://www.h6.dion.ne.jp/~j_moto/html/program_myfo01.html
 *      �̃\�[�X��������������....������ɁAyuv�ł̂����Ƃ���
 *      program_myfo03.html�����������ƂɋC�Â�on_
 *
 */

#ifndef DECREASECOLORMC1_H
#define DECREASECOLORMC1_H

#pragma once

#include <assert.h>
#include <algorithm>


/// clut�ԍ���A�ԂłȂ��A�X�^�b�N�ɗ��߂����̂��o�����ꂵ�Ċ��蓖�Ă�.
#define DECREASECOLORMC_USE_MUBERSTK



/// ���f�B�A���J�b�g�Ȍ��F�ϊ�. 32�r�b�g�F(������)��8�r�b�g�F�ɕϊ�.
template<typename DMY=void>
class DecreaseColor {
public:
    DecreaseColor();
    ~DecreaseColor();

    /// �ϊ�.
    bool conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize);

    /// on �� YUV�łȂ� RGB�ŏ�������. �� G��Y, R��U, B��V���g�p.
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

    /// �������̃s�N�Z���f�[�^. rgb���[�h�ł́Ab,r,g��.
    struct pixDat_t {
        pixDat_int_t    v;
        pixDat_int_t    u;
        pixDat_int_t    y;
        unsigned char   no;                 ///< �����̃O���[�v�ԍ�(�قڐF�ԍ��ƂȂ�..�����g�p�F������ƂÂ��)
        unsigned char   idx;                ///< �F�ԍ�(���g�p�F���߂鏈���̂��߂ɗp��)
    };

    /// clut�𐶐�����̂Ɏg��.
    struct clutDat_t {
        double      y;                      ///< y�̍��v.
        double      u;                      ///< u�̍��v.
        double      v;                      ///< v�̍��v.
        double      num;                    ///< �s�N�Z�����v.
    };

    unsigned*       clut_;                  ///< ���H�������ʂ�clut.
    unsigned char*  dstPic_;                ///< ���F���ʂ�����o�b�t�@. NULL���Əo�͂���(clut�݂̂̎擾��).
    pixDat_t*       pixDat_;                ///< �������̃s�N�Z���f�[�^.
    const unsigned* srcPic_;                ///< ���̃s�N�Z���f�[�^.
    unsigned        w_;                     ///< ����
    unsigned        h_;                     ///< �c��
    unsigned        size_;                  ///< �s�N�Z�����v
    unsigned        clutTop_;               ///< clut�̊J�n�ԍ�.
    unsigned        clutSize_;              ///< clut��.

    bool            modeRGB_;               ///< YUV�łȂ�GRB��p����
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



/// �ϊ�.
///
template<typename DMY>
bool DecreaseColor<DMY>::conv(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize)
{
    // ������
    if (init(dstPic, srcPic, w, h, dstClut, clutSize) == false)
        return false;

    // ���f�B�A���J�b�g�Ȍ��F����
    convMain(0, clutSize);

    // clut���쐬����
    makeClut();

    // clut��p���āA24bit�F��256�F(clutSize�F)�Ɍ��F.
    makeDstPic();

    // �J��
    delete pixDat_;
    pixDat_ = 0;
    return true;
}



/// ����̏�����.
///
template<typename DMY>
bool DecreaseColor<DMY>::init(unsigned char* dstPic, const unsigned* srcPic, unsigned w, unsigned h, unsigned* dstClut, unsigned clutSize)
{
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



/// ���Ԓl�����߂�
///
template<typename DMY> inline
unsigned DecreaseColor<DMY>::calcMid(unsigned minVal, unsigned maxVal, unsigned val, unsigned total) const
{
    // �s�N�Z�����𔽉f
    unsigned mid = val / total;
    mid += (mid <= minVal);
    return mid;

    // // �P���Ɋ��邾���̏ꍇ.
    // return (minVal + maxVal + 1) / 2;
}



/// ���f�B�A���J�b�g�Ȍ��F�̃��C������. �ċA���s.
/// @param  no      �擪�ԍ�
/// @param  num     ��������ԍ��͈̔�/��.
///
template<typename DMY>
int DecreaseColor<DMY>::convMain(int no, int num)
{
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

    // �Ƃ肠�����́Ay,u,v�͈͂�ݒ�. �� rgb���[�h�ł�g,r,b�ɑ���.
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

    // �������Ȃ����Ă��Ƃ͂��łɂP�F�݂̂Ȃ�ŁA�����ł��Ȃ��̂ŋA��.
    if (sum <= 1 || (lenY == 0 && lenU == 0 && lenV == 0))
        return num;     // ����̏���������i�K��no�Ԃ�num�ݒ肳��Ă�̂ŁA���̂܂�num��Ԃ�.

    int mode;
    // ��ԁA�������镔����I������
    // �����l�̏ꍇ�� Y,U,V�̏��ɑI�������
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
                pixDat->no = (pixDat->v < mid) ? (++c1, no1) : (++c2, no2);         // �Â������珈��
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



/// clut�𐶐�.
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

    // yuv->rgb�ϊ�. �� clut�ԍ����l�߂�.
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

    // clutSize_��clut���쐬����
    assert(clutSize_ >= 0 && clutSize_ <= 256);
    std::memcpy(clut_, clut, sizeof(unsigned) * clutSize_);
}



/// (a,y,u,v) 1�F�� argb�ɕϊ�.
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



/// ���F�����s�N�Z���f�[�^�𐶐�
///
template<typename DMY>
void DecreaseColor<DMY>::makeDstPic() {
    unsigned char   *dstPic = dstPic_;
    if (dstPic == 0)
        return;     // clut�쐬�����̂Ƃ���null������A������

    pixDat_t    *pixDat = &pixDat_[0];
    for (unsigned i = 0; i < size_; ++i) {
        //x *dstPic++  = pixDat->no;    // �����A�P���Ƀ��f�B�A���J�b�g�����Ƃ��̔ԍ������̂܂܃s�N�Z���l�Ƃ���.
        *dstPic++  = pixDat->idx;
        ++pixDat;
    }
}


#endif
