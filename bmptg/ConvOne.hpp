/**
 *  @file   ConvOne.h
 *  @brief  �摜�ϊ�
 *  @author Masashi Kitamura
 *  @date   2000-??-??
 *  @note
 *      2006    bmptg.c ���番��.
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
    int         toneType;   // 0=rgb����, yuv-y����
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
        int     lcr_ex;     // 0=�ʏ� 1=�t�@�C�����{�̂̍Ō�̈ꎞ����Ȃ獶�l��(l)�����Ȃ�E�l��(r)�ɂ���
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
    int         monoNear;           // �قڃO���C�Ȃ�O���C�Ƃ݂Ȃ�
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
    int         rszN;       //���T�C�Y��
    int         rszK[2];    //�g�k���ɗp����W�� 1..
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
    unsigned    bokashiMergeRateRGB;    // R,G,B �ʂɃ��[�g���w��ł���悤�ɃJ���[�w��ɂ���
    unsigned    bokashiMaskGenCol1;     // �ڂ��������ł̃}�X�N���𐶐����邽�߂̐F�l1
    unsigned    bokashiMaskGenCol2;     // �ڂ��������ł̃}�X�N���𐶐����邽�߂̐F�l1
    int         colNum;
    int         clutOfs;
    int         clutIndShft;
    int         genAlpEx;               // ����(0,0)�̃s�N�Z���𔲂��F�Ƃ��āA�摜��4�����甲���F�̃���h��Ԃ��w��.
    char const* clutTxtName;
    char const* exDstExt;               // �g���q
    unsigned    alpMin;                 // �������͈�min
    unsigned    alpMax;                 // �������͈�max
    int         alpToCol;               // ������(��)min�`max�̃s�N�Z���ɁA�FalpToCol�ƃ��u�����h���āA���l��0xff�ɂ���.
    int         nukiumeRgb;             // �����F�Ƃ��Ė��߂�rgb�l.
    int         alpBitForBpp8;          // A2I6�̂悤�ȃ���N�r�b�g�A�C���f�b�N�X��M�r�b�g(N+M=8)��256�F�摜�ɂ���. �l��N.
    char const* alphaPlaneFileName;     // ���v���[���p�̉摜��ǂݍ��ޏꍇ.
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
/** ��t�@�C���ϊ� */
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
    unsigned        clut_[CLUT_NUM * 2];        ///< clut. �{����CLUT_NUM�����A�F���J�E���g�ŗ]���ɐ������Ɨ̈�̂��߁A*2���Ă���

    ConvOne_Opts    opts_;
};


#endif
