/**
 *  @file JpgEncoder.hpp
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
#ifndef JPGENCODER_HPP
#define JPGENCODER_HPP

#include <assert.h>

/// 24ビット色画像をjpgデータに圧縮. (libjpeg.libのラッパー)
/// ※1ピクセルは、メモリにR,G,B順に並んだモノ＝リトルエンディアン値としてはBGR順
class JpgEncoder {
public:
    JpgEncoder() {;}
    ~JpgEncoder() {;}

    JpgEncoder(unsigned char* dst, unsigned dstSz, const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned* pSz=0) {
        unsigned sz = write(dst,dstSz, src, wid, hei, quality, widByt, dir);
        if (pSz)
            *pSz = sz;
    }

    /// 内部で確保しているメモリを開放. 辻褄合わせ用のダミー.
    void    release() {;}

    /// pix に24ビット色画像を展開する. サイズは0だとデフォルトのまま. dirは0が左上から1なら左下から.
    static unsigned     write(unsigned char* dst, unsigned dstSz, const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned mono=0);

    /// malloc したメモリにjpgデータを入れて返す.
    static unsigned char*  write(const void* src, unsigned wid, unsigned hei, unsigned quality=90, unsigned widByt=0, unsigned dir=0, unsigned*pSz=0);

    /// writeでポインタを受け取った場合の対となるメモリ開放.
    static void     deallocate(void* p);

    /// 横幅をバイト数に変換.
    static unsigned wid2byt(unsigned wid, unsigned algn=1) {
        assert( algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16 );
        return (wid*3 + algn-1) & ~(algn-1);
    }
};

#endif  // JPGENCODER_H
