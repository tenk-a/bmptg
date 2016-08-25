/**
 *	@file	RegOpt.h
 *	@brief	tspiro.exe�Ɛ�psusie�v���O�C�������W�X�g������Ă���肷��.
 *	@author	Masashi KITAMURA
 *	date	200?
 *	note
 *		2007	bpp32use�֌W�̎���
 */

#ifndef REGOPT_H
#define REGOPT_H
// #include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


#define REGOPT_VER			0x0100
#define REGOPT_S_VER		0x0100
#define REGOPT_CLUT_NUM 	2048
#define REGOPT_FILE_NAME	"Software\\tenk\\TSPIRO"	//���W�X�g���ɍ����

typedef struct regopt_t {
	// regopt�R���g���[������̎w���Bx�͖������ior�p��)
	SHORT	ver;		// ���WORD. �݊������ێ����Ă���Ԃ͌Œ�̒l. ���� �o�[�W�����ԍ�
	SHORT	size;		// ���̍\���̂̃T�C�Y
	BYTE	enable; 	// bit0:���W�X�g���̓��e��	  0=�g��Ȃ� 1=�g��
						// bit1:�摜���̎擾��v��  0=���Ȃ�	 1=����
	BYTE	alpType;	// �����̈��� 0:������  1:�u�����h  2:���Z  3:���Z  4:PS���[�����u�����h
	BYTE	alpMode;	// 0:�S�̂Ƀ��l�𔽉f  1:���l�� 0�ȊO�̃s�N�Z���݂̂ɔ��f
	BYTE	alpRev; 	// ���l�̈����� 0:0xFF�Ȃ�s�����`0�Ȃ瓧��  1:0�Ȃ�s�����`0xFF�Ȃ瓧��
	BYTE	alpHalf;	// 0:�ʏ� 1:���l�� 128��100%�ƂȂ�悤�Ɉ���
	BYTE	keyMode;	// ���F���� 0:����	1:���ׂă_�C���N�g�J���[  2:256�F�ȉ���clut�ԍ�,���F�̓_�C���N�g
	BYTE	aspReq; 	//x �\��:���摜�̃h�b�g�䂪1:1�łȂ��Ƃ����� 0:���Ȃ� 2:����
	BYTE	bpp32use;	// Bpp 24�łȂ� 32 �Ƃ��āA�摜���Ăяo�����֕Ԃ�.
	SHORT	tone;		// �P�x. 1000 �� 100%�Ƃ���
	SHORT	alpVal; 	// �s�N�Z���ŗL�̃��l�������0..255�i�K�ɂ���
	SHORT	clutOfs;	// clut�e�[�u�������̒l�����炵�ĕ`��B
	SHORT	clutKey;	// clut�ł̔����F�ԍ�
	DWORD	colKey; 	// �����F RGB�l�B
	//
	DWORD	gridCol;	// �F
	DWORD	bgCol1; 	// BG�F1
	DWORD	bgCol2; 	// BG�F2
	SHORT	gridW;		// �O���b�h�̉��� 2�`1024
	SHORT	gridH;		// �O���b�h�̏c�� 2�`1024
	BYTE	bgFileFlg;	//x �\��:BG�\���v�� 0:���Ȃ�  1:����
	BYTE	bgGridMode; // BG�F��O���b�h���ꊇ 0:off  1:on
	BYTE	gridFlg;	// �O���b�h���`��� 0:off  1:on
	BYTE	rngChk; 	//x (�O���b�h�͈͂����߂�. �p�Ă���)
	//char bgname[1024];	//x bg�摜��. bmp�̂݁B		// bgname�͕ʂ̐�p�L�[�ɂ���
	BYTE	mapOffSw;	// �}�b�v���摜�̃}�b�v�𖳎�(�����e�N�X�`�������̂܂ܕ\��)
	BYTE	dmy[3];
	DWORD	rsv[16-12]; //
} regopt_t;


typedef struct regopt_s_t {
	// regopt�R���g���[���ւ̃��|�[�g. TSPIRO v0.50�ł͂܂����Q�ƁB�܂� x�t���͏����̗\��
	SHORT	var;			// ���WORD. �݊������ێ����Ă���Ԃ͌Œ�̒l. ���� �o�[�W�����ԍ�
	SHORT	size;			// ���̍\���̂̃T�C�Y

	DWORD	time;			// timeGetTime() �̒l�B�X�V�`�F�b�N�p
	SHORT	bpp;			// ���� BPP. spi�Ăяo�����ɂ͂��ׂ� 24�r�b�g�F�Ƃ��ēn����邽��
	SHORT	clutNum;		// ����clut�̐�(�������ő� REGOPT_CLUT_NUM)
	LONG	w;				// ���摜�̉���
	LONG	h;				// ���摜�̏c��
	LONG	x0; 			// ���摜�̎n�_x
	LONG	y0; 			// ���摜�̎n�_y
	LONG	xresol; 		// ���摜�ɂ��������h�b�g��(bmp��)
	LONG	yresol; 		// ���摜�ɂ������c�h�b�g��(bmp��)
	short	clutKey;		// ���摜��clut���̔����F�ԍ�+1 (0�̎������F����. 1�ȏ�̎��A-1����Δ����F�ԍ�)
	short	dmy0;			//x
	LONG	colorKey;		// ���摜�̃_�C���N�g�J���[�ł̔����F (0�̎������F����. 1�ȏ�̎�, -1����Δ����F)
	//int clut[CLUT_NUM];	// ���W�X�g���ɐݒ肷��clut // clut�͕ʂ̐�p�L�[�ɂ���
	DWORD	rsv2[16-9]; 	//
} regopt_s_t;


int RegOpt_Get(regopt_t *ro);
int RegOpt_GetPicture(regopt_t *ro, BYTE *pix, int w, int h, int dbpp, DWORD *clut0, int clutNum, const BYTE* src, int srcWidByt, int srcBpp);
int RegOpt_Put(regopt_s_t *rs, int w, int h, int bpp, const DWORD *clut, int clutNum, const char *fname);


#ifdef __cplusplus
}
#endif

#endif
