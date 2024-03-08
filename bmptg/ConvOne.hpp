/**
 *  @file   ConvOne.h
 *  @brief  画像変換
 *  @author Masashi Kitamura
 *  @date   2000-??-??
 *  @note
 *      2006    bmptg.c から分離.
 */

#ifndef CONVONE_H
#define CONVONE_H


#include "subr.h"
#include "BmImgLoad.h"
#include "BmImgSave.h"


struct ConvOne_Opts {
public:
    int         verbose;

    int         bpp;
    int         dstFmt;

    int         g555val;
    int         alpNon;
    int         alpModeI;
    int         alpModeO;
    int         colKey;
    int         colKeyNA;
    int         clearColIfAlp0;
    int         alpBokasi;
    int         nukiClut;
    int         nukiRctFlg;
    int         genMaskFlg;
    int         tone;
    int         toneType;   // 0=rgb操作, yuv-y操作
    int         encMode;
    int         fullColFlg;
    int         srcBpp;
    int         ditBpp;
    int         ditTyp;
    int         ditAlpFlg;
    int         dir;
    int         rotR90;
    double      rotR;
    struct vv_t {
        int     flg;
        int     lcr;        // 0=left 1=center 2=right
        int     umd;        // 0=up   1=mid    2=down
        int     lcr_ex;     // 0=通常 1=ファイル名本体の最後の一時が奇数なら左詰め(l)偶数なら右詰め(r)にする
        int     w , h , x , y;
        int     wf, hf, xf, yf;
        int     sw, sh, sx, sy;
    };
    vv_t        vv[2];
    int         vvIdx;
    int         mapMode;
    int         mapTexW;
    int         mapTexH;
    int         mapMode3_y_min;
    int         celSzW;
    int         celSzH;
    int         celStyl;
    int         mapStyl;
    int         mapNoCmp;
    int         mapEx256x256;
    int         clutAlpFlg;
    int         clutAlpNum;
    int         clutBpp;
    int         saveInfFile;
    int         mono;
    int         monoNear;           // ほぼグレイならグレイとみなす
    int         monoChRGB;
    int         monoToAlp;
    double      monoToAlp_rate;
    int         monoToAlp_ofs;
    int         colMul;
    int         colChSquare;
    double      pixScale[4];
    int         pixScaleType;
    int         colrot;
    int         bytSkp;
    int         binW,binH,binBpp, binEndian;
    int         lvlY, lvlUV;
    int         quality;
    int         quality_grey;
    int         rszN;       //リサイズ回数
    int         rszK[2];    //拡縮時に用いる係数 1..
    double      rszXpar[2];
    double      rszYpar[2];
    int         rszXsz[2];
    int         rszYsz[2];
    int         rszType;

    int         decreaseColorMode;
    int         decreaseColorMode2;
    double      decreaseColorParam[4];
    int         bitCom;
    int         kizu;
    int         filterType;
    int         bokashiCnt;
    int         bokashiAlpSikii;
    unsigned    bokashiMergeRateRGB;    // R,G,B 別にレートを指定できるようにカラー指定にする
    unsigned    bokashiMaskGenCol1;     // ぼかし合成でのマスクαを生成するための色値1
    unsigned    bokashiMaskGenCol2;     // ぼかし合成でのマスクαを生成するための色値1
    int         colNum;
    int         clutOfs;
    int         clutIndShft;
    int         genAlpEx;               // 左上(0,0)のピクセルを抜き色として、画像の4隅から抜き色のαを塗りつぶす指定.
    char const* clutTxtName;
    char const* exDstExt;               // 拡張子
    unsigned    alpMin;                 // 半透明範囲min
    unsigned    alpMax;                 // 半透明範囲max
    int         alpToCol;               // 半透明(α)min～maxのピクセルに、色alpToColとαブレンドして、α値を0xffにする.
    int         nukiumeRgb;             // 抜き色として埋めるrgb値.
    int         alpBitForBpp8;          // A2I6のようなαにNビット、インデックスにMビット(N+M=8)の256色画像にする. 値はN.
    char const* alphaPlaneFileName;     // αプレーン用の画像を読み込む場合.
    int         startX;
    int         startY;

    enum {CLUT_NUM = 64 * 1024};
    unsigned    fixedClut_[CLUT_NUM];
    int         clutChgFlg[CLUT_NUM];
    int         clutChg[CLUT_NUM];

public:
    ConvOne_Opts();
    // ~ConvOne_Opts() {}

    void readFixedClut(const char* fname);
    bool isFixedClut() const { return decreaseColorMode == 1 || decreaseColorMode == 2 || decreaseColorMode >= 7; }
};




/* ------------------------------------------------------------------------ */
/** 一ファイル変換 */
class ConvOne {
public:
    ConvOne();
    ~ConvOne();

    int run(const char *srcName, const char *dstName);

    ConvOne_Opts& opts() { return opts_; }

private:
    int main();

    void init(const char *srcName, const char *dstName);
    bool readFile();
    bool imageLoad();
    void adjustAlpha();
    void swapARGB();
    void loadAlphaPlaneFile();
    void changeClut();
    void bitCom();
    void revAlpha();
    void monoToAlpha();
    void setClutNukiCol();
    void reflectColKey();
    void easyPaintRoundAlpha();
    void clearColorIfAlpha0();
    void alphaBokashi();
    void resizeCanvas(int n);
    void changeSrcBpp();
    void reverseImage();
    void rotR90(int type);
    void rotateImage();
    void toMono();
    void mulCol();
    void colChSquare();
    void changeTone();
    void convNukiBlack();
    void resizeImage1st();
    void filter();
    void resizeImage2nd();
    void aptRect();
    void patternDither();
	void errorDiffusion1b();
    void alphaBlendByColor();
    void setDstBpp();
    void checkSrcDstBpp();
    void decreaseColor();
    void changeChipAndMap();
    void reverseOutputAlpha();
    void changeMaskImage();
    bool saveImage();
    int  writeFile();
    void term();

private:
    void saveMapFile(const char *name, const char *ext, uint8_t *map, int mapSz, uint8_t *pix, int pixSz);
    void saveInfFile(const char *oname, int w, int h, int bpp, int mapMode, bm_opt_t *bo);
    void clutTxtSave(const char *clutTxtName, const char *dstName, unsigned *clut, int dstColN);

private:
    //enum {CLUT_NUM = 64 * 1024};
    enum {CLUT_NUM = ConvOne_Opts::CLUT_NUM};
    const char*     dstName_;
    const char*     srcName_;
    bm_opt_t*       bo_;

    bool            varbose_;
    bool            fullColFlg_;
    bool            mono_;
    int             sz_;
    int             w_;
    int             h_;
    int             bpp_;
    int             pixBpp_;
    int             pixWb_;
    int             pixH_;
    int             srcFmt_;
    int             dstColN_;
    int             dstBpp_;
    int             colNum_;
    int             mapSz_;
    int             encMode_;
    int             nukiClut_;
    uint8_t*        pix_;
    uint8_t*        map_;
    uint8_t*        src_;
    uint8_t*        dat_;
    uint8_t*        dst_;
    int             srcW_;
    int             srcH_;

    bm_opt_t        bopt_;
    unsigned        clut_[CLUT_NUM * 2];        ///< clut. 本来はCLUT_NUMだが、色数カウントで余分に数える作業領域のため、*2している

    ConvOne_Opts    opts_;
};


#endif
