/**
 *  @file   HuffmanEncode.hpp
 *  @brief  huffman encode
 *  @author Masahi Kitamura  (Original program by Haruhiko Okumura "CåæåÍÇ…ÇÊÇÈç≈êVÉAÉãÉSÉäÉYÉÄéñìT")
 */

#ifndef FKS_HUFFMANDECODE_HPP
#define FKS_HUFFMANDECODE_HPP

#include <cstddef>
#include <cstring>
#include <cstdint>


namespace fks {


#ifdef FKS_PEEK64LE
#define FKS_L_PEEK64LE(a)       FKS_PEEK64LE(a)
#else
#define FKS_L_PEEK64LE(a)       (*(std::uint64_t*)(a))
#endif

#if __cplusplus >= 201103L || defined(static_assert)
 #define FKS_L_STATIC_ASSERT(c, m)  static_assert(c, #m)
#else
 #define FKS_L_STATIC_ASSERT(c, m)  typedef char Static_Assert_##m[(c) ? 1/*OK*/ : -1/*NG*/]
#endif


namespace detail_of_HuffmanDecode {
    template<class U> struct idx_t_sel {};
    template<> struct idx_t_sel<std::uint8_t>  { typedef std::int16_t type; };
    template<> struct idx_t_sel<std::uint16_t> { typedef std::int32_t type; };
}



/// Huffman Decoder
template<class T=std::uint8_t, unsigned B=0>
class HuffmanDecode {
public:
    enum {      value_bits = (B > 0) ? B : sizeof(T)*8 };
    typedef T   value_type;

    HuffmanDecode();

    void        reset();
    std::size_t decode(T* dst, std::size_t dstSize, std::uint8_t const* src, std::size_t srcSize);

    bool        decodeSet(std::uint8_t const* src, std::size_t srcSize);
    std::size_t decodeGet(T* dst, std::size_t dstSize);

    bool        isCompress() const { return comp_; }
    std::size_t decodeSize() const { return decSize_; }
    value_type  decodeGet1();
    value_type  decodeGet1Comp();
    value_type  decodeGet1Nocomp() { return (srcp_ < srcEnd_) ? *srcp_++ : 0; }

    static std::size_t  decodeSize(std::uint8_t const* src, std::size_t srcSize) {
        return (src && srcSize > 8) ? std::size_t(FKS_L_PEEK64LE(src) & ~(std::uint64_t(1)<<63)) : 0;
    }

    enum Error { ERROR_NONE, ERROR_INVALID, ERROR_DIST_SIZE_IS_SMALL, ERROR_BROKEN_DATA, };
    Error       errorNo() const { return errorNo_; }

    template<class VEC>
    bool        decode(VEC& dstVec, std::uint8_t const* src, std::size_t srcSize) {
        typedef typename VEC::value_type vec_value_type;
        FKS_L_STATIC_ASSERT((sizeof(vec_value_type) == sizeof(value_type) && vec_value_type(-1) > 0), VEC_value_type_is_not_T);
        dstVec.clear();
        std::size_t decSize = decodeSize(src, srcSize);
        dstVec.resize(decSize);
        T  tmp[1] = {0};
        T* dst = dstVec.empty() ? tmp : reinterpret_cast<T*>(&dstVec[0]);
        decSize = decode(dst, decSize, src, srcSize);
        if (errorNo_) {
            dstVec.clear(); //VEC().swap(encVec);
            return false;
        }
        dstVec.resize(decSize);
        return true;
    }

private:
    unsigned    getBit();
    unsigned    getbits(int n);
    int         reedTree();

private:
    typedef typename detail_of_HuffmanDecode::idx_t_sel<value_type>::type idx_t;

