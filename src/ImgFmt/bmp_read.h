/**
 *  @file   bmp_read.h
 *  @brief  ���������bmp�摜��W�J����.
 *  @author Masashi Kitamura
 * @note
 *  ��  ���o�͂ŐF����摜�T�C�Y�̈Ⴄ���k�f�[�^�̓W�J�� malloc&free���g�p
 */
#ifndef BMP_READ_H
#define BMP_READ_H


#if defined __cplusplus
extern "C" {
#endif


/// ���������bmp�f�[�^����A�����A�c���A�r�b�g/�s�N�Z���Aclut�̐F�����擾����.
int  bmp_getHdr(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, int *clutSize_p);

/// ���������bmp�f�[�^����A�����A�c���A�r�b�g/�s�N�Z���Aclut�̐F���A�摜�̔䗦�A���擾����.
int bmp_getHdrEx(const void *bmp_data, int *wp, int *hp, int *bppp, int *clutNump, int *resolXp, int *resolYp);


/// ���������bmp�f�[�^����A�s�N�Z���f�[�^�Aclut���擾����. (�T�C�Y�͗\��bmp_getHdr�Ŏ擾���ėp�ӂ̂���)
int  bmp_read(const void *bmp_data, void *dst, int widthByte, int h, int bpp, void *clut, int dir);

/// ���������bmp�f�[�^����Aclut �݂̂�clutSize�܂Ŏ擾����. 1�F��A8R8G8B8.
int  bmp_getClut(const void *bmp_data, void *clut, int clutSize);


/// bmp_getHdr�Ŏ擾��������,bpp��艡���o�C�g�����v�Z����.
int  bmp_width2byte(int w, int bpp, int algnByte);


#if defined __cplusplus
};
#endif




// ===========================================================================
// (��L�֐��̎g�p��)

// inline ���w��ł���ꍇ.
#if (defined __cplusplus) || (defined inline) || (__STDC_VERSION__ >= 199901L) || (defined __GNUC__)
#include <assert.h>

// �\�� stdlib.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDLIB/*VC,BCC*/) || (defined __STDLIB_H/*DMC,BCC*/) || (defined _STDLIB_H_/*GCC*/) || (defined _STDLIB_H_INCLUDED/*watcom*/)
#include <stdlib.h>     // calloc,free�̂���.

#if defined __cplusplus
static inline void* bmp_readMalloc(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=0, int dir=0);
#endif

/** bmp_data���摜��W�J�Amalloc�����������ɓ���ĕԂ�.
 *  ���������� *w_p,*h_p,*bpp_p,clut�ɉ���,�c��,bpp,clut(4*256)������. ��clut��clut�t�摜�̎��̂�.
 *  dstBpp �͐��������摜��bpp��ݒ�. 0�Ȃ�摜���g��bpp.            ���F��������ݒ�͒P�������őΏ�.
 *  algn �͉����o�C�g���̃A���C�����g. 1,2,4,8,16��z��. �ʏ��1, win-bitmap�p�Ȃ�4��ݒ�̂���.
 *  dir �� 0�Ȃ�ド�C������A1�Ȃ牺���C������擾.  win-bitmap�p�Ȃ�1��ݒ�̂���.
 */
static inline void* bmp_readMalloc(const void *bmp_data, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
    int     w, h, bpp, csz, wb;
    void    *m = NULL;

    assert(bmp_data && dstBpp >= 0);
    assert(algn == 1 || algn == 2 || algn == 4 || algn == 8 || algn == 16);

    if (bmp_getHdr(bmp_data,&w,&h,&bpp,&csz) == 0)
        return NULL;

    if (dstBpp <= 0)
        dstBpp = bpp;

    wb = bmp_width2byte(w, dstBpp, algn);
    m  = calloc(1, wb*h);

    if (bmp_read(bmp_data, m, wb, h, dstBpp, clut, dir) == 0) {
        free(m);
        return NULL;
    }

    if (w_p)   *w_p   = w;
    if (h_p)   *h_p   = h;
    if (bpp_p) *bpp_p = bpp;
    return m;
}
#endif


// �\�� stdio.h ��include���Ă���Ƃ��̂ݗ��p�\.
#if (defined _INC_STDIO/*VC,BCC*/) || (defined __STDIO_H/*DMC,BCC*/) || (defined _STDIO_H_) || (defined _STDIO_H_INCLUDED/*watcom*/)
#include <stdio.h>

#if defined __cplusplus
static void* bmp_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp=0, int algn=1, int dir=0);
#endif

static inline void* bmp_read_file(const char *fname, int *w_p, int *h_p, int *bpp_p, void* clut, int dstBpp, int algn, int dir)
{
    void*   pix = NULL;
    FILE*   fp  = fopen(fname, "rb");
    if (fp) {
        size_t l;
        fseek(fp, 0, SEEK_END);
        l = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (l) {
            void* dat = malloc(l);
            if (dat) {
                size_t sz = fread(dat, 1, l, fp);
                if (sz == l)
                    pix  = bmp_readMalloc(dat, w_p, h_p, bpp_p, clut, dstBpp, algn, dir);
                free(dat);
            }
        }
        fclose(fp);
    }
    return pix;

}
#endif

#endif  // inline���g����ꍇ.




#endif  // BMP_READ_H
