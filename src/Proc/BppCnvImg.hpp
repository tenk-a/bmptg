/**
 *  @file   BppCnvImg.h
 *  @brief  ��ɉ摜�� bpp �≡���A���C�����g�𒲐����邽�߂̃N���X.
 *  @author Masashi KITAMURA
 */

#ifndef BPPCNVIMG_H
#define BPPCNVIMG_H


#include <assert.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <functional>
#if defined _WIN32 == 0
#include <stdint.h>
#else
#include <windows.h>
#endif



// ----------------------------------------------------------------------------
// �������[�m�ہE�J���p �N���X. malloc,free�̑�p�i.
// typedef std::allocator<unsigned char>  BppCnvImg_byte_allocator;
// �ł������Aallocate(n), deallocate(p,n)�݂̂���΂����̂�..

#ifndef BppCnvImg_byte_allocator
struct BppCnvImg_byte_allocator {
  #if defined _WINDOWS  // win�A�v���Ȃ� win-api��.
    static unsigned char* allocate(unsigned n) {
        void *alc_mem_ptr = ::LocalAlloc(LMEM_FIXED, n);
        assert(alc_mem_ptr != 0);
        return (unsigned char*)alc_mem_ptr;
    }

    static void deallocate(unsigned char* p, unsigned) {
        if (p)
            ::LocalFree(p);
    }
  #else
    static unsigned char* allocate(unsigned n) {
        void *alc_mem_ptr = std::malloc(n);
        assert(alc_mem_ptr != 0);   // if (alc_mem_ptr) throw std::bad_alloc();
        return (unsigned char*)alc_mem_ptr;
    }

    static void deallocate(unsigned char* p, unsigned) {
        // if (p)
            std::free(p);
    }
  #endif
};
#endif



// ----------------------------------------------------------------------------

/** bpp�≡���A���C�����g�̕ϊ��p�̉摜�N���X.
 *  �� template�ɂ��Ă�̂́A�w�b�_�݂̂Ƀ��[�`��������������....
 */
template<class A=BppCnvImg_byte_allocator>
class BppCnvImg_T : private A {
public:
    // bpp������
    enum Fmt {          // bpp�ɑ΂���⏕�I�ȃt�H�[�}�b�g���.
        FMT_N        =  0,      // �f�t�H���g�w�蓙
        FMT_I1       =  1,      //bpp=1     2�F
        FMT_I2       =  2,      //bpp=2     4�F
        FMT_I4       =  4,      //bpp=4     16�F
        FMT_I8       =  8,      //bpp=8     256�F
        FMT_ARGB4444 = 12,
        FMT_GRB655   = 13,
        FMT_BRG556   = 14,
        FMT_ARGB1555 = 15,
        FMT_RGB565   = 16,
        FMT_RGB      = 24,      //bpp
        FMT_ARGB     = 32,      //bpp
    };

public:
    BppCnvImg_T() { zeroclear(); }
    ~BppCnvImg_T() { release(); }

    /// �R�s�[�R���X�g���N�^.
    BppCnvImg_T(const BppCnvImg_T& rhs);

    /// �쐬. alc=true�̂Ƃ�pix,clut�̃������𓮓I�Ɏ擾���ĊǗ�.
    /// alc=false�̎��͈��������̂܂ܕێ�(�J�����͍s��Ȃ�).
    BppCnvImg_T(bool alc, void* pix, unsigned w, unsigned h, unsigned bppFmt, unsigned algn
                , unsigned* clut, unsigned clutSize=unsigned(-1))
        : pix_(0), clut_(0)
    {
        create(alc, pix, w, h, bppFmt, algn, clut, clutSize);
    }

    /// w*h ��bpp(Fmt)�̖��n�̉摜���쐬.
    BppCnvImg_T(unsigned w, unsigned h, unsigned bppFmt, unsigned align=1, unsigned clutSize=unsigned(-1))
        : pix_(0), clut_(0)
    {
        create(true, NULL, w, h, bppFmt, align, NULL, clutSize);
    }

    /// src���w��fmt�ɕϊ����č쐬.
    BppCnvImg_T(const BppCnvImg_T& src, unsigned bppFmt, unsigned align=1)
        : pix_(0), clut_(0)
    {
        create(src, bppFmt, align);
    }

    /// �쐬. alc=true�̂Ƃ�pix,clut�̃������𓮓I�Ɏ擾���ĊǗ�.
    /// alc=false�̎��͈��������̂܂ܕێ�(�J�����͍s��Ȃ�).
    bool create(bool alc, void* pix, unsigned w, unsigned h, unsigned bppFmt, unsigned srcAlgn
                , unsigned* clut, unsigned clutSize=unsigned(-1) );

    /// w*h ��bpp(Fmt)�̖��n�̉摜���쐬.
    bool create(unsigned w, unsigned h, unsigned bppFmt, unsigned align=1, unsigned clutSize=unsigned(-1)) {
        return create(true, NULL, w, h, bppFmt, align, NULL, clutSize);
    }

    /// src ����bpp�ƃA���C�����g��ύX�������m���쐬.
    bool create(const BppCnvImg_T& src, unsigned bppFmt, unsigned align=1);

    /// �J��.
    void release();

    /// �R�s�[.
    BppCnvImg_T& operator=(const BppCnvImg_T& rhs) {
        BppCnvImg_T(rhs).swap(*this);
        return *this;
    }

    /// ����.
    void swap(BppCnvImg_T& rhs);

    // �����o�[����Ԃ�.
    unsigned    width()       const { return w_; }
    unsigned    height()      const { return h_; }
    unsigned    bpp()         const { return bpp_; }
    unsigned    fmt()         const { return fmt_; }
    unsigned    widthByte()   const { return wb_; }
    unsigned    align()       const { return align_; }

    unsigned char*       image()            { return pix_; }
    const unsigned char* image()      const { return pix_; }
    unsigned             imageBytes() const { return wb_ * h_; }

    unsigned*           clut()              { return clut_; }
    const unsigned*     clut()        const { return clut_; }
    unsigned            clutSize()    const { return clutSz_; }

    void    getClut(unsigned* clut, unsigned clutSize=0);
    void    setClut(const unsigned* clut, unsigned clutSize);

    /// fmtBpp�`���ɕϊ�����w*h����pix�ɐݒ�.
    bool    getImage(void* pix, unsigned w=0, unsigned h=0, unsigned fmtBpp=0, unsigned algn=0
                        , unsigned* clut=0, unsigned clutSize=0, int ox=0, int oy=0);

    /// src�̉摜���Adst�ցAdst�̏��ݒ�ŕϊ����Đݒ�. ���F->clut�ϊ��͍s��Ȃ�.
    // (ox,oy)��src���̊J�n�_.
    //  �� clut�悩��F���̌���clut��ւ̕ϊ��́A�P���ɏ�ʃr�b�g���̂Ă邾��.
    //      ���F���K�v�Ȃ��convEx��p���邩�A�\�ߌ��F���Ă�������.
    static bool conv(BppCnvImg_T& dst, const BppCnvImg_T& src, int ox=0, int oy=0);
    bool conv(const BppCnvImg_T& src, int ox=0, int oy=0) { return conv(*this,src,ox,oy); }

    /// src�̉摜���Adst�ցAdst�̏��ݒ�ŕϊ����Đݒ�. �K�v�Ȃ�Ό��F���s��(�Ȉ�).
    static bool convEx(BppCnvImg_T& dst, const BppCnvImg_T& src, int ox=0, int oy=0);
    bool convEx(const BppCnvImg_T& src, int ox=0, int oy=0) {return convEx(*this,src,ox,oy);}

