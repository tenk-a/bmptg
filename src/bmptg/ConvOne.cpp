/**
 *  @file   ConvOne.cpp
 *  @brief  画像変換.
 *  @author Masashi Kitamura
 *  @date   2000-??-??
 *  @note
 *      2006    bmptg.c から分離.
 */

#include "def.h"
#include "ConvOne.hpp"
#include "pix32.h"
#include "pix8.h"
#include "gen.h"
#include "beta.h"
#include "mem_mac.h"
#include "PaternDither.hpp"
#include "ErrorDiffusion.hpp"
#include "DecreaseColorMC.hpp"
#include "DecreaseColorHst.hpp"
#include "DecreaseColorIfWithin256.hpp"
#include "DecreaseColorLowBpp.hpp"
#include "FixedClut256.hpp"
#include "GrayClut.hpp"
#include "pix32_colCnv.h"
#include "pix32_kyuv.h"
#include "pix32_resize.h"
#include "pix32_rotate.h"
#include "Pix32_Paint.hpp"
#include "pix32_bokashi.h"
#include "pix32_filter.h"
#include "pix_subr.h"
#include "BmImgLoad.h"
#include "BmImgSave.h"
#ifdef USE_MY_FMT
#include "my.h"
#endif


// int32_t,uint32_t がlongで定義されていると、このソースではまずいので、ヤッパリ自前設定に変更.
typedef unsigned      UINT32_T;
typedef unsigned char UINT8_T;


/** 変換に関するオプション要素の初期化.
 */
ConvOne_Opts::ConvOne_Opts() {
    memset(this, 0, sizeof(*this));
    this->tone      = -1;
    this->srcBpp    = -1;
    this->colKey    = -1;
    this->nukiClut  = -1;
    this->rszXpar[0]= this->rszYpar[0] = 100.0;
    this->rszXpar[1]= this->rszYpar[1] = 100.0;
    this->rszK[0]   = 1;
    this->rszK[1]   = 1;
    this->g555val   = -1;
    this->verbose   = 1;
    this->clutBpp   = 24;
    this->dstFmt    = BM_FMT_TGA;
    this->colNum    = 0; //0x7fffffff;

    this->quality   = -1;
    this->quality_grey = -1;

    this->alpMin    = 0x00;         // 半透明範囲min
    this->alpMax    = 0xff;         // 半透明範囲max
    this->alpToCol  = -1;           // 半透明(α)min～maxのピクセルに、色alpToColとαブレンドして、α値を0xffにする.
    this->clutAlpNum = -1;          // 減色時のα数. 自動設定.

    this->monoToAlp_rate = 1.0;

    this->pixScale[0] = 1.0;
    this->pixScale[1] = 1.0;
    this->pixScale[2] = 1.0;
    this->pixScale[3] = 1.0;

    this->decreaseColorParam[0] = -1;
    this->decreaseColorParam[1] = 1.2f;
    this->decreaseColorParam[2] = 1.0;
    this->decreaseColorParam[3] = 1.0;

    this->rszType = 1;  // バイキュービック.
}


void ConvOne_Opts::readFixedClut(const char* fname)
{
    int sz = 0;
    char* src = (char*)fil_loadE(fname, 0, 0, &sz);
    if (sz == 0)
        return;
    unsigned n = 0;
    char*    s = strtok(src, ", \t\r\n{};");
    while (s && s < src+sz && *s && n < CLUT_NUM) {
        unsigned v = strtoul(s, NULL, 0);
        fixedClut_[n++] = v;
        s = strtok(NULL, ", \t\r\n{};");
    }
}



//----------------------------------------------------------------------------

ConvOne::ConvOne()
{
//  bmImgLoadMgr_ = new BmImgLoadMgr();
//  bmImgSaveMgr_ = new BmImgSaveMgr();
    init(NULL,NULL);
}


ConvOne::~ConvOne()
{
    term();
//  delete bmImgLoadMgr_;
//  delete bmImgSaveMgr_;
}

/** 画像をひとつ変換する.
 *  @param  srcName     入力ファイル名.
 *  @param  dstName     出力ファイル名.
 */
int ConvOne::run(const char *srcName, const char *dstName) {
    init(srcName, dstName);
    int rc = main();
    term();
    return rc;
}


/// 変換のメイン.
int ConvOne::main() {
    if (readFile() == 0)                // ファイル読み込み.
        return 0;
    if (imageLoad() == 0)               // ファイルバイナリから画像を取得.
        return 0;
    adjustAlpha();                      // α値が全て0なら0xff化.
    swapARGB();                         // argbの順番並べ換え.
    loadAlphaPlaneFile();               // αプレーンにモノクロ画像を読み込む.
    changeClut();                       // パレット調整.
    bitCom();                           // 色値のビットを反転する.
    revAlpha();                         // α値反転.
    monoToAlpha();                      // rgb値よりモノクロ値を求めそれをαにする.
    setClutNukiCol();                   // 256色で抜き色がユーザー指定されていない場合、clut_のα=0のものを抜き色として採用する.
    reflectColKey();                    // 抜き色(カラーキー) を反映する.
    easyPaintRoundAlpha();              // 左上(0,0)のピクセルを抜き色として、画像の4隅から抜き色のαを塗りつぶす.
    clearColorIfAlpha0();               // a=0ならrgbも0クリア.
    alphaBokashi();                     // BUを前提にαをぼかす.
    resizeCanvas(0);                    // 画像サイズの切り抜き変更.
    resizeCanvas(1);                    // 画像サイズの切り抜き変更.
    changeSrcBpp();                     // 入力のbpp_に色を修正.
    reverseImage();                     // 上下左右反転.
    rotR90(opts_.rotR90);               // 左右90°.
    rotateImage();                      // 任意角回転.
    setDstBpp();                        // 出力bppを設定.
    checkSrcDstBpp();                   // clut画同士の変換で出力bppより色番号が多い時にソース側をフルカラー化(減色するの前提)

    convNukiBlack();                    // 抜き色が色(0,0,0)固定のハード向けに、真黒を(0,0,m)に、抜きドットを(0,0,0)に変換.
    resizeImage1st();                   // 縦横拡縮サイズ変更   // 初回.
    filter();                           // フィルタ (ぼかし)
    resizeImage2nd();                   // 縦横拡縮サイズ変更   2回目.
    aptRect();                          // 抜き色|αで範囲を求めてサイズ変更.
    toMono();                           // モノクロ化.
    alphaBlendByColor();                // 指定色とαをブレンドし、αを0 or 255 にする.

    mulCol();                           // 各ピクセルに色を乗ずる.
    colChSquare();                      // ARGB各々を二乗する.
    changeTone();                       // 抜き色以外の色のトーンを opts_.tone％に変換.

    patternDither();                    // 誤差拡散・パターンディザを施す.
    decreaseColor();                    // 多色で出力がclut_付きなら、安易減色を行う.

    changeChipAndMap();                 // チップ(セル)＆マップ化.
    reverseOutputAlpha();               // 出力のαを反転する必要があるとき.
    changeMaskImage();                  // マスク画像生成のとき.
    if (saveImage() == 0)               // 出力ファイルイメージを生成.
        return 0;
    if (writeFile() == 0)               // ファイル書き込み.
        return 0;
    return 1;
}


/// 初期化.
void ConvOne::init(const char *srcName, const char *dstName) {
    sz_         = 0;
    w_          = 0;
    h_          = 0;
    bpp_        = 0;
    pixBpp_     = 0;
    pixWb_      = 0;
    pixH_       = 0;
    srcFmt_     = 0;
    dstColN_    = 0;
    colNum_     = 0;
    mapSz_      = 0;
    nukiClut_   = -1;
    pix_        = NULL;
    map_        = NULL;
    src_        = NULL;
    dat_        = NULL;
    dst_        = NULL;
    srcW_       = 0;
    srcH_       = 0;

    srcName_    = srcName;
    dstName_    = dstName;

    dstBpp_     = opts_.bpp;
    encMode_    = opts_.encMode;
    varbose_    = opts_.verbose != 0;
    fullColFlg_ = opts_.fullColFlg != 0;
    mono_       = false;

    // 画像ロードセーブのオプション要素の設定.
    bo_         = &bopt_;
    memset(bo_, 0, sizeof(*bo_));

    bo_->lvlY    = opts_.lvlY;
    bo_->lvlUV   = opts_.lvlUV;
    bo_->celW    = opts_.celSzW;
    bo_->celH    = opts_.celSzH;
    bo_->mapTexW = opts_.mapTexW;
    bo_->mapTexH = opts_.mapTexH;
    bo_->clutBpp = opts_.clutBpp;
    bo_->quality = opts_.quality;
    bo_->quality_grey = opts_.quality_grey;
    bo_->alpBitForBpp8 = opts_.alpBitForBpp8;
    bo_->x0      = opts_.startX;
    bo_->y0      = opts_.startY;

    // パレットクリア.
    memset(clut_, 0, sizeof(clut_));
}


/// ファイル読み込み.
bool ConvOne::readFile() {
    int bytSkp = opts_.bytSkp;

    // ファイルロード.
    if (varbose_) {
        if (dstName_ && dstName_[0])
            printf("%s -> %s  ", srcName_, dstName_);
        else
            printf("%-20s ", srcName_);
    }
    src_ = (UINT8_T*)fil_load(srcName_, NULL, 0, &sz_);
    if (src_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s をオープンできません.\n", srcName_);
        return 0;
    }
    if (sz_ == 0) {
        if (varbose_) printf("\n");
        printf("%s は 0byteです.\n", srcName_);
        return 0;
    }
    dat_ = src_;
    if (bytSkp && sz_ > bytSkp) {   // 先頭 バイトをスキップするとき.
        dat_ += bytSkp;
        sz_  -= bytSkp;
    }
    return 1;
}


