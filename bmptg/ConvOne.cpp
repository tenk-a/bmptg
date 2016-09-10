/**
 *  @file   ConvOne.cpp
 *  @brief  �摜�ϊ�
 *  @author Masashi Kitamura
 *  @date   2000-??-??
 *  @note
 *      2006    bmptg.c ���番��.
 */

#include "def.h"
#include "ConvOne.hpp"
#include "pix32.h"
#include "pix8.h"
#include "gen.h"
#include "beta.h"
#include "mem_mac.h"
#include "PaternDither.hpp"
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


// int32_t,uint32_t ��long�Œ�`����Ă���ƁA���̃\�[�X�ł͂܂����̂ŁA���b�p�����O�ݒ�ɕύX.
typedef unsigned      UINT32_T;
typedef unsigned char UINT8_T;


/** �ϊ��Ɋւ���I�v�V�����v�f�̏�����
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

    this->quality   = 80;
    this->quality_grey = -1;

    this->alpMin    = 0x00;         // �������͈�min
    this->alpMax    = 0xff;         // �������͈�max
    this->alpToCol  = -1;           // ������(��)min�`max�̃s�N�Z���ɁA�FalpToCol�ƃ��u�����h���āA���l��0xff�ɂ���.
    this->clutAlpNum = -1;          // ���F���̃���. �����ݒ�.

    this->monoToAlp_rate = 1.0;

    this->pixScale[0] = 1.0;
    this->pixScale[1] = 1.0;
    this->pixScale[2] = 1.0;
    this->pixScale[3] = 1.0;

    this->decreaseColorParam[0] = -1;
    this->decreaseColorParam[1] = 1.2f;
    this->decreaseColorParam[2] = 1.0;
    this->decreaseColorParam[3] = 1.0;

    this->rszType = 1;  // �o�C�L���[�r�b�N
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

/** �摜���ЂƂϊ�����
 *  @param  srcName     ���̓t�@�C����
 *  @param  dstName     �o�̓t�@�C����
 */
int ConvOne::run(const char *srcName, const char *dstName) {
    init(srcName, dstName);
    int rc = main();
    term();
    return rc;
}


/// �ϊ��̃��C��
int ConvOne::main() {
    if (readFile() == 0)                // �t�@�C���ǂݍ���
        return 0;
    if (imageLoad() == 0)               // �t�@�C���o�C�i������摜���擾
        return 0;
    adjustAlpha();                      // ���l���S��0�Ȃ�0xff��
    swapARGB();                         // argb�̏��ԕ��׊���
    loadAlphaPlaneFile();               // ���v���[���Ƀ��m�N���摜��ǂݍ���.
    changeClut();                       // �p���b�g����
    bitCom();                           // �F�l�̃r�b�g�𔽓]����
    revAlpha();                         // ���l���]
    monoToAlpha();                      // rgb�l��胂�m�N���l�����߂�������ɂ���.
    setClutNukiCol();                   // 256�F�Ŕ����F�����[�U�[�w�肳��Ă��Ȃ��ꍇ�Aclut_�̃�=0�̂��̂𔲂��F�Ƃ��č̗p����B
    reflectColKey();                    // �����F(�J���[�L�[) �𔽉f����
    easyPaintRoundAlpha();              // ����(0,0)�̃s�N�Z���𔲂��F�Ƃ��āA�摜��4�����甲���F�̃���h��Ԃ�
    resizeCanvas(0);                    // �摜�T�C�Y�̐؂蔲���ύX
    resizeCanvas(1);                    // �摜�T�C�Y�̐؂蔲���ύX
    changeSrcBpp();                     // ���͂�bpp_�ɐF���C���B
    reverseImage();                     // �㉺���E���]
    rotR90(opts_.rotR90);               // ���E90��
	rotateImage();						// �C�ӊp��]
    setDstBpp();                        // �o��bpp��ݒ�.
	checkSrcDstBpp();					// clut�擯�m�̕ϊ��ŏo��bpp���F�ԍ����������Ƀ\�[�X�����t���J���[��(���F����̑O��)

	convNukiBlack();                    // �����F���F(0,0,0)�Œ�̃n�[�h�����ɁA�^����(0,0,m)�ɁA�����h�b�g��(0,0,0)�ɕϊ�.
    resizeImage1st();                   // �c���g�k�T�C�Y�ύX   // ����
    filter();                           // �t�B���^ (�ڂ���)
    resizeImage2nd();                   // �c���g�k�T�C�Y�ύX   2���
    aptRect();                          // �����F|���Ŕ͈͂����߂ăT�C�Y�ύX
	patternDither();					// �p�^�[���f�B�U���{��
    alphaBlendByColor();                // �w��F�ƃ����u�����h���A����0 or 255 �ɂ���
	
	toMono();                           // ���m�N����
    mulCol();                           // �e�s�N�Z���ɐF���悸��
    colChSquare();						// ARGB�e�X���悷��
    changeTone();                       // �����F�ȊO�̐F�̃g�[���� opts_.tone���ɕϊ�

    decreaseColor();                    // ���F�ŏo�͂�clut_�t���Ȃ�A���Ռ��F���s��
    changeChipAndMap();                 // �`�b�v(�Z��)���}�b�v��
    reverseOutputAlpha();               // �o�͂̃��𔽓]����K�v������Ƃ�
    changeMaskImage();                  // �}�X�N�摜�����̂Ƃ�
    if (saveImage() == 0)               // �o�̓t�@�C���C���[�W�𐶐�
        return 0;
    if (writeFile() == 0)               // �t�@�C����������
        return 0;
    return 1;
}