    // 0�Ȃ� little endian  1�Ȃ�big endian
    void setSrcEndian(bool sw)  { flags_ &= ~4; flags_ |= int(sw)<<2; }
    bool srcEndian() const      { return (flags_ & 4) != 0; }
    void setDstEndian(bool sw)  { flags_ &= ~8; flags_ |= int(sw)<<3; }
    bool dstEndian() const      { return (flags_ & 8) != 0; }

    void revY();                ///< �㉺���]����.
    void swapBitOrder();        ///< bpp=1,2,4�̂Ƃ��A�P�o�C�g���̋l�ߏ��𔽓]����.
    void swapByteOrder();       ///< bpp> 8�̂Ƃ��A�P�s�N�Z�����̃o�C�g�̏��𔽓]����.

    bool isUseAlpha() const;    ///< ���������g���Ă����true(�s�����Ɠ��������݂��Ă���ꍇ��true)

    /// conv(),convEx()�ɂ�16�r�b�g�F��֕ϊ����鎞, �p�^�[���f�B�U��K�p.
    void setDitherMode(bool sw=true) { flags_ &= ~2; flags_ |= int(sw)<<1; }

    /// �p�^�[���f�B�U��K�p���邩�ۂ��̏�Ԃ�Ԃ�.
    bool ditherMode() const { return (flags_ & 2) != 0; }

    /// dst��clut��ŁAsrc�� dst.clutSize()�F�ȉ������F���g���Ă��Ȃ��ꍇ�Asrc��dst�ɕϊ�.
    static bool     convToClutImage(BppCnvImg_T& dst, const BppCnvImg_T& src, int ox, int oy);
    bool convToClutImage(const BppCnvImg_T& src, int ox=0, int oy=0) { return convToClutImage(*this,src,ox,oy); }

    unsigned getPixXY(int x, int y) const;          ///< (x,y)�̈ʒu�̃s�N�Z��(color index �ԍ�). �Ԓl�͑��F�Ȃ�ARGB8888�ɕϊ������l.
    unsigned getPixArgb(int x, int y) const;        ///< (x,y)�̈ʒu�̐F��Ԃ�. �Ԓl�͑��F�Ȃ�ARGB8888�ɕϊ������l.
    void     setPixXY(int x, int y, unsigned c);    ///< (x,y)�̈ʒu�Ƀs�N�Z����ݒ�. c�͑��F�̏ꍇ��ARGB8888

    /// ����,bpp,�A���C�����g���牡���o�C�g�������߂�.
    static unsigned widthToBytes(unsigned w, unsigned fmtBpp, unsigned algn);

    /// EFmt �̒l���� ���ۂ�bpp�����߂�.
    static unsigned fmtToBpp(Fmt fmt);

    // little endian�l�ł�ARGB����ɁAargb���������摜�� a,r,g,b�̏��Ԃ�ύX.
    void swapToAGRB() { swapARGB( 1, false); }
    void swapToABRG() { swapARGB( 3, false); }
    void swapToABGR() { swapARGB( 5, false); }
    void swapToRGBA() { swapARGB( 6, false); }
    void swapToGRBA() { swapARGB( 7, false); }
    void swapToBRGA() { swapARGB( 9, false); }
    void swapToBGRA() { swapARGB(11, false); }

    // little endian�l�ł�ARGB����ɁAargb���ɂȂ�悤�� a,r,g,b�̏��Ԃ�ύX.
    void swapFromAGRB() { swapARGB( 1, true); }
    void swapFromABRG() { swapARGB( 3, true); }
    void swapFromABGR() { swapARGB( 5, true); }
    void swapFromRGBA() { swapARGB( 6, true); }
    void swapFromGRBA() { swapARGB( 7, true); }
    void swapFromBRGA() { swapARGB( 9, true); }
    void swapFromBGRA() { swapARGB(11, true); }

    void swapARGB(unsigned rotNo, bool fromMode);

    void clearImage() { memset(image(), 0, imageBytes()); }
    void clearClut()  { if (clut_ && clutSz_) memset(clut_, 0, sizeof(clut_[0])*clutSz_); }
  #if 0
    /// ���I�Ɋm�ۂ����C���[�W�̃�������Ԃ��A�N���X���̂͏�����.
    unsigned char* removeImage(unsigned** ppClut);
  #endif

private:
    void            zeroclear() { memset(&pix_, 0, (char*)&flags_+sizeof(flags_) - (char*)&pix_); }

    static bool     conv_sameFmt(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src);
    struct          DecreaseColor_Hst;
    static bool     decreaseColor(BppCnvImg_T& dst, const BppCnvImg_T& src, int ox, int oy, int alpNum=-1);
    static unsigned decreaseColor_1(BppCnvImg_T& dst, const BppCnvImg_T& src, int ox, int oy, unsigned clutSize, int idx, int minA, int maxA);

    static unsigned argb(unsigned char a,unsigned char r,unsigned char g,unsigned char b) { return (a << 24) | (r << 16) | (g << 8) | b; }
    static unsigned argb_a(unsigned c) { return (unsigned char)(c >> 24); }
    static unsigned argb_r(unsigned c) { return (unsigned char)(c >> 16); }
    static unsigned argb_g(unsigned c) { return (unsigned char)(c >>  8); }
    static unsigned argb_b(unsigned c) { return (unsigned char)(c); }

    static unsigned revByteU16(unsigned c) { return ((unsigned char)(c >> 8)) | ((unsigned char)c << 8); }
    static unsigned revByteU32(unsigned c) { return ((unsigned char)(c >> 24))| ((c >> 8) & 0xff00)| ((c&0xff00) << 8) | ((unsigned char)c << 24); }

    template<typename T>
    static T        clamp(T v, T mi, T ma) { return (v < mi) ? mi : (ma < v) ? ma : v; }

    template<typename T>
    static unsigned binary_find_tbl_n(T* tbl, unsigned num, const T& key);
    template<typename T>
    static unsigned binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key);

private:    // �����o�[�ϐ�.
    unsigned char*  pix_;           ///< �摜�s�N�Z��.
    unsigned        w_;             ///< ����.
    unsigned        h_;             ///< �c��.
    unsigned*       clut_;          ///< �F�e�[�u��.
    unsigned        clutSz_;        ///< �F�e�[�u���̃T�C�Y.
    unsigned        wb_;            ///< �����o�C�g��.
    unsigned char   bpp_;           ///< �P�s�N�Z��������̃r�b�g��.
    unsigned char   align_;         ///< �����A���C�����g.
    unsigned char   fmt_;           ///< enum Fmt �̒l.
    unsigned char   flags_;         /**< bit0: pix_�̃������̏��L�҂�.
                                     *  bit1:�K�v�Ȃ�dither����.
                                     *  bit2:����bit/byte order��BE
                                     *  bit2:����bit/byte order��BE
                                     */
};



/// �R�s�[�R���X�g���N�^. �C���[�W��w*h>0�Ȃ�΃������m�ۂ��ăR�s�[.
template<class A>
BppCnvImg_T<A>::BppCnvImg_T(const BppCnvImg_T& src)
    : pix_(0), clut_(0)
{
    create(true, src.pix_, src.w_, src.h_, src.fmt_, src.align_, src.clut_, src.clutSz_);
}



/** �쐬.
 */