/// ファイルバイナリから画像を取得.
bool ConvOne::imageLoad() {
    int binBpp = opts_.binBpp;

    // 画像のヘッダ情報取得.
    if (binBpp == 0) {
        srcFmt_ = bm_getHdr(dat_, sz_, &w_, &h_, &bpp_, &colNum_);
    } else {
        srcFmt_ = BM_FMT_BETA;
        w_      = opts_.binW;
        h_      = opts_.binH;
        bpp_    = binBpp;
        colNum_ = (bpp_ < 9) ? (1<<bpp_) : 0;
        if (h_ < 0) {
            int wb = WID2BYT(w_, bpp_);
            h_ = 0;
            if (wb > 0)
                h_  = (sz_ + wb - 1) / wb;
        }
    }
    if (varbose_) {
        printf("[%d:%d*%d]",bpp_,w_,h_);
      #ifdef MY_H
        MY_imageLoad_log(srcFmt_);
      #endif
    }
    // 出力名が無けりゃ、たぶんただの情報表示.
    if (dstName_ == NULL || dstName_[0] == '\0') {
        if (varbose_) printf("\n");
        freeE(src_);
        return 0;
    }

    bo_->srcW = w_;
    bo_->srcH = h_;
    srcW_    = w_;
    srcH_    = h_;
    if (srcFmt_ == 0) {
        if (varbose_) printf("\n");
        printf("%s : 未対応の画像フォーマットのファイルを読みこんだ.\n", srcName_);
        freeE(src_);
        return 0;
    }

    pixBpp_ = (bpp_ <= 8) ? 8 : 32; // clut_ 付き画は一旦 256色画に、多色画は 32bit色画にする.

    //
    fullColFlg_ = opts_.fullColFlg || opts_.dstFmt == BM_FMT_JPG || (opts_.bpp == 0 && (opts_.rszN || opts_.bokashiCnt));
  #ifdef MY_H
    fullColFlg_ = fullColFlg_ || MY_IMAGELOAD_EX_FULLCOLFLG(opts_.dstFmt);
  #endif
    if (fullColFlg_)
        pixBpp_     = 32;

  #ifdef MY_H
    MY_imageLoad_MyFmtSetting();
  #endif

    pixH_  = h_;
    if (opts_.mapMode && bo_->celW && bo_->celH)        // マップ化.
        pixH_ = ((h_ + bo_->celH-1) / bo_->celH) * bo_->celH;
    pixWb_ = WID2BYT(w_, pixBpp_);
    pix_ = (UINT8_T*)calloc(1, pixWb_ * pixH_ + 16);
    if (pix_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s 読みこみためのメモリを確保できません.\n", srcName_);
        freeE(src_);
        return 0;
    }

    // 画像展開.
    int c;
    DBG_PRINTF(("rd %x %x %d, %d, %d, %d, %x, %d\n", src_, pix_, pixWb_, w_, h_ , pixBpp_, clut_, 0));
    if (binBpp == 0) {  // 通常の画像ファイル読み込み.
        c = bm_read(dat_, sz_, pix_, pixWb_, h_, pixBpp_, clut_, 0);
    } else {            // ベタ画像 読み込み.
        c = WID2BYT(opts_.binW, binBpp);
        if (sz_ < c * pixH_) {
            UINT8_T *p = (UINT8_T*)callocE(c , h_);
            memcpy(p, dat_, sz_);
            freeE(src_);
            src_ = dat_ = p;
        }
        if (binBpp <= 8) beta_genMonoClut(clut_, binBpp);   // ダミーclutを用意.
        c = beta_conv(pix_, pixWb_, h_, pixBpp_,  dat_, c, binBpp, clut_, 0, opts_.binEndian, 0);
    }
    freeE(src_);
    if (c == 0) {
        if (varbose_) printf("\n");
        printf("%s : 画像読み込みに失敗\n", srcName_);
        freeE(pix_);
        return 0;
    }
    return 1;
}


/** αプレーンにモノクロ画像を読み込む.
 */
void ConvOne::loadAlphaPlaneFile()
{
    if (!opts_.alphaPlaneFileName)
        return;
    int         sz   = 0;
    void*       dat  = fil_load(opts_.alphaPlaneFileName, NULL, 0, &sz);
    if (dat) {
        int         w    = 0;
        int         h    = 0;
        int         bpp  = 0;
        unsigned*   apic = bm_load32(dat, sz, &w, &h, &bpp);
        if (apic) {
            unsigned* pix = (unsigned*)pix_;
            for (unsigned y = 0; y < unsigned(h_); ++y) {
                for (unsigned x = 0; x < unsigned(w_); ++x) {
                    int a = 0;
                    if (x < unsigned(w) && y < unsigned(h)) {
                        unsigned cc = apic[y*w+x];
                        if (bpp == 32) {
                            a = ARGB_A(cc);     // alpha付画像なら alpha値を取得.
                        } else {
                            a = ARGB_G(cc);     // r=g=b な画像ファイルの読込前提でGプレーンを取得.
                            // a = PIX32_RGB_TO_YUV_Y_x_VAL(PIX32_GET_R(cc), PIX32_GET_G(cc), PIX32_GET_B(cc), 1);
                        }
                    }
                    unsigned c = pix[y*w_+x];
                    c &= 0xFFFFFF;
                    c |= a << 24;
                    pix[y*w_+x] = c;
                }
            }
            freeE(apic);
        }
    }
    // 有効なのは1ファイルのみなのでファイル名を削除.
    freeE((void*)opts_.alphaPlaneFileName);
    opts_.alphaPlaneFileName = NULL;
}


// -------------------------------------------------------------------
// ピクセルデータの各種変換.


/// argbの順番並べ換え.
void ConvOne::swapARGB() {
    int colrot = opts_.colrot;
    if (colrot) {
        if (pixBpp_ == 8)   pix32_swapARGB(clut_,  1, 256, PIX32_SWAPARGB(colrot));
        else                pix32_swapARGB((UINT32_T*)pix_ , w_,  h_, PIX32_SWAPARGB(colrot));
    }
}


/// パレット調整.
void ConvOne::changeClut() {
    int         clutOfs     = opts_.clutOfs;
    const int*  clutChgFlg  = opts_.clutChgFlg;
    const int*  clutChg     = opts_.clutChg;
    if (bpp_ <= 8) {
        unsigned clutNum = colNum_ > 256 ? colNum_ : 256;   // CLUT_NUM
        if (clutNum > CLUT_NUM)
            clutNum = CLUT_NUM;
        for (int n = 0; n < clutNum; n++) {
            if (clutChgFlg[n])
                clut_[n] = clutChg[n];
        }
        if (clutOfs > 0) {
            UINT32_T    clutTmp[CLUT_NUM];
            memset(clutTmp, 0, 4*clutNum);
            for (int n = 0; n < clutNum; n++) {
                clutTmp[n] = clut_[(clutOfs+n) % clutNum];
            }
            memcpy(clut_, clutTmp, 4*clutNum);
        }
    }
}


/// 色値のビットを反転する.
void ConvOne::bitCom() {
    int sw = opts_.bitCom;
    if (sw) {
        if (pixBpp_ == 8)   pix_bitCom((UINT8_T *)pix_, w_, h_);
        else                pix_bitCom((UINT32_T*)pix_, w_, h_); //x printf("多色ではビット反転は未対応.\n");
    }
}


/// α値調整.
void ConvOne::adjustAlpha() {
    if (opts_.alpNon || bpp_ == 24 || (bpp_ <= 8 && pix32_isAlphaAllZero(clut_,1,256))) {
        // α値なし画像ならばαを付加. (カラーキーをありえない-1にして、全てのピクセルのαを0xffにする)
        if (pixBpp_ == 8)   pix32_genColKeyToAlpha(clut_, 1, 256, (unsigned)-1);
        else                pix32_genColKeyToAlpha((UINT32_T*)pix_, w_, h_, (unsigned) -1);
    }
}


/// α値反転.
void ConvOne::revAlpha() {
    // 入力のαを反転するとき.
    if (opts_.alpModeI) {
        if (pixBpp_ == 8)   pix32_revAlpha(clut_, 1, 256);
        else                pix32_revAlpha((UINT32_T*)pix_, w_, h_);
    }
}


/// 256色で抜き色がユーザー指定されていない場合、clut_中の最初に見つかったα=0のものを抜き色として採用する.
void ConvOne::setClutNukiCol() {
    if (pixBpp_ == 8) {
        nukiClut_ = opts_.nukiClut;
        if (nukiClut_ == -1 && opts_.colKey != -1) {    // clut-indexでなく抜き色指定があったばあい、それを探してみる.
            int idx = 256;
            unsigned colKey = opts_.colKey;
            unsigned mask   = (colKey <= 0xFFFFFF) ? 0xFFFFFF : 0xFFFFFFFF;
            while (--idx >= 0) {
                if ((clut_[idx] & mask) == colKey) {
                    nukiClut_ = idx;
                    break;
                }
            }
        }
        if (nukiClut_ == -1) {
            if (ARGB_A(clut_[0]) == 0) {
                nukiClut_ = 0;
            } else if (ARGB_A(clut_[255]) == 0) {
                nukiClut_ = 255;
            } else {
                for (unsigned i = 1; i < 255; ++i) {
                    UINT32_T  c = clut_[i];
                    if (ARGB_A(c) == 0) {
                        nukiClut_ = i;
                        break;
                    }
                }
            }
        }
        // 抜き色指定があれば、その色のαを 0にする.
        if (nukiClut_ >= 0 && nukiClut_ <= 255 && opts_.colKeyNA == 0) {
            clut_[nukiClut_] &= 0xFFFFFF;
        }
    }
}


/// 抜き色(カラーキー) を反映する.
void ConvOne::reflectColKey() {
    if (opts_.colKeyNA)
        return;
    int colKey    = opts_.colKey;
    //int alpBokasi = opts_.alpBokasi;
    if (colKey != -1) {
        if (pixBpp_ == 8) {
            pix32_clearAlphaOfColKey(clut_, 1, 256, colKey);
        } else {
            pix32_clearAlphaOfColKey((UINT32_T*)pix_, w_, h_, colKey);
        }
    }
}