/// ������
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

    // �摜���[�h�Z�[�u�̃I�v�V�����v�f�̐ݒ�
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

    // �p���b�g�N���A
    memset(clut_, 0, sizeof(clut_));
}


/// �t�@�C���ǂݍ���
bool ConvOne::readFile() {
    int bytSkp = opts_.bytSkp;

    // �t�@�C�����[�h
    if (varbose_) {
        if (dstName_ && dstName_[0])
            printf("%s -> %s  ", srcName_, dstName_);
        else
            printf("%-20s ", srcName_);
    }
    src_ = (UINT8_T*)fil_load(srcName_, NULL, 0, &sz_);
    if (src_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s ���I�[�v���ł��܂���B\n", srcName_);
        return 0;
    }
    dat_ = src_;
    if (bytSkp && sz_ > bytSkp) {   // �擪 �o�C�g���X�L�b�v����Ƃ�
        dat_ += bytSkp;
        sz_  -= bytSkp;
    }
    return 1;
}


/// �t�@�C���o�C�i������摜���擾
bool ConvOne::imageLoad() {
    int binBpp = opts_.binBpp;

    // �摜�̃w�b�_���擾
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
    // �o�͖����������A���Ԃ񂽂��̏��\�����B
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
        printf("%s : ���Ή��̉摜�t�H�[�}�b�g�̃t�@�C����ǂ݂���\n", srcName_);
        freeE(src_);
        return 0;
    }

    pixBpp_ = (bpp_ <= 8) ? 8 : 32; // clut_ �t����͈�U 256�F��ɁA���F��� 32bit�F��ɂ���

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
    if (opts_.mapMode && bo_->celW && bo_->celH)        // �}�b�v��
        pixH_ = ((h_ + bo_->celH-1) / bo_->celH) * bo_->celH;
    pixWb_ = WID2BYT(w_, pixBpp_);
    pix_ = (UINT8_T*)calloc(1, pixWb_ * pixH_ + 16);
    if (pix_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s �ǂ݂��݂��߂̃��������m�ۂł��܂���\n", srcName_);
        freeE(src_);
        return 0;
    }

    // �摜�W�J
    int c;
    DBG_PRINTF(("rd %x %x %d, %d, %d, %d, %x, %d\n", src_, pix_, pixWb_, w_, h_ , pixBpp_, clut_, 0));
    if (binBpp == 0) {  // �ʏ�̉摜�t�@�C���ǂݍ���
        c = bm_read(dat_, sz_, pix_, pixWb_, h_, pixBpp_, clut_, 0);
    } else {            // �x�^�摜 �ǂݍ���
        c = WID2BYT(opts_.binW, binBpp);
        if (sz_ < c * pixH_) {
            UINT8_T *p = (UINT8_T*)callocE(c , h_);
            memcpy(p, dat_, sz_);
            freeE(src_);
            src_ = dat_ = p;
        }
        if (binBpp <= 8) beta_genMonoClut(clut_, binBpp);   // �_�~�[clut��p��
        c = beta_conv(pix_, pixWb_, h_, pixBpp_,  dat_, c, binBpp, clut_, 0, opts_.binEndian, 0);
    }
    freeE(src_);
    if (c == 0) {
        if (varbose_) printf("\n");
        printf("%s : �摜�ǂݍ��݂Ɏ��s\n", srcName_);
        freeE(pix_);
        return 0;
    }
    return 1;
}