template<class A>
bool BppCnvImg_T<A>::create(bool alcRq, void* pix, unsigned w, unsigned h, unsigned fmtBpp, unsigned algn, unsigned* clut, unsigned clutSz)
{
    assert(w > 0 && h > 0 && fmtBpp > 0);
    assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);
    release();
    w_      = w;
    h_      = h;
    bpp_    = (unsigned char) fmtToBpp(Fmt(fmtBpp));
    align_  = (unsigned char) algn;
    wb_     = widthToBytes(w, fmtBpp, algn);
    flags_  = alcRq;
    pix_    = (unsigned char*) pix;
    clut_   = clut;
    if ((int)clutSz < 0)
        clutSz = (bpp_ <= 8) ? 1 << bpp_ : 0;
    clutSz_ = clutSz;
    fmt_    = (unsigned char) fmtBpp;
    if (alcRq) {    // �������J���`���̂���ꍇ�́A�������[�m��.
        pix_ = A::allocate(wb_ * h_);
        if (pix_) {
            if (pix == 0)
                std::memset(pix_, 0, wb_ * h_);
            else
                std::memcpy(pix_, pix, wb_ * h_);
        } else {
            clut_ = 0;
            return false;
        }
        if (clutSz > 0) {
            unsigned cn = clutSz;
            if (cn < 256)
                cn = 256;
            clut_ = (unsigned*)A::allocate(sizeof(*clut) * cn);
            if (clut_) {
                setClut(clut, clutSz);
            } else {
                A::deallocate(pix_, wb_ * h_);
                return false;
            }
        }
    }
    return true;
}



template<class A>
bool BppCnvImg_T<A>::create(const BppCnvImg_T<A>& src, unsigned bppFmt, unsigned algn) {
    if (create(src.width(), src.height(), bppFmt, algn)) {
        if (conv(src))
            return true;
        release();
    }
    return false;
}


/** �J��.
 */
template<class A>
void BppCnvImg_T<A>::release() {
    if (flags_ & 1) {   // pix�̃������̊J���ӔC������Ȃ�
        if (pix_)
            A::deallocate(pix_, wb_ * h_ );
        if (clut_)
            A::deallocate((unsigned char*)clut_, sizeof(*clut_) * clutSz_ );
    }
    zeroclear();
    return;
}



#if 0
/** ���I�Ɋm�ۂ����C���[�W�̃�������Ԃ��A�N���X���̂͏�����.
 */
template<class A>
unsigned char* BppCnvImg_T::removeImage(unsigned** ppClut=0) {
    unsigned char* p;
    if (ppClut) {
        *ppClut = clut_;
        if ((flags_&1) == false) {
            p = A::allocator(sizeof(*clut_)*clutSz_);
            if (p)
                memcpy(p, clut_, sizeof(*clut_)*clutSz_);
        }
    }
    p = pix_;
    if ((flags_&1) == false) {
        p = A::allocator(wb_*h_);
        if (p)
            memcpy(p, pix_, wb_*h_);
    }
    pix_ = 0;
    release();
    return p;
}
#endif



template<class A>
void BppCnvImg_T<A>::getClut(unsigned* clut, unsigned clutSize) {
    if (clutSize > clutSz_ || clutSize == 0)
        clutSize = clutSz_;
    memcpy(clut, clut_, clutSize * sizeof(unsigned));
}



template<class A>
void BppCnvImg_T<A>::setClut(const unsigned* clut, unsigned clutSize) {
    if (clutSize > clutSz_ || clutSize == 0)
        clutSize = clutSz_;
    if (clutSize && clut_) {
        for (unsigned i = 0; i < clutSz_; ++i)
            clut_[i] = 0xFF000000;
        if (clut)
            memcpy(clut_, clut, clutSize * sizeof(unsigned));
    }
}





/** fmtBpp�`����w*h�̉摜�Ƃ��āA���g��(ox,oy)�̈ʒu����A�ϊ�����pix��clut�ɐݒ�.
 *  w,h,fmtBpp,algn �� 0 ���ƁA�N���X���g�̒l���̗p.
 *  �Ăь���pix,clut�ɕK�v�T�C�Y�̃�������p�ӂ��Ă��邱��.
 */
template<class A>
bool BppCnvImg_T<A>::getImage(void* pix, unsigned w, unsigned h, unsigned fmtBpp, unsigned algn, unsigned* clut, unsigned clutSize, int ox, int oy) {
    if (w == 0)
        w = w_;
    if (h == 0)
        h = h_;
    if (fmtBpp == 0)
        fmtBpp = fmt_;
    if (algn == 0)
        algn = align_;
    BppCnvImg_T tmp(false, pix, w_, h_, fmtBpp, algn, clut, clutSize);
    tmp.setDstEndian( this->dstEndian() );
    return conv(tmp, *this, ox, oy);
}



/** src�̉摜���Adst�ցAdst�̏��ݒ�ŕϊ����Đݒ�. (ox,oy)��src���̊J�n�_.
 *  - ���F����clut��ւ̕ϊ��͍s��Ȃ�(���s����).
 *  - clut�悩��F���̌���clut��ւ̕ϊ��́A�P���ɏ�ʃr�b�g���̂Ă邾��.
 *    ���F���K�v�Ȃ��convEx��p���邩�A�\�ߌ��F���Ă�������.
 */
template<class A>
bool BppCnvImg_T<A>::conv(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox, int oy)
{
    // �܂���clut�̃R�s�[.
    if ( dst.bpp() <= 8 && src.clut() && src.clutSize() )
        dst.setClut(src.clut(), src.clutSize());

    unsigned  dstFmt = dst.fmt();

    // ����t�H�[�}�b�g�̎��̓A���C�����g�����̂ݎ��s.
    if (src.fmt() == dstFmt && ox == 0 && oy == 0)
        return conv_sameFmt(dst, src);

    int      w = dst.width();
    int      h = dst.height();
    int      x;
    int      y;
    unsigned c;
    if (src.fmt() <= 8) {   // ���͂�clut��ŁA
        if (dstFmt <= 8) {  // �o�͂�clut��̎�.
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    c = src.getPixXY(x+ox,y+oy);
                    dst.setPixXY(x,y,c);
                }
            }
            return true;
        } else {            // clut�悩�瑽�F��ւ̕ϊ���.
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    c = src.getPixXY(x+ox,y+oy);
                    c = src.clut()[c];
                    dst.setPixXY(x,y,c);
                }
            }
            return true;
        }
    } else {
        if (dstFmt > 8) {   // ���F��Ԃ�bpp�ϊ��̎�.
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    c = src.getPixXY(x+ox,y+oy);
                    dst.setPixXY(x,y,c);
                }
            }
            return true;
        } else {            // ���F�悩��clut��ւ̎�.
            // �F����clut�Ɏ��܂�ꍇ�̂ݕϊ�.
            return convToClutImage(dst,src,ox,oy);
        }
    }
}



/// �����t�H�[�}�b�g���m�̕ϊ�(��������).
template<class A>
bool BppCnvImg_T<A>::conv_sameFmt(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src)
{
    const unsigned char*    s = src.image();
    unsigned char*          d = dst.image();
    unsigned srcWb  = src.widthByte();
    unsigned dstWb  = dst.widthByte();
    unsigned l      = (dstWb < srcWb) ? dstWb : srcWb;
    unsigned h      = dst.height();
    if (h > src.height())
        h = src.height();
    for (unsigned y = 0; y < h; ++y) {
        std::memcpy(d, s, l);
        d += dstWb;
        s += srcWb;
    }
    return true;
}



/** x,y�̈ʒu�̃s�N�Z�����擾. (bpp <= 8�̏ꍇ�Acolor index �ԍ�).
 */