/// 左上(0,0)のピクセルを抜き色として、画像の4隅から抜き色のαを塗りつぶす.
void ConvOne::easyPaintRoundAlpha() {
    if (opts_.genAlpEx && pixBpp_ == 32) {
        if (varbose_) printf("->[XCA]");

        UINT32_T* pix = reinterpret_cast<UINT32_T*>(pix_);
        //x pix32_genAlpEx(pix, w_, h_);
        // 一旦すべてのαをon.
        unsigned size = w_ * h_;
        for (unsigned i = 0; i < size; ++i)
            pix[i] |= 0xff000000;

        // 左上の点の色が続く範囲を四隅ともしらべ、ARGB(0,0,0,0) で埋める.
        int dif = abs(opts_.genAlpEx - 1);
            if (dif > 255)
                dif = 255;
        Pix32_Paint(pix, (unsigned)w_, (unsigned)h_, 0x00000000, -1, -1, dif /*, 0xFFF8F8F8*/);
    }
}

void ConvOne::clearColorIfAlpha0() {
    int clearColIfAlp0 = opts_.clearColIfAlp0;
    if (clearColIfAlp0) {
        if (pixBpp_ == 8)
            pix32_clearColorIfAlpha0(clut_, 256, 1);
        else
            pix32_clearColorIfAlpha0((UINT32_T*)pix_, w_, h_);
    }
}

/// 抜き色(カラーキー) を反映する.
void ConvOne::alphaBokashi() {
    int alpBokasi = opts_.alpBokasi;
    if (alpBokasi > 0 && pixBpp_ == 32) {
        pix32_alpBokasi((UINT32_T*)pix_, w_, h_, alpBokasi);      // 透明・不透明の境目のαをぼかす.
    }
}