/** ���v���[���Ƀ��m�N���摜��ǂݍ���.
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
                            a = ARGB_A(cc);     // alpha�t�摜�Ȃ� alpha�l���擾.
                        } else {
                            a = ARGB_G(cc);     // r=g=b �ȉ摜�t�@�C���̓Ǎ��O���G�v���[�����擾.
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
    // �L���Ȃ̂�1�t�@�C���݂̂Ȃ̂Ńt�@�C�������폜.
    freeE((void*)opts_.alphaPlaneFileName);
    opts_.alphaPlaneFileName = NULL;
}


// -------------------------------------------------------------------
// �s�N�Z���f�[�^�̊e��ϊ�


/// argb�̏��ԕ��׊���
void ConvOne::swapARGB() {
    int colrot = opts_.colrot;
    if (colrot) {
        if (pixBpp_ == 8)   pix32_swapARGB(clut_,  1, 256, PIX32_SWAPARGB(colrot));
        else                pix32_swapARGB((UINT32_T*)pix_ , w_,  h_, PIX32_SWAPARGB(colrot));
    }
}


/// �p���b�g����
void ConvOne::changeClut() {
    int         clutOfs     = opts_.clutOfs;
    const int*  clutChgFlg  = opts_.clutChgFlg;
    const int*  clutChg     = opts_.clutChg;
    if (bpp_ <= 8) {
        for (int n = 0; n < 256; n++) {
            if (clutChgFlg[n])
                clut_[n] = clutChg[n];
        }
        if (clutOfs > 0) {
            UINT32_T    clutTmp[256];
            memset(clutTmp, 0, 4*256);
            for (int n = 0; n < 256; n++) {
                clutTmp[n] = clut_[(UINT8_T)(clutOfs+n)];
            }
            memcpy(clut_, clutTmp, 4*256);
        }
    }
}


/// �F�l�̃r�b�g�𔽓]����
void ConvOne::bitCom() {
    int sw = opts_.bitCom;
    if (sw) {
        if (pixBpp_ == 8)   pix_bitCom((UINT8_T *)pix_, w_, h_);
        else                pix_bitCom((UINT32_T*)pix_, w_, h_); //x printf("���F�ł̓r�b�g���]�͖��Ή�\n");
    }
}


/// ���l����
void ConvOne::adjustAlpha() {
    if (opts_.alpNon || bpp_ == 24 || (bpp_ <= 8 && pix32_isAlphaAllZero(clut_,1,256))) {
        // ���l�Ȃ��摜�Ȃ�΃���t��. (�J���[�L�[�����肦�Ȃ�-1�ɂ��āA�S�Ẵs�N�Z���̃���0xff�ɂ���)
        if (pixBpp_ == 8)   pix32_genColKeyToAlpha(clut_, 1, 256, (unsigned)-1);
        else                pix32_genColKeyToAlpha((UINT32_T*)pix_, w_, h_, (unsigned) -1);
    }
}


/// ���l���]
void ConvOne::revAlpha() {
    // ���͂̃��𔽓]����Ƃ�
    if (opts_.alpModeI) {
        if (pixBpp_ == 8)   pix32_revAlpha(clut_, 1, 256);
        else                pix32_revAlpha((UINT32_T*)pix_, w_, h_);
    }
}


/// 256�F�Ŕ����F�����[�U�[�w�肳��Ă��Ȃ��ꍇ�Aclut_���̍ŏ��Ɍ���������=0�̂��̂𔲂��F�Ƃ��č̗p����B
void ConvOne::setClutNukiCol() {
    if (pixBpp_ == 8) {
        nukiClut_ = opts_.nukiClut;
        if (nukiClut_ == -1 && opts_.colKey != -1) {	// clut-index�łȂ������F�w�肪�������΂����A�����T���Ă݂�
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
        // �����F�w�肪����΁A���̐F�̃��� 0�ɂ���
        if (nukiClut_ >= 0 && nukiClut_ <= 255 && opts_.colKeyNA == 0) {
            clut_[nukiClut_] &= 0xFFFFFF;
        }
    }
}


/// �����F(�J���[�L�[) �𔽉f����
void ConvOne::reflectColKey() {
	if (opts_.colKeyNA)
		return;
    int colKey    = opts_.colKey;
    int alpBokasi = opts_.alpBokasi;
    if (colKey != -1) {
        if (pixBpp_ == 8) {
            pix32_clearAlphaOfColKey(clut_, 1, 256, colKey);
        } else {
            pix32_clearAlphaOfColKey((UINT32_T*)pix_, w_, h_, colKey);
            if (alpBokasi) {
                pix32_alpBokasi((UINT32_T*)pix_, w_, h_, 255);      // �����A�s�����̋��ڂ̃����ڂ���
            }
        }
    }
}


/// ����(0,0)�̃s�N�Z���𔲂��F�Ƃ��āA�摜��4�����甲���F�̃���h��Ԃ�
void ConvOne::easyPaintRoundAlpha() {
    if (opts_.genAlpEx && pixBpp_ == 32) {
        if (varbose_) printf("->[XCA]");

        UINT32_T* pix = reinterpret_cast<UINT32_T*>(pix_);
        //x pix32_genAlpEx(pix, w_, h_);
        // ��U���ׂẴ���on.
        unsigned size = w_ * h_;
        for (unsigned i = 0; i < size; ++i)
            pix[i] |= 0xff000000;

        // ����̓_�̐F�������͈͂��l���Ƃ�����ׁAARGB(0,0,0,0) �Ŗ��߂�.
        Pix32_Paint(pix, (unsigned)w_, (unsigned)h_, 0x00000000, -1, -1, 0xFFF8F8F8);

        // ����������Ƃڂ���
        pix32_alpBokasi(pix, int(w_), int(h_), 0);
    }
}


/// �摜�T�C�Y�̐؂蔲���ύX
void ConvOne::resizeCanvas(int n) {
	if (n >= opts_.vvIdx)
		return;
	ConvOne_Opts::vv_t*	opts_vv = &opts_.vv[n];
    if (opts_.vv[n].flg) {     // �w��T�C�Y�̉摜�̈�ɁApix_����`�]������
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
		if (opts_vv->lcr_ex) {	// �t�@�C�����{�̂̍Ō�̈ꕶ���������̎��A������ō��l�߁E�E�l�߂�؂�ւ��鏈��
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

		if (vv_lcr == 1) {				// ������
			vv_x  += (vv_w - vv_sw) / 2;
			vv_sx += (w_   - vv_sw) / 2;
		} else if (vv_lcr == 2) {		// �E��
			vv_x  += (vv_w - vv_sw);
			vv_sx += (w_   - vv_sw);
		}
		if (opts_vv->umd == 1) {		// �c������
			vv_y  += (vv_h - vv_sh) / 2;
			vv_sy += (h_   - vv_sh) / 2;
		} else if (opts_vv->umd == 2) {	// ����
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

    if (bo_->celW) {        // �w��h�b�g�P�ʂ̃T�C�Y�ɍ��킹����
        int sw=w_, sh=h_;
        if (varbose_) printf("->cel(%d*%d)",bo_->celW,bo_->celH);
        if (pixBpp_ == 8) gen_celSz8(&pix_, &w_, &h_, bo_->celW, bo_->celH, nukiClut_);
        else             gen_celSz32(&pix_, &w_, &h_, bo_->celW, bo_->celH, opts_.colKey);
        if (sw!=w_ || sh != h_) if (varbose_) printf("[%d*%d]",w_,h_);
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// ���͂�bpp_�ɐF���C���B
void ConvOne::changeSrcBpp() {
    int srcBpp = opts_.srcBpp;
    if (srcBpp >= 0) {  // ���͂�bpp_�ɐF���C���B
        if (varbose_) printf("->bpp_(%d)", srcBpp);
        if (pixBpp_ == 8)  pix32_bppChg(clut_, 1, 256, srcBpp);
        else               pix32_bppChg((UINT32_T*)pix_, w_, h_, srcBpp);
    }
}


/// �㉺���E���]
void ConvOne::reverseImage() {
	int dir = opts_.dir;
 #if 1	// ???? atode
	if (opts_.rotR90 < 0) {
		dir ^= 3;
	}
 #endif
	if (dir) {            // ���]����
        if (varbose_) printf("->rev%s%s", (dir&1)?"X":"", (dir&2)?"Y":"");
        if (pixBpp_ == 8)  pix_revXY((UINT8_T*)pix_, w_, h_, dir);
        else               pix_revXY((UINT32_T*)pix_, w_, h_, dir);
    }
}


/// �E90����].
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


/// �ER����].
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


/// ���m�N��������
void ConvOne::toMono() {
	//bo_->mono = opts_.mono;
	if (opts_.monoNear) {		// �O���C�ɋ߂��F�Ȃ�A���m�N�������ɂ���
		if ((pixBpp_ == 8 && GrayClut<>::isNearGrey(clut_, 1, 256))
			|| GrayClut<>::isNearGrey((UINT32_T*)pix_, w_, h_)
		){
			mono_ = true;
		}
	}
    if (opts_.mono || mono_) {
		mono_ = true;
		int monoCh = opts_.monoChRGB;
		if (monoCh == 0) {	// �ʏ�̃��m�N����
	        if (varbose_) printf("->mono");
	        if (pixBpp_ == 8) pix32_toMono(clut_, 1, 256);
	        else              pix32_toMono((UINT32_T*)pix_, w_, h_);
		} else {			// �w��(RGBA)�`�����l����p���ă��m�N����
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


/// �e�s�N�Z���ɐF���悸��
void ConvOne::mulCol() {
    if (opts_.colMul) {     // �e�s�N�Z���ɐF���悸��
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


/// RGBA�e�`�����l�����悷��
void ConvOne::colChSquare() {
    if (opts_.colChSquare) {	// RGBA�e�`�����l�����悷��
        if (varbose_) printf("->colChSquare");
        if (pixBpp_ == 8) pix32_colChSquare(clut_, 1, 256);
        else              pix32_colChSquare((UINT32_T*)pix_, w_, h_);
		mono_ = false;
    }
}


/// �����F�ȊO�̐F�̃g�[���� opts_.tone���ɕϊ�
void ConvOne::changeTone() {
    if (opts_.tone >= 0) {      // �����F�ȊO�̐F�̃g�[���� opts_.tone���ɕϊ�
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


/// �o��BPP ������B
void ConvOne::setDstBpp()
{
    // �I�v�V�����w�肳��Ă��炻��ɂ���.
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
        if (opts_.dstFmt == BM_FMT_BMP) // bmp�͉����32�r�b�g�o�͂ɂȂ�ƕs�����Ƃ��āA�w��̂Ȃ�����24�r�b�g�F�ɂ��Ƃ�.
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


/** srcBpp,dstBpp����clut��ŁAdstBpp < srcBpp �̎��Adst�Ɏ��܂肫��Ȃ�src�����邩�`�F�b�N
 *  src�̐F�ԍ����͂ݏo���悤�Ȃ�A�������Asrc ���t���J���[�ɕϊ�(���F�ɔC��)
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


/// �����F���F(0,0,0)�Œ�̃n�[�h�����ɁA�^����(0,0,m)�ɁA�����h�b�g��(0,0,0)�ɕϊ�.
void ConvOne::convNukiBlack() {
    if (opts_.g555val>=0) { // ��<=opts_.g555val�̃s�N�Z����(0,0,0)�ɁA�ȊO��(0,0,0)��(0,0,m)��, 1�`m��m��.
        int abpp = (dstBpp_ > 8) ? dstBpp_ : (opts_.srcBpp > 0) ? opts_.srcBpp : (opts_.ditBpp > 0) ? opts_.ditBpp : 32;
        if (varbose_) printf("->[-xg]");
        if (pixBpp_ == 8) pix32_blackAMskGen(clut_, 1, 256, opts_.g555val, abpp);
        else              pix32_blackAMskGen((UINT32_T*)pix_, w_, h_, opts_.g555val, abpp);
    }
}


/// �c���g�k�T�C�Y�ύX  // ����
void ConvOne::resizeImage1st() {
    if (opts_.rszN) {   // �c���g�k�T�C�Y�ύX   // ����
        int     rszW;
        int     rszH;
        UINT8_T     *pix2;
        int    c = 0;
        // for (c = 0; c < opts_.rszN; c++)
        {
            if (opts_.rszXsz[c] || opts_.rszYsz[c]) {   //rszXsz,rszYsz���g���ꍇ
                if (opts_.rszK[c] && (opts_.rszXsz[c] && opts_.rszYsz[c])) {    // -xrq �䗦��ς����A�ǂ��炩����A�������̕ӂ��W���X�g�ɂ���B
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
                printf("�g�k��̃T�C�Y�����������Ȃ�(%d,%d)\n", rszW, rszH);
            pix2 = (UINT8_T*)callocE(1, 4 * ((rszW+15)&~15)*(rszH+1) + 256);
            if (pixBpp_ == 8)
                pix8_resize(pix2, rszW, rszH, pix_, w_, h_);    //x printf("CLUT�t�摜�̂܂܂ł͊g�k�ł��Ȃ�\n");
            else
                pix32_resize((UINT32_T*)pix2, rszW, rszH, (UINT32_T*)pix_, w_, h_, opts_.rszType, pixBpp_ > 24);
            w_ = rszW, h_ = rszH;
            freeE(pix_);
            pix_ = pix2;
        }
        pixWb_ = WID2BYT(w_, pixBpp_);
    }
}


/// �t�B���^ (�ڂ���)
void ConvOne::filter() {
    if (opts_.filterType && pixBpp_ == 32) {
        if (opts_.filterType == 1) {    // �ȈՂڂ���
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

        } else if (opts_.filterType == 2) { // �ʏ�摜�Ƃڂ����摜������
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
                    // 2�F�̎w�肩��A�er,g,b �̒l�����͈̔͂Ɏ��܂����s�N�Z���݂̂���=0xff�ɂ����摜�����.
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
                // ���摜pix_ �� �ڂ����摜pix2 ��pix2���̊����w��(0�`1.0)�ō�������pix3���쐬
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
        } else if (opts_.filterType == 3) { // �ȈՃV���[�v
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
        }
    }
}


/// �c���g�k�T�C�Y�ύX  2���
void ConvOne::resizeImage2nd() {
    if (opts_.rszN >= 2) {  // �c���g�k�T�C�Y�ύX   2���
        int     rszW;
        int     rszH;
        UINT8_T *pix2;
        int     c = 1;
        // for (c = 0; c < opts_.rszN; c++)
        {
            if (opts_.rszXsz[c] || opts_.rszYsz[c]) {   //rszXsz,rszYsz���g���ꍇ
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
                printf("�g�k��̃T�C�Y�����������Ȃ�(%d,%d)\n", rszW, rszH);
            pix2 = (UINT8_T*)callocE(1, 4 * ((rszW+15)&~15)*(rszH+1) + 256);
            if (pixBpp_ == 8)
                printf("CLUT�t�摜�̂܂܂ł͊g�k�ł��Ȃ�(-if�ő��F���ł��܂�)\n");
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


/// �����F|���Ŕ͈͂����߂ăT�C�Y�ύX
void ConvOne::aptRect() {
    if (opts_.nukiRctFlg) { // �����F|���Ŕ͈͂����߂ăT�C�Y�ύX
        int gridMd = (opts_.nukiRctFlg & 0xF) - 1;
        gridMd |= ((opts_.nukiRctFlg>>4) & 3) << 1;     // 0�Ȃ�wh�Ƃ� 2(0x10)�Ȃ�w_�̂� 4(0x20)�Ȃ�h_�̂�
        int x0 = 0;
        int y0 = 0;
        if (pixBpp_ == 8) gen_nukiRect8(&pix_, &w_, &h_, &x0, &y0, bo_->celW, bo_->celH, nukiClut_, gridMd);
        else             gen_nukiRect32(&pix_, &w_, &h_, &x0, &y0, bo_->celW, bo_->celH, gridMd);
        if (varbose_) printf("->(%d,%d)[%d*%d]",x0,y0,w_,h_);
        bo_->x0 += x0;
        bo_->y0 += y0;

        pixWb_ = WID2BYT(w_, pixBpp_);
        //* srcW_, srcH_ �ǂ�����?
    }
}

/// �p�^�[���f�B�U���{��
void ConvOne::patternDither() {
    if (opts_.ditBpp && pixBpp_ == 32) {    // �f�B�U���{��
        int dbpp,ditBpp = opts_.ditBpp;
        if (ditBpp <= 0) {              // �f�t�H���g�̐F�̃r�b�g�����o�͂ɍ��킹�đI��
            if (opts_.isFixedClut()) {	// jp or win �Œ�clut�̏ꍇ
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
        dbpp = (8 < dstBpp_ && dstBpp_ < ditBpp) ? dstBpp_ : ditBpp;
        if (varbose_) printf("->dit%d:0x%x", dbpp,opts_.ditTyp);
        PaternDither    paternDither;
        paternDither.conv((UINT32_T*)pix_, (UINT32_T*)pix_, w_, h_, dbpp, opts_.ditTyp | (opts_.ditAlpFlg << 8));
    }
}


/// �w��F�ƃ����u�����h���A����0 or 255 �ɂ���
void ConvOne::alphaBlendByColor() {
    int col = opts_.alpToCol;
    if (col >= 0) {
        if (pixBpp_ == 8) pix32_alphaBlendByColor(clut_,  1, 256, col, opts_.nukiumeRgb, opts_.alpMin, opts_.alpMax);
        else              pix32_alphaBlendByColor((UINT32_T*)pix_ , w_,  h_, col, opts_.nukiumeRgb, opts_.alpMin, opts_.alpMax);
        if (varbose_) printf("->alpToCol(%06x)",col);
    }
}


/// ���� ���F�ŏo�͂�clut_�t���Ȃ�A���Ռ��F���s��
void ConvOne::decreaseColor() {
    if (pixBpp_ > 8 && dstBpp_ <= 8) {
        UINT8_T *p       = (UINT8_T*)callocE(w_ , 2+h_);
        int     clutSize = 1 << dstBpp_;
        int     alpNum   = opts_.clutAlpNum;
        bool    alpFlg   = opts_.clutAlpFlg != 0;
        int     md       = opts_.decreaseColorMode;
        int     colNum   = opts_.colNum;

        memset(clut_, 0, sizeof clut_);
        if (colNum < 2 || colNum > clutSize)
            colNum = clutSize;

        if (opts_.alpBitForBpp8 == 0) {
            switch (md) {
            case 1: // ���{��80�N��p�\�R���R���� 16�F,256�F(G3R3B2)�Œ�p���b�g.
            case 2: // Win�V�X�e���p���b�g
            case 7: // �������p���b�g
				if (varbose_)
                    printf("->DftlClt%c%d", (md>1)?'W':'J', dstBpp_);
				--md;
				if (md >= 6) md = md - 6 + 2;
                FixedClut256<>::getFixedClut256(clut_, 256, dstBpp_, md);
                //if (alpNum)
                //  clut_[0] &= 0xFFFFFF;
                FixedClut256<>::decreaseColor(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);
                break;

			default:	// �͈͊O�Ȃ�Ƃ肠�����A���f�B�A���J�b�g(yuv)��.
            //case 0:
				md = -1;
				// ����
            case 3: // ���f�B�A���J�b�g(yuv)
            case 4: // ���f�B�A���J�b�g(rgb)
            case 5: // �p�x�� clut
				if (alpFlg == 0 && alpNum < 0 && colNum >= (1 << dstBpp_) && (mono_ || GrayClut<>::isGrey((UINT32_T*)pix_, w_, h_))) {
					// ���m�N���摜��p�̌��F
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
	                // 32�r�b�g�F�悪�A���Ƃ��clutNum�F�ȓ��Ȃ�A���̂܂ܕϊ�.
                    if (varbose_)
                        printf("->clt%d", dstBpp_);
                } else if ((dstBpp_ == 1 || dstBpp_ == 2) && colNum >= (1<<dstBpp_)) {
					// 1bit�F,2bit�F��p�̌��F����
					if (dstBpp_ == 1)
						DecreaseColorLowBpp<>::convPix32ToBpp1(p, (UINT32_T*)pix_, w_, h_, clut_);
					else
						DecreaseColorLowBpp<>::convPix32ToBpp2(p, (UINT32_T*)pix_, w_, h_, clut_);
					if (varbose_)
	                    printf("->cltBpp%d", 1 << dstBpp_);
				} else if (md == 5) {	// �p�x�� clut �Ō��F.
                    //int a = (opts_.alpMin >= 0) ? opts_.alpMin : 4;
                    DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, clut_, colNum, alpNum);
                    if (varbose_) {
                        printf("->decreaseCol%d", dstBpp_);
                    }
				} else {            // 256�F��葽���̂ŗv���F.
					if (md < 0)
						md = 3;
                    memset(clut_, 0, sizeof clut_);
                    // ���f�B�A���J�b�g�Ȍ��F.
                    DecreaseColorMC     rcmc;
                    rcmc.setModeRGB(md == 4);               // rgb��yuv���̐ݒ�
                    if (opts_.decreaseColorParam[0] >= 0) { // �b��I�Ȃς�߁[���ݒ�
                        rcmc.setCalcMidParam(
                            (int  )(opts_.decreaseColorParam[0]),
                            (float)(opts_.decreaseColorParam[1]),
                            (int  )(opts_.decreaseColorParam[2]),
                            (float)(opts_.decreaseColorParam[3])
                        );
                    }
                    // ���p�����[�^�̐ݒ�.(����, ��bit��p����ꍇ��bit��, ���̍ŏ��l, ���̍ő�l)
                    rcmc.setAlphaParam(alpNum, 0, opts_.alpMin, opts_.alpMax);
                    // �R���o�[�g.
                    rcmc.conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);

                    if (varbose_)
                        printf("->decreaseColMC%s%d", (md == 3) ?"yuv":"rgb", dstBpp_);
                }
                break;

            case 6:
                if (varbose_) {
                    printf("->fixedClut%d", dstBpp_);
                }
                //int a = (opts_.alpMin >= 0) ? opts_.alpMin : 4;
                DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, opts_.fixedClut_, colNum, alpNum);
                break;
            }

        } else {    // a3i5,a5i3�̐�p����.
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

            // 32�r�b�g�F�悪�A���Ƃ��clutNum�F�ȓ��Ȃ�A���̂܂ܕϊ�.
            if (DecreaseColorIfWithin256<>::conv(p, (UINT32_T*)pix_, w_, h_, clut_, colNum, 0/*alpFlg*/)) {
                if (varbose_)
                    printf("->cltA%dI%d", opts_.alpBitForBpp8, idxBit);

            } else {            // colNum�F��葽���̂ŗv���F.
                memset(clut_, 0, sizeof clut_);
                if (md == 5) {
                    if (varbose_)
                        printf("->decreaseColA%d", dstBpp_);
                    DecreaseColorHst<>(p, (UINT32_T*)pix_, w_, h_, clut_, colNum);
                } else {
                    if (varbose_)
                        printf("->decreaseCol_A%dI%d", opts_.alpBitForBpp8, idxBit);
                    // ���f�B�A���J�b�g�Ȍ��F.
                    DecreaseColorMC     rcmc;
                    rcmc.setModeRGB(0/*md == 4*/);          // rgb��yuv���̐ݒ�
                    if (opts_.decreaseColorParam[0] >= 0) { // �b��I�Ȃς�߁[���ݒ�
                        rcmc.setCalcMidParam(
                            (int  )(opts_.decreaseColorParam[0]),
                            (float)(opts_.decreaseColorParam[1]),
                            (float)(opts_.decreaseColorParam[2]),
                            (float)(opts_.decreaseColorParam[3])
                        );
                    }
                    // ���p�����[�^�̐ݒ�.(����, ��bit��p����ꍇ��bit��, ���̍ŏ��l, ���̍ő�l)
                    rcmc.setAlphaParam(0/*alpNum*/, 0/*opts_.alpBitForBpp8*/, opts_.alpMin, opts_.alpMax);
                    // �R���o�[�g.
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


/// �`�b�v(�Z��)���}�b�v��
void ConvOne::changeChipAndMap() {
    if (opts_.mapMode && bo_->celW && bo_->celH) {      // �}�b�v��
        int     celNum;
        int     styl  = opts_.celStyl | (opts_.mapNoCmp << 1) | (opts_.mapEx256x256<<3);	// 1bit|2bit|1bit
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


// �o�͂̃��𔽓]����K�v������Ƃ�
void ConvOne::reverseOutputAlpha() {
    if (opts_.alpModeO) {       // �o�͂̃��𔽓]����K�v������Ƃ�
        if (varbose_) printf("->����");
        if (pixBpp_ == 8) pix32_revAlpha(clut_, 1, 256);
        else              pix32_revAlpha((UINT32_T*)pix_, w_, h_);
    }
}


/// �}�X�N�摜�����̂Ƃ�
void ConvOne::changeMaskImage() {
    if (opts_.genMaskFlg) {
        if (varbose_) printf("->mask��");
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


/// �o�̓t�@�C���C���[�W�𐶐�
bool ConvOne::saveImage() {
	bool     rc   = true;
	uint8_t* pix2 = NULL;
    // �o�͏���
    // ���F�̐F���̂��߂�dstBpp_�𗬗p���Ēl���͈͊O�̏ꍇ������̂ŁA�����ɒ��덇�킹
    dstBpp_ = bm_chkDstBpp(opts_.dstFmt, dstBpp_);

	// �o��BPP�w��Ȃ��ꍇ�A24bit��Ȃ�\�Ȃ�clut��ɂ���(�T�C�Y�k���̂���. 32bit��(alpha�t)��clut��t�H�[�}�b�g�͔����Ȃ̂ŉ��.
	int dstFmt = opts_.dstFmt;
	if (opts_.bpp == 0 && pixBpp_ == 32) {
		if (dstBpp_ == 24 && (dstFmt == BM_FMT_PNG || dstFmt == BM_FMT_TGA || dstFmt == BM_FMT_BMP
			#ifdef MY_H
							|| MY_IS_AUTO_BPP8_FMT(dstFmt)
			#endif
		)) {
			unsigned clutSize = 256;
			// ���m�������ς݂̏ꍇ
			if (/*mono_ ||*/ GrayClut<>::isGrey((uint32_t const*)pix_, w_, h_)) {
				if (varbose_) printf("->auto-mono");
				pix2 = new uint8_t[w_ * h_ + 16];
				GrayClut<>::getFixedGreyClut(clut_, 256, 8);
				GrayClut<>::fromGreyToBpp8(pix2, (uint32_t const*)pix_, w_, h_);
				mono_ = true;
			} else {	// �F���� 256�ȉ��Ȃ� clut��ɕϊ�
				pix2 = DecreaseColorIfWithin256<>::convToNewArray((uint32_t*)pix_, w_, h_, clut_, clutSize, false, 0xFFFFFFFF);
				if (pix2 && varbose_) {
					if (clutSize <= 16)
						printf("->auto-16");
					else
						printf("->auto-256");
				}
			}
			if (pix2) {	// �ϊ�������
				freeE(pix_);
				dstBpp_  = (clutSize <= 16 && dstFmt != BM_FMT_TGA) ? 4 : 8;
				pix_ 	 = pix2;
				pixBpp_  = 8;
				dstColN_ = 1 << dstBpp_;
				pixWb_   = w_;
			}
		} else if (dstFmt == BM_FMT_JPG) {
			if (GrayClut<>::isGrey((uint32_t*)pix_, w_, h_))
				mono_ = true;
		}
	}

	bo_->mono = mono_;

    if (opts_.clutTxtName && dstBpp_ <= 8) {
        clutTxtSave(opts_.clutTxtName, dstName_, clut_, dstColN_);
    }

    // �o�b�t�@�m��
    int c = bm_encodeWorkSize(dstFmt, w_, h_, dstBpp_);
    dst_  = (UINT8_T*)calloc(1, c);
    if (dst_ == NULL) {
        if (varbose_) printf("\n");
        printf("%s �o�͂̂��߂̃��������m�ۂł��܂���\n", dstName_);
        term();
		rc = false;
		goto RET;
    }

    // �o�͉摜�𐶐�
    if (dstFmt != BM_FMT_NON) {
        int clutAlpFlg = opts_.clutAlpFlg;
        if (clutAlpFlg == 0 && pixBpp_ == 8 && dstBpp_ <= 8) {
            clutAlpFlg = pix32_isUseAlpha(clut_, 1<<dstBpp_, 1);
        }
        int dir_flgs = 0;
		dir_flgs    |= opts_.encMode    << BM_FLAG_EX_ENC_SH;												// ���ʂ̈��k�����邩
        dir_flgs    |= clutAlpFlg       << BM_FLAG_CLUT_ALP_SH;												// alpha�tclut�ɂ��邩
        dir_flgs    |= (opts_.colKeyNA == 0 && (opts_.colKey >= 0 || nukiClut_ >= 0)) << BM_FLAG_COLKEY_SH;	// �����F�����邩
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
            //x printf("%s �� %s �ɕϊ����ɃG���[������܂���\n", srcName_, opts_.dstExt);
            printf("%s �� �ϊ����ɃG���[������܂���\n", srcName_);
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


/// �t�@�C����������
int ConvOne::writeFile() {
    if (opts_.saveInfFile) {    // �摜���̃e�L�X�g�t�@�C�����쐬
        saveInfFile(dstName_, w_, h_, dstBpp_, opts_.mapMode, bo_);
    }

    if (opts_.dstFmt != BM_FMT_NON) {
        if (opts_.mapMode >= 2 && map_) {   // �e�N�X�`�������}�b�v�t�@�C�������̏ꍇ
            saveMapFile(dstName_, opts_.exDstExt, map_, mapSz_, dst_, sz_);

        } else {
            if (map_) {                 // �}�b�v���t�@�C���݂̂̐���
                saveMapFile(dstName_, NULL, map_, mapSz_, NULL, 0);
            }

            // �t�@�C������
            if (strlen(dstName_) < FIL_NMSZ-4) {        // ���łɃt�@�C�����������ꍇ�A.bak����
                char            tmpName[FIL_NMSZ];
                sprintf(tmpName,"%s.bak", dstName_);
                remove(tmpName);
                rename(dstName_, tmpName);
            }
            if (fil_save(dstName_, dst_, sz_) == NULL) {
                printf("%s �̏������݂Ɏ��s���܂���\a\n", dstName_);
            }
        }
    }
    return 1;
}

/// �I������
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

/// �}�b�v�t�@�C������
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

    if (pix == NULL) {  // �w�b�_�݂̂̂Ƃ�
        if (fil_save(nm, map, mapSz) == NULL) {
            printf("%s �̏������݂Ɏ��s���܂���\a\n", nm);
        }
    } else {            // �摜���ꏏ�ɔ[�߂�Ƃ�
        FILE *fp = fopenE(nm, "wb");
        mapSz = (mapSz + 31) & ~31;
        POKEiD(map+4, mapSz);           // �摜�ւ̃I�t�Z�b�g��ݒ�
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


/// �摜���e�L�X�g�t�@�C���o��
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
        if (mapMode) {  // celW��>0�Ȃ�celH���K��>0
            fprintf(fp,"MAPSIZE=%d,%d\n", w/bo->celW, h/bo->celH);
            if (mapMode == 3 && bo->celW == bo->mapTexW && bo->celH == bo->mapTexH) {
                fprintf(fp,"1X1MAP_TEXTURE\n");
            }
        }
    }
    fclose(fp);
}


/// clut�e�L�X�g�̏o��
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