template<class A>
unsigned BppCnvImg_T<A>::getPixXY(int x, int y) const
{
    //�� �����ʂ̊֐��ɏ��������Ċ֐��|�C���^�o�R�ŌĂяo���ׂ����낤���A�ʓ|�Ȃ�Ō��.
    const unsigned char*    pix = pix_;
    unsigned    bppFmt  = fmt_;
    unsigned    wb      = wb_;
    unsigned    c;

    if ((unsigned)x >= w_ || (unsigned)y >= h_)
        return 0;       // �蔲����0��Ԃ�.

    if (bppFmt <= 8) {
        if (bppFmt <= 2) {
            if (bppFmt <= 1) {  // bpp=1
                c =  pix[y * wb + (x >> 3)];
                x &= 7;
                if ((flags_ & 4) == 0)  // ���͂�little endian �̎�
                    x ^= 7;             // x = 7 - x;
                c >>= x;
                c &= 1;
                return c;
            } else {            // bpp=2
                c =  pix[y * wb + (x >> 2)];

                x  = (x & 3) << 1;
                if ((flags_ & 4) == 0)  // ���͂�little endian �̎�
                    x ^= 6;             // x = 6 - x;
                c >>= x;
                c &= 3;
                return c;
            }

        } else {
            if (bppFmt <= 4) {  // bpp=4
                c =  pix[y * wb + (x>>1)];
                x = (x&1) << 2;
                if ((flags_ & 4) == 0)  // ���͂�little endian �̎�
                    x ^= 4;             // x = 4 - x;
                c >>= x;
                c &= 15;
                return c;
            } else {            // bpp=8
                return pix[y * wb + x];
            }
        }

    } else {
        unsigned a, r, g, b;

        if (bppFmt <= 16) {         // 16�r�b�g�F.
            c = *(unsigned short*)&pix[y * wb + x*2];
          #if defined BIG_ENDIAN
            if ((flags_ & 4) == 0)  // ���͂�little endian �̎�
                c = revByteU16(c);
          #else
            if (flags_ & 4)         // ���͂�big endian �̎�
                c = revByteU16(c);
          #endif
            if (bppFmt == FMT_ARGB1555) {
                a = (c & 0x8000) ? 0xff : 0;
                r = (c >> 7) & 0xF8; r |= r >> 5;       // 0111110000000000
                g = (c >> 2) & 0xF8; g |= g >> 5;       // 0000001111100000
                b = (c << 3) & 0xF8; b |= b >> 5;       // 0000000000011111
            } else if (bppFmt == FMT_ARGB4444) {
                a = (c >> 12) & 15; a |= a << 4;
                r = (c >>  8) & 15; r |= r << 4;
                g = (c >>  4) & 15; g |= g << 4;
                b = (c      ) & 15; b |= b << 4;
            } else {
                a = 0xff;
                if (bppFmt == FMT_GRB655) {
                    g = (c >> 8) & 0xFC; g |= g >> 6;       // 1111110000000000
                    r = (c >> 2) & 0xF8; r |= r >> 5;       // 0000001111100000
                    b = (c << 3) & 0xF8; b |= b >> 5;       // 0000000000011111
                } else if (bppFmt == FMT_BRG556) {
                    b = (c >> 8) & 0xF8; b |= b >> 5;       // 1111100000000000
                    r = (c >> 3) & 0xF8; r |= r >> 5;       // 0000011111000000
                    g = (c << 2) & 0xFC; g |= g >> 6;       // 0000000000111111
                } else {    // FMT_RGB565
                    r = (c >> 8) & 0xF8; r |= r >> 5;       // 1111100000000000
                    g = (c >> 3) & 0xFC; g |= g >> 6;       // 0000011111100000
                    b = (c << 3) & 0xF8; b |= b >> 5;       // 0000000000011111
                }
            }
            return argb((unsigned char)a, (unsigned char)r,(unsigned char)g,(unsigned char)b);

        } else if (bppFmt <= 24) {  // 24�r�b�g�F.
            if ((flags_ & 4) == 0) {    // ���͂�little endian �̎�
                b = pix[y * wb + x*3 + 0];
                g = pix[y * wb + x*3 + 1];
                r = pix[y * wb + x*3 + 2];
            } else {                    // ���͂�big endian �̎�
                r = pix[y * wb + x*3 + 0];
                g = pix[y * wb + x*3 + 1];
                b = pix[y * wb + x*3 + 2];
            }
            a = 0xff;
            return argb((unsigned char)a, (unsigned char)r,(unsigned char)g,(unsigned char)b);

        } else {                // 32�r�b�g�F.
            c = *(unsigned*)&pix[y * wb + x*4];
          #if defined BIG_ENDIAN
            if ((flags_ & 4) == 0)  // ���͂�little endian �̎�
                c = revByteU32(c);
          #else
            if (flags_ & 4)         // ���͂�big endian �̎�
                c = revByteU32(c);
          #endif
            return c;
        }
    }
}



/// x,y�̈ʒu�̐F���擾.
template<class A> inline
unsigned BppCnvImg_T<A>::getPixArgb(int x, int y) const         ///< (x,y)�̈ʒu�̃s�N�Z��(color index �ԍ�).
{
    unsigned c = getPixXY(x,y);
    if (bpp_ <= 8)
        c = clut_[c];
    return c;
}



/// x,y�̈ʒu�̃s�N�Z�����擾. (bpp <= 8�̏ꍇ�Acolor index �ԍ�).
template<class A>
void     BppCnvImg_T<A>::setPixXY(int x, int y, unsigned c)
{
    //�� �����ʂ̊֐��ɏ��������Ċ֐��|�C���^�o�R�ŌĂяo���ׂ����낤���A�ʓ|�Ȃ�Ō��.
    unsigned        bppFmt = fmt_;
    unsigned        wb     = wb_;
    unsigned char*  d      = &pix_[y * wb];

    if ((unsigned)x >= w_ || (unsigned)y >= h_)
        return;

    if (bppFmt <= 8) {
        if (bppFmt <= 2) {
            if (bppFmt <= 1) {  // bpp = 1
                c  = c != 0;
                d += x >> 3;
                x &= 7;
                if ((flags_ & 8) == 0)  // �o�͂�little endian �̎�
                    x ^= 7;             // x = 7 - x;
                *d &= ~(1 << x);
                *d |= c << x;

            } else {            // bpp = 2
                d += x >> 2;
                x = (x&3) << 1;
                if ((flags_ & 8) == 0)  // �o�͂�little endian �̎�
                    x ^= 6;             // x = 6 - x;
                *d &= ~(3 << x);
                *d |= (c&3) << x;
            }

        } else {                // bpp = 4
            if (bppFmt <= 4) {
                if ((flags_ & 8) == 0) {    // �o�͂�little endian �̎�
                    if ((x & 1) == 0) d[x>>1] =  (unsigned char)(c & 15);
                    else              d[x>>1] |= (unsigned char)(c << 4);
                } else {                    // �o�͂�big endian �̎�
                    if ((x & 1) == 0) d[x>>1] =  (unsigned char)(c << 4);
                    else              d[x>>1] |= (unsigned char)(c & 15);
                }
            } else {            // bpp = 8
                d[x] = (unsigned char)c;
            }
        }

    } else {
        if (bppFmt <= 16) {         // bpp = 16
          #if 1
            static const signed char dit2[2][2] = { {  - 1,     1}, {    2,     0} };
            static const signed char dit3[2][2] = { { 0- 3,  0+ 1}, { 0+ 3,  0- 1} };
            static const signed char dit4[2][2] = { { 0- 7,  0+ 3}, { 0+ 7,  0- 3} };
          #else
            static const signed char dit2[2][2] = { {    0,     2}, {    3,     1} };
            static const signed char dit3[2][2] = { { 3- 3,  3+ 1}, { 3+ 4,  3- 1} };
            static const signed char dit4[2][2] = { { 7- 7,  7+ 3}, { 7+ 8,  7- 3} };
          #endif

            int r = (unsigned char)(c >> 16);
            int g = (unsigned char)(c >>  8);
            int b = (unsigned char)(c >>  0);
            int a = ((unsigned)c >> 24);
            if (bppFmt == FMT_ARGB4444) {
                if (flags_ & 2) {   // dither���[�h�̎�
                    int u = x & 1, v = y & 1;
                    r = clamp(r + dit4[v][u], 0, 255);
                    g = clamp(g + dit4[v][u], 0, 255);
                    b = clamp(b + dit4[v][u], 0, 255);
                }
                a = (a + 15) >> 4; if (a > 15) a = 15;
                c = (a << 12) | ((r >> 4)<<8) | ((g >> 4)<<4) | (b >> 4);
            } else if (bppFmt == FMT_ARGB1555) {
                if (flags_ & 2) {   // dither���[�h�̎�
                    int u = x & 1, v = y & 1;
                    r = clamp(r + dit3[v][u], 0, 255);
                    g = clamp(g + dit3[v][u], 0, 255);
                    b = clamp(b + dit3[v][u], 0, 255);
                }
                a = (a) ? 0x8000 : 0;
                c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
            } else {
                if (flags_ & 2) {   // dither���[�h�̎�
                    int u = x & 1, v = y & 1;
                    r = clamp(r + dit3[v][u], 0, 255);
                    g = clamp(g + dit2[v][u], 0, 255);
                    b = clamp(b + dit3[v][u], 0, 255);
                }
                if (bppFmt == FMT_GRB655) {
                    c = ((g >> 2)<<10) | ((r >> 3)<<5) | (b >> 3);
                } else if (bppFmt == FMT_BRG556) {
                    c = ((b >> 3)<<11) | ((r >> 3)<<6) | (g >> 2);
                } else {    // FMT_RGB565
                    c = ((r >> 3)<<11) | ((g >> 2)<<5) | (b >> 3);
                }
            }
          #if defined BIG_ENDIAN
            if ((flags_ & 8) == 0)  // �o�͂�little endian �̎�
                c = revByteU16(c);
          #else
            if (flags_ & 8)         // �o�͂�big endian �̎�
                c = revByteU16(c);
          #endif
            d += x*2;
            *(unsigned short*)d = (unsigned short) c;

        } else if (bppFmt <= 24) {  // bpp = 24
            d += x*3;
            if ((flags_ & 8) == 0) { // �o�͂�little endian �̎�
                d[0] = (unsigned char)(c >>  0);    // b
                d[1] = (unsigned char)(c >>  8);    // g
                d[2] = (unsigned char)(c >> 16);    // r
            } else {                // �o�͂� big endian �̎�
                d[0] = (unsigned char)(c >> 16);    // r
                d[1] = (unsigned char)(c >>  8);    // g
                d[2] = (unsigned char)(c >>  0);    // b
            }

        } else {                // bpp = 32
          #if defined BIG_ENDIAN
            if ((flags_ & 8) == 0)  // �o�͂�little endian �̎�
                c = revByteU32(c);
          #else
            if (flags_ & 8)         // �o�͂�big endian �̎�
                c = revByteU32(c);
          #endif
            d += x*4;
            *(unsigned*)d = c;
        }
    }
}



