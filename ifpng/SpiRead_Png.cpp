/**
 *  @file   SpiRead_Png.c
 *  @brief  Susie用 png プラグ v0.50
 *  @author Masashi KITAMURA (tenk*)
 *  @date   2007-06
 */

#include <windows.h>
#include "../Spi/SpiRead.h"
#include "../ImgFmt/libpng/png.h"
#include "../ImgFmt/PngDecoder.hpp"
#include "../Proc/BppCnvImg.hpp"


#ifdef USE_REGOPT
 #define ABOUT_MSG "png to DIB v0.50 (TSPIRO) writen by tenk*"
#else
 #define ABOUT_MSG "png to DIB ver 0.50 writen by tenk*"
#endif




/// IsSupported,GetPictureInfoでファイル読込をする場合の、
/// 画像フォーマット判定に必要なヘッダサイズ. clutは読まないので少量ですむ..かも.
DWORD       SpiRead_headerSize = 0;     // 安全のため全部読み込む...




/// プラグイン情報の文字列配列.
extern const char*  SpiRead_pluginInfoMsg[] = {
    "00IN",                                     /* 0 : Plug-in API ver. */
    ABOUT_MSG,                                  /* 1 : About.. */
    "*.png",                                    /* 2 : 拡張子 */
    "png",                                      /* 3 : 形式名 */
    "",
};



/// pDataの画像データをサポートしているか(1)否か(0)を返す. fnameは判定補助のファイル名.
BOOL SpiRead_isSupported(const char* /*fname*/, const BYTE* pData)
{
    return PngDecoder::isSupported(pData) != 0;
}



/** 画像に α情報があるか? (全てα=0か全てα=0xffならα情報無し)
 */
static inline int pix32_haveAlpha(const unsigned *p, unsigned w, unsigned h)
{
    int             chk;
    const unsigned* e  = p + w * h;
    assert(p != 0 && w != 0 && h != 0);
    chk = (unsigned char)(*p++ >> 24);
    if (0 < chk && chk < 255)
        return true;
    while (p < e) {
        int a   = (unsigned char)(*p++ >> 24);
        if (a != chk)
            return true;
    }
    return false;
}


/** pDataの画像情報を返す.
 *  GetPictureInfoと GetPictureの両方から呼ばれる.
 *  値を入れて返すが、デフォルト値でよければ何もしなくてよい.
 *  pClut256以外は必ずアドレスがある.
 *  clutを設定する場合は、1色は α8R8G8B8 で、
 *  α値(0透明 1～254半透明 255不透明)も設定して返す.
 */
BOOL SpiRead_getInfo(
        const BYTE*         pData,          // 展開元データ.
        unsigned            dataSize,       // 展開元データのバイト数.
        int*                pWidth,         // 横幅を入れて返す.
        int*                pHeight,        // 縦幅を入れて返す.
        int*                pBpp,           // 画像の１ピクセルあたりのビット数.(展開予定の、ではなく)
        DWORD*              pClut256,       // 256色までの色パレット(clut)を入れて返す. アドレス0なら無視.
        SpiRead_InfoOpt*    pInfoOpt)       // オプション的な情報を返す場合用.
{
    if (PngDecoder::isSupported(pData) == false)
        return false;

    PngDecoder  dec(pData, dataSize);

    *pWidth  = dec.width();
    *pHeight = dec.height();
    *pBpp    = dec.bpp();
    unsigned  clutSize = dec.clutSize();
    pInfoOpt->clutSize = clutSize;
    if (pClut256 && clutSize) {
        dec.getClut((unsigned*)pClut256);
        pInfoOpt->alphaFlag = pix32_haveAlpha((unsigned*)pClut256, 1, clutSize);
    }
    if (dec.bpp() == 15 || dec.bpp() == 32)
        pInfoOpt->alphaFlag = true;

    return 1;
}



/** pDataを pixBppビット色画像に展開してpPixに入れて返す.
 *  - bmpと同様に、横幅バイト数は4の倍数, 下ラインから詰める.
 *
 *  - ピクセル配置はbmpに合わせる.
 *  - pixBppは、通常は
 *      元bppが 1なら 1, 元bpp 2～4 なら 4, 元bpp 5～8なら 8、元9以上なら24
 *    が設定されてくる。ただし USE_DIB32 が定義設定されている場合は、
 *      透明色|α付画像に対しては bpp 32
 *    が設定されうる。また、
 *    tspiroレジストリオプション用に USE_REGOPT が定義されている場合は
 *      元bppが 8以下なら 8、元9以上は 32
 *    が設定されうる. (USE_DIB32やUSE_REGOPT未定義なら気にしない)
 *
 *  - fnameは複数ファイル構成のデータ向け、だが、fname=NULLの状況もあるので注意.
 */
int  SpiRead_getPix(const BYTE* pData, unsigned dataSize, BYTE* pPix, DWORD pixBpp, const char* /*fname*/)
{
    if (PngDecoder::isSupported(pData) == false)
        return false;

    PngDecoder  dec(pData, dataSize);
    dec.revY();                 // bmpは下ラインから詰めるので反転.

    if (dec.bpp() < 8)          // 1,2,4bit色の場合、バイト中の詰め順が逆.
        dec.setBigEndian();

    int rc;
    if (dec.bpp() == pixBpp) {  // 入出力のbppが同じなら、そのまま展開.
        if (pixBpp == 24)       // 24ビット色化なら、αは無しに.
            dec.stripAlpha();
        dec.setWidthAlign(4);   // bmpの横幅は４バイトでアライメント.
        rc = dec.read(pPix);

    } else {                    // bpp調整をするとき.
        unsigned w = dec.width();
        unsigned h = dec.height();
        BppCnvImg src(w, h, dec.bpp(), 1);
        if (src.clutSize())
            dec.getClut(src.clut(), src.clutSize());
        rc =  dec.read( src.image() );
        if (rc) {
            BppCnvImg dst(false, pPix, w, h, pixBpp, 4, NULL, 0);
            if (dst.bpp() < 8)  // bmp はバイト内は上位ビットから詰めるので.
                dst.setDstEndian(1);
            dst.conv(src);
        }
    }
    return rc;
}
