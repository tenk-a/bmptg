/**
 *  @file JpgDecoder.hpp
 *  @brief  メモリ上のjpg画像データを展開する.
 *  @author Masashi KITAMURA
 *  @note
 *      - Independent JPEG Groupのlibjpeg.libを用いています。
 *
 *      - 使い方
 *          - setData(binData,size) でメモリー上のjpgデータを設定し、
 *            その後 read()すればmallocしたメモリに24ビット色画像を得る.
 *          - あるいはsetData()後, サイズwidthByte()*height() 以上のメモリを
 *            呼び元で用意してread(pix, ...)を使う.
 */
#ifndef JPGDECODER_HPP
#define JPGDECODER_HPP

#include <cstdlib>      // callocをヘッダで使用するため.
#include <cassert>
#include <string.h>
#include <memory.h>



/// メモリ上のjpg画像データを展開する. (libjpeg.libのラッパー)
class JpgDecoder {
public:
    JpgDecoder();
    ~JpgDecoder();
    JpgDecoder(const void* binData, unsigned binDataSize, bool thumbnail=false);

    /// 内部で確保しているメモリを開放.
    void        release();

    static bool isSupported(const void* data) {
        const unsigned char* b = (unsigned char*)data;
      #if 1
        return (b[0] == 0xFF && b[1] == 0xD8);
      #else
        if (b[0] != 0xFF || b[1] != 0xD8)
            return false;
        if (b[2] == 0xFF && b[3] == 0xE1 && memcmp(b+6,"JFIF", 4) == 0)
            return true;
        return (b[2] == 0xFF && b[3] == 0xE1 && memcmp(b+6,"Exif", 4) == 0);
      #endif
    }

    /// メモリ上のjpgデータを設定. ポインタを保持するだけなのでread()を終える前にbinDataを破壊しないこと.
    bool        setData(const void* binData, unsigned binDataSize, bool thumbnail=false);

    unsigned    bpp()        const { return bpp_; }             ///< ビット/ピクセル. 24のみ.
    unsigned    width()      const;                             ///< 横幅.
    unsigned    height()     const;                             ///< 縦幅.
    unsigned    widthByte()  const { return width()*bpp_>>3; }  ///< 横バイト数.
    unsigned    widthByte(unsigned algn) const;                 ///< algnバイトにアライメント済の横幅.

    unsigned    imageByte()  const { return widthByte() * height(); }

    /// pix に24ビット色画像を展開する. サイズは0だとデフォルトのまま. dirは0が左上から1なら左下から.
    bool        read(void* pix, unsigned widByt=0, unsigned hei=0, unsigned dir=0);

    /// malloc したメモリに画像を入れて返す.
    void*       read(unsigned widAlgn=1, unsigned dir=0);

    unsigned    originalWidth() const { return originalWidth_; }    // thumbnail指定時の元サイズ.
    unsigned    originalHeight() const { return originalHeight_; }  // thumbnail指定時の元サイズ.

    unsigned const* clut() const { return gray_clut_; }

private:
    struct jpeg_decompress_struct*  info_;
    struct jpeg_error_mgr*          jerr_;
    void*                           binData_;
    unsigned                        binDataSize_;
    bool                            closeRq_;
    bool                            thumbnail_;
    unsigned                        originalWidth_;
    unsigned                        originalHeight_;
    int                             bpp_;
    unsigned                        gray_clut_[256];
};



/** algnバイトにアライメント済みの横幅バイト数を返す.
 */
inline unsigned     JpgDecoder::widthByte(unsigned algn) const {
    assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
    return (widthByte() + algn-1) & ~(algn-1);
}



#endif  // JPGDECODER_H