// ---------------------------------------------------------------------------

/** ����.
 */
template<class A>
void BppCnvImg_T<A>::swap(BppCnvImg_T<A>& rhs) {
    unsigned char*  p;
    unsigned*       c;
    unsigned        i;
    p = pix_    ,   pix_    = rhs.pix_      ,   rhs.pix_    = p;
    i = w_      ,   w_      = rhs.w_        ,   rhs.w_      = i;
    i = h_      ,   h_      = rhs.h_        ,   rhs.h_      = i;
    c = clut_   ,   clut_   = rhs.clut_     ,   rhs.clut_   = c;
    i = clutSz_ ,   clutSz_ = rhs.clutSz_   ,   rhs.clutSz_ = i;
    i = wb_     ,   wb_     = rhs.wb_       ,   rhs.wb_     = i;
    i = bpp_    ,   bpp_    = rhs.bpp_      ,   rhs.bpp_    = i;
    i = align_  ,   align_   = rhs.align_   ,   rhs.align_  = i;
    i = flags_  ,   flags_  = rhs.flags_    ,   rhs.flags_  = i;
    i = fmt_    ,   fmt_    = rhs.fmt_      ,   rhs.fmt_    = i;
}



/** clut��̂Ƃ��́A1�o�C�g���̋l�ߏ����t�ɂ���.
 */
template<class A>
void BppCnvImg_T<A>::swapBitOrder() {
    unsigned char*  p = pix_;
    unsigned        l = wb_ * h_;
    unsigned        c;

    if (p == 0 || l == 0 || bpp_ >= 8)
        return;

    if (bpp_ <= 2) {
        if (bpp_ <= 1) {
            // bpp == 1
            do {
                c    = *p;
                c    = ((c >> 7) & 1)
                     | ((c >> 5) & 2)
                     | ((c >> 3) & 4)
                     | ((c >> 1) & 1)
                     | ((c & 8) << 1)
                     | ((c & 4) << 5)
                     | ((c & 2) << 3)
                     | ((c & 1) << 7);
                *p++ = c;
            } while (--l);
        } else {
            // bpp == 2
            do {
                c    = *p;
                c    = ((c >> 0x06) &  3) | ((c >> 2) &  0x0C)
                     | ((c &  0x0C) << 2) | ((c &  3) << 0x06);
                *p++ = c;
            } while (--l);
        }
    } else {
        // bpp == 4
        do {
            c    = *p;
            c    = ((c >> 4) & 15) | ((c & 15) << 4);
            *p++ = c;
        } while (--l);
    }
}



/** ���F�̎���1�s�N�Z���̃o�C�g�����t�ɂ���.
 */
template<class A>
void BppCnvImg_T<A>::swapByteOrder() {
    unsigned char*  p = pix_;
    unsigned        l = wb_ * h_;
    unsigned        c;

    if (p == 0 || l == 0 || bpp_ <= 8)
        return;

    if (bpp_ <= 16) {
        unsigned short* pp = (unsigned short*)p;
        l >>= 1;
        do {
            c       = *pp;
            c       = revByteU16(c);
            *pp++   = c;
        } while (--l);
    } else if (bpp_ <= 24) {
        for (unsigned y = 0; y < h_; ++y) {
            p = &pix_[y * wb_];
            for (unsigned x = 0; x < w_; ++x) {
                c       =  p[0];
                p[0]    =  p[2];
                p[2]    =  c;
                p       += 3;
            }
        }
    } else {
        unsigned* pp = (unsigned*)p;
        l >>= 2;
        do {
            c       = *pp;
            c       = revByteU32(c);
            *pp++   = c;
        } while (--l);
    }
}



/** �摜�ɔ�����(�����)���g���Ă���摜��? (�S�ă�=0���S�ă�=0xff�Ȃ疢�g�p����)
 */
template<class A>
bool BppCnvImg_T<A>::isUseAlpha() const
{
    unsigned  w = width();
    unsigned  h = height();
    unsigned  num = w * h;

    if (image() == 0 || num == 0)
        return false;

    if (fmt_ <= 8) {
        unsigned*       clt= clut();
        unsigned*       p  = (unsigned*)image();
        const unsigned* e  = p + num;
        chk = (unsigned char)(clt[ *p++ ] >> 24);
        if (0 < chk && chk < 255)
            return true;
        while (p < e) {
            int a   = (unsigned char)(clt[ *p++ ] >> 24);
            if (a != chk)
                return true;
        }
    } else if (fmt_ == 32) {
        unsigned*       p  = (unsigned*)image();
        const unsigned* e  = p + num;
        chk = (unsigned char)(*p++ >> 24);
        if (0 < chk && chk < 255)
            return true;
        while (p < e) {
            int a   = (unsigned char)(*p++ >> 24);
            if (a != chk)
                return true;
        }
    } else if (fmt_ == FMT_ARGB4444) {
        unsigned*       p  = (unsigned*)image();
        const unsigned* e  = p + num;
        chk = (*p++ >> 12) & 15;
        if (0 < chk && chk < 15)
            return true;
        while (p < e) {
            int a   = (*p++ >> 12) & 15;
            if (a != chk)
                return true;
        }
    } else if (fmt_ == FMT_ARGB1555) {
        unsigned short*         p  = (unsigned*)image();
        const unsigned short*   e  = p + num;
        chk = *p++ >> 15;
        while (p < e) {
            int a = *p++ >> 15;
            if (a != chk)
                return true;
        }
    }
    return false;
}



