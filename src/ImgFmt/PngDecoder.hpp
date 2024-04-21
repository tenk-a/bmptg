/**
 *  @file PngDecoder.hpp
 *  @brief  メモリ上のpng画像データを展開する.
 *  @author Masashi KITAMURA
 *  @note
 *
 *      - 使い方
 *          - setData(binData,size) でメモリー上のpngデータを設定し、
 *            その後 read()すればmallocしたメモリに24ビット色画像を得る.
 *          - あるいはsetData()後, サイズwidthByte()*height() 以上のメモリを
 *            呼び元で用意してread(pix, ...)を使う.
 */
#ifndef PNGDECODER_HPP
#define PNGDECODER_HPP

#include <stddef.h>
#include <assert.h>

#include "libpng/png.h"


/// メモリ上のpng画像データを展開する. (libjpeg.libのラッパー)
class PngDecoder {
public:
    PngDecoder();
    ~PngDecoder();
    PngDecoder(const void* binData, unsigned binDataSize);

    static bool isSupported(const void* data);

    /// 内部で確保しているメモリを開放.
    void        release();

    /// メモリ上のpngデータを設定. ポインタを保持するだけなのでread()を終える前にbinDataを破壊しないこと.
    bool        setData(const void* binData, unsigned binDataSize) throw();

    unsigned    bpp()        const { return bpp_; }         ///< ビット/ピクセル. 24のみ.
    unsigned    width()      const { return width_; }       ///< 横幅.
    unsigned    height()     const { return height_; }      ///< 縦幅.
    unsigned    widthByte(unsigned algn=0) const;           ///< algnバイトにアライメント済の横幅バイト数.

    void        setBigEndian() { bigEndian_  = true; }
    void        revY()         { revY_ = true; }
    void        stripAlpha()   { stripAlpha_  = true; }     ///< α付き画像の場合でも,αを無視.
    void        toTrueColor()  { toTrueColor_ = true; bpp_ = haveAlpha_ ? 32 : 24; }
    void        setWidthAlign(unsigned align);

    // α関係の挙動がへんになってしまう.
    void        toClutBpp8()   { toClutBpp8_  = true; bpp_ = 8; }

    unsigned    clutSize()   const { return clutSize_; }
    unsigned    getClut(unsigned* clut, unsigned clutSize=0);

    unsigned    imageByte()  const;                         ///< 展開に必要なバイト数を返す.
    bool        read(void* img);

    void*       read(unsigned widAlgn=1, unsigned dir=0);

private:
    static void raw_read_data(struct png_struct_def* png_ptr, unsigned char* data, size_t length);

private:
    const unsigned char*    binData_;
    struct png_struct_def*  png_ptr_;
    struct png_info_def*    info_ptr_;
    unsigned                binDataSize_;
    unsigned                cur_;

    unsigned                width_;
    unsigned                height_;
    int                     orgBpp_;
    int                     bpp_;
    unsigned                clutSize_;
    bool                    haveAlpha_;

    int                     color_type_;
    int                     interlace_type_;
    unsigned                sig_read_;

    unsigned                align_;
    bool                    revY_;
    bool                    bigEndian_;
    bool                    toTrueColor_;
    bool                    toClutBpp8_;
    bool                    stripAlpha_;

    bool                    closeRq_;
};

#endif  // PNGDECODER_H
