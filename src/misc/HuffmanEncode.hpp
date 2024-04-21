/**
 *  @file   HuffmanEncode.hpp
 *  @brief  huffman encode
 *  @author Masahi Kitamura  (Original program by Haruhiko Okumura "C言語による最新アルゴリズム事典")
 */

#ifndef FKS_HUFFMANENCODE_HPP
#define FKS_HUFFMANENCODE_HPP

#include <cstddef>
#include <cstring>
#include <cstdint>


namespace fks {


#ifdef FKS_POKE64LE
#define FKS_L_POKE64LE(a,v)     FKS_POKE64LE(a,v)
#else
#define FKS_L_POKE64LE(a,v)     (*(std::uint64_t*)(a) = (v))
#endif

#if __cplusplus >= 201103L || defined(static_assert)
 #define FKS_L_STATIC_ASSERT(c, m)  static_assert(c, #m)
#else
 #define FKS_L_STATIC_ASSERT(c, m)  typedef char Static_Assert_##m[(c) ? 1/*OK*/ : -1/*NG*/]
#endif


namespace detail_of_HuffmanEncode {
    template<class U> struct idx_t_sel {};
    template<> struct idx_t_sel<std::uint8_t>  { typedef std::int16_t type; };
    template<> struct idx_t_sel<std::uint16_t> { typedef std::int32_t type; };
}



/// Huffman Encoder
template<class T=std::uint8_t, unsigned B=0>
class HuffmanEncode {
public:
    enum {      value_bits = (B > 0) ? B : sizeof(T)*8 };
    typedef T   value_type;

    HuffmanEncode();

    void        reset();
    std::size_t encode(std::uint8_t* dst, std::size_t dstSize, T const* src, std::size_t srcSize);
    bool        isCompressed() const { return compressed_; }
    std::size_t tableSize() const { return tableSize_; }

    enum Error { ERROR_NONE, ERROR_INVALID, ERROR_DIST_SIZE_IS_SMALL };
    Error       errorNo() const { return errorNo_; }

    template<class VEC>
    bool encode(VEC& encVec, T const* src, std::size_t srcSize) {
        typedef typename VEC::value_type vec_value_type;
        FKS_L_STATIC_ASSERT((sizeof(vec_value_type) == 1 && vec_value_type(127) > 0), VEC_value_t_is_not_uint8_t);
        std::size_t srcBytes = srcSize * sizeof(T);
        encVec.clear();
        encVec.resize(srcBytes + 16);
        std::size_t dstBytes = encode(static_cast<std::uint8_t*>(&encVec[0]), encVec.size(), src, srcSize);
        if (errorNo_) {
            encVec.clear(); //VEC().swap(encVec);
            return false;
        }
        encVec.resize(dstBytes);
        return true;
    }

private:
    void downHeep(std::size_t i);
    void writeTree(unsigned i);
    void putBit(unsigned bit);
    void putbits(int n, unsigned x);
    void putBitEnd();

private:
    typedef typename detail_of_HuffmanEncode::idx_t_sel<T>::type   idx_t;

    enum {
        BITBUF_BITS     = 8,
        VAL_MAX         = 1 << value_bits,  ///< 文字の種類.
        VAL_MASK        = VAL_MAX - 1,
        NODE_SIZE       = VAL_MAX * 2 - 1
    };

private:
    // 出力バッファ.
    std::uint8_t*   dstp_;
    std::uint8_t*   dstEnd_;
    std::size_t     bitBuf_;    ///< ビット入出力バッファ.
    int             putCount_;  ///< ビット入出力カウンタ.
    bool            compressed_;
    bool            isOver_;
    Error           errorNo_;

    // 優先待ち行列用ヒープ.
    int             heapSize_;
    idx_t           heap_[NODE_SIZE];