/** �㉺���]����.
 */
template<class A>
void BppCnvImg_T<A>::revY()
{
    assert(pix_ && wb_ > 0 && h_ );
    unsigned        wb = wb_;
    unsigned char*  f  = pix_;
    unsigned char*  b  = pix_ + wb * (h_ - 1);

    for (unsigned y = 0; y < h_/2; ++y) {
        for (unsigned x = 0; x < wb_; ++x) {
            unsigned char c = f[x];
            f[x]            = b[x];
            b[x]            = c;
        }
        f += wb;
        b -= wb;
    }
}



/** argb�̏��Ԃ����ւ���.
 */
template<class A>
void BppCnvImg_T<A>::swapARGB(unsigned rotNo, bool fromMode)
{
    enum { B=0,G=1,R=2,A=3};
    static const unsigned char tbl[][4] = {
      // b g r a        //      mem     little endian�l(�v���O�������͂�����)
        {B,G,R,A,},     // 0    bgra    argb
        {B,R,G,A,},     // 1    brga    agrb
        {G,B,R,A,},     // 2    gbra    arbg
        {G,R,B,A,},     // 3    grba    abrg
        {R,B,G,A,},     // 4    rbga    agbr
        {R,G,B,A,},     // 5    rgba    abgr
        {A,B,G,R,},     // 6    abgr    rgba
        {A,B,R,G,},     // 7    abrg    grba
        {A,G,B,R,},     // 8    agbr    rbga
        {A,G,R,B,},     // 9    agrb    brga
        {A,R,B,G,},     //10    arbg    gbra
        {A,R,G,B,},     //11    argb    bgra
        // ���̃e�[�u������A,R,G,B�͈Ӗ��Ƃ��Ă͐��l�̂ق����悢��...
    };
    const unsigned char* d  = tbl[rotNo];
    const unsigned char* s  = tbl[0];
    if (fromMode)
        std::swap(d,s);
    unsigned char   *p;
    unsigned        num, w, h;
    unsigned        bc = bpp();

    if (bc <= 8) {
        p       = (unsigned char*)clut();
        num     = clutSize();
        bc      = 32;
    } else {
        p       = (unsigned char*)image();
        w       = width();
        h       = height();
        num     = w * h;
    }

    if (bc == 32) {
        for (unsigned n = 0; n < num; ++n) {
            unsigned char b = p[s[B]];
            unsigned char g = p[s[G]];
            unsigned char r = p[s[R]];
            unsigned char a = p[s[A]];
            p[d[B]] = b;
            p[d[G]] = g;
            p[d[R]] = r;
            p[d[A]] = a;
            p += 4;
        }
    } else {
        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                unsigned        c = getPixXY(x,y);
                unsigned char   p[4], q[4];
                q[0]    = (unsigned char)c;
                q[1]    = (unsigned char)(c >>  8);
                q[2]    = (unsigned char)(c >> 16);
                q[3]    = (unsigned char)(c >> 24);
                p[d[B]] = q[s[B]];
                p[d[G]] = q[s[G]];
                p[d[R]] = q[s[R]];
                p[d[A]] = q[s[A]];
                c = (p[A] << 24) | (p[R] << 16) | (p[G] << 8) | p[B];
                setPixXY(x,y,c);
            }
        }
    }
}



/** ����,bpp,�A���C�����g���牡���o�C�g�������߂�.
 */
template<class A>
unsigned BppCnvImg_T<A>::widthToBytes(unsigned w, unsigned bpp, unsigned algn) {
    unsigned wb = (w * fmtToBpp(Fmt(bpp)) + 7) >> 3;
    wb = (wb + algn-1) & ~(algn-1);
    return wb;
}



/** �t�H�[�}�b�g����bpp�����߂�.
 */
template<class A>
unsigned BppCnvImg_T<A>::fmtToBpp(Fmt fmt) {
    return "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40\40\40\40\40\40\40\40\40\40\40"[fmt-1];
}



// ---------------------------------------------------------------------------

/** ���F�t�H�[�}�b�g����clutSize�ȉ������F�����Ȃ��Ȃ�A�ϊ�����. ����Ȃ�ϊ����Ȃ�.
 */
template<class A>
bool BppCnvImg_T<A>::convToClutImage(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox, int oy)
{
    if (dst.clut() == 0 || dst.clutSize() == 0 || dst.bpp() > 8)
        return false;

    /// ���ۂ̐F�����AclutSize�ȓ��Ɏ��܂邩���`�F�b�N.
    unsigned clut[256+1];       // ��]���Ɋm��.
    unsigned clutSize = dst.clutSize();
    unsigned num = 0;
    if (clutSize > 256)
        clutSize = 256;
    unsigned w = dst.width();
    unsigned h = dst.height();
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            unsigned c = src.getPixArgb(x+ox,y+oy);
            binary_insert_tbl_n(clut, num, c);
            if (num > clutSize) // �F���������Ă���A�ϊ����Ȃ�.
                return false;
        }
    }

    // clut���Ɏ��܂�F���������ꍇ.
    dst.setClut(&clut[0], num);
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            unsigned c = src.getPixArgb(x+ox,y+oy);
            unsigned n = binary_find_tbl_n(&clut[0], num, c);
            dst.setPixXY(x,y,n);
        }
    }
    return true;
}



template<class A>
template<typename T>
unsigned BppCnvImg_T<A>::binary_find_tbl_n(T* tbl, unsigned num, const T& key)
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


/** �e�[�u��pTbl�ɒlkey��ǉ�. �͈̓`�F�b�N�͗\�ߍs���Ă��邱�ƑO��I
 *  @return �e�[�u������key�̈ʒu.
 */
template<class A>
template<typename T>
static unsigned BppCnvImg_T<A>::binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key) {
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
            return mid; /* �������̂��݂������̂Œǉ����Ȃ� */
        }
    }

    // �V�K�o�^
    ++rNum;

    // �o�^�ӏ��̃��������󂯂�
    for (hi = rNum; --hi > mid;) {
        pTbl[hi] = pTbl[hi-1];
    }

    // �o�^
    pTbl[mid] = key;
    return mid;
}




// ---------------------------------------------------------------------------


/** src�̉摜���Adst�ցAdst�̏��ݒ�ŕϊ����Đݒ�. (ox,oy)��src���̊J�n�_.
 *  - ���F����clut���A�F���̏��Ȃ�clut��ւ̕ϊ��ł͌��F�������s��(�Ȉ�).
 */