/// 画像サイズの切り抜き変更.
void ConvOne::resizeCanvas(int n) {
    if (n >= opts_.vvIdx)
        return;
    ConvOne_Opts::vv_t* opts_vv = &opts_.vv[n];
    if (opts_.vv[n].flg) {     // 指定サイズの画像領域に、pix_を矩形転送する.
        int vv_w = (opts_vv->w || opts_vv->wf) ? opts_vv->w : w_;
        int vv_h = (opts_vv->h || opts_vv->hf) ? opts_vv->h : h_;
        if (vv_w < 0 || opts_vv->wf)
            vv_w = w_ + vv_w;
        if (vv_h < 0 || opts_vv->hf)
            vv_h = h_ + vv_h;

        int vv_sw = opts_vv->sw;
        int vv_sh = opts_vv->sh;
        if (vv_sw <= 0)
            vv_sw = w_ + vv_sw;
        if (vv_sh <= 0)
            vv_sh = h_ + vv_sh;

        int vv_x  = opts_vv->x;
        int vv_y  = opts_vv->y;
        int vv_sx = opts_vv->sx;
        int vv_sy = opts_vv->sy;

        int vv_lcr = opts_vv->lcr;
        if (opts_vv->lcr_ex) {  // ファイル名本体の最後の一文字が数字の時、偶数奇数で左詰め・右詰めを切り替える処理.
            char const* e = fname_getExt(srcName_);
            if (srcName_ <= e - 2) {
                int c = e[-2];
                if (c == '0' || c == '2' || c == '4' || c == '6' || c == '8') {
                    vv_lcr = 2;
                } else if (c == '1' || c == '3' || c == '5' || c == '7' || c == '9') {
                    vv_lcr = 0;
                }
            }
        }

        if (vv_lcr == 1) {              // 中央寄せ.
            vv_x  += (vv_w - vv_sw) / 2;
            vv_sx += (w_   - vv_sw) / 2;
        } else if (vv_lcr == 2) {       // 右寄せ.
            vv_x  += (vv_w - vv_sw);
            vv_sx += (w_   - vv_sw);
        }
        if (opts_vv->umd == 1) {        // 縦中央寄せ.
            vv_y  += (vv_h - vv_sh) / 2;
            vv_sy += (h_   - vv_sh) / 2;
        } else if (opts_vv->umd == 2) { // 下寄せ.
            vv_y  += (vv_h - vv_sh);
            vv_sy += (h_   - vv_sh);
        }
        if (vv_x < 0) {
            vv_sw -= -vv_x;
            vv_sx += -vv_x;
            vv_x   = 0;
        }
        if (vv_y < 0) {
            vv_sh -= -vv_y;
            vv_sy += -vv_y;
            vv_y = 0;
        }
        if (pixBpp_ == 8) {
            gen_newSizePix8 (&pix_, &w_, &h_, vv_sw, vv_sh, vv_sx, vv_sy
                            , nukiClut_, vv_w, vv_h, vv_x, vv_y);
        } else {
            int colKey = opts_.colKey;
            if (opts_.colKeyNA) {
                if (colKey <= 0xFFFFFF)
                    colKey = 0xFF000000 | colKey;
            } else {
                if (colKey == -1) {
                    colKey = 0;
                }
            }
            gen_newSizePix32(&pix_, &w_, &h_, vv_sw, vv_sh, vv_sx, vv_sy
                            , colKey , vv_w, vv_h, vv_x, vv_y);
        }
        if (varbose_) {
            printf("->(%d,%d %d*%d)(%d,%d)[%d*%d]",vv_sx,vv_sy,vv_sw,vv_sh,vv_x,vv_y,w_,h_);
            //printf("->[%d*%d]",w_,h_);
        }
        pixWb_ = WID2BYT(w_, pixBpp_);
        srcW_  = w_;
        srcH_  = h_;
    }

    if (bo_->celW) {        // 指定ドット単位のサイズに合わせ直す.
        int sw=w_, sh=h_;
        if (varbose_) printf("->cel(%d*%d)",bo_->celW,bo_->celH);
        if (pixBpp_ == 8) gen_celSz8(&pix_, &w_, &h_, bo_->celW, bo_->celH, nukiClut_);
        else             gen_celSz32(&pix_, &w_, &h_, bo_->celW, bo_->celH, opts_.colKey);
        if (sw!=w_ || sh != h_) if (varbose_) printf("[%d*%d]",w_,h_);
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// 入力のbpp_に色を修正.
void ConvOne::changeSrcBpp() {
    int srcBpp = opts_.srcBpp;
    if (srcBpp >= 0) {  // 入力のbpp_に色を修正.
        if (varbose_) printf("->bpp_(%d)", srcBpp);
        if (pixBpp_ == 8)  pix32_bppChg(clut_, 1, 256, srcBpp);
        else               pix32_bppChg((UINT32_T*)pix_, w_, h_, srcBpp);
    }
}


/// 上下左右反転.
void ConvOne::reverseImage() {
    int dir = opts_.dir;
 #if 1  // ???? atode
    if (opts_.rotR90 < 0) {
        dir ^= 3;
    }
 #endif
    if (dir) {            // 反転処理.
        if (varbose_) printf("->rev%s%s", (dir&1)?"X":"", (dir&2)?"Y":"");
        if (pixBpp_ == 8)  pix_revXY((UINT8_T*)pix_, w_, h_, dir);
        else               pix_revXY((UINT32_T*)pix_, w_, h_, dir);
    }
}


/// 右90°回転.
void ConvOne::rotR90(int type) {
    if (type) {
        if (varbose_) printf("->rot%c90", type < 0 ? 'L' : 'R');
        UINT8_T* p = (UINT8_T*)callocE( 1, pixWb_ * h_ );
        if (pixBpp_ == 8)  pix_rotR90((UINT8_T*)p, (const UINT8_T*)pix_, w_, h_);
        else               pix_rotR90((UINT32_T*)p, (const UINT32_T*)pix_, w_, h_);
        std::swap(w_, h_);
        freeE(pix_);
        pix_   = p;
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// 右R°回転.
void ConvOne::rotateImage() {
    double rotR = opts_.rotR;
    rotR = fmod(rotR + 360.0, 360.0);
    if (rotR == 0.0)
        return;
    if (rotR == 90) {
        rotR90(1);
        return;
    } else if (rotR == 180) {
        if (varbose_) printf("->rot180");
        if (pixBpp_ == 8)  pix_revXY((UINT8_T*)pix_, w_, h_, 3);
        else               pix_revXY((UINT32_T*)pix_, w_, h_, 3);
        return;
    } else if (rotR == 270) {
        rotR90(-1);
        return;
    }
    if (pixBpp_ != 32) {
        printf("->(not rot(%5.1f))", rotR);
        return;
    }

    if (varbose_) printf("->rot(%5.1f)", rotR);

    int colKey = opts_.colKey;
    if (opts_.colKeyNA) {
        if (colKey <= 0xFFFFFF)
            colKey = 0xFF000000 | colKey;
    } else {
        if (colKey == -1) {
            colKey = 0;
        }
    }

    pix32_rotate_dst_t dst = {0};
    if (pix32_rotate(&dst, (UINT32_T const*)pix_, w_, h_, -rotR, (UINT32_T)colKey, pixBpp_ > 24, opts_.rszType) == 0 || !dst.mallocMem) {
        printf("\n'not enough memory!'\n");
        return;
    }
    freeE(pix_);
    pix_   = (UINT8_T*)dst.mallocMem;
    w_     = dst.w;
    h_     = dst.h;
    pixWb_ = WID2BYT(w_, pixBpp_);
}


/// モノクロ化.
void ConvOne::toMono() {
    //bo_->mono = opts_.mono;
    if (opts_.monoNear) {       // グレイに近い色なら、モノクロ扱いにする.
        if ((pixBpp_ == 8 && GrayClut<>::isNearGrey(clut_, 1, 256))
            || GrayClut<>::isNearGrey((UINT32_T*)pix_, w_, h_)
        ){
            mono_ = true;
        }
    }
    if (opts_.mono || mono_) {
        mono_ = true;
        int monoCh = opts_.monoChRGB;
        if (monoCh == 0) {  // 通常のモノクロ化.
            if (varbose_) printf("->mono");
            if (pixBpp_ == 8) pix32_toMono(clut_, 1, 256);
            else              pix32_toMono((UINT32_T*)pix_, w_, h_);
        } else {            // 指定(RGBA)チャンネルを用いてモノクロ化.
            --monoCh;
            if (varbose_) {
                static char const s_monoCh[4] = {'B','G','R','A'};
                printf("->monoCh%c", s_monoCh[monoCh]);
            }
            if (pixBpp_ == 8) pix32_chARGBtoMono(clut_, 1, 256, monoCh);
            else              pix32_chARGBtoMono((UINT32_T*)pix_, w_, h_, monoCh);
        }
    }
}


/// 各ピクセルに色を乗ずる.
void ConvOne::mulCol() {
    if (opts_.colMul) {     // 各ピクセルに色を乗ずる.
        if (varbose_) printf("->colMul[%x]", opts_.colMul);
        if (pixBpp_ == 8) pix32_colMul(clut_, 1, 256, opts_.colMul);
        else              pix32_colMul((UINT32_T*)pix_, w_, h_, opts_.colMul);
        mono_ = false;
    }
    if (opts_.pixScale[0] != 1.0 || opts_.pixScale[1] != 1.0 || opts_.pixScale[2] != 1.0 || opts_.pixScale[3] != 1.0) {
        if (opts_.pixScaleType == 0) {
            if (varbose_) printf("->rgbScale[%4.2f:%4.2f:%4.2f:%4.2f]", opts_.pixScale[0], opts_.pixScale[1], opts_.pixScale[2], opts_.pixScale[3] );
            if (pixBpp_ == 8) pix32_argbScale(clut_, 1, 256, opts_.pixScale);
            else              pix32_argbScale((UINT32_T*)pix_, w_, h_, opts_.pixScale);
        } else {
            if (varbose_) printf("->yuvScale[%4.2f:%4.2f:%4.2f:%4.2f]", opts_.pixScale[0], opts_.pixScale[1], opts_.pixScale[2], opts_.pixScale[3] );
            if (pixBpp_ == 8) pix32_ayuvScale(clut_, 1, 256, opts_.pixScale);
            else              pix32_ayuvScale((UINT32_T*)pix_, w_, h_, opts_.pixScale);
        }
        mono_ = false;
    }
}


/// RGBA各チャンネルを二乗する.
void ConvOne::colChSquare() {
    if (opts_.colChSquare) {    // RGBA各チャンネルを二乗する.
        if (varbose_) printf("->colChSquare");
        if (pixBpp_ == 8) pix32_colChSquare(clut_, 1, 256);
        else              pix32_colChSquare((UINT32_T*)pix_, w_, h_);
        mono_ = false;
    }
}


/// 抜き色以外の色のトーンを opts_.tone％に変換.
void ConvOne::changeTone() {
    if (opts_.tone >= 0) {      // 抜き色以外の色のトーンを opts_.tone％に変換.
        if (opts_.toneType == 0) {
            if (varbose_) printf("->toneRgb[%d]", opts_.tone);
            if (pixBpp_ == 8) pix32_changeRgbTone(clut_, 1, 256, opts_.tone / 100.0);
            else              pix32_changeRgbTone((UINT32_T*)pix_, w_, h_, opts_.tone / 100.0);
        } else {
            if (varbose_) printf("->toneY[%d]", opts_.tone);
            if (pixBpp_ == 8) pix32_changeTone(clut_, 1, 256, opts_.tone / 100.0);
            else              pix32_changeTone((UINT32_T*)pix_, w_, h_, opts_.tone / 100.0);
            mono_ = false;
        }
    }
}


void ConvOne::monoToAlpha() {
    if (opts_.monoToAlp) {
        if (pixBpp_ == 8)   pix32_monoToAlpha(clut_, 1, 256, opts_.monoToAlp_rate, opts_.monoToAlp_ofs);
        else                pix32_monoToAlpha((UINT32_T*)pix_, w_, h_, opts_.monoToAlp_rate, opts_.monoToAlp_ofs);
    }
}


/// 出力BPP を決定.
void ConvOne::setDstBpp()
{
    // オプション指定されてたらそれにする.
    if (dstBpp_ > 0) {
        ;
    } else if (fullColFlg_ && bpp_ < 24) {
        int f;
        if (pixBpp_ <= 8)
            f = pix32_isUseAlpha(clut_, 1, 256);
        else
            f = pix32_isUseAlpha((UINT32_T*)pix_, w_, h_);
        if (f)
            dstBpp_ = 32;
        else
            dstBpp_ = 24;
        if (opts_.dstFmt == BM_FMT_BMP) // bmpは下手に32ビット出力になると不味いとして、指定のない限り24ビット色にしとく.
            dstBpp_ = 24;
    } else if (bpp_== 4 && colNum_ > 16) {
        dstBpp_ = 8;
    } else {
        dstBpp_ = bpp_;
    }
    dstColN_ = 1 << dstBpp_;
    if (dstColN_ < opts_.colNum) {
        if (colNum_ >= opts_.colNum)
            dstColN_ = opts_.colNum;
        else if (dstColN_ < colNum_)
            dstColN_ = colNum_;
    }
    if (dstColN_ > CLUT_NUM)
        dstColN_ = CLUT_NUM;
    if (dstBpp_ > 8)
        dstColN_ = 0;
}


/** srcBpp,dstBpp共にclut画で、dstBpp < srcBpp の時、dstに収まりきらないsrcがあるかチェック.
 *  srcの色番号がはみ出すようなら、いっそ、src をフルカラーに変換(減色に任す)
 */
void ConvOne::checkSrcDstBpp()
{
    if (dstBpp_ >= pixBpp_ || dstBpp_ > 8 || pixBpp_ != 8)
        return;
    if (pix8_hasPixOutOfIdx(pix_, w_, h_, (1 << dstBpp_) - 1)) {
        uint32_t* pix2 = (uint32_t*)callocE(1, w_ * h_ * sizeof(uint32_t));
        int       wb   = WID2BYT(w_, 32);
        beta_conv(pix2, wb, h_, 32,  pix_, pixWb_, pixBpp_, clut_, 0, 0, 0);
        freeE(pix_);
        pix_    = (uint8_t*)pix2;
        pixWb_  = wb;
        pixBpp_ = 32;
    }
}


/// 抜き色が色(0,0,0)固定のハード向けに、真黒を(0,0,m)に、抜きドットを(0,0,0)に変換.
void ConvOne::convNukiBlack() {
    if (opts_.g555val>=0) { // α<=opts_.g555valのピクセルを(0,0,0)に、以外の(0,0,0)を(0,0,m)に, 1～mをmに.
        int abpp = (dstBpp_ > 8) ? dstBpp_ : (opts_.srcBpp > 0) ? opts_.srcBpp : (opts_.ditBpp > 0) ? opts_.ditBpp : 32;
        if (varbose_) printf("->[-xg]");
        if (pixBpp_ == 8) pix32_blackAMskGen(clut_, 1, 256, opts_.g555val, abpp);
        else              pix32_blackAMskGen((UINT32_T*)pix_, w_, h_, opts_.g555val, abpp);
    }
}


/// 縦横拡縮サイズ変更  // 初回.
void ConvOne::resizeImage1st() {
    if (opts_.rszN) {   // 縦横拡縮サイズ変更   // 初回.
        int     rszW;
        int     rszH;
        UINT8_T     *pix2;
        int    c = 0;
        // for (c = 0; c < opts_.rszN; c++)
        {
            if (opts_.rszXsz[c] || opts_.rszYsz[c]) {   //rszXsz,rszYszを使う場合.
                if (opts_.rszK[c] && (opts_.rszXsz[c] && opts_.rszYsz[c])) {    // -xrq 比率を変えず、どちらか一方、狭い方の辺をジャストにする.
                    double rw = (double)opts_.rszXsz[c] / (double)w_;
                    double rh = (double)opts_.rszYsz[c] / (double)h_;
                    if (rw >= rh || opts_.rszYsz[c] == 0) {
                        rszW = opts_.rszXsz[c];
                        rszH = h_ * opts_.rszXsz[c] / w_;
                    } else {
                        rszW = w_ * opts_.rszYsz[c] / h_;
                        rszH = opts_.rszYsz[c];
                    }

                } else {
                    rszW = (opts_.rszXsz[c]) ? opts_.rszXsz[c] : w_ * opts_.rszYsz[c] / h_;
                    rszH = (opts_.rszYsz[c]) ? opts_.rszYsz[c] : h_ * opts_.rszXsz[c] / w_;
                }
                srcW_ = (int)((double)srcW_ * (double)rszW / (double)w_);
                srcH_ = (int)((double)srcH_ * (double)rszH / (double)h_);
            } else {
                double rszXpar = opts_.rszXpar[c];
                double rszYpar = opts_.rszYpar[c];
                if (rszXpar == 0)
                    rszXpar = 100.0;
                if (rszYpar == 0)
                    rszYpar = rszXpar;
                rszW  = (int)(w_    * rszXpar / 100.0);
                rszH  = (int)(h_    * rszYpar / 100.0);
                srcW_ = (int)(srcW_ * rszXpar / 100.0);
                srcH_ = (int)(srcH_ * rszXpar / 100.0);
            }
            if (varbose_) printf("->rsz[%d,%d]", rszW,rszH);
            if (rszW <= 0 || rszH <= 0)
                printf("拡縮後のサイズがおかしくなる(%d,%d)\n", rszW, rszH);
            pix2 = (UINT8_T*)callocE(1, 4 * ((rszW+15)&~15)*(rszH+1) + 256);
            if (pixBpp_ == 8)
                pix8_resize(pix2, rszW, rszH, pix_, w_, h_);    //x printf("CLUT付画像のままでは拡縮できない\n");
            else
                pix32_resize((UINT32_T*)pix2, rszW, rszH, (UINT32_T*)pix_, w_, h_, opts_.rszType, pixBpp_ > 24);
            w_ = rszW, h_ = rszH;
            freeE(pix_);
            pix_ = pix2;
        }
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// フィルタ (ぼかし)
void ConvOne::filter() {
    if (opts_.filterType && pixBpp_ == 32) {
        if (opts_.filterType == 1) {    // 簡易ぼかし.
            if (opts_.bokashiCnt) {
                int i;
                if (varbose_) printf("->bokashi1[%d]", opts_.bokashiCnt);
                for (i = 0; i < opts_.bokashiCnt; ++i) {
                    UINT8_T *pix2 = (UINT8_T*)callocE(1, w_*h_*4);
                    //pix32_bokashi1((UINT32_T*)pix2, (UINT32_T*)pix_, w_, h_);
                    pix32_bokashi((UINT32_T*)pix2, (UINT32_T*)pix_, w_, h_);
                    freeE( pix_ );
                    pix_ = pix2;
                }
            }

        } else if (opts_.filterType == 2) { // 通常画像とぼかし画像を合成.
            if (opts_.bokashiCnt) {
                //if (varbose_) printf("->bokashi2[%d,%d%%]", opts_.bokashiCnt, (int)(opts_.bokashiMergeRate*100));
                if (varbose_) printf("->bokashi2[%d,%x]", opts_.bokashiCnt, (int)(opts_.bokashiMergeRateRGB));
                UINT32_T *pix0 = NULL;
                UINT32_T *pix1 = (UINT32_T*)pix_;
                UINT32_T *pix2 = (UINT32_T*)pix_;
                int i;
                if (opts_.bokashiMaskGenCol1 == 0 && opts_.bokashiMaskGenCol2 == 0xffffff) {
                } else {
                    pix1 = pix0 = (UINT32_T*)callocE(1, w_*h_*4);
                    // 2色の指定から、各r,g,b の値がその範囲に収まったピクセルのみをα=0xffにした画像を作る.
                    pix32_genCol2MaskAlp(pix0, (UINT32_T*)pix_, w_, h_, opts_.bokashiMaskGenCol1, opts_.bokashiMaskGenCol2);
                }
                for (i = 0; i < opts_.bokashiCnt; ++i) {
                    pix2 = (UINT32_T*)callocE(1, w_*h_*4);
                    //pix32_bokashi1(pix2, pix1, w_, h_);
                    pix32_bokashi(pix2, pix1, w_, h_);
                    if (pix0) freeE( pix0 );
                    pix1 = pix2;
                    pix0 = pix2;
                }
                UINT32_T *pix3 = (UINT32_T*)callocE(1, w_*h_*4);
                // 元画像pix_ と ぼかし画像pix2 をpix2側の割合指定(0～1.0)で合成してpix3を作成.
                if (opts_.bokashiMaskGenCol1 == 0 && opts_.bokashiMaskGenCol2 == 0xffffff) {
                    //x pix32_merge((UINT32_T*)pix3, (UINT32_T*)pix_, (UINT32_T*)pix2, w_, h_, opts_.bokashiMergeRate);
                    pix32_merge(pix3, (UINT32_T*)pix_, pix2, w_, h_, opts_.bokashiMergeRateRGB);
                } else {
                    pix32_mergeSrc2Alp(pix3, (UINT32_T*)pix_, pix2, w_, h_, opts_.bokashiMergeRateRGB);
                }
                freeE(pix_);
                freeE(pix2);
                pix_ = (UINT8_T*)pix3;
            }
        } else if (opts_.filterType == 3) { // 簡易シャープ.
            if (opts_.bokashiCnt) {
                int i;
                if (varbose_) printf("->sharp[%d]", opts_.bokashiCnt);
                for (i = 0; i < opts_.bokashiCnt; ++i) {
                    UINT8_T *pix2 = (UINT8_T*)callocE(1, w_*h_*4);
                    pix32_sharp((UINT32_T*)pix2, (UINT32_T*)pix_, w_, h_);
                    freeE( pix_ );
                    pix_ = pix2;
                }
            }
        } else if (opts_.filterType == 4) { // alp 0..254 のピクセル付近をぼかす(255はぼかさない)
            if (opts_.bokashiCnt) {
                int i;
                if (varbose_) printf("->bokashi(a<=%d)[%d]", opts_.bokashiAlpSikii, opts_.bokashiCnt);
                for (i = 0; i < opts_.bokashiCnt; ++i) {
                    UINT8_T *pix2 = (UINT8_T*)callocE(1, w_*h_*4);
                    //pix32_bokashi1((UINT32_T*)pix2, (UINT32_T*)pix_, w_, h_);
                    pix32_bokashiAlpLtEqThreshold((UINT32_T*)pix2, (UINT32_T*)pix_, w_, h_, opts_.bokashiAlpSikii-1);
                    freeE( pix_ );
                    pix_ = pix2;
                }
            }

        }
    }
}


/// 縦横拡縮サイズ変更  2回目.
void ConvOne::resizeImage2nd() {
    if (opts_.rszN >= 2) {  // 縦横拡縮サイズ変更   2回目.
        int     rszW;
        int     rszH;
        UINT8_T *pix2;
        int     c = 1;
        // for (c = 0; c < opts_.rszN; c++)
        {
            if (opts_.rszXsz[c] || opts_.rszYsz[c]) {   //rszXsz,rszYszを使う場合.
                rszW  = (opts_.rszXsz[c]) ? opts_.rszXsz[c] : w_ * opts_.rszYsz[c] / h_;
                rszH  = (opts_.rszYsz[c]) ? opts_.rszYsz[c] : h_ * opts_.rszXsz[c] / w_;
                srcW_ = (int)((double)srcW_ * (double)rszW / (double)w_);
                srcH_ = (int)((double)srcH_ * (double)rszH / (double)h_);
            } else {
                double rszXpar = opts_.rszXpar[c];
                double rszYpar = opts_.rszYpar[c];
                if (rszXpar == 0)
                    rszXpar = 100.0;
                if (rszYpar == 0)
                    rszYpar = rszXpar;
                rszW  = (int)(w_    * rszXpar / 100.0);
                rszH  = (int)(h_    * rszYpar / 100.0);
                srcW_ = (int)(srcW_ * rszXpar / 100.0);
                srcH_ = (int)(srcH_ * rszXpar / 100.0);
            }

            if (varbose_) printf("->rsz[%d,%d]", rszW,rszH);
            if (rszW <= 0 || rszH <= 0)
                printf("拡縮後のサイズがおかしくなる(%d,%d)\n", rszW, rszH);
            pix2 = (UINT8_T*)callocE(1, 4 * ((rszW+15)&~15)*(rszH+1) + 256);
            if (pixBpp_ == 8)
                printf("CLUT付画像のままでは拡縮できない(-ifで多色化できます)\n");
            else
                pix32_resize((UINT32_T*)pix2, rszW, rszH, (UINT32_T*)pix_, w_, h_, opts_.rszType, pixBpp_ > 24);
            w_ = rszW;
            h_ = rszH;
            freeE(pix_);
            pix_ = pix2;
        }
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// 抜き色|αで範囲を求めてサイズ変更.
void ConvOne::aptRect() {
    if (opts_.nukiRctFlg) { // 抜き色|αで範囲を求めてサイズ変更.
        int gridMd = (opts_.nukiRctFlg & 0xF) - 1;
        gridMd |= ((opts_.nukiRctFlg>>4) & 3) << 1;     // 0ならwhとも 2(0x10)ならw_のみ 4(0x20)ならh_のみ.
        int x0 = 0;
        int y0 = 0;
        if (pixBpp_ == 8) gen_nukiRect8(&pix_, &w_, &h_, &x0, &y0, bo_->celW, bo_->celH, nukiClut_, gridMd);
        else             gen_nukiRect32(&pix_, &w_, &h_, &x0, &y0, bo_->celW, bo_->celH, gridMd);
        if (varbose_) printf("->(%d,%d)[%d*%d]",x0,y0,w_,h_);
        bo_->x0 += x0;
        bo_->y0 += y0;

        pixWb_ = WID2BYT(w_, pixBpp_);
        //* srcW_, srcH_ どうする?
    }
}

/// パターンディザを施す.
void ConvOne::patternDither() {
    if ((opts_.ditBpp || opts_.ditTyp) && pixBpp_ == 32) {    // ディザを施す.
        int ditBpp = opts_.ditBpp;
        if (ditBpp <= 0) {              // デフォルトの色のビット数を出力に合わせて選ぶ.
            if (mono_) {
                ditBpp  = (dstBpp_ <=  8) ? dstBpp_*3
                        : (dstBpp_ <  12) ? 3*3
                        : (dstBpp_ <= 12) ? 4*3
                        : (dstBpp_ <= 15) ? 5*3
                        : (dstBpp_ <= 18) ? 6*3
                        :                   8*3;
            } else if (opts_.isFixedClut()) {  // jp or win 固定clutの場合.
                ditBpp  = (dstBpp_ <   3) ? 15
                        : (dstBpp_ <=  6) ?  6
                        : (dstBpp_ <=  8) ? 15
                        : (dstBpp_ <=  9) ?  9
                        : (dstBpp_ <= 12) ? 12
                        : (dstBpp_ == 16) ? 16
                        :                   15;
            } else {
                ditBpp  = (dstBpp_ <   3) ? 15
                        : (dstBpp_ <=  9) ? dstBpp_
                        : (dstBpp_ <= 12) ? 12
                        : (dstBpp_ == 16) ? 16
                        :                   15;
            }
        }
        int dbpp = (8 < dstBpp_ && dstBpp_ < ditBpp) ? dstBpp_ : ditBpp;
        if (opts_.ditTyp & 0xC000) {
            errorDiffusion1b(dbpp);
        } else {
        if (varbose_) printf("->dit%d:0x%x", dbpp,opts_.ditTyp);
            PaternDither    paternDither;
            paternDither.conv((UINT32_T*)pix_, (UINT32_T*)pix_, w_, h_, dbpp, opts_.ditTyp | (opts_.ditAlpFlg << 8));
        }
    }
}

/// 誤差拡散.
void ConvOne::errorDiffusion1b(int dpp) {
    // RGBを何階調にするか.
    unsigned const toneSizeTbl[][3] = {
        // r   g   b
        {  2,  2,  2, },    // 0 :  3bit r1g1b1     8色.
        {  3,  3,  3, },    // 1 :  4bit           27色.
        {  4,  4,  4, },    // 2 :  6bit r2g2b2    64色.
        {  5,  5,  5, },    // 3 :  7bit          125色.
        {  6,  6,  6, },    // 4 :  8bit          216色. sp,(xterm)
        {  8,  8,  4, },    // 5 :  8bit r3g3b2   256色.
        {  8,  8,  8, },    // 6 :  9bit r3g3b3   512色.
        { 16, 16, 16, },    // 7 : 12bit r4g4b4  4096色.
        { 32, 32, 32, },    // 8 : 15bit r5g5b5 32768色.
        { 32, 64, 32, },    // 9 : 16bit r5g6b5 65536色.
        { 64, 64, 64, },    //10 : 18bit r6g6b6 262144色.
        {  7,  9,  4, },    //11 :  8bit         252色. win clut
        {  7,  7,  7, },    //12 :  9bit
    };
    bool noErrDif = (opts_.ditTyp & 0x8000) == 0;
//noErrDif = true;
    int ditType  = opts_.ditTyp & 3;
    int toneType;
    bool mono = mono_; //|| opts_.mono;
    if (mono) {
        toneType = (dstBpp_ <=  1 || colNum_ <= 2) ? 0
                 : (                 colNum_ <= 3) ? 1
                 : (dstBpp_ <=  2 || colNum_ <= 4) ? 2
                 : (                 colNum_ <= 5) ? 3
                 : (                 colNum_ <= 6) ? 4
                 : (                 colNum_ <= 7) ? 12
                 : (dstBpp_ <=  3 || colNum_ <= 8) ? 6
                 : (dstBpp_ <=  4) ? 7
                 : (dstBpp_ <=  5) ? 8
                 :                  10;
        ditType |= 0x100;
    } else {
        toneType = (dstBpp_ <=  5 || colNum_ <= 32) ? 0
                 : (dstBpp_ <=  6 || colNum_ <= 64) ? 2
                 : (dstBpp_ <=  7 || colNum_ <= 128) ? 3
                 : (dstBpp_ <=  8) ? 5
                 : (dstBpp_ <= 11) ? 6
                 : (dstBpp_ <= 12) ? 7
                 : (dstBpp_ <= 15) ? 8
                 :                   9;
    }
    if (toneType == 5) {
        switch (opts_.decreaseColorMode) {
        //case DCM_FIX_JP:      toneType = 5; break;
        case DCM_FIX_WIN:       toneType =11; break;
        case DCM_FIX_XTERM:     toneType = 4; ditType |= 0x10; break;
        case DCM_FIX_G6R6B6C40: toneType = 4; break;
        default: break;
        }
    }
    if (noErrDif)
        ditType |= 0x1000;

    unsigned const* toneSize = toneSizeTbl[toneType];
    ErrorDiffusion ed;

	bool digColorMode = (opts_.decreaseColorMode == DCM_FIX_JP) || (opts_.decreaseColorMode == DCM_FIX_WIN);
    if (digColorMode && dstBpp_ <= 4) {
        unsigned colNum  = 1 << dstBpp_;
        if (colNum > opts_.colNum && opts_.colNum)
            colNum = opts_.colNum;
        unsigned monoCol = opts_.monoCol;
        if ((monoCol&0xffffff) == 0)
            monoCol = 0xffffffff;
		if (dstBpp_ <= 3) {
		    ed.convDigital8( (UINT32_T*)pix_, (UINT32_T*)pix_, w_, h_, ditType, colNum, monoCol);
		} else {
		    ed.convDigital16((UINT32_T*)pix_, (UINT32_T*)pix_, w_, h_, ditType, colNum, monoCol);
		}
    } else {
	    ed.conv((UINT32_T*)pix_, (UINT32_T*)pix_, w_, h_, ditType, toneSize, NULL);
	}
}

/// 指定色とαをブレンドし、αを0 or 255 にする.
void ConvOne::alphaBlendByColor() {
    int col = opts_.alpToCol;
    if (col >= 0) {
        if (pixBpp_ == 8) pix32_alphaBlendByColor(clut_,  1, 256, col, opts_.nukiumeRgb, opts_.alpMin, opts_.alpMax);
        else              pix32_alphaBlendByColor((UINT32_T*)pix_ , w_,  h_, col, opts_.nukiumeRgb, opts_.alpMin, opts_.alpMax);
        if (varbose_) printf("->alpToCol(%06x)",col);
    }
}


/// 内部 多色で出力がclut_付きなら、安易減色を行う.
void ConvOne::decreaseColor() {
    if (pixBpp_ > 8 && dstBpp_ <= 8) {
        UINT8_T *p       = (UINT8_T*)callocE(w_ , 2+h_);
        int     clutSize = 1 << dstBpp_;
        int     alpNum   = opts_.clutAlpNum;
        bool    alpFlg   = opts_.clutAlpFlg != 0;
        int     dcm      = int(opts_.decreaseColorMode);
        int     decrType = opts_.decreaseColorMode2;
        int     colNum   = opts_.colNum;
        memset(clut_, 0, sizeof clut_);
        if (colNum < 2 || colNum > clutSize)
            colNum = clutSize;

        if (opts_.alpBitForBpp8 == 0) {
            switch (dcm) {
            case DCM_FIX_JP:        //0 日本の80年代パソコン由来の 16色,256色(G3R3B2)固定パレット.
            case DCM_FIX_WIN:       //1 固定 Winシステムパレット.
            case DCM_FIX_XTERM:     //2 固定 xterm256.
            case DCM_FIX_G6R6B6C40: //3 固定 rgb 6*6*6+40.
            //case DCM_FIX_OTAMESHI1: //4 お試しパレットO
            //case DCM_FIX_OTAMESHI2: //5 お試しパレットP
                {
                    int md = int(dcm);
                    if (md >= 7) md = md - 7 + 3;
                    --md;
                    if (varbose_) {
                        static char const tbl[] = "JWXVOP";
                        printf("->DftlClt%c%d", tbl[md], dstBpp_);
                    }
                    FixedClut256<>::getFixedClut256(clut_, 256, dstBpp_, md);
                    //if (alpNum)
                    //  clut_[0] &= 0xFFFFFF;
                    bool mono = mono_ || opts_.mono;
                    if (dstBpp_ == 3 && (decrType == 2 /*|| (mono && colNum <= 4)*/)) {
                        //colNum = 8;
                        FixedClut256<>::decreaseColorRGB111(p, (UINT32_T*)pix_, w_, h_, (md == 1));
                    } else {
                        colNum = 1 << dstBpp_;
                        unsigned  idx  = 0;
                        if (dstBpp_ == 7) {         // 128色のとき.
                            idx = 1;                // 先頭の抜き色スキップ.
                            colNum = 5 * 5 * 5;     // 各5階調.
                        } else if (md == 2) {       // xterm256 システム16色をスキップ.
                            if (colNum == 216) {    // 6*6*6 各6階調.
                                idx = 16;
                            } else {
                                idx = 1;            // (0,0,0)は２つあるので先頭のを抜き色扱いにしておく.
                                if (colNum == 256)
                                    --colNum;
                            }
                        } else if (md == 3) {   // b5r5b5c40    先頭に抜き色１色あるのを飛ばす.
                            if (colNum == 216) {    // 6*6*6 各6階調.
                                idx = 1;
                            }
                        }
                        FixedClut256<>::decreaseColor(p, (UINT32_T*)pix_, w_, h_, clut_, colNum+idx, idx, 0, 0, decrType);
                    }
                }
                break;

            default:    // 範囲外ならとりあえず、メディアンカット(yuv)へ.
            //case 0:
                dcm = DCM_UNKOWN;
                // 続く.
            case DCM_MC_YUV: // メディアンカット(yuv)
            case DCM_MC_RGB: // メディアンカット(rgb)
            case DCM_HIST:   // 頻度順 clut
                if (alpFlg == 0 && alpNum < 0 && colNum >= (1 << dstBpp_) && (mono_ || GrayClut<>::isGrey((UINT32_T*)pix_, w_, h_))) {
                    // モノクロ画像専用の減色.
                    if (dstBpp_ > 4 && colNum >= 256) {
                        GrayClut<>::getFixedGreyClut(clut_, 256, 8);
                        GrayClut<>::fromGreyToBpp8(p, (UINT32_T*)pix_, w_, h_);
                    } else if (dstBpp_ >= 3) {
                        GrayClut<>::fromGreyToBpp4Clut(p, (UINT32_T*)pix_, w_, h_, clut_);
                    } else if (dstBpp_ >= 2) {
                        GrayClut<>::fromGreyToBpp2Clut(p, (UINT32_T*)pix_, w_, h_, clut_);
                    } else {
                        GrayClut<>::fromGreyToBpp1Clut(p, (UINT32_T*)pix_, w_, h_, clut_);
                    }
                    if (varbose_)
                    printf("->cltGry%d", 1 << dstBpp_);
                } else if (DecreaseColorIfWithin256<>::conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum, alpFlg)) {
                    // 32ビット色画が、もとよりclutNum色以内なら、そのまま変換.
                    if (varbose_)
                        printf("->clt%d", dstBpp_);
                } else if ((dstBpp_ == 1 || dstBpp_ == 2) && colNum >= (1<<dstBpp_)) {
                    // 1bit色,2bit色専用の減色処理.
                    if (dstBpp_ == 1)
                        DecreaseColorLowBpp<>::convPix32ToBpp1(p, (UINT32_T*)pix_, w_, h_, clut_);
                    else
                        DecreaseColorLowBpp<>::convPix32ToBpp2(p, (UINT32_T*)pix_, w_, h_, clut_);
                    if (varbose_)
                        printf("->cltBpp%d", 1 << dstBpp_);
                } else if (dcm == DCM_HIST) {   // 頻度順 clut で減色.
                    //int a = (opts_.alpMin >= 0) ? opts_.alpMin : 4;
                    DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, clut_, colNum, alpNum);
                    if (varbose_) {
                        printf("->decreaseCol%d", dstBpp_);
                    }
                } else {            // 256色より多いので要減色.
                    if (dcm == DCM_UNKOWN)
                        dcm = DCM_MC_YUV;
                    memset(clut_, 0, sizeof clut_);
                    // メディアンカットな減色.
                    DecreaseColorMC     rcmc;
                    rcmc.setModeRGB(dcm == DCM_MC_RGB);      // rgbかyuvかの設定.
                    if (opts_.decreaseColorParam[0] >= 0) { // 暫定的なぱらめーた設定.
                        rcmc.setCalcMidParam(
                            (int  )(opts_.decreaseColorParam[0]),
                            (float)(opts_.decreaseColorParam[1]),
                            (int  )(opts_.decreaseColorParam[2]),
                            (float)(opts_.decreaseColorParam[3])
                        );
                    }
                    // αパラメータの設定.(α数, αbitを用いる場合のbit数, αの最小値, αの最大値)
                    rcmc.setAlphaParam(alpNum, 0, opts_.alpMin, opts_.alpMax);
                    // コンバート.
                    rcmc.conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);

                    if (varbose_)
                        printf("->decreaseColMC%s%d", (dcm == DCM_MC_YUV) ?"yuv":"rgb", dstBpp_);
                }
                break;

            case DCM_FIX_FILE:
                if (varbose_) {
                    printf("->fixedClut%d", dstBpp_);
                }
                //int a = (opts_.alpMin >= 0) ? opts_.alpMin : 4;
                DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, opts_.fixedClut_, colNum, alpNum);
                break;
            }

        } else {    // a3i5,a5i3の専用処理.
            UINT8_T* alpPln = 0;
            unsigned idxBit = 8 - opts_.alpBitForBpp8;
            unsigned cnum   = 1 << idxBit;
            if (colNum <= 0 || unsigned(colNum) > cnum)
                colNum = cnum;

            alpPln = (UINT8_T*)callocE(1,w_*h_);
            unsigned whsize = w_ * h_;
            for (unsigned i = 0; i < whsize; ++i) {
                unsigned a = ((UINT32_T*)pix_)[i] >> 24;
                a          = ( a + (cnum-1)/2);
                if (a > 255)
                    a = 255;
                alpPln[i]  = a;
            }

            pix32_clearAlpha((UINT32_T*)pix_, w_, h_, 0xff);

            // 32ビット色画が、もとよりclutNum色以内なら、そのまま変換.
            if (DecreaseColorIfWithin256<>::conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum, 0/*alpFlg*/)) {
                if (varbose_)
                    printf("->cltA%dI%d", opts_.alpBitForBpp8, idxBit);

            } else {            // colNum色より多いので要減色.
                memset(clut_, 0, sizeof clut_);
                if (dcm == DCM_HIST) {
                    if (varbose_)
                        printf("->decreaseColA%d", dstBpp_);
                    DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);
                } else {
                    if (varbose_)
                        printf("->decreaseCol_A%dI%d", opts_.alpBitForBpp8, idxBit);
                    // メディアンカットな減色.
                    DecreaseColorMC     rcmc;
                    rcmc.setModeRGB(0/*dcm == 4*/);          // rgbかyuvかの設定.
                    if (opts_.decreaseColorParam[0] >= 0) { // 暫定的なぱらめーた設定.
                        rcmc.setCalcMidParam(
                            (int  )(opts_.decreaseColorParam[0]),
                            (float)(opts_.decreaseColorParam[1]),
                            (float)(opts_.decreaseColorParam[2]),
                            (float)(opts_.decreaseColorParam[3])
                        );
                    }
                    // αパラメータの設定.(α数, αbitを用いる場合のbit数, αの最小値, αの最大値)
                    rcmc.setAlphaParam(0/*alpNum*/, 0/*opts_.alpBitForBpp8*/, opts_.alpMin, opts_.alpMax);
                    // コンバート.
                    rcmc.conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);
                }
            }
            whsize = w_ * h_;
            for (unsigned i = 0; i < whsize; ++i) {
                unsigned a = alpPln[i] >> idxBit;
                p[i] |= a << idxBit;
            }
            alpNum = 1 << opts_.alpBitForBpp8;
            assert(alpNum > 1);
            for (unsigned i = 0; i < (unsigned)alpNum; ++i) {
                unsigned  a = i * 255 / (alpNum-1);
                for (unsigned j = 0; j < cnum; ++j) {
                    clut_[i*cnum+j] = (a<<24) | (clut_[j] & 0xffffff);
                }
            }
            freeE(alpPln);
        }
        freeE(pix_);

        pix_    = p;
        pixBpp_ = 8;    //dstBpp_;
        pixWb_  = WID2BYT(w_, pixBpp_);
    }
}


/// チップ(セル)＆マップ化.
void ConvOne::changeChipAndMap() {
    if (opts_.mapMode && bo_->celW && bo_->celH) {      // マップ化.
        int     celNum;
        int     styl  = opts_.celStyl | (opts_.mapNoCmp << 1) | (opts_.mapEx256x256<<3);    // 1bit|2bit|1bit
        int     f     = 1;
      #ifdef MY_H
        f     |= MY_CHANGE_CHP_AND_MAP_APPEND_FLAG(opts_.dstFmt);
      #endif
        if (opts_.mapMode == 3) {
            if      (w_ <=   64 && h_ <= 1024) bo_->celW = bo_->mapTexW =   64, bo_->celH = bo_->mapTexH = 1024;
            else if (w_ <=  128 && h_ <=  512) bo_->celW = bo_->mapTexW =  128, bo_->celH = bo_->mapTexH =  512;
            else if (w_ <=  256 && h_ <=  256) bo_->celW = bo_->mapTexW =  256, bo_->celH = bo_->mapTexH =  256;
            else if (w_ <=  512 && h_ <=  128) bo_->celW = bo_->mapTexW =  512, bo_->celH = bo_->mapTexH =  128;
            else if (w_ <=  512 && h_ <=  128) bo_->celW = bo_->mapTexW = 1024, bo_->celH = bo_->mapTexH =   64;
        }
        if (varbose_) {
            if (bo_->celW == bo_->mapTexW && bo_->celH == bo_->mapTexH) {
                printf("->1X1mapTEX(%d*%d)", bo_->celW, bo_->celH);
            } else {
                printf("->map_(%d*%d)", w_/bo_->celW, h_/bo_->celH);
            }
        }
        //gen_clmp_setMinY(opts_.mapMode3_y_min);
        if (pixBpp_ == 8) {
            gen_clmp8 (&map_, &mapSz_, &celNum, &pix_, &w_, &h_, bo_->x0, bo_->y0
                , bo_->mapTexW, bo_->mapTexH, bo_->celW, bo_->celH, f, dstBpp_
                , nukiClut_, styl, clut_, srcW_, srcH_);
        } else {
            gen_clmp32(&map_,&mapSz_, &celNum, &pix_, &w_, &h_, bo_->x0, bo_->y0
                , bo_->mapTexW, bo_->mapTexH, bo_->celW, bo_->celH, f, dstBpp_
                , 0        , styl, NULL , srcW_, srcH_);
        }
        if (varbose_) {
            if (bo_->mapTexH > 0)
                printf("[%d:%d*%d::%d]", celNum, w_,h_, (h_+bo_->mapTexH-1)/bo_->mapTexH);
            else
                printf("[%d:%d*%d]", celNum, w_,h_);
        }
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// 出力のαを反転する必要があるとき..
void ConvOne::reverseOutputAlpha() {
    if (opts_.alpModeO) {       // 出力のαを反転する必要があるとき..
        if (varbose_) printf("->α反.");
        if (pixBpp_ == 8) pix32_revAlpha(clut_, 1, 256);
        else              pix32_revAlpha((UINT32_T*)pix_, w_, h_);
    }
}


/// マスク画像生成のとき.
///
void ConvOne::changeMaskImage() {
    if (opts_.genMaskFlg) {
        if (varbose_) printf("->mask化.");
        if (pixBpp_ == 8) {
            //pix32_alpToMask(clut_, 1, 256, dstBpp_);
            int m = 8 - dstBpp_;
            if (m < 0)
                m = 0;
            for (int n = 0; n < w_*h_; n++) {
                int c = (clut_[pix_[n]] >> 24);
                c     = (c + (1<<m)-1) >> m;
                pix_[n] = (UINT8_T)c;
            }
            for (int n = 0; n < (1<<dstBpp_); n++) {
                int c    = n << (8-dstBpp_);
                clut_[n] = ARGB(c,c,c,c);
            }
        } else {
            pix32_alphaToRGB((UINT32_T*)pix_, w_, h_);
            //pix32_bppChg(pix_, w_, h_, dstBpp_);
        }
    }
}


/// 出力ファイルイメージを生成..
bool ConvOne::saveImage() {
    bool     rc   = true;
    uint8_t* pix2 = NULL;
    // 出力準備.
    // 減色の色数のためにdstBpp_を流用して値が範囲外の場合があるので、強引に辻褄合わせ.
    dstBpp_ = bm_chkDstBpp(opts_.dstFmt, dstBpp_);

    // 出力BPP指定ない場合、24bit画なら可能ならclut画にする(サイズ縮小のため. 32bit画(alpha付)のclut画フォーマットは微妙なので回避.
    int dstFmt = opts_.dstFmt;
    if (opts_.bpp == 0 && pixBpp_ == 32) {
        if (dstBpp_ == 24 && (dstFmt == BM_FMT_PNG || dstFmt == BM_FMT_TGA || dstFmt == BM_FMT_BMP
            #ifdef MY_H
                            || MY_IS_AUTO_BPP8_FMT(dstFmt)
            #endif
        )) {
            unsigned clutSize = 256;
            // モノラル化済みの場合.
            if (/*mono_ ||*/ GrayClut<>::isGrey((uint32_t const*)pix_, w_, h_)) {
                if (varbose_) printf("->auto-mono");
                pix2 = new uint8_t[w_ * h_ + 16];
                GrayClut<>::getFixedGreyClut(clut_, 256, 8);
                GrayClut<>::fromGreyToBpp8(pix2, (uint32_t const*)pix_, w_, h_);
                mono_ = true;
            } else {    // 色数が 256以下なら clut画に変換.
                pix2 = DecreaseColorIfWithin256<>::convToNewArray((uint32_t*)pix_, w_, h_, clut_, clutSize, false, 0xFFFFFFFF);
                if (pix2 && varbose_) {
                    if (clutSize <= 16)
                        printf("->auto-16");
                    else
                        printf("->auto-256");
                }
            }
            if (pix2) { // 変換成功時.
                freeE(pix_);
                dstBpp_  = (clutSize <= 16 && dstFmt != BM_FMT_TGA) ? 4 : 8;
                pix_     = pix2;
                pixBpp_  = 8;
                dstColN_ = 1 << dstBpp_;
                pixWb_   = w_;
            }
        } else if (dstFmt == BM_FMT_JPG || dstFmt == BM_FMT_PNG) {
            if (GrayClut<>::isGrey((uint32_t*)pix_, w_, h_))
                mono_ = true;
            //else if (dstFmt == BM_FMT_PNG && GrayClut<>::isGreyRGB((uint32_t*)pix_, w_, h_))
            //  mono_ = true;
        }
    }

    bo_->mono = mono_;

    if (opts_.clutTxtName && dstBpp_ <= 8) {
        clutTxtSave(opts_.clutTxtName, dstName_, clut_, dstColN_);
    }

    // バッファ確保.
    int c = bm_encodeWorkSize(dstFmt, w_, h_, dstBpp_);
    dst_  = (UINT8_T*)calloc(1, c);
    if (dst_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s 出力のためのメモリを確保できません.\n", dstName_);
        term();
        rc = false;
        goto RET;
    }

    // 出力画像を生成.
    if (dstFmt != BM_FMT_NON) {
        int clutAlpFlg = opts_.clutAlpFlg;
        if (clutAlpFlg == 0 && pixBpp_ == 8 && dstBpp_ <= 8) {
            clutAlpFlg = pix32_isUseAlpha(clut_, 1<<dstBpp_, 1);
        }
        int dir_flgs = 0;
        dir_flgs    |= opts_.encMode    << BM_FLAG_EX_ENC_SH;                                               // 特別の圧縮をするか.
        dir_flgs    |= clutAlpFlg       << BM_FLAG_CLUT_ALP_SH;                                             // alpha付clutにするか.
        dir_flgs    |= (opts_.colKeyNA == 0 && (opts_.colKey >= 0 || nukiClut_ >= 0)) << BM_FLAG_COLKEY_SH; // 抜き色があるか.
        if (dstFmt == BM_FMT_JPG) {
            printf("->JpgQ=%2d", ((bo_->mono && bo_->quality_grey >= 0) ? bo_->quality_grey : bo_->quality));
        }
        bo_->clutNum = dstColN_;
        bo_->nukiCo  = -1;
        bo_->nukiCoI = -1;
        if (!opts_.colKeyNA) {
            if (dstBpp_ <= 8 && nukiClut_ >= 0 && nukiClut_ < (1 << dstBpp_)) {
                bo_->nukiCoI = nukiClut_;
            } else if (dstBpp_ > 8 && opts_.colKey != -1) {
                bo_->nukiCo  = opts_.colKey & 0xFFFFFF;
            }
        }
        sz_   = bm_write(dstFmt, dst_, w_, h_, dstBpp_, pix_, pixWb_, pixBpp_, clut_, dir_flgs, bo_);
        if (sz_ <= 0) {
            if (varbose_) printf("\n");
            //x printf("%s を %s に変換中にエラーがありました\n", srcName_, opts_.dstExt);
            printf("%s を 変換中にエラーがありました.\n", srcName_);
            term();
            rc = false;
            goto RET;
        }
    }

 RET:
    if (pix2 && pix_ == pix2)
        delete[] pix2;
    else
        free(pix_);
    pix_ = NULL;
    return rc;
}


/// ファイル書き込み.
int ConvOne::writeFile() {
    if (opts_.saveInfFile) {    // 画像情報のテキストファイルを作成.
        saveInfFile(dstName_, w_, h_, dstBpp_, opts_.mapMode, bo_);
    }

    if (opts_.dstFmt != BM_FMT_NON) {
        if (opts_.mapMode >= 2 && map_) {   // テクスチャ内蔵マップファイル生成の場合.
            saveMapFile(dstName_, opts_.exDstExt, map_, mapSz_, dst_, sz_);

        } else {
            if (map_) {                 // マップ情報ファイルのみの生成.
                saveMapFile(dstName_, NULL, map_, mapSz_, NULL, 0);
            }

            // ファイル生成.
            if (strlen(dstName_) < FIL_NMSZ-4) {        // すでにファイルがあった場合、.bak生成.
                char            tmpName[FIL_NMSZ];
                sprintf(tmpName,"%s.bak", dstName_);
                remove(tmpName);
                rename(dstName_, tmpName);
            }
            if (fil_save(dstName_, dst_, sz_) == NULL) {
                printf("%s の書き込みに失敗しました.\a\n", dstName_);
            }
        }
    }
    return 1;
}

/// 終了処理.
void ConvOne::term() {
    if (map_) {
        freeE(map_);
        map_ = NULL;
    }
    if (dst_) {
        freeE(dst_);
        dst_ = NULL;
    }

    if (varbose_) printf("\n");
}


// -------------------------------------------------------------------

/// マップファイル生成.
void ConvOne::saveMapFile(const char *name, const char *ext, UINT8_T *map, int mapSz, UINT8_T *pix, int pixSz) {
    char nm[FIL_NMSZ];
    char tmpName[FIL_NMSZ];

    //if (ext == NULL)
    //  ext = "mp";
    fname_chgExt(strcpy(nm, name), (pix) ?ext:"mh");
    if (strlen(nm) < FIL_NMSZ-4) {
        sprintf(tmpName,"%s.bak", nm);
        remove(tmpName);
        rename(nm, tmpName);
    }

    if (pix == NULL) {  // ヘッダのみのとき.
        if (fil_save(nm, map, mapSz) == NULL) {
            printf("%s の書き込みに失敗しました.\a\n", nm);
        }
    } else {            // 画像も一緒に納めるとき.
        FILE *fp = fopenE(nm, "wb");
        mapSz = (mapSz + 31) & ~31;
        POKEiD(map+4, mapSz);           // 画像へのオフセットを設定.
      #if 1
        {   UINT8_T *m = (UINT8_T*)mallocE(mapSz+pixSz);
            memcpy(m, map, mapSz);
            memcpy(m+mapSz, pix, pixSz);
          #ifdef MY_H
            MY_saveMapFile_subr();
          #endif
            fwriteE(m, 1, mapSz+pixSz, fp);
            freeE(m);
        }
      #else
        fwriteE(map, 1, mapSz, fp);
        fwriteE(pix, 1, pixSz, fp);
      #endif
        fcloseE(fp);
    }
    //x freeE(map);
}


/// 画像情報テキストファイル出力.
void ConvOne::saveInfFile(const char *oname, int w, int h, int bpp, int mapMode, bm_opt_t *bo) {
    FILE *fp;
    char nm[FIL_NMSZ];

    strcpy(nm, oname);
    fname_chgExt(nm, "t");
    fp = fopenE(nm, "wt");
    fprintf(fp,"FILE=%s\n", oname);
    fprintf(fp,"SIZE=%d,%d\n", w, h);
    fprintf(fp,"BPP=%d\n", bpp);
    fprintf(fp,"POS=%d,%d\n", bo->x0, bo->y0);
    if (bo->srcW != w || bo->srcH != h)
        fprintf(fp,"WSIZE=%d,%d\n", bo->srcW, bo->srcH);
    if (bo->celW) {
        fprintf(fp,"CELSIZE=%d,%d\n", bo->celW, bo->celH);
        if (mapMode) {  // celWが>0ならcelHも必ず>0
            fprintf(fp,"MAPSIZE=%d,%d\n", w/bo->celW, h/bo->celH);
            if (mapMode == 3 && bo->celW == bo->mapTexW && bo->celH == bo->mapTexH) {
                fprintf(fp,"1X1MAP_TEXTURE\n");
            }
        }
    }
    fclose(fp);
}


/// clutテキストの出力.
void ConvOne::clutTxtSave(const char *clutTxtName, const char *dstName, unsigned *clut, int dstColN) {
    char name[FIL_NMSZ];
    FILE *fp;
    int i;

    if (clutTxtName[0]) {
        strcpy(name, clutTxtName);
    } else {
        strcpy(name, dstName);
        fname_chgExt(name, "c_t");
    }
    fp = fopen(name,"wt");
    if (fp) {
        for (i = 0; i < dstColN; i++) {
            fprintf(fp,"\t0x%08x,", clut[i]);
            if ((i & 15) == 15)
                fprintf(fp,"\n");
        }
        if (i & 15)
            fprintf(fp,"\n");
        fclose(fp);
    }
}
