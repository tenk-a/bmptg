/**
 *  @file   bmptg.cpp
 *  @brief  �摜�R���o�[�^
 *  @author Masashi Kitamura
 *  @date   2000-??-??
 *  @note
 *  2000        v1.00 �����bmp2tga,tga2bmp �֌W�𓝍������c�[����.
 *  2001-01-30  tga��16bit�F�̃��̈������|�J���Ă��̂�(0x80�łȂ�0xFF��)�C��.
 *  2001-03     �f�B�U������. clut��ւ̌��F�@�\����.
 *  2001-04     bmp-rle�֌W
 *  2001-07     v2.00 �g�k���o�C���j�A�T���v�����O��p�������@�ɕύX�B
 *  2003-11     MY4 �ǉ�
 *  2004        (�Ȃ񂩃o�O�Ƃ肵���悤��... ���ƃ\�[�X�̋L�q�X�^�C�����C��)
 *  2005-09     cel&map�ł̃������[�j��o�O�̏C��
 *  2005-??     �ڂ����֌W�̒ǉ�
 *  2005-11     -xca �l���𔲂��F�Ńy�C���g���ă��v���[���𐶐�
 *  2006        conv_one()�̃X�p�Q�b�e�B��Ԃ͐h�߂���̂�c++��class�ɏ�����.
 *  2007        v2.10 �����Y��Ă��I�v�V������usage�ɋL�q.�܂��P�y�[�W��������߂�.
 *  2007-05     v2.20 ���f�B�A���J�b�g�@�̌��F���T�|�[�g( ���t���F�����������܂Ƃ���)
 *  2007-05     v2.21 aNiM��8�r�b�g�F���F���[�h��ǉ�.
 *  2007-06     v2.30 jpg�Ή�.
 *  2007-06     v2.31 png�Ή�.
 *  2007-07     v2.32 ���F�֌W�C��, �E��].
 *  200?-??     v2.33 -xv �Ō��摜��(x,y)���w��\��.
 *  2015-??     v2.34 �o�C���j�A���F�C��
 *  2015-03     v2.35 �o�C�L���[�r�b�N�g�k�ǉ�.��������f�t�H���g��.
 *  2015-10     v2.36 -cgc -cem -xvc -xvr -xjl -xqg �ǉ�. �o��bpp�w�薳���Ń��m�N���������ꍇ�͂Ȃ�ׂ�256�F�摜�ŏo��.
 *  2016-08     v2.37 lanczos-3,spline36,NearestNeighbor�g�k�ǉ�. bilinear,bicubic �����ł�yuv�~��rgb�ŏ���. bilinear��0.5�s�N�Z������C��.
 *                    -xv��u|m|d|e�I�v�V�����ǉ�. -xv�n2�x�w��\��. -xl�p�~. -xi[R] �C�ӊp��]�ǉ�.
 *  2016-08     v2.38 ���m�N���挸�F�����P. 1bpp��,2bpp��o�͐�p�̌��F��ǉ�. �g�k��]�ł̃��̕⊮�̓o�C���j�A�݂̂�.
 *                    clut��œ���bpp>�o��bpp���ɏo��bpp�Ɏ��܂�Ȃ��s�N�Z��������Έ�U�t���J���[��.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "subr.h"
#include "ConvOne.hpp"
#ifdef USE_MY_FMT
#include "my.h"
#else
#define MY_USAGE_1
#define MY_USAGE_2
#endif

char *g_appName;


void usage(void)
{
    printf("usage> %s [-opts] file(s)   // v2.39 " __DATE__ "  by tenk*\n", g_appName);
    printf(
       "  bmp tga jpg png ���摜�𑊌݂ɕϊ�.\n"
       "  ���l�͗L����00:�����`0xFF:�s����. �������0xFF�Ƃ��Ĕ��F�̓�=0�Ƃ��ď���.\n"
       MY_USAGE_1
       "\n"
       "  :FMT          �o�̓t�H�[�}�b�g��FMT(bmp tga jpg png bin non)��.\n"
       "                bin �̓w�b�_&clut���s�N�Z���݂̂̃x�^�o��.\n"
       "                non �͌o�߃��b�Z�[�W�݂̂ŁA�t�@�C���ϊ���.\n"
       "  -f[FMT]       :FMT�ɓ���\n"
       "  -b[N]         �o�͂�N bit�F��(1,4,8,12,15,16,24,32).\n"
       "                (���ʃr�b�g�؎̂Ȃ̂ŁA�K�v�Ȃ�-xd�Ńf�B�U���F��)\n"
       "  -v-           �������b�Z�[�W�����炷.\n"
       "\n"
       " [�t�@�C����]\n"
       "  -o[FILE]      1�t�@�C���ϊ��ł̏o�͖�.\n"
       "  -d[DIR]       �o�̓f�B���N�g����DIR��.\n"
       "  -s[DIR]       ���̓f�B���N�g����DIR��.\n"
       "  -ed[EXT]      �o�͂̊g���q�� EXT ��.\n"
       "  -es[EXT]      ���͂̏ȗ����g���q��EXT��.\n"
       "  -u            ���t���V������Εϊ�.\n"
       "\n"
       "  [�o�C�i������]\n"
       "  -ig[W:H:B:E]  B�r�b�g�FW*H�̃x�^��(�܂��̓o�C�i���t�@�C��)����.\n"
       "  -is[N]        ����FILE�擪N byte�X�L�b�v.\n"
       "\n"
       " [�F�֌W]\n"
       "  -if           clut����t���J���[(24or32�r�b�g�F)�ɂ��ē���.\n"
       "  -ib[N]        ���͎�(���̕ϊ�������O)�ɁAN(3�`24) bit�F�ɕϊ�.\n"
       "                24 = R8G8B8    23 = R8G8B7   22 = R7G8B7\n"
       "                21 = R7G7B7    ...........    3 = R1G1B1\n"
       "  -xd[N:T]      �f�B�U��N(3�`24)bit�F�Ɍ��F. (T:0-7)\n"
       "                 N= �w����@�� -ib[N] �ɓ���\n"
       "                 T=  0:�f�B�U����\n"
       "                     1:�p�^�[���f�B�U 2x2\n"
       "                     2:�p�^�[���f�B�U 4x4\n"
       "                     3:�p�^�[���f�B�U 8x8\n"
       //x "                    +4: �덷�g�U����(����:����o�O���Ă�͗l?)\n"
       "                 +0x80:A,G �� R,B �ƂŃf�B�U�}�g���b�N�X��΂ɂ���\n"
       "                +0x100:�����f�B�U����. �L���r�b�g����G�ɓ���\n"
       "\n"
       "  -cq[ARGB]     ARGB ���ABGRA,AGRB ���̏��ɕ��ъ�.\n"
       "  -t[N]         RGB�l�� N%% �ɂ��ďo��.\n"
       "  -tt[N]        �P�x�� N%% �ɂ��ďo��.\n"
       "  -cm[C]        �e�s�N�Z���ɐFC���悸��.\n"
       "  -cem          R,G,B,A�l�e�X���悷��.\n"
       "  -cs[A:R:G:B]  �����w���rgb�s�N�Z���̊e�l�����{���邩���w��\n"
       "  -ct[A:Y:U:V]  �����w���yuv�s�N�Z���̊e�l�����{���邩���w��\n"
       "  -cg           ���m�N����\n"
       "  -cgc[R|G|B|A] R,G,B,A �����ꂩ�̃`�����l����p���ă��m�N����.\n"
       "  -cf[C]        ���F�� C (�ȗ���000000)��.\n"
       "  (-ca[C]       -cf�g���Ŕ��F���Ӄ��ڂ���. ������)\n"
       "  -cfn[C]       -cf�ɂقړ������� ��=0�ɂ��Ȃ�(-vv�ł̖��ߐF)\n"
       "  -an           ���͉惿�l�𖳎�(��������).\n"
       "  -ao           ���`�����l����mask�摜�������t�@�C�����o��.\n"
       "  -ai[FILE]     FILE�����`�����l���ɓǂݍ���\n"
       "  -ari          ���̓����].\n"
       "  -aro          �o�̓����].\n"
       "  -am[A:B]      ���F���֌W�̕ϊ��ŗp���郿�͈�[A,B]. A�ȉ��͓���,B�ȏ�͕s����.\n"
       "  -ac[C:C2]     �s�N�Z���ƐFC�����u�����h����=0 or 0xFF. C2�w��̓�=0����RGB�l.\n"
       "  -ag[RATE:OFS] �s�N�Z��RGB��胂�m�N���l�����߂�������Ƃ���.\n"
       "                 RATE,OFS������ΐF�̕ϊ� (r,g,b)*RATE+OFS ���ɍs��\n"
       "  -ad           �����F�Ɣ񔲂��F�̍�̃����ڂ����ȈՏ���(��BU�쐬��)\n"
       "                (v2.38�ȑO�� -ca -xca �ŋ@�\�������̂𕪗�)\n"
       "  -az           ��=0�Ȃ�RGB��0�ɂ���\n"
       "\n"
       " [clut�֌W]\n"
       "  -cpc[FILE]    clut���e��c�p�e�L�X�g�o��.\n"
       "  -cp[N]        �Œ�clut�ݒ�or���F�A���S���Y���w��.\n"
       "                1:jp�Œ�clut             2:win�V�X�e��clut.\n"
       "                3:���f�B�A���J�b�g(yuv)  4:���f�B�A���J�b�g(rgb)\n"
       "                5:�p�x��\n"
       "                N�̎w��̂Ȃ��ꍇ��-cp3\n"
       "                -cp3:M:L (-cp4��)\n"
       "                  M=0:�����l�͒P����2�Ŋ���������. M=1:�p�x�𔽉f(�f�t�H���g)\n"
       "                  L=1�`4.0:Y,U,V,A�̑I���ŁAY�l��L�{���čs��(�f�t�H���g1.2)\n"
       "  -cn[N]        ���F����1<<bpp�����̐F���ɂ������ꍇ�Ɏw��\n"
       "\n"
       "  -ap           ���tclut�ŏo��. ���ꂩ���F�w��(-ap)���Ȃ��ƃ���clut�ɂȂ�\n"
       "  -ap[N]        N:���F������. N>=2�͋����I��-cp3���f�B�A���J�b�g(yuv)�ɂȂ�\n"
       "  -ci[I]        clut��̔��F�ԍ���I��.\n"
       "  -cc[I:C]      clut �� i(0�`255)�Ԃ�F C �ɒu��. I���Ⴆ�Ε����w��L��\n"
       "  -co[N]        clut�̓Y����/���g�� N���炷. (-cis[N]������)\n"
       "  -cb[N]        tga�ł̏o��clut��BPP.\n"
       "\n"
       " [�T�C�Y�ϊ�]\n"
       "  -xrs[W:H:K]   W*H�Ɋg�k.\n"
       "  -xrp[N]       �c���̒�����N���Ɋg�k.\n"
       "  -xrc          �g��Ƀo�C�L���[�r�b�N�@��p����(�f�t�H���g).\n"
       "  -xrb          �g��Ƀo�C���j�A�@��p����.\n"
       "  -xre          �g���spline36��p����.\n"
       "  -xrl          �g�k��lanczos-3��p����.\n"
       "  -xrn          �g�k�Ƀj�A���X�g�l�C�o�[�@��p����.\n"
       "  -xn           �s�����͈݂͂̂ɂ���.\n"
      #ifdef MY_H
       "                -mc[N:M]�w�莞��NxM�`�b�v�P�ʂŋ��߂�.\n"
      #endif
       "  -xv[W:H:X:Y:w:h:x:y]  ���摜��(x,y)��w*h��W*H���(X,Y)�Ƀ��[�h�����摜�ɂ���.\n"
       "                        W,H�� 0�Ȃ�摜�T�C�Y.\n"
       "                -xv+10:+5 �̂悤��+�ŋL�q����Ή摜�T�C�Y�ɑ΂��鑝���ɂȂ�.\n"
       "  -xvc[W:H:X:Y:w:h:x:y] �Z���^�����O��-xv. X:Y,x:y�̓Z���^�����O��I�t�Z�b�g\n"
       "  -xvr[W:H:X:Y:w:h:x:y] �E(��)���-xv. X:Y,x:y�͉E���̃I�t�Z�b�g\n"
       "  -xve[W:H:X:Y:w:h:x:y] �t�@�C�����̍Ōオ�����Ŋ�Ȃ�-xv,�����Ȃ�-xvr\n"
       "  -xv?u[�c],-xv?m[�c],-xv?d[�c] -xv �̏c��ʒu�w��\n"
       "                ?��l|c|r�̉��w��, u|m|r �͏c ���|������|���� �w��\n"
       //"  -xl[M:N]      -xv+M:+N:M:N �ɓ���\n"
      #ifdef MY_H
       "\n"
       " [�`�b�v&�}�b�v]\n"
       "  -mc[N:M]      N*N(N*M)�h�b�g�P��(�`�b�v)��\n"
       "  -mh[W]        �`�b�v&�}�b�v��. W:�e�N�X�`������\n"
       "  -mp[W]        �e�N�X�`�����܂߂��t�@�C���ł̃`�b�v&�}�b�v��.\n"
       "  -mn           ���`�b�v�Z�߂��s��Ȃ�(�����`�b�v�l�߂͍s��)\n"
       "  -mnn          ���`�b�v���`�b�v��Z�߂Ȃ�.\n"
       "  -mb           -mh,-mp�ɂ����ă`�b�v�̎��͂P�h�b�g�𑝐B�����e�N�X�`���𐶐�\n"
       "  -mq[TW:TH]    -mp �Ɠ��l�B������256x256���̉摜�Ȃ�1*1�}�b�v�ɂ���\n"
       "  -mi           ���e�L�X�g����.\n"
       "  -my           �}�b�v���ł̑������̓��ʔ�\n"
       "  -xnn          -xn�Ɠ��l�����A-mc���Ɏn�_���̓`�b�v�P�ʉ������A���肬���I��.\n"
       "  -xo[X:Y]      �摜�t�H�[�}�b�g�̎n�_����ݒ�\n"
      #endif
       "\n"
       " [���̑��̕ϊ�]\n"
       "  -xe           ���k���ďo��(tga�̂�).\n"
       "  -xq[N]        jpg���k���̃N�I���e�B(��)\n"
       "  -xqg[N]       ���m�N���摜�p��jpg���k���̃N�I���e�B(��)\n"
       "  -xx           x�������].\n"
       "  -xy           y�������].\n"
       "  -xj           �E90����].\n"
       "  -xjl          ��90����].\n"
       "  -xi[R]        �ER����].\n"
       "  -xb           �s�N�Z���l�̃r�b�g���](2�l��ł̔��]��z��).\n"
       "  -xg[N]        ��<=N�̓_��(0,0,0)�ɁA�o��bpp��(0,0,0)�ɂȂ�_���ߎ��̕ʐF��.\n"
       "  -xca[N]       ����(0,0)�̐F�𔲂��F�Ƃ���4�����烿=0�̐F�Ńy�C���g. N=1-256�K���͈�\n"
       "                (�o�X�g�A�b�v���G�쐬��z��)\n"
       "  -cgn[R|G|B|A] ���m�N���摜�ۂ���� -cg|-cgc �̓���(������)\n"
       "  -xf[..]       �ڂ����t�B���^����.\n"
       "   -xf1:N                     �ڂ���[3x3:4/2/1]��N��.\n"
       "   (-xf2:N:LeapARGB           ����:-xf1�g���łڂ����摜�ƒʏ�������)\n"
       "   (-xf2:N:LeapARGB:RGB1:RGB2 ����:�����2�F���琶�������}�X�N�����f)\n"
        MY_USAGE_2
       "\n"
    );
    exit(1);
}



/* ------------------------------------------------------------------------ */