template<class A>
bool BppCnvImg_T<A>::convEx(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox, int oy)
{
    // �܂���clut�̃R�s�[.
    if ( dst.bpp() <= 8 && src.clut() && src.clutSize() )
        dst.setClut(src.clut(), src.clutSize());

    unsigned  dstFmt = dst.fmt();

    // ����t�H�[�}�b�g�̎��̓A���C�����g�����̂ݎ��s.
    if (src.fmt() == dstFmt && ox == 0 && oy == 0)
        return conv_sameFmt(dst, src);

    int      w = dst.width();
    int      h = dst.height();
    int      x;
    int      y;
    unsigned c;
    if (src.fmt() <= 8) {   // ���͂�clut��ŁA
        if (dstFmt <= 8) {  // �o�͂�clut��̎�.
            if (dstFmt >= src.fmt()) {  // ���F�̕K�v�̂Ȃ���.
                for (y = 0; y < h; ++y) {
                    for (x = 0; x < w; ++x) {
                        c = src.getPixXY(x+ox,y+oy);
                        dst.setPixXY(x,y,c);
                    }
                }
                return true;
            } else {        // ���F�̕K�v������Ƃ�.
                return decreaseColor(dst,src,ox,oy);
            }
        } else {            // clut�悩�瑽�F��ւ̕ϊ���.
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    c = src.getPixXY(x+ox,y+oy);
                    c = src.clut()[c];
                    dst.setPixXY(x,y,c);
                }
            }
            return true;
        }
    } else {
        if (dstFmt > 8) {   // ���F��Ԃ�bpp�ϊ��̎�.
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    c = src.getPixXY(x+ox,y+oy);
                    dst.setPixXY(x,y,c);
                }
            }
            return true;
        } else {            // ���F�悩��clut��ւ̎�.
            // �F����clut�Ɏ��܂�ꍇ�̂ݕϊ�.

            return decreaseColor(dst,src,ox,oy);
        }
    }
}



/// 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A>
bool BppCnvImg_T<A>::decreaseColor(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox, int oy, int alpNum)
{
    if (convToClutImage(dst,src,ox,oy))
        return true;

    unsigned w = dst.width();
    unsigned h = dst.height();

    // �s�N�Z����clut������. (�����摜��Ԃɂ���)
    dst.clearImage();
    dst.clearClut();

    // ����,������,�s�����̑��݃`�F�b�N
    enum { ALP_D = 4 };
    enum { MIN_A = ALP_D, MAX_A = 255-ALP_D };
    bool tranf = false;
    bool alpf  = false;
    bool nalpf = false;
    if (alpNum != 0) {
        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; ++x) {
                unsigned c = src.getPixArgb(x,y);
                unsigned a = argb_a( c );
                if (a < MIN_A) {
                    tranf = 1;
                } else if ( a <= MAX_A ) {
                    alpf  = 1;
                } else {
                    nalpf = 1;
                }
            }
        }
    }

    unsigned clutSize = dst.clutSize();
    if (nalpf == 0) {
        // �s�������Ȃ��ꍇ�́A���ׂẴp���b�g�𔼓�������.
        return decreaseColor_1(dst,src,ox,oy, clutSize, tranf, MIN_A, 255) != 0;
    } else if (alpf == false || clutSize < 32) {
        // �s�����Ɣ����F�݂̂̂Ƃ�
        return decreaseColor_1(dst,src,ox,oy, clutSize, tranf, MAX_A+1, 255) != 0;
    } else {
        // �s�����Ɣ�����������Ƃ� (�o�X�g�A�b�v��/����������z��)
        unsigned idx = (unsigned)alpNum;
        if (alpNum < 0) {       // �K���ɔ������������߂�w�肾����.(256�F����32�F).
            idx = clutSize / 8;
            if (idx < 16)
                idx = 16;
        }
        idx  = decreaseColor_1(dst,src, ox,oy, idx     , tranf, MIN_A, MAX_A);
        return decreaseColor_1(dst,src, ox,oy, clutSize, idx, MAX_A+1, 255) != 0;
    }
}




/// ���F���̕p�x�J�E���g�̂��߂̍\����.
template<class A>
struct BppCnvImg_T<A>::DecreaseColor_Hst {
  #if defined _WIN32                // vc��stdint.h���Ȃ��̂ł��̑΍�.
    typedef unsigned __int64 sum_t; // win�n�R���p�C����vc�݊��ŌÂ�����__int64������.
  #else
    typedef uint64_t         sum_t;
  #endif
    sum_t           g;
    sum_t           r;
    sum_t           b;
    sum_t           a;
    std::size_t     num;
    short           no;

    bool operator<(const DecreaseColor_Hst& r) const { return this->num > r.num; }
};




/// 32�r�b�g�F���8�r�b�g�F��ɕϊ�.
template<class A>
unsigned BppCnvImg_T<A>::decreaseColor_1(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox, int oy, unsigned clutSize, int idx, int minA, int maxA)
{
    assert(clutSize <= 256 && clutSize >= idx+1);
    enum { ALP_D = 4 };
    unsigned    bi = 4;
    if (maxA == 255)    // ���łȂ��ʏ�̌��F�ŐF�������Ȃ���.
        bi = (clutSize <= 8) ? 1 : (clutSize <= 12) ? 2 : (clutSize <= 48) ? 3 : 4;
  RETRY:
    unsigned    l=1<<bi, sh=8-bi;
    unsigned    hstNum = 1 + l*l*l;
    if (hstNum < 257)
        hstNum = 257;
    DecreaseColor_Hst* pHst = (DecreaseColor_Hst*)A::allocate( sizeof(DecreaseColor_Hst) *  hstNum );
    std::memset(pHst, 0, sizeof(DecreaseColor_Hst) * hstNum);
    unsigned w = dst.width();
    unsigned h = dst.height();

    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            unsigned c = src.getPixArgb(x,y);
            int      a = argb_a(c);
            a          = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
            unsigned g = argb_g(c);
            unsigned r = argb_r(c);
            unsigned b = argb_b(c);
            if (minA <= a && a <= maxA) {
                unsigned no  = 1 + (g>>sh) * l*l + (r>>sh) * l + (b>>sh);
                DecreaseColor_Hst* h = &pHst[no];
                ++h->num;
                h->no = no;
                h->a += a;
                h->g += g;
                h->r += r;
                h->b += b;
            }
        }
    }

    unsigned colNum = clutSize - idx;
    // �������m����clutSize��I�o(�擪�Ɉړ�).
    std::nth_element( &pHst[0], &pHst[colNum], &pHst[hstNum], std::less<DecreaseColor_Hst>() );

    unsigned* pClut = dst.clut();
    std::memset(pClut+idx, 0, colNum*sizeof(pClut[0]));

    // �������̂���F���擾.
    unsigned n = 0;
    for (unsigned j = 0; j < colNum; ++j) {
        const DecreaseColor_Hst*    h = &pHst[j];
        std::size_t num = h->num;
        if (num == 0)
            continue;   // break;
        unsigned a    = unsigned(h->a / num);
        unsigned g    = unsigned(h->g / num);
        unsigned r    = unsigned(h->r / num);
        unsigned b    = unsigned(h->b / num);
        a   = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
        pClut[n+idx] = argb(a,r,g,b);
        ++n;
    }

    if (clutSize >= 128 && n <= clutSize-32 && (bi >= 4 && bi < 7) ) {
        // clut���������܂��Ă���F�[�x�𑝂₷(�����炭�P�F��c�[�g���J���[�n�̉摜���낤)
        ++bi;
        goto RETRY;
    }

    // ���ۂɎg�p����Ă���clut�������߂�.
    if (n < 2)  // �������Œ�2�F�͎g�����Ƃ�.
        n = 2;
    clutSize = n + idx;

    // �Â����̏��ɕ��ג���.
    std::sort( &pClut[idx], &pClut[clutSize] );

    // �����clut�ŁA���ׂẴs�N�Z����clutSize�F��.
    // �߂��F�̏����Ƃ��āA
    //   - a,r,g,b�̒l���߂����m(�F���m�̍������������m)
    //   - 1�F����r,g,b�̋P�x�̑召�֌W���A�߂����m
    // ���`�F�b�N.
    std::memset(pHst, 0, sizeof(DecreaseColor_Hst) * clutSize);
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            unsigned ii = 0;
            unsigned    c   = src.getPixArgb(x,y);
            int         a   = argb_a(c);
            int         g   = argb_g(c);
            int         r   = argb_r(c);
            int         b   = argb_b(c);
            int         o   = g - r;
            int         p   = r - b;
            int         q   = b - g;

            a = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
            if (minA <= a && a <= maxA) {
                unsigned k = 0xFFFFFFFF;
                for (unsigned i = idx; i < clutSize; ++i) {
                    unsigned ic = pClut[i];
                    int      ia = argb_a(ic);
                    int      ig = argb_g(ic);
                    int      ir = argb_r(ic);
                    int      ib = argb_b(ic);
                    int      io = ig - ir;
                    int      ip = ir - ib;
                    int      iq = ib - ig;

                    int      aa = ia - a;
                    int      gg = ig - g;
                    int      rr = ir - r;
                    int      bb = ib - b;
                    int      oo = io - o;
                    int      pp = ip - p;
                    int      qq = iq - q;
                    unsigned ik = 3*aa*aa + gg*gg + rr*rr + bb*bb + oo*oo + pp*pp + qq*qq;
                    if (ik < k) {
                        k  = ik;
                        ii = i;
                    }
                }
                dst.setPixXY(x,y,ii);

                // �g�p����F�l�̍��v������.
                DecreaseColor_Hst* t = &pHst[ ii ];
                ++t->num;
                t->a += a;
                t->g += g;
                t->r += r;
                t->b += b;
            }
        }
    }

    // clut�̊e�F�����ۂ̃s�N�Z���̐F�̕��ςɂ���.
    for (unsigned i = idx; i < clutSize; ++i) {
        const DecreaseColor_Hst*    t    = &pHst[i];
        unsigned    n    = t->num;
        unsigned    g=0, r=0, b=0, a=0;
        if (n) {
            a = unsigned(t->a / n);
            a = (a < ALP_D) ? 0 : (a > 255-ALP_D) ? 255 : a;
            g = unsigned(t->g / n);
            r = unsigned(t->r / n);
            b = unsigned(t->b / n);
        }
        pClut[i] = argb(a,r,g,b);
    }

    A::deallocate((unsigned char*)pHst, hstNum);
    return clutSize;
}




