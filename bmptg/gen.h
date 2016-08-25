/**
 *  @file gen.h
 *  @brief  �s�N�Z���f�[�^�̃T�C�Y�����R�s�[��
 *  @author Masashi Kitamura
 *  @date   199?
 */
#ifndef GEN_H
#define GEN_H
#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

void gen_newSizePix8(uint8_t **a_pix, int *a_w, int *a_h, int sw,int sh,int sx,int sy, int colKey, int vv_w,int vv_h,int vv_x,int vv_y);
void gen_newSizePix32(uint8_t **a_pix, int *a_w,int *a_h, int sw,int sh,int sx,int sy, int colKey, int vv_w,int vv_h,int vv_x,int vv_y);
void gen_celSz8(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey);
void gen_celSz32(uint8_t **a_pix, int *a_w, int *a_h, int celW, int celH, int colKey);
void gen_nukiRect8(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int nukiClut, int grdMd);
void gen_nukiRect32(uint8_t **a_pix, int *a_w, int *a_h, int *a_x, int *a_y, int celW, int celH, int grdMd);

int gen_clmp8(uint8_t **a_map, int *a_mapSz, int *a_celNum, uint8_t **a_pix, int *a_w, int *a_h, int x0, int y0, int texW, int texH, int cw, int ch, int md, int bpp, int nukiCo, int styl, void *clut, int w0, int h0);
int gen_clmp32(uint8_t **a_map, int *a_mapSz, int *a_celNum, uint8_t **a_pix, int *a_w, int *a_h, int x0, int y0, int texW, int texH, int cw, int ch, int md, int bpp, int nukiCo, int styl, void *clut, int w0, int h0);

#ifdef __cplusplus
};
#endif

/*
�� �Z��(�`�b�v)���}�b�v���B
���Ƃ���640x480 �̉摜��16x16�̃Z���P�ʂɂ΂炵�ă_�u��Ȃ�����
���ׂȂ�����256x256���̃e�N�X�`���摜�� �Ǝ��̃}�b�v�t�@�C����
�����\�B
�@-mh,-mp�ɂ��A�摜���A�w���`(-mcN:M)�ŃZ���ɕ������āA
�����̃Z���𓝈ꂵ�ĉ摜�����炵�ċl�ߍ��킹���e�N�X�`����
�𐶐��ł��邪�A���̂Ƃ��΂ɂȂ�}�b�v�i�w�b�_�j���͈ȉ���
�悤�Ȃ�B
�i�l�̓��g���G���f�B�A��(�C���e����)�B�w�b�_���͂R�Q�o�C�g)

�@OFFS  SIZE    name    �⑫
�@00    4       id      "MAP\0" or "MAP\1"
�@04    4       offs    �e�N�X�`���摜�ւ̃I�t�Z�b�g(-mp���̂݁j
�@08    4       rsv.    �\��. �l=0
�@0C    1       flags   �t���O. 2=1�Z��������1�o�C�g. �ȊO��2�o�C�g
�@0D    1       bpp     �e�N�X�`����BPP
�@0E    2       celNum  �Z���̐�
�@10    2       mapW    �}�b�v�̉���(�Z����)
�@12    2       mapH    �}�b�v�̏c��(�Z����)
�@14    2       celW    �Z���̉���(�s�N�Z����)
�@16    2       celH    �Z���̉���(�s�N�Z����)
�@18    2       orgW    ���摜�̉���(�s�N�Z����)
�@1A    2       orgH    ���摜�̉���(�s�N�Z����)
�@1C    2       x0      �\�����W�I�t�Z�b�g�w(�s�N�Z����)
�@1E    2       y0      �\�����W�I�t�Z�b�g�x(�s�N�Z����)
�@30    mapW*mapH       �}�b�v���

�E�P�Z���̑�������
�@�@�@1�o�C�g�̂Ƃ��A0:�����Z�� 1..255:�Z���ԍ�
�@�@�@2�o�C�g�̂Ƃ�
�@�@�@�@bit 13-0:�@0:�����Z�� 1�` �Z���ԍ�
�@�@�@�@bit 14:�@�@�������� 0=�܂܂Ȃ��Z���@1=�܂ރZ��
�@�@�@�@bit 15:�@�@0:�����F,�������F���܂ރZ���@1=�������F�݂̂̃Z��

�E�e�N�X�`���́A�Z���P�Ԃ��炪���������ɍ�����E�ɏォ�牺�ɋl�߂�
�@�z�u�����B0�Ԃ̓����Z���悤�̉摜�͗p�ӂ���Ȃ��̂Œ��ӁB
*/

#endif