    enum {
        VAL_MAX         = 1 << value_bits,
        NODE_SIZE       = VAL_MAX * 2 - 1
    };

private:
    std::uint8_t const* srcp_;
    std::uint8_t const* srcEnd_;
    std::uint8_t const* src_;
    std::size_t         decSize_;
    unsigned            bitBuf_;
    int                 bitBufCount_;
    int                 avail_;
    int                 root_;
    Error               errorNo_;
    bool                comp_;
    idx_t               left_[NODE_SIZE];
    idx_t               right_[NODE_SIZE];
};


template<class T, unsigned B>
HuffmanDecode<T,B>::HuffmanDecode() {
    FKS_L_STATIC_ASSERT((sizeof(value_type) <= 2 && value_type(-1) > 0), T_is_neither_uint8_t_nor_uint16_t);
    FKS_L_STATIC_ASSERT((0 < value_bits && value_bits <= sizeof(T) * 8), B_out_of_range);
    reset();
}

template<class T, unsigned B>
void HuffmanDecode<T,B>::reset() {
    std::memset(this, 0, sizeof(*this));
}


/** decode
 */
template<class T, unsigned B>
std::size_t HuffmanDecode<T,B>::decode(T* dst, std::size_t dstSize, std::uint8_t const* src, std::size_t srcSize) {
    if (decodeSet(src, srcSize))
        return decodeGet(dst, dstSize);
    return false;
}


/** read 1 bit
 */
template<class T, unsigned B>
inline unsigned HuffmanDecode<T,B>::getBit() {
    if (--bitBufCount_ >= 0)
        return (bitBuf_ >> bitBufCount_) & 1U;
    bitBufCount_ = 7;
    if (srcp_ >= srcEnd_) {
        errorNo_ = ERROR_BROKEN_DATA;
        return 0;
    }
    bitBuf_   = *srcp_++;
    return (bitBuf_ >> 7) & 1U;
}

/** read n bits
 */
template<class T, unsigned B>
unsigned HuffmanDecode<T,B>::getbits(int n) {
    #define FKS_TMP_rightBits(n, x)     ((x) & ((1U << (n)) - 1U))
    unsigned x = 0;
    while (n > bitBufCount_) {
        n        -= bitBufCount_;
        x        |= FKS_TMP_rightBits(bitBufCount_, bitBuf_) << n;
        if (srcp_ >= srcEnd_) {
            errorNo_ = ERROR_BROKEN_DATA;
            return 0;
        }
        bitBuf_      = *srcp_++;
        bitBufCount_ = 8;
    }
    bitBufCount_ -= n;
    return x | FKS_TMP_rightBits(n, bitBuf_ >> bitBufCount_);
    #undef FKS_TMP_rightBits
}

/** read tree
 */
template<class T, unsigned B>
int HuffmanDecode<T,B>::reedTree() {
    if (errorNo_)
        return -1;
    if ( getBit() ) {   // node
        int     i;
        if ( (i = avail_++) >= NODE_SIZE ) {
            errorNo_ = ERROR_BROKEN_DATA;
            return -1;
        }
        left_ [i]   = reedTree();
        right_[i]   = reedTree();
        return i;
    } else {            // leaf
        return int( getbits(value_bits) );      // get value
    }
}

/**
 */
template<class T, unsigned B>
bool HuffmanDecode<T,B>::decodeSet(std::uint8_t const* src, std::size_t srcSize) {
    if (src == NULL || srcSize < 8) {
        errorNo_ = ERROR_INVALID;
        return false;
    }

    // get decode size
    enum { HDRSZ            = 8/*sizeof(std::uint64_t)*/ };
    std::uint64_t info      = FKS_L_PEEK64LE(src);
    std::uint64_t decBytes  = info & ~(std::uint64_t(1) << 63);
    decSize_                = std::size_t(decBytes / sizeof(T));

    if (decBytes % sizeof(T)) { // uint16_t though the size is odd
        errorNo_ = ERROR_BROKEN_DATA;
        return false;
    }

    if (decBytes == 0) {
        return false;
    }

    src_    = src;
    srcp_   = src + HDRSZ;
    srcEnd_ = src + srcSize;
    comp_   = ((info >> 63) & 1) != 0;
    if (comp_ == false) {  // non-compress?
        if (decBytes > srcSize - HDRSZ) {
            errorNo_ = ERROR_BROKEN_DATA;
            return false;
        }
        return true;
    }

    avail_   = VAL_MAX;
    root_    = reedTree();
    if (errorNo_)
        return false;
    return true;
}

/**
 */
template<class T, unsigned B>
std::size_t HuffmanDecode<T,B>::decodeGet(T* dst, std::size_t dstSize) {
    if (dstSize < decSize_) {
        errorNo_ = ERROR_DIST_SIZE_IS_SMALL;
        return 0;
    }
    if (dst == NULL || dstSize == 0) {
        errorNo_ = ERROR_INVALID;
        return 0;
    }

    if (comp_ == false) {
        std::memcpy(dst, srcp_, std::size_t(decSize_) * sizeof(T));
    } else {
        idx_t*      right   = &right_[0];
        idx_t*      left    = &left_[0];
        std::size_t decSize = decSize_;
        int         root    = root_;
        for (std::size_t k = 0; k < decSize; ++k) {
            int j = root;
            while (j >= VAL_MAX) {
                if ( getBit() )
                    j = right[j];
                else
                    j = left[j];
            }
            dst[k] = j;
        }
    }
    return decSize_;
}

/**
 */
template<class T, unsigned B>
T HuffmanDecode<T,B>::decodeGet1() {
    if (comp_) {
        int     j = root_;
        while (j >= VAL_MAX) {
            if ( getBit() )
                j = right_[j];
            else
                j = left_[j];
        }
        return j;
    } else {
        if (srcp_ < srcEnd_)
            return *srcp_++;
        return 0;
    }
}

/**
 */
template<class T, unsigned B>
inline T HuffmanDecode<T,B>::decodeGet1Comp() {
    int     j = root_;
    while (j >= VAL_MAX) {
        if ( getBit() )
            j = right_[j];
        else
            j = left_[j];
    }
    return j;
}



#undef FKS_L_PEEK64LE
#undef FKS_L_STATIC_ASSERT

}   // namespace fks


#endif  // FKS_HUFFMANDECODE_HPP
