/**
 *  @file   SpiRead_FontX2.cpp
 *  @brief  Susie用 FontX2 プラグ v0.6
 *  @author Masashi KITAMURA (tenk*)
 *  @note
 *  2004    0.5
 *  2007-06 0.6 Spi各プラグインの共通処理部分と非共通部分を分離
 *          (その非共通部分)
 */

#include <windows.h>
#include "FontX2Data.h"
#include "BppCnvImg.hpp"
#include "SpiRead.h"
#ifdef USE_REGOPT
#include "RegOpt.h"
#endif

#ifdef USE_REGOPT
 #define ABOUT_MSG "fontx2 to DIB v0.60 (TSPIRO) writen by tenk*"
#else
 #define ABOUT_MSG "fontx2 to DIB ver 0.60 writen by tenk*"
#endif




/// IsSupported,GetPictureInfoでファイル読込をする場合の、
/// 画像フォーマット判定に必要なヘッダサイズ.
DWORD       SpiRead_headerSize = 32;




/// プラグイン情報の文字列配列.
extern const char*  SpiRead_pluginInfoMsg[] = {
    "00IN",                             /* 0 : Plug-in API ver. */
    ABOUT_MSG,                          /* 1 : About.. */
    "*.fnt;*.fnx;*.fx2;*.tlf",          /* 2 : 拡張子. 適当... */
    "FontX2",                           /* 3 : 形式名 */
    "",
};



/// pDataの画像データをサポートしているか(1)否か(0)を返す. fnameは判定補助のファイル名.
BOOL SpiRead_isSupported(const char* /*fname*/, const BYTE* pData)
{
    return FontX2SampleImage<>::isSupported(pData) != 0;
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
        unsigned            /*dataSize*/,   // 展開元データのバイト数.
        int*                pWidth,         // 横幅を入れて返す.
        int*                pHeight,        // 縦幅を入れて返す.
        int*                pBpp,           // 画像の１ピクセルあたりのビット数.(展開予定の、ではなく)
        DWORD*              pClut256,       // 256色までの色パレット(clut)を入れて返す. アドレス0なら無視.
        SpiRead_InfoOpt*    pInfoOpt)       // オプション的な情報を返す場合用.
{
    if (FontX2SampleImage<>::isSupported(pData) == false)
        return false;

  #ifdef USE_REGOPT // TSPIRO用の時は、"予備"ボタンがonなら横1文字にする.
    regopt_t   ro;
    RegOpt_Get(&ro);
    FontX2SampleImage<> dec(pData, (ro.mapOffSw != 0));
  #else
    FontX2SampleImage<> dec(pData, 0);
  #endif

    *pWidth  = dec.width();
    *pHeight = dec.height();
    *pBpp    = dec.bpp();
    unsigned  clutSize = dec.clutSize();
    pInfoOpt->clutSize = clutSize;
    if (pClut256 && clutSize) {
        dec.getClut((unsigned*)pClut256);
    }
    return 1;
}



/** pDataを pixBppビット色画像に展開してpPixに入れて返す.
 *  - bmpと同様に、横幅バイト数は4の倍数, 下ラインから詰める.
 *
 *  - ピクセル配置はbmpに合わせる.
 *  - pixBppは、通常は
 *      元bppが 1なら 1, 元bpp 2～4 なら 4, 元bpp 5～8なら 8、元9以上なら24
 *    が設定されてくる。
 *    ただし USE_DIB32 が定義設定されている場合は、
 *      透明色|α付画像に対しては bpp 32
 *    が設定されうる。また、
 *    tspiroレジストリオプション用に USE_REGOPT が定義されている場合は
 *      元bppが 8以下なら 8、元9以上は 32
 *    が設定されうる. (USE_DIB32 や USE_REGOPT が未定義なら気にしない)
 *
 *  - fnameは複数ファイル構成のデータ向け、だが、fname=NULLの状況もあるので注意.
 */
int  SpiRead_getPix(const BYTE* pData, unsigned /*dataSize*/, BYTE* pPix, DWORD pixBpp, const char* /*fname*/)
{
    if (FontX2SampleImage<>::isSupported(pData) == false)
        return false;

  #ifdef USE_REGOPT // TSPIRO用の時は、"予備"ボタンがonなら横1文字にする.
    regopt_t   ro;
    RegOpt_Get(&ro);
    FontX2SampleImage<> dec(pData, (ro.mapOffSw != 0));
  #else
    FontX2SampleImage<> dec(pData, 0);
  #endif

    int rc;
    if (dec.bpp() == pixBpp) {  // 入出力のbppが同じなら、そのまま展開.
        rc = dec.read(pPix, 4, 1);

    } else {                    // bpp調整をするとき.
        unsigned w = dec.width();
        unsigned h = dec.height();
        BppCnvImg src(w, h, dec.bpp(), 1);
        if (src.clutSize())
            dec.getClut(src.clut(), src.clutSize());
        rc =  dec.read( src.image(), 1, 1 );
        if (rc)
            rc = src.getImage(pPix, w, h, pixBpp, 4, NULL, 0);
    }
    return rc;
}