// ---------------------------------------------------------------------------


template<class A>
bool convGRB332(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox=0, int oy=0)
{
    if (dst.bpp() != 8 || dst.clut() == 0 || dst.clutSize() < 256)
        return false;

    unsigned    c;
    int         r, g, b;
    for (c = 0; c < 256; ++c) {
        r = ((c & 0x1c) << 3);
        r = r | (r >> 3) | (r >> 6);
        g = c & 0xE0;
        g = g | (g >> 3) | (g >> 6);
        b = c & 3;
        b = (b << 6) | (b << 4) | (b << 2) | b;
        dst.clut()[c] = (0xff << 24) | (r << 16) | (g << 8) | b;
    }

    bool dither = dst.ditherMode(); // dither���[�h�̎�
    for (unsigned y = 0; y < dst.height(); ++y) {
        for (unsigned x = 0; x < dst.width(); ++x) {
            c = src.getPixArgb(x+ox,y+oy);
            r = (unsigned char)(c >> 16);
            g = (unsigned char)(c >>  8);
            b = (unsigned char)c;
            if (dither) {
              #if 1
                static const signed char dit5[2][2] = { {  -15,   + 7}, {  +16,   - 7} };
                static const signed char dit6[2][2] = { {  -31,   +15}, {  +32,   -15} };
              #else
                static const signed char dit5[2][2] = { {15-15, 15+ 7}, {15+16, 15- 7} };
                static const signed char dit6[2][2] = { {31-31, 31+15}, {31+32, 31-15} };
              #endif
                int u = x & 1, v = y & 1;
                r += dit5[v][u]; if (r < 0) r = 0; else if (r > 255) r = 255;
                g += dit5[v][u]; if (g < 0) g = 0; else if (g > 255) g = 255;
                b += dit6[v][u]; if (b < 0) b = 0; else if (b > 255) b = 255;
            }
            c = (g & 0xE0) | ((r >> 3) & 0x1C) | ((b>>6) & 3);
            dst.setPixXY(x, y, c);
        }
    }
    return true;
}



template<class A>
bool convGRB111(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox=0, int oy=0)
{
    if ((dst.bpp() > 8) || dst.clut() == 0 || dst.clutSize() < 8 || dst.bpp() < 3)
        return false;

    for (unsigned i = 0; i < dst.clutSize(); ++i) {
        static const unsigned clutG1R1B1[8] = {
            0xFF000000,0xFF0000FF,0xFFFF0000,0xFFFF00FF,
            0xFF00FF00,0xFF00FFFF,0xFFFFFF00,0xFFFFFFFF,
        };
        dst.clut()[i] = clutG1R1B1[i&7];
    }

    bool dither = dst.ditherMode(); // dither���[�h�̎�
    for (unsigned y = 0; y < dst.height(); ++y) {
        for (unsigned x = 0; x < dst.width(); ++x) {
            unsigned c = src.getPixArgb(x+ox,y+oy);
            int      r = (unsigned char)(c >> 16);
            int      g = (unsigned char)(c >>  8);
            int      b = (unsigned char)c;
            if (dither) {
                // �K���Ȑݒ�.
                static const signed char dit7[2][2] = { {  -96,   +48}, {  +96,   - 48} };
                int u = x & 1, v = y & 1;
                r = (r + dit7[v][u]) >= 128;
                g = (g + dit7[v][u]) >= 128;
                b = (b + dit7[v][u]) >= 128;
                c = (g << 2) | (r << 1) | b;
            } else {
                c = ((g >> 5) & 4) | ((r >> 6) & 2) | ((b>>7) & 1);
            }
            dst.setPixXY(x, y, c);
        }
    }
    return true;
}



template<class A>
bool convGray(BppCnvImg_T<A>& dst, const BppCnvImg_T<A>& src, int ox=0, int oy=0)
{
    if (dst.bpp() > 8) {
        for (unsigned y = 0; y < dst.height(); ++y) {
            for (unsigned x = 0; x < dst.width(); ++x) {
                unsigned c = src.getPixArgb(x+ox,y+oy);
                unsigned a = c & 0xFF000000;
                unsigned char r = c >> 16;
                unsigned char g = c >>  8;
                unsigned char b = c;
                c = ( 5866*g +  2989*r + 1145*b) / 10000;
                c = a | (c << 16) | (c << 8) | c;
                dst.setPixXY(x, y, c);
            }
        }
        return true;

    } else {
        unsigned clutSize = dst.clutSize();
        if (clutSize < 2)
            return false;
        if (clutSize > unsigned(1 << dst.bpp()))
            clutSize = 1 << dst.bpp();
        int m = clutSize - 1;
        for (unsigned i = 0; i < clutSize; ++i) {
            unsigned c = 255 * i / m;
            dst.clut()[i] = (0xff << 24) | (c << 16) | (c << 8) | c;
        }
        int di   = dst.ditherMode();
        int geta = 255 / ( m * 2);
        int dit[2][2][2] = {
            { {   geta,  geta }, {   geta,  geta} },
            { {-2*geta, +geta }, { 2*geta, -geta} },
        };
        for (unsigned y = 0; y < dst.height(); ++y) {
            for (unsigned x = 0; x < dst.width(); ++x) {
                int           c = (int)src.getPixArgb(x+ox,y+oy);
                unsigned char r = c >> 16;
                unsigned char g = c >>  8;
                unsigned char b = c;
                c = ( 5866*g +  2989*r + 1145*b + dit[di][y&1][x&1]*10000) * m / (255*10000);
                if (c > 255) c = 255;
                dst.setPixXY(x, y, c);
            }
        }
    }
    return true;
}



// ---------------------------------------------------------------------------

typedef BppCnvImg_T<>   BppCnvImg;





#endif  // BPPCNVIMG_H