    // Huffman木.
    idx_t           parent_[NODE_SIZE];
    idx_t           left_[NODE_SIZE];
    idx_t           right_[NODE_SIZE];
    std::size_t     freq_[NODE_SIZE];       ///< 各文字の出現頻度.
    bool            codeBit_[VAL_MAX];      ///< 符号語.
    std::size_t     tableSize_;
};


template<class T, unsigned B>
HuffmanEncode<T,B>::HuffmanEncode() {
    FKS_L_STATIC_ASSERT((sizeof(T) <= 2 && T(-1) > 0), T_is_neither_uint8_t_nor_uint16_t);
    FKS_L_STATIC_ASSERT((0 < value_bits && value_bits <= sizeof(T) * 8), B_out_of_range);
    reset();
}

template<class T, unsigned B>
inline void HuffmanEncode<T,B>::reset() {
    std::memset(this, 0, sizeof(*this));
}

/** write 1 bit
 */
template<class T, unsigned B>
inline void HuffmanEncode<T,B>::putBit(unsigned bit) {
    --putCount_;
    if (bit)
        bitBuf_ |= (1 << putCount_);
    if (!putCount_) {
        if (dstp_ >= dstEnd_) {
            isOver_ = true;
            return;
        }
        *dstp_++  = bitBuf_;
        bitBuf_   = 0;
        putCount_ = BITBUF_BITS;
    }
}

/** write n bits
 */
template<class T, unsigned B>
inline void HuffmanEncode<T,B>::putbits(int n, unsigned x) {
    #define FKS_TMP_rightBits(n, x)   ((x) & ((std::size_t(1) << (n)) - 1U))  /* xの右nビット */
    while (n >= putCount_) {
        if (dstp_ >= dstEnd_) {
            isOver_ = true;
            return;
        }
        n -= putCount_;
        unsigned xn = x >> n;
        bitBuf_  |= FKS_TMP_rightBits(putCount_, xn);
        *dstp_++  = bitBuf_;
        putCount_ = BITBUF_BITS;
        bitBuf_   = 0;
    }
    putCount_ -= n;
    bitBuf_ |= FKS_TMP_rightBits(n, x) << putCount_;
    #undef FKS_TMP_rightBits
}

/** バッファの残りをフラッシュ.
 */
template<class T, unsigned B>
void HuffmanEncode<T,B>::putBitEnd() {
    putbits(BITBUF_BITS-1, 0);
}


/** encode
 */
template<class T, unsigned B>
std::size_t HuffmanEncode<T,B>::encode(std::uint8_t* dst, std::size_t dstSize, T const* src, std::size_t srcSize) {
    if (dst == NULL || dstSize < 8) {
        errorNo_ = ERROR_INVALID;
        return 0;
    }

    // 出力バッファ初期化.
    std::size_t srcBytes = srcSize * sizeof(T);

    // 出力バッファ初期化.
    enum { HDRSZ = 8/*sizeof(std::uint64_t)*/ };
    dstp_     = dst;
    dstEnd_   = dst + dstSize;
    dstp_     += HDRSZ;    //for size info
    putCount_ = BITBUF_BITS;
    bitBuf_   = 0;

    if (src == NULL || srcSize == 0) {       // ソースがないか0なら終了.
        FKS_L_POKE64LE(dstp_, 0);
        return HDRSZ;
    }

    // 頻度の初期化.
    std::size_t*    freq = freq_;
    for (unsigned i  = 0; i < VAL_MAX; ++i)
        freq[i] = 0;

    for (std::size_t l = 0; l < srcSize; ++l) {
        ++freq[src[l] & VAL_MASK];
    }

    idx_t* heap = heap_;
    heap[1]     = 0;                            // 長さ0のファイルに備える.
    heapSize_   = 0;

    // 優先待ち行列に登録.
    for (unsigned i = 0; i < VAL_MAX; ++i) {
        if (freq[i] != 0)
            heap[++heapSize_] = i;
    }

    for (std::size_t i = heapSize_ / 2; i >= 1; --i)
        downHeep(i);                        // ヒープ作り.

    idx_t*  parent  = parent_;
    for (unsigned i = 0; i < NODE_SIZE; ++i)
        parent[i]   = 0;                    // 念のため.

    int k           = heap[1];              // 以下のループが1回も実行されない場合に備える.
    int avail       = VAL_MAX;              // 以下のループでハフマン木を作る.

    while (heapSize_ > 1) {                 // 2個以上残りがある間.
        int     i   = heap[1];              // 最小の要素を取り出す.
        heap[1]     = heap[heapSize_--];
        downHeep(1);                        // ヒープ再構成.
        int     j   = heap[1];              // 次に最小の要素を取り出す.
        k           = ++avail;              // 新しい節を生成する.
        freq[k]    = freq[i] + freq[j];     // 頻度を合計.
        heap[1]     = k;
        downHeep(1);                        // 待ち行列に登録.
        parent[i]   = k;
        parent[j]   = -k;                   // 木を作る.
        left_[k]    = i;
        right_[k]   = j;                    // 〃.
    }

    writeTree(k);                           // 木を出力.

    tableSize_  = dstp_ - dst;              // 表の大きさ.

    if (tableSize_ < srcBytes) {
        bool*   codeBit = codeBit_;
        for (std::size_t l = 0; l < srcSize; ++l) {
            int j = src[l] & VAL_MASK;
            int k = 0;
            while ( (j = parent[j]) != 0 ) {
                if (j > 0)
                    codeBit[k++] = 0;
                else {
                    codeBit[k++] = 1;
                    j = -j;
                }
            }
            while (--k >= 0)
                putBit(codeBit[k]);
        }
        putBitEnd();
    }

    std::size_t bytes = dstp_ - dst;
    if (!isOver_ && HDRSZ < srcBytes && bytes < srcBytes) {
        std::uint64_t   info = (std::uint64_t(1) << 63) | srcBytes;
        FKS_L_POKE64LE(dst, info);
        compressed_ = true;
    } else {
        tableSize_ = 0;
        bytes      = srcBytes + HDRSZ;
        if (dstSize < bytes) {
            errorNo_ = ERROR_DIST_SIZE_IS_SMALL;
            return 0;
        }
        FKS_L_POKE64LE(dst, srcBytes);
        if (srcBytes)
            std::memcpy(dst+HDRSZ, src, srcBytes);
    }
    return bytes;
}

/** 優先待ち行列に挿入.
 */
template<class T, unsigned B>
void HuffmanEncode<T,B>::downHeep(std::size_t i) {
    std::size_t     heapSize = heapSize_;
    idx_t*          heap     = heap_;
    int             k        = heap[i];
    std::size_t     j;
 #if 1
    if ( (j = 2 * i) <= heapSize ) {
        std::size_t*    freq = freq_;
        do {
            if (j < heapSize && freq[heap[j]] > freq[heap[j + 1]])
                j++;
            if (freq[k] <= freq[heap[j]])
                break;
            heap[i] = heap[j];
            i       = j;
        } while ( (j = 2 * i) <= heapSize );
    }
 #else
    std::size_t*    freq     = freq_;
    while ( (j = 2 * i) <= heapSize ) {
        if (j < heapSize && freq[heap[j]] > freq[heap[j + 1]])
            j++;
        if (freq[k] <= freq[heap[j]])
            break;
        heap[i] = heap[j];
        i       = j;
    }
 #endif
    heap[i]     = k;
}

/** 枝を出力.
 */
template<class T, unsigned B>
void HuffmanEncode<T,B>::writeTree(unsigned i) {
    if (i < VAL_MAX) {              // 葉.
        putBit(0);
        putbits(value_bits, i);     // 文字そのもの.
    } else {                        // 節.
        putBit(1);
        writeTree(left_[i]);        // 左の枝.
        writeTree(right_[i]);       // 右の枝.
    }
}


#undef FKS_L_POKE64LE
#undef FKS_L_STATIC_ASSERT

}   // namespace fks


#endif  // HUFFMANENCODE_HPP
