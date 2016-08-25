/**
 *  @file   SpiRead_Png.c
 *  @brief  Susie�p png �v���O v0.50
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




/// IsSupported,GetPictureInfo�Ńt�@�C���Ǎ�������ꍇ�́A
/// �摜�t�H�[�}�b�g����ɕK�v�ȃw�b�_�T�C�Y. clut�͓ǂ܂Ȃ��̂ŏ��ʂł���..����.
DWORD       SpiRead_headerSize = 0;     // ���S�̂��ߑS���ǂݍ���...




/// �v���O�C�����̕�����z��.
extern const char*  SpiRead_pluginInfoMsg[] = {
    "00IN",                                     /* 0 : Plug-in API ver. */
    ABOUT_MSG,                                  /* 1 : About.. */
    "*.png",                                    /* 2 : �g���q */
    "png",                                      /* 3 : �`���� */
    "",
};



/// pData�̉摜�f�[�^���T�|�[�g���Ă��邩(1)�ۂ�(0)��Ԃ�. fname�͔���⏕�̃t�@�C����.
BOOL SpiRead_isSupported(const char* /*fname*/, const BYTE* pData)
{
    return PngDecoder::isSupported(pData) != 0;
}



/** �摜�� ����񂪂��邩? (�S�ă�=0���S�ă�=0xff�Ȃ烿��񖳂�)
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


/** pData�̉摜����Ԃ�.
 *  GetPictureInfo�� GetPicture�̗�������Ă΂��.
 *  �l�����ĕԂ����A�f�t�H���g�l�ł悯��Ή������Ȃ��Ă悢.
 *  pClut256�ȊO�͕K���A�h���X������.
 *  clut��ݒ肷��ꍇ�́A1�F�� ��8R8G8B8 �ŁA
 *  ���l(0���� 1�`254������ 255�s����)���ݒ肵�ĕԂ�.
 */
BOOL SpiRead_getInfo(
        const BYTE*         pData,          // �W�J���f�[�^.
        unsigned            dataSize,       // �W�J���f�[�^�̃o�C�g��.
        int*                pWidth,         // ���������ĕԂ�.
        int*                pHeight,        // �c�������ĕԂ�.
        int*                pBpp,           // �摜�̂P�s�N�Z��������̃r�b�g��.(�W�J�\��́A�ł͂Ȃ�)
        DWORD*              pClut256,       // 256�F�܂ł̐F�p���b�g(clut)�����ĕԂ�. �A�h���X0�Ȃ疳��.
        SpiRead_InfoOpt*    pInfoOpt)       // �I�v�V�����I�ȏ���Ԃ��ꍇ�p.
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



/** pData�� pixBpp�r�b�g�F�摜�ɓW�J����pPix�ɓ���ĕԂ�.
 *  - bmp�Ɠ��l�ɁA�����o�C�g����4�̔{��, �����C������l�߂�.
 *
 *  - �s�N�Z���z�u��bmp�ɍ��킹��.
 *  - pixBpp�́A�ʏ��
 *      ��bpp�� 1�Ȃ� 1, ��bpp 2�`4 �Ȃ� 4, ��bpp 5�`8�Ȃ� 8�A��9�ȏ�Ȃ�24
 *    ���ݒ肳��Ă���B������ USE_DIB32 ����`�ݒ肳��Ă���ꍇ�́A
 *      �����F|���t�摜�ɑ΂��Ă� bpp 32
 *    ���ݒ肳�ꂤ��B�܂��A
 *    tspiro���W�X�g���I�v�V�����p�� USE_REGOPT ����`����Ă���ꍇ��
 *      ��bpp�� 8�ȉ��Ȃ� 8�A��9�ȏ�� 32
 *    ���ݒ肳�ꂤ��. (USE_DIB32��USE_REGOPT����`�Ȃ�C�ɂ��Ȃ�)
 *
 *  - fname�͕����t�@�C���\���̃f�[�^�����A�����Afname=NULL�̏󋵂�����̂Œ���.
 */
int  SpiRead_getPix(const BYTE* pData, unsigned dataSize, BYTE* pPix, DWORD pixBpp, const char* /*fname*/)
{
    if (PngDecoder::isSupported(pData) == false)
        return false;

    PngDecoder  dec(pData, dataSize);
    dec.revY();                 // bmp�͉����C������l�߂�̂Ŕ��].

    if (dec.bpp() < 8)          // 1,2,4bit�F�̏ꍇ�A�o�C�g���̋l�ߏ����t.
        dec.setBigEndian();

    int rc;
    if (dec.bpp() == pixBpp) {  // ���o�͂�bpp�������Ȃ�A���̂܂ܓW�J.
        if (pixBpp == 24)       // 24�r�b�g�F���Ȃ�A���͖�����.
            dec.stripAlpha();
        dec.setWidthAlign(4);   // bmp�̉����͂S�o�C�g�ŃA���C�����g.
        rc = dec.read(pPix);

    } else {                    // bpp����������Ƃ�.
        unsigned w = dec.width();
        unsigned h = dec.height();
        BppCnvImg src(w, h, dec.bpp(), 1);
        if (src.clutSize())
            dec.getClut(src.clut(), src.clutSize());
        rc =  dec.read( src.image() );
        if (rc) {
            BppCnvImg dst(false, pPix, w, h, pixBpp, 4, NULL, 0);
            if (dst.bpp() < 8)  // bmp �̓o�C�g���͏�ʃr�b�g����l�߂�̂�.
                dst.setDstEndian(1);
            dst.conv(src);
        }
    }
    return rc;
}