/// �I�v�V�������
struct Opts {
public:
    ConvOne_Opts*   convOne_opts;
    char*           oname;
    char*           dstDir;
    char*           srcDir;
    char const*     srcExt;
    char const*     dstExt;
    int             updateFlg;
    int             dispInfo;

public:
    Opts(ConvOne_Opts& coo);
    ~Opts() {;}

    int     scan(const char *a);

private:
    static double   strExprD(const char *p, const char ** a_p, int* a_err);
    static int      strToI (const char* &p, int base) { return (int)strtol(p, (char**)&p, base); }
    static unsigned strToUI(const char* &p, int base) { return (unsigned)strtoul(p, (char**)&p, base); }
};




/** �I�v�V�����̏����� */
Opts::Opts(ConvOne_Opts& convOne_opts)
{
    memset(this, 0, sizeof(*this));
    this->convOne_opts  = &convOne_opts;
    this->srcDir        = NULL;
    this->dstDir        = NULL;
    this->srcExt        = "bmp";
    this->dstExt        = "tga";
    this->convOne_opts->dstFmt  = BM_FMT_TGA;
}



/** �I�v�V�����̏��� */
int Opts::scan(const char *a)
{
    ConvOne_Opts* o = this->convOne_opts;
    const char  *p;
    int         c;

    p = a;
    if (*p == '-')
        p++;
    c = *p++;
    c = toupper(c);
    switch (c) {
    case 'A':
        c = *p++, c = toupper(c);
        if (c == 'N') {         // -an
            o->alpNon = 1;
        } else if (c == 'O') {  // -ao
            o->genMaskFlg = 1;
        } else if (c == 'C') {  // -ac
            o->alpToCol   = strToUI(p,16);
            o->nukiumeRgb = o->alpToCol;
            if (*p != 0) {
                ++p;
                o->nukiumeRgb = strToUI(p, 16);
            }
        } else if (c == 'D') {  // -ad
            o->alpBokasi = (*p != '-');

        } else if (c == 'G') {  // -ag
            o->monoToAlp = (*p != '-');
            if (*p == 0) {
                o->monoToAlp_rate = 1.0;
                o->monoToAlp_ofs  = 0;
            } else {
                o->monoToAlp_rate = strExprD(p,&p,0);
                if (*p != '\0') {
                    ++p;
                    o->monoToAlp_ofs = strToI(p, 0);
                }
            }
        } else if (c == 'M') {  // -am
            o->alpMin = strToUI(p,0);
            if (*p) {
                ++p;
                o->alpMax = strToUI(p,0);
            }
            if (o->alpMax <= o->alpMin || o->alpMax > 255)
                o->alpMax = 255;
        } else if (c == 'P') {  // -ap
            o->clutAlpFlg = 1;
            if (*p) {
                o->clutAlpNum = strToUI(p,0);
            } else {
                o->clutAlpNum = -1; // ���clut�����K���ɋ��߂�.
            }
            if (o->clutAlpNum >= 256)
                o->clutAlpNum = 256;
        } else if (c == 'R') {  // -ar
            if (*p == 'o' || *p == 'O') //-aro
                o->alpModeO = 1;
            else
                o->alpModeI = 1;
        } else if (c == 'I') {  // -ai
            o->alphaPlaneFileName = strdup(p);
            o->fullColFlg         = 1;
        } else if (c == 'Z') {  // -az
            o->clearColIfAlp0 = (*p != '-');

      #if 0 //����
        } else if (c == 'I') {
            if (*p == 0)
                o->alpModeI = 1;
            else
                o->alpModeI = strToUI(p,16);
        } else if (c == 'O') {
            if (*p == 0)
                o->alpModeO = 1;
            else
                o->alpModeO = strToUI(p,16);
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'B':
        if (isdigit(*p)) {
            o->bpp = c = strToUI(p, 0);
            if (strchr("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0c\x0f\x10\x18\x20", c) == NULL)
                err_abortMsg("-b �̎w��l���z��O(%d)\n", c);
        } else {
            c = 0;
            if      (stricmp(p, "A2I6") == 0) c = 2;
            else if (stricmp(p, "A3I5") == 0) c = 3;
            else if (stricmp(p, "A4I4") == 0) c = 4;
            else if (stricmp(p, "A5I3") == 0) c = 5;
            else if (stricmp(p, "A6I2") == 0) c = 6;
            if (c) {
                o->fullColFlg      = 1;
                o->bpp             = 8;
                o->alpBitForBpp8   = c;
                o->decreaseColorMode = 3;       // �Ƃ肠����yuv�ȃ��f�B�A���J�b�g�@�Ō��F���邱�Ƃɂ���
            }
        }
        break;

    case 'C':
        c = *p++, c = toupper(c);
        switch (c) {
        case 'O':   //-co
            o->clutOfs = strToUI(p, 0);
            break;

        case 'F':   //-cf
            if (toupper(*p) == 'N') {
                ++p;
                o->colKey   = strToUI(p, 16);
                o->colKeyNA = 1;
            } else {
                o->colKey = strToUI(p, 16);
            }
            break;

        case 'A':   //-ca
            o->colKey = strToUI(p, 16);
            o->alpBokasi = 1;
            break;

        case 'B':   //-cb
            o->clutBpp = strToI(p, 0);
            break;

        case 'E':   //-ce
            if (toupper(*p) == 'M') {   // -cem
                o->colChSquare = *(p+1) != '-';
            }
            break;

        case 'G':   //-cg
            o->mono = (*p != '-');
            if (toupper(*p) == 'C') {   // -cgc
                int t = toupper(*(unsigned char*)(p+1));
                o->monoChRGB = (t == 'B') + (t == 'G')*2 + (t == 'R')*3 + (t == 'A')*4;
            } else if (toupper(*p) == 'N') {    // -cgn
                int t = toupper(*(unsigned char*)(p+1));
                o->mono      = 0;
                o->monoNear  = 1;
                o->monoChRGB = (t == 'B') + (t == 'G')*2 + (t == 'R')*3 + (t == 'A')*4;
            }
            break;

        case 'M':   //-cm
            o->colMul = strToUI(p, 16);
            if ((o->colMul & 0xFF000000) == 0)
                o->colMul |= 0xFF000000;
            break;

        case 'S':   //-cs
        case 'T':   //-ct
            o->pixScaleType = (c == 'T');
            o->pixScale[0]  = o->pixScale[1] = o->pixScale[2] = o->pixScale[3] = 1.0;
            o->pixScale[0] = strExprD(p,&p,0);
            if (*p) {
                o->pixScale[1] = strExprD(p+1,&p,0);
                if (*p) {
                    o->pixScale[2] = strExprD(p+1,&p,0);
                    if (*p) {
                        o->pixScale[3] = strExprD(p+1,&p,0);
                    }
                }
            }
            break;

        case 'N':   //-cn
            o->colNum = strToUI(p,0);
            break;

        case 'C':   //-cc
            c = strToI(p, 0);
            if (c < 0 || c > 255)
                goto OPT_ERR;
            o->clutChgFlg[c] = 1;
            if (*p) {
                ++p;
                o->clutChg[c] = strToUI(p, 16);
            }
            break;

        case 'I':   //-ci
            if (toupper(*p) == 'S') {   // -cis
                ++p;
                o->clutIndShft = strToUI(p, 0);
                o->clutOfs     = o->clutIndShft;
            } else { // -ci
                o->nukiClut    = strToUI(p, 0);
            }
            break;

        case 'P':   //-cp
            if (toupper(*p) == 'C') {   //-cpc
                o->clutTxtName = strdupE(p+1);
            } else if (toupper(*p) == 'f') {    // -cpf �O���p���b�g�t�@�C����p���Č��F
                o->readFixedClut(p+1);
                o->decreaseColorMode = 6;
                o->fullColFlg        = 1;
            } else {
                o->fullColFlg      = 1;
                //x o->dfltClutCg = (*p == 0) ? 1 : strToUI(p, 0);
                o->decreaseColorMode = (*p == 0) ? 1 : strToUI(p,0);
                if (*p) {
                    o->decreaseColorParam[0] = strExprD(p+1,&p,0);
                    if (*p) {
                        o->decreaseColorParam[1] = strExprD(p+1,&p,0);
                        if (*p) {
                            o->decreaseColorParam[2] = strExprD(p+1,&p,0);
                            if (*p) {
                                o->decreaseColorParam[3] = strExprD(p+1,&p,0);
                            }
                        }
                    }
                }
            }
            break;

        case 'Q':   //-cq
            {
                static const char *tbl[] = {
                    "argb", "ragb", "agrb", "garb", "rgab", "grab",
                    "arbg", "rabg", "abrg", "barg", "rbag", "brag",
                    "agbr", "gabr", "abgr", "bagr", "gbar", "bgar",
                    "rgba", "grba", "rbga", "brga", "gbra", "bgra",
                };
                if (stricmp(p,"rgb") == 0) p = "argb";
                if (stricmp(p,"rbg") == 0) p = "arbg";
                if (stricmp(p,"grb") == 0) p = "agrb";
                if (stricmp(p,"gbr") == 0) p = "agbr";
                if (stricmp(p,"brg") == 0) p = "abrg";
                if (stricmp(p,"bgr") == 0) p = "abgr";
                for (c = 0; c < 24; c++) {
                    if (stricmp(p, tbl[c]) == 0) {
                        o->colrot = c;
                        break;
                    }
                }
                if (c == 24)
                    goto OPT_ERR;
            }
            break;

        //case 'Y':
        //
        //  break;

        default:
            goto OPT_ERR;
        }
        break;

    case 'E':
        c = *p++, c = toupper(c);
        if (c == 'D') {
            this->dstExt = strdupE(p);
            o->exDstExt = strdupE(p);
        } else if (c == 'S') {
            this->srcExt = strdupE(p);
        } else {
            goto OPT_ERR;
        }
        break;

    case ':':
    case 'F':
        if (stricmp(p, "NON") == 0) {
            o->dstFmt    = BM_FMT_NON;
            this->dstExt = "";
        } else if (stricmp(p, "BMP") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "bmp";
        } else if (stricmp(p, "DIB") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "dib";
      #if 1
        } else if (stricmp(p, "RLE") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "rle";
            o->encMode = 1;
      #endif
        } else if (stricmp(p, "TGA") == 0) {
            o->dstFmt    = BM_FMT_TGA;
            this->dstExt = "tga";
      #if defined USE_JPG
        } else if (stricmp(p, "JPG") == 0 || stricmp(p, "JPEG") == 0) {
            o->dstFmt    = BM_FMT_JPG;
            this->dstExt = "jpg";
      #endif
      #if defined USE_PNG
        } else if (stricmp(p, "PNG") == 0) {
            o->dstFmt    = BM_FMT_PNG;
            this->dstExt = "png";
      #endif
        } else if (stricmp(p, "BIN") == 0) {
            o->dstFmt    = BM_FMT_BETA;
            this->dstExt = "bin";
      #ifdef MY_H
        } else if (MY_opt_fmtCheck(p, o, &this->dstExt)) {
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'H':
        c = *p++, c = toupper(c);
        if (c == 0) {
            usage();
      #ifdef MY_H
        } else if (c == 'R') {
            o->lvlY = strToI(p, 0);
            if (*p != 0) {
                p++;
                o->lvlUV = strToI(p, 0);
            }
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'I':
        c = *p++, c = toupper(c);
        if (c == 'F') {                 //-if
            o->fullColFlg = (*p != '-');
        } else if (c == 'B') {          //-ib
            if (*p == 0)
                o->srcBpp = 24;
            else
                o->srcBpp = strToI(p, 0);
        } else if (c == 'S') {          //-is
            o->bytSkp = strToI(p, 0);
        } else if (c == 'G') {          //-ig
            o->binW = 256;
            o->binH = -1;
            o->binBpp = 8;
            o->binW = strToI(p, 0);
            if (*p) {
                ++p;
                o->binH = strToI(p, 0);
                if (*p) {
                    ++p;
                    o->binBpp = strToI(p, 0);
                    if (*p) {
                        ++p;
                        o->binEndian = strToI(p, 0);
                    }
                }
            }
            if (o->binBpp <= 0 || o->binBpp > 32 || o->binW <= 0 || o->binH == 0)
                goto OPT_ERR;
        } else if (c == 'N') {
            this->dispInfo = (*p != '-');
        } else {
            goto OPT_ERR;
        }
        break;

    case 'M':
        c = *p++, c = toupper(c);
        if (c == 'C') { //-mc
            o->celSzW = strToI(p,0);
            if (o->celSzW <= 0)
                goto OPT_ERR;
            o->celSzH = o->celSzW;
            if (*p) {
                p++;
                o->celSzH = strToI(p,0);
                if (o->celSzH <= 0)
                    goto OPT_ERR;
            }
        } else if (c == 'B') {  //-mb
            o->celStyl = (*p != '-') ? 1 : 0;
        } else if (c == 'T') {  //-mt   �Ƃ肠�����A�p�~
            o->mapStyl = (*p != '-') ? 1 : 0;
        } else if (c == 'N') {  //-mn   ���Z���`�F�b�N�����Ȃ�. -mnn �����L�����������Ȃ�(�X�v���C�g�n�[�h�����̃f�[�^�R���o�[�g)
            o->mapNoCmp = (*p == 'n' || *p == 'N') ? 3 : (*p != '-') ? 1 : 0;
        } else if (c == 'Y') {  //-my   �}�b�v���̑������Ԃ̓��ʔ�
            o->mapEx256x256 = (*p != '-') ? 1 : 0;
        } else if (c == 'H' || c == 'P' || c == 'Q') {  //-mh,-mp,-mq
            o->mapMode = (c == 'H') ? 1 : (c == 'P') ? 2 : 3;
            o->mapTexW = (*p) ? strToI(p, 0) : 256;
            o->mapTexH = 0;
            if (*p) {
                p++;
                o->mapTexH = (*p) ? strToI(p, 0) : 256;
                if (*p) {
                    ++p;
                    o->mapMode3_y_min = strToI(p, 0);
                }
            }
            if (o->celSzW == 0) {
                o->celSzW = o->celSzH = 32;
            }
            if (o->mapTexW < o->celSzW || (o->mapTexH && o->mapTexH < o->celSzH)) {
                err_abortMsg("�}�b�v�����̃e�N�X�`������������������(-mp�̑O��-mc�ŃT�C�Y��ݒ肵�Ă�������)\n");
                goto OPT_ERR;
            }
        } else if (c == 'I') {      //-mi
            o->saveInfFile = 1;
      #if 0
        } else if (c == 'O' && toupper(p[0]) == 'j' && toupper(p[1]) == 'i') {  //-moji
            p += 2;
            o->fntX = strToI(p, 10);
            if (*p) {
                o->fntY = strToI(p+1, 10);
                if (*p) {
                    o->fntC = strToUI(p+1, 16);
                    if (*p) {
                        ++p;
                        o->fntD = strToI(p, 10);
                        if (*p) {
                            o->fntStr = strdupE(p+1);
                        }
                    }
                }
            }
            if (o->fntD < 5)
                o->fntD = 16;
            break;
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'O': // -o
        this->oname = strdupE(p);
        break;

    case 'D': // -d
        this->dstDir = strdupE(p);
        fname_delLastDirSep(this->dstDir);
        break;

    case 'S': // -s
        this->srcDir = strdupE(p);
        fname_delLastDirSep(this->srcDir);
        break;

    case 'T':   // -t
        o->toneType = 0;
        if (*p == 0) {
            o->tone = 50;
        } else if (toupper(*p) == 'T') {    // -tt
            o->toneType = 1;
            o->tone     = 50;
            if (p[1]) {
                ++p;
                o->tone = strToI(p,10);
            }
        } else {
            o->tone = strToI(p,10);
        }
        break;

    case 'U':   // -u
        this->updateFlg = (*p == '-') ? 0 : 1;
        break;

    case 'V':   // -v
        o->verbose = (*p == '-') ? 0 : 1;
        break;

    case 'X':
        c = *p++;
        c = toupper(c);
        switch (c) {
        case 'D':   //-xd
            o->fullColFlg      = 1;
            if (*p == 'A' || *p == 'a') {   // -xda
                o->ditAlpFlg = 1;
                p++;
            }
            o->ditBpp = strToI(p,0);
            if (o->ditBpp <= 0)
                o->ditBpp = -1;
            else if (o->ditBpp >= 24)
                err_abortMsg("-xd[%d] ���ƃf�B�U���s���܂���\n");
            o->ditTyp = 2;
            if (*p) {
                p++;
                o->ditTyp = strToI(p,0);
            }
            break;

        case 'E':   //-xe
          #ifdef MY_H
            c = MY_opt_checkXE(p);
            if (c)
                o->encMode = c;
            else
          #endif
                o->encMode = (*p == '-') ? 0 : (*p == '\0') ? 1 : strToI(p,10);
            break;

        case 'G':   //-xg
            if (*p == 0)
                o->g555val = 0;
            else
                o->g555val = strToI(p,0);
            if (o->g555val < 0 || o->g555val > 255)
                goto OPT_ERR;
            break;

        case 'R':   //-xr
            {
                if (o->rszN > 2)
                    err_abortMsg("-xrs�܂���-xrp ��3�ȏ�w�肳��Ă���\n");
                c = *p++, c = toupper(c);
                o->rszK[o->rszN] = 0;
                if (c == 'P') {         //-xrp
                    o->rszXpar[o->rszN] = strExprD(p,&p, 0);
                    if (*p) {
                        o->rszYpar[o->rszN] = strExprD(p+1,&p, 0);
                    } else {
                        o->rszYpar[o->rszN] = o->rszXpar[o->rszN];
                    }
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1, 10);
                    o->rszN++;
                } else if (c == 'S') {  //-xrs
                    o->rszXsz[o->rszN] = int(strExprD(p, &p, 0));
                    o->rszYsz[o->rszN] = 0;
                    if (*p) {
                        o->rszYsz[o->rszN] = int(strExprD(p+1, &p, 0));
                    }
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1, 10);
                    o->rszN++;
                } else if (c == 'Q') {  //-xrq
                    o->rszXsz[o->rszN] = int(strExprD(p, &p, 0));
                    o->rszYsz[o->rszN] = 0;
                    if (*p)
                        o->rszYsz[o->rszN] = int(strExprD(p+1, &p, 0));
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1,10);
                    o->rszK[o->rszN] = 1;
                    o->rszN++;
                } else if (c == 'N') {  //-xrn �j�A���X�g�l�C�o�[
                    o->rszType = 0;
                } else if (c == 'B') {  //-xrb �o�C���j�A
                    o->rszType = 1;
                } else if (c == 'C') {  //-xrc �o�C�L���[�r�b�N
                    o->rszType = 2;
                } else if (c == 'E') {  //-xre spline36
                    o->rszType = 3;
                } else if (c == 'L') {  //-xrl lanczos-3
                    o->rszType = 4;
                }
            }
            break;

        case 'Q':   // -xq
            if (toupper(*p) == 'G') {   // -xqg
                ++p;
                o->quality_grey = strToI(p, 10);
            } else {
                o->quality = strToI(p, 10);
            }
            break;

        case 'X':   //-xx
            o->dir |= 1;
            break;

        case 'Y':   //-xy
            o->dir |= 2;
            break;

        case 'J':   //-xj
            o->rotR90 = 1;
            if (toupper(*p) == 'L') {
                o->rotR90 = -1;
            }
            break;

        case 'I':   //-xi
            o->rotR = strtod(p, (char**)&p);
            break;

        case 'N':   //-xn
            o->nukiRctFlg = 1;
            if (*p == 'n' || *p == 'N') {
                o->nukiRctFlg = 2;
                ++p;
            }
            if (*p == 'x' || *p == 'X') {
                o->nukiRctFlg |= 0x10;
            } else if (*p == 'y' || *p == 'Y') {
                o->nukiRctFlg |= 0x20;
            }
            break;

        case 'V':   //-xv
            if (*p) {
                if (o->vvIdx >= 2)
                    goto OPT_ERR;
                ConvOne_Opts::vv_t* ov = &o->vv[o->vvIdx++];
                ov->flg = 1;
                ov->lcr = 0;
                ov->umd = -1;
                ov->lcr_ex = 0;
                for (;;) {
                    int c2 = toupper(*p);
                    if (c2 == 'C') {        // -xvc
                        ov->lcr = 1;
                        ++p;
                    } else if (c2 == 'R') { // -xvr
                        ov->lcr = 2;
                        ++p;
                    } else if (c2 == 'L') { // -xvl
                        ov->lcr = 0;
                        ++p;
                    } else if (c2 == 'U') { // -xvu
                        ov->umd = 0;
                        ++p;
                    } else if (c2 == 'M') { // -xvm
                        ov->umd = 1;
                        ++p;
                    } else if (c2 == 'D') { // -xvd
                        ov->umd = 2;
                        ++p;
                    } else if (c2 == 'E') { // -xve
                        ov->lcr_ex = 1;
                        if (ov->umd < 0)
                            ov->umd = 1;    // umd���ݒ�Ȃ�c������
                        ++p;
                    } else {
                        break;
                    }
                }
                if (ov->umd < 0)
                    ov->umd = ov->lcr;
                // W
                ov->w = 0;
                ov->h = 0;
                ov->x = 0;
                ov->y = 0;
                ov->sw= 0;
                ov->sh= 0;
                ov->sx= 0;
                ov->sy= 0;
                if (*p != ':' && *p != ',') {
                    ov->wf = 0;
                    if (*p == '+') {
                        ov->wf = 1;
                        p++;
                    }
                    ov->w = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // H
                if (*p != ':' && *p != ',') {
                    ov->hf = 0;
                    if (*p == '+') {
                        ov->hf = 1;
                        p++;
                    }
                    ov->h = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // X
                if (*p != ':' && *p != ',') {
                    ov->xf = 0;
                    //if (*p == '+') {
                    //    ov->xf = 1;
                    //    p++;
                    //}
                    ov->x = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // Y
                if (*p != ':' && *p != ',') {
                    ov->yf = 0;
                    //if (*p == '+') {
                    //    ov->yf = 1;
                    //    p++;
                    //}
                    ov->y = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // w
                if (*p != ':' && *p != ',') {
                    ov->sw = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // h
                if (*p != ':' && *p != ',') {
                    ov->sh = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // x
                if (*p != ':' && *p != ',') {
                    ov->sx = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // y
                if (*p != ':' && *p != ',') {
                    ov->sy = strToI(p, 10);
                }
            } else {
                goto OPT_ERR;
            }
            break;
      #if 0 // �Ƃ��Ɍ��s��-xv�\�[�X�Ɩ������ăo�O���Ă�̂ō폜
        case 'L':   //-xl
            if (*p) {
                if (o->vvIdx >= 2)
                    goto OPT_ERR;
                ConvOne_Opts::vv_t* ov = &o->vv[o->vvIdx++];
                ov->flg = 1;
                // W
                if (*p != ':' && *p != ',') {
                    ov->wf = 1;
                    ov->x  = ov->w = strToI(p, 10);
                }
                // H
                if (*p != 0) {
                    ov->hf = 1;
                    ++p;
                    ov->y  = ov->h = strToI(p, 10);
                }
            } else {
                goto OPT_ERR;
            }
            break;
      #endif
        case 'B':   //-xb
            o->bitCom = (*p == '-') ? 0 : 1;
            break;

        case 'F':   //-xf   �t�B���^
            o->filterType = strToI(p, 10);
            if (o->filterType == 1 || o->filterType == 2 || o->filterType == 3 || o->filterType == 4) {
                o->bokashiCnt = 1;
                if (*p != 0) {
                    ++p;
                    o->bokashiCnt = strToI(p, 10);
                    if (*p != 0) {  // -xf2 -xf3 �p
                        //o->bokashiMergeRate = strToI(p+1, 10) / 100.0;
                        ++p;
                        o->bokashiMergeRateRGB = strToUI(p, 16);
                        o->bokashiMaskGenCol1  = 0x000000;
                        o->bokashiMaskGenCol2  = 0xffffff;
                        if (*p != 0) {  // xf3 �p
                            ++p;
                            o->bokashiMaskGenCol1 = strToUI(p, 16);
                            if (*p != 0) {
                                ++p;
                                o->bokashiMaskGenCol2 = strToUI(p, 16);
                            }
                        }
                    }
                }
            }
            break;

        case 'C':   // -xca
            if (*p == 'a' || *p == 'A') {
                ++p;
                o->genAlpEx = 8;
                if (*p < 0x80 && isdigit(*p))
                    o->genAlpEx = strToUI(p, 10);
            } else {
                goto OPT_ERR;
            }

        case 'O':
            o->startX = strToI(p,0);
            o->startY = 0;
            if (*p) {
                ++p;
                o->startY = strToI(p,0);
            }
            break;

      #ifdef MY_H
        case 'K':   //-xk
            MY_setOptXK();
            break;
      #endif

        default:
            goto OPT_ERR;
        }
        break;

    case 'Z':
        dbgExLog_setSw(*p != '-');
        break;

    case '\0':
    case '?':
        usage();

    default:
  OPT_ERR:
        err_abortMsg("Incorrect command line option : %s\n", a);
    }
    return 0;
}



double Opts::strExprD(const char *p, const char ** a_p, int* a_err)
{
    double val = 0;
    int err = strExpr(p, (const char **)a_p, &val);
    if (a_err)
        *a_err = err;
    return val;
}



/* ------------------------------------------------------------------------ */

/// ���X�|���X�t�@�C���̓���
static void getResFile(char *name, slist_t **a_fnames, Opts& opts)
{
    char        buf[1024*64];
    char        *p;

    TXT1_openE(name);
    while (TXT1_getsE(buf, sizeof buf)) {
        p = strtok(buf, " \t\n");
        do {
            if (*p == ';') {
                break;
            } else if (*p == '-') {
                opts.scan(p);
            } else if (*p == ':') {
                opts.scan(p);
            } else {
                slist_add(a_fnames, p);
            }
            p = strtok(NULL, " \t\n");
        } while (p);
    }
    TXT1_close();
}



int main(int argc, char *argv[])
{
    char        inam[FIL_NMSZ];
    char        onam[FIL_NMSZ];
    char        tmpnam[FIL_NMSZ];
    slist_t     *fnames = NULL;
    slist_t     *fl;

    g_appName = strlwr(strdupE(fname_baseName(argv[0])));
    if (argc < 2)
        usage();

    ConvOne convOne;
    Opts    opts(convOne.opts());

    /* �R�}���h���C����� */
    for (int i = 1; i < argc; i++) {
        char* p = argv[i];
        if (*p == '-') {
            opts.scan(p);
        } else if (*p == ':') {
            opts.scan(p);
        } else if (*p == '@') {
            getResFile(p+1, &fnames, opts);
        } else {
            slist_add(&fnames, p);
        }
    }

    if (fnames == NULL) {
        err_abortMsg("�t�@�C�������w�肵�Ă�������\n");
    }

    if (opts.convOne_opts->mapMode >= 2) {  // ����map�t�@�C���̊g���q��ݒ肷��
        if (opts.convOne_opts->exDstExt == NULL)
            opts.convOne_opts->exDstExt = "mp";
        opts.dstExt = opts.convOne_opts->exDstExt;
    }

    if (opts.dispInfo) {
        for (fl = fnames; fl != NULL; fl = fl->link) {
            char nm[FIL_NMSZ], mdir[FIL_NMSZ];
            mdir[0] = 0;
            if (opts.srcDir)
                fname_getMidDir(mdir, fl->s);
            fname_dirNameAddExt(inam, opts.srcDir, fl->s, opts.srcExt);
            if (fil_findFirstName(nm, inam)) {      // �t�@�C��������������
                do {
                    // �o�͖���ݒ�
                    convOne.run(nm, NULL);
                } while (fil_findNextName(nm));
            }
        }
    } else {
        for (fl = fnames; fl != NULL; fl = fl->link) {
            char nm[FIL_NMSZ], mdir[FIL_NMSZ];
            mdir[0] = 0;
            if (opts.srcDir)
                fname_getMidDir(mdir, fl->s);
            fname_dirNameAddExt(inam, opts.srcDir, fl->s, opts.srcExt);
            if (fil_findFirstName(nm, inam)) {      // �t�@�C��������������
                do {
                    // �o�͖���ݒ�
                    if (opts.oname) {
                        char*   p = opts.oname;
                        if ((p[0] == '\\' || p[1] == ':') && opts.dstDir)
                            p = strcpy(tmpnam, fname_baseName(p));
                        if (opts.oname)
                            fname_dirNameAddExt(onam, opts.dstDir, p, opts.dstExt);
                        else
                            fname_dirNameChgExt(onam, opts.dstDir, p, opts.dstExt);
                    } else {
                        fname_dirDirNameChgExt(onam, opts.dstDir, mdir, fname_baseName(nm), opts.dstExt);
                    }
                    if (opts.updateFlg == 0 || fil_fdateCmp(onam, nm) < 0) {    //��ɕϊ����Aonam�̓��t���Â����
                        // ���ۂ̕ϊ�����
                        convOne.run(nm, onam);
                    }
                    if (opts.oname) // �o�̓t�@�C�������w�肳��Ă����ꍇ�͂P�t�@�C���̂�.
                        break;
                } while (fil_findNextName(nm));
            }
        }
    }
    return 0;
}
