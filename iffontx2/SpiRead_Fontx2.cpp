/**
 *  @file   SpiRead_FontX2.cpp
 *  @brief  Susie�p FontX2 �v���O v0.6
 *  @author Masashi KITAMURA (tenk*)
 *  @note
 *  2004    0.5
 *  2007-06 0.6 Spi�e�v���O�C���̋��ʏ��������Ɣ񋤒ʕ����𕪗�
 *          (���̔񋤒ʕ���)
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




/// IsSupported,GetPictureInfo�Ńt�@�C���Ǎ�������ꍇ�́A
/// �摜�t�H�[�}�b�g����ɕK�v�ȃw�b�_�T�C�Y.
DWORD       SpiRead_headerSize = 32;




/// �v���O�C�����̕�����z��.
extern const char*  SpiRead_pluginInfoMsg[] = {
    "00IN",                             /* 0 : Plug-in API ver. */
    ABOUT_MSG,                          /* 1 : About.. */
    "*.fnt;*.fnx;*.fx2;*.tlf",          /* 2 : �g���q. �K��... */
    "FontX2",                           /* 3 : �`���� */
    "",
};



/// pData�̉摜�f�[�^���T�|�[�g���Ă��邩(1)�ۂ�(0)��Ԃ�. fname�͔���⏕�̃t�@�C����.
BOOL SpiRead_isSupported(const char* /*fname*/, const BYTE* pData)
{
    return FontX2SampleImage<>::isSupported(pData) != 0;
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
        unsigned            /*dataSize*/,   // �W�J���f�[�^�̃o�C�g��.
        int*                pWidth,         // ���������ĕԂ�.
        int*                pHeight,        // �c�������ĕԂ�.
        int*                pBpp,           // �摜�̂P�s�N�Z��������̃r�b�g��.(�W�J�\��́A�ł͂Ȃ�)
        DWORD*              pClut256,       // 256�F�܂ł̐F�p���b�g(clut)�����ĕԂ�. �A�h���X0�Ȃ疳��.
        SpiRead_InfoOpt*    pInfoOpt)       // �I�v�V�����I�ȏ���Ԃ��ꍇ�p.
{
    if (FontX2SampleImage<>::isSupported(pData) == false)
        return false;

  #ifdef USE_REGOPT // TSPIRO�p�̎��́A"�\��"�{�^����on�Ȃ牡1�����ɂ���.
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



/** pData�� pixBpp�r�b�g�F�摜�ɓW�J����pPix�ɓ���ĕԂ�.
 *  - bmp�Ɠ��l�ɁA�����o�C�g����4�̔{��, �����C������l�߂�.
 *
 *  - �s�N�Z���z�u��bmp�ɍ��킹��.
 *  - pixBpp�́A�ʏ��
 *      ��bpp�� 1�Ȃ� 1, ��bpp 2�`4 �Ȃ� 4, ��bpp 5�`8�Ȃ� 8�A��9�ȏ�Ȃ�24
 *    ���ݒ肳��Ă���B
 *    ������ USE_DIB32 ����`�ݒ肳��Ă���ꍇ�́A
 *      �����F|���t�摜�ɑ΂��Ă� bpp 32
 *    ���ݒ肳�ꂤ��B�܂��A
 *    tspiro���W�X�g���I�v�V�����p�� USE_REGOPT ����`����Ă���ꍇ��
 *      ��bpp�� 8�ȉ��Ȃ� 8�A��9�ȏ�� 32
 *    ���ݒ肳�ꂤ��. (USE_DIB32 �� USE_REGOPT ������`�Ȃ�C�ɂ��Ȃ�)
 *
 *  - fname�͕����t�@�C���\���̃f�[�^�����A�����Afname=NULL�̏󋵂�����̂Œ���.
 */
int  SpiRead_getPix(const BYTE* pData, unsigned /*dataSize*/, BYTE* pPix, DWORD pixBpp, const char* /*fname*/)
{
    if (FontX2SampleImage<>::isSupported(pData) == false)
        return false;

  #ifdef USE_REGOPT // TSPIRO�p�̎��́A"�\��"�{�^����on�Ȃ牡1�����ɂ���.
    regopt_t   ro;
    RegOpt_Get(&ro);
    FontX2SampleImage<> dec(pData, (ro.mapOffSw != 0));
  #else
    FontX2SampleImage<> dec(pData, 0);
  #endif

    int rc;
    if (dec.bpp() == pixBpp) {  // ���o�͂�bpp�������Ȃ�A���̂܂ܓW�J.
        rc = dec.read(pPix, 4, 1);

    } else {                    // bpp����������Ƃ�.
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
