/**
 *	@file	mem_mac.h
 *	@brief	�������A�N�Z�X�̂��߂̃}�N��
 *	@author	�k����j	NBB00541@nifty.com
 */

#ifndef MEM_MAC_H
#define MEM_MAC_H

// ��{�^�Ƃ���uint8_t,uint16_t,uint32_t,uint64_t����`����Ă��邱��
#include "def.h"


//-------------------------------------------------------------
// int8��int16�̒l���Ȃ�����΂炵���肷��}�N��
//-------------------------------------------------------------

/// 8bit������㉺�ɂȂ���16�r�b�g���ɂ���
#define BB(a,b) 		((((uint8_t)(a))<<8)|(uint8_t)(b))

/// 16bit������㉺�ɂȂ���32�r�b�g���ɂ���
#define WW(a,b) 		((((uint16_t)(a))<<16)|(uint16_t)(b))

/// 32bit������㉺�ɂȂ���64�r�b�g���ɂ���
#define LL(a,b)			((((uint64_t)(a))<<32)|(uint32_t)(b))

/// 8bit��4����ʂ��珇�ɂȂ���32�r�b�g���ɂ���
#define BBBB(a,b,c,d)	((((uint8_t)(a))<<24)|(((uint8_t)(b))<<16)|(((uint8_t)(c))<<8)|((uint8_t)(d)))

#ifdef BIG_ENDIAN
#define CC(a,b)			BB(a,b)			///< 2�o�C�g�������int16�Ƃ��Ĉ������ꍇ�́A�萔��r�p�}�N��
#define CCCC(a,b,c,d)	BBBB(a,b,c,d)	///< 4�o�C�g�������int32�Ƃ��Ĉ������ꍇ�́A�萔��r�p�}�N��
#else	// LITTLE_ENDIAN
#define CC(a,b)			BB(b,a)			///< 2�o�C�g�������int16�Ƃ��Ĉ������ꍇ�́A�萔��r�p�}�N��
#define CCCC(a,b,c,d)	BBBB(d,c,b,a)	///< 4�o�C�g�������int32�Ƃ��Ĉ������ꍇ�́A�萔��r�p�}�N��
#endif

#define GLB(a)			((uint8_t)(a))						///< a�� int16�^�Ƃ��ĉ��ʃo�C�g�̒l���擾
#define GHB(a)			((uint8_t)(((uint16_t)(a))>>8))		///< a�� int16�^�Ƃ��ď�ʃo�C�g�̒l���擾
#define GLLB(a) 		((uint8_t)(a))						///< a�� int32�^�Ƃ��čŉ��ʃo�C�g�̒l���擾
#define GLHB(a) 		((uint8_t)(((uint16_t)(a))>>8))		///< a�� int32�^�Ƃ��ĉ�2�o�C�g�ڂ̒l���擾
#define GHLB(a) 		((uint8_t)(((uint32_t)(a))>>16))	///< a�� int32�^�Ƃ��ĉ�3�o�C�g�ڂ̒l���擾
#define GHHB(a) 		((uint8_t)(((uint32_t)(a))>>24))	///< a�� int32�^�Ƃ��ĉ�4�o�C�g�ڂ̒l���擾
#define GLW(a)			((uint16_t)(a))						///< int32�^�Ƃ��Ă� a �̉�16�r�b�g�̒l���擾
#define GHW(a)			((uint16_t)(((uint32_t)(a))>>16))	///< int32�^�Ƃ��Ă� a �̏�16�r�b�g�̒l���擾



//-------------------------------------------------------------
// �A���C�����g���C�ɂ����A�������փA�N�Z�X���邽�߂̃}�N��
//-------------------------------------------------------------

#define	PEEKB(a)		(*(const uint8_t *)(a))			///< �A�h���X a ���� 1�o�C�g�ǂݍ���
#define	POKEB(a,b)		(*(uint8_t *)(a) = (b))			///< �A�h���X a �� 1�o�C�g��������

//	�C���e��/���g���G���f�B�A���E�f�[�^�ɃA�N�Z�X����ꍇ
#if defined _M_IX86	 || defined _X86_ 	// X86 �́A�A���C�����g���C�ɂ���K�v���Ȃ��̂Œ��ڃA�N�Z�X
#define	PEEKiW(a)		(*(const uint16_t *)(a))
#define	PEEKiD(a)		(*(const uint32_t *)(a))
#define PEEKiB3(s)		((*(const uint16_t*)(s)) | ((*(const uint8_t*)((s)+2))<<16))
#define	POKEiW(a,b)		(*(uint16_t *)(a) = (b))
#define	POKEiD(a,b)		(*(uint32_t *)(a) = (b))
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEiW(a,GLW(b)))
#else					//�A���C�����g�΍�ŁA1�o�C�g�ÂA�N�Z�X
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#define PEEKiB3(s)		BBBB(0, ((const uint8_t*)(s))[2], ((const uint8_t*)(s))[1], ((const uint8_t*)(s))[0])
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a), GLLB(b)))
#endif

//		���g���[��/�r�b�O�G���f�B�A���E�f�[�^�ɃA�N�Z�X����ꍇ
#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define PEEKmB3(s)		BBBB(0, ((const uint8_t*)(s))[0], ((const uint8_t*)(s))[1], ((const uint8_t*)(s))[2])
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))
#define POKEmB3(a,b)	(POKEB((a)+0, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a)+2, GLLB(b)))

//		����CPU�́A�f�t�H���g�̃G���f�B�A���ŃA�N�Z�X����ꍇ(�o�C�g�P�ʂ̃A�h���X�\)
#ifdef BIG_ENDIAN
#define	PEEKW(a)		PEEKmW(a)
#define	PEEKB3(a)		PEEKmB3(a)
#define	PEEKD(a)		PEEKmD(a)
#define	POKEW(a,b)		POKEmW(a,b)
#define	POKEB3(a,b)		POKEmB3(a,b)
#define	POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define	PEEKW(a)		PEEKiW(a)
#define	PEEKB3(a)		PEEKiB3(a)
#define	PEEKD(a)		PEEKiD(a)
#define	POKEW(a,b)		POKEiW(a,b)
#define	POKEB3(a,b)		POKEiB3(a,b)
#define	POKED(a,b)		POKEiD(a,b)
#endif


//-------------------------------------------------------------
// �������E�R�s�[�}�N��. �A���C�����g�͗��p�ґ��ŋC�����邱��
//-------------------------------------------------------------

#define STREND(p)			((p)+ strlen(p))

/// ty�^��0�� �A�h���X d���� sz/sizeof(ty)��������
#define MEMCLR_TY(d, sz, ty) do {ty *d__ = (ty *)(d); unsigned c___ = (sz)/sizeof(ty); do {*d___++ = 0;} while(--c___);} while(0)
#define MEMCLR(d, sz)		MEMCLR_TY(d, s, sz, uint8_t)			///< d�����sz�o�C�g��0�N���A.
#define MEMCLR2(d, sz)		MEMCLR_TY(d, s, sz, uint16_t)			///< d�����sz�o�C�g��0�N���A. �A�h���X,�T�C�Y�Ƃ���2�̔{���̂��ƁB
#define MEMCLR4(d, sz)		MEMCLR_TY(d, s, sz, uint32_t)			///< d�����sz�o�C�g��0�N���A. �A�h���X,�T�C�Y�Ƃ���4�̔{���̂��ƁB
#define MEMCLR8(d, sz)		MEMCLR_TY(d, s, sz, uint64_t)			///< d�����sz�o�C�g��0�N���A. �A�h���X,�T�C�Y�Ƃ���8�̔{���̂��ƁB
#define MEMCLR16(d, sz)		MEMCLR_TY(d, s, sz, __UINT128)			///< d�����sz�o�C�g��0�N���A. �A�h���X,�T�C�Y�Ƃ���16�̔{���̂��ƁB

//#ifndef ZeroMemory
//#define ZeroMemory(a,sz)	MEMCLR(a,sz)
//#endif

/// ty�^��s�� �A�h���X d���� sz/sizeof(ty)��������
#define MEMSET_TY(d, s, sz, ty) do {ty *d___ = (ty *)(d); ty s___  = (ty)(s);   unsigned c___ = (sz)/sizeof(ty); do {*d___++ = s___;} while(--c___);} while(0)
#define MEMSET(d, s, sz)	MEMSET_TY(d, s, sz, uint8_t)			///< d �֒ls��sz�o�C�g��������.
#define MEMSET2(d, s, sz)	MEMSET_TY(d, s, sz, uint16_t)			///< d �֒ls��sz/2��������. �A�h���X,�T�C�Y�Ƃ���2�̔{���̂��ƁB
#define MEMSET4(d, s, sz)	MEMSET_TY(d, s, sz, uint32_t)			///< d �֒ls��sz/4��������. �A�h���X,�T�C�Y�Ƃ���4�̔{���̂��ƁB
#define MEMSET8(d, s, sz)	MEMSET_TY(d, s, sz, uint64_t)			///< d �֒ls��sz/8��������. �A�h���X,�T�C�Y�Ƃ���8�̔{���̂��ƁB

/// ty�^�̃|�C���^�Ƃ���s����d�� sz/sizeof(ty)�R�s�[����
#define MEMCPY_TY_TR(d, s, sz, ty, EXPR) do {ty *d___ = (ty *)(d); ty *s___ = (ty *)(s); unsigned c___ = ((unsigned)(sz)/sizeof(ty)); do {(EXPR);} while(--c___);} while(0)
#define MEMCPY_TY(d, s, sz, ty) MEMCPY_TY_TR(d,s,sz,ty, (*d___++ = *s___++))
#define MEMCPY(d, s, sz)	MEMCPY_TY(d, s, sz, int8_t)			///< s ���� d �� sz�o�C�g�R�s�[.
#define MEMCPY2(d, s, sz)	MEMCPY_TY(d, s, sz, int16_t)			///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���2�̔{���̂��ƁB
#define MEMCPY4(d, s, sz)	MEMCPY_TY(d, s, sz, int32_t)			///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���4�̔{���̂��ƁB
#define MEMCPY8(d, s, sz)	MEMCPY_TY(d, s, sz, int64_t)			///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���8�̔{���̂��ƁB
#define MEMCPY16(d, s, sz)	MEMCPY_TY(d, s, sz, __INT128)		///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���16�̔{���̂��ƁB
///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���32�̔{���̂��ƁB
#define MEMCPY32(d, s, sz)	MEMCPY_TY_TR(d, s, sz, __INT128, (*d___++ = *s___++, *d___++ = *s___++))
///< s ���� d �� sz�o�C�g�R�s�[. �A�h���X,�T�C�Y�Ƃ���64�̔{���̂��ƁB
#define MEMCPY64(d, s, sz)	MEMCPY_TY_TR(d, s, sz, __INT128, (*d___++ = *s___++, *d___++ = *s___++, *d___++ = *s___++, *d___++ = *s___++))

/// ty�^�̃|�C���^�Ƃ���s����d�֌�납�� sz/sizeof(ty)�R�s�[����
#define MEMRCPY_TY(d,s, sz, ty) do {ty *d___ = (ty *)(d); ty *s___ = (ty *)(s); unsigned c___ = ((unsigned)(sz)/sizeof(ty)); while (c___-- > 0) {d___[c___] = s___[c___]; } } while (0)
#define MEMRCPY(d, s, sz)	MEMRCPY_TY(d, s, sz, int8_t)
#define MEMRCPY2(d, s, sz)	MEMRCPY_TY(d, s, sz, int16_t)
#define MEMRCPY4(d, s, sz)	MEMRCPY_TY(d, s, sz, int32_t)
#define MEMRCPY8(d, s, sz)	MEMRCPY_TY(d, s, sz, int64_t)
#define MEMRCPY16(d, s, sz)	MEMRCPY_TY(d, s, sz, __INT128)



//-------------------------------------------------------------
// �摜���������́A�}�N��
//-------------------------------------------------------------
//	�F�l argb �� win��bmp/dib�����8�r�b�g�� a,r,g,b��
//  �� bpp   1�s�N�Z��������̃r�b�g��

/// bpp �� �o�C�g���Ɍv�Z
#define BPP2BYT(bpp)		(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)

/// ������bpp���o�C�g�����v�Z
#define WID2BYT(w,bpp)		(((w) * "\1\2\4\4\10\10\10\10\20\20\20\20\20\20\20\20\30\30\30\30\30\30\30\30\40\40\40\40\40\40\40\40"[(bpp)-1] + 7) >> 3)

/// ������bpp���o�C�g�����v�Z.���̂Ƃ�bmp��������4�̔{���ɂ���
#define WID2BYT4(w,bpp) 	((WID2BYT(w,bpp) + 3) & ~3)

/// �o�C�g����bpp ��艡���s�N�Z���������߂�
#define BYT2WID(w,bpp)		(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

/// 8bit�l��a,r,g,b���q���� 32bit �̐F�l�ɂ���
#define ARGB(a,r,g,b)		((((uint8_t)(a))<<24)|(((uint8_t)(r))<<16)|(((uint8_t)(g))<<8)|((uint8_t)(b)))

/// argb�l���� blue�̒l���擾
#define ARGB_B(argb)		((uint8_t)(argb))

/// argb�l���� green�̒l���擾
#define ARGB_G(argb)		((uint8_t)((argb)>>8))

/// argb�l���� green�̒l���擾
#define ARGB_R(argb)		((uint8_t)((argb)>>16))

/// argb�l���� alpha�̒l���擾
#define ARGB_A(argb)		(((uint32_t)(argb))>>24)

/// argb�l�̊e�l���m���|����/256�Ŋ���
#define ARGBxARGB(x,y)		BBBB((GHHB(x)*GHHB(y))>>8, GHLB(x)*(GHLB(y))>>8, GLHB(x)*(GLHB(y))>>8, GLLB(x)*(GLLB(y))>>8)

/// argb�l�̕��т�abgr ���тɕϊ�����
#define ARGBtoABGR(c)		(((c) & 0xFF00FF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// a8r8g8b8���������̂� a4r4g4b4 �ɕϊ�
#define ARGB_8888to4444(c)	((((c)>>16)&0xF000)|(((c)>>12)&0x0F00)|(((c)>>8)&0xF0)|((uint8_t)(c)>>4))

/// a4r4g4b4���������̂� a8r8g8b8 �ɕϊ�
#define ARGB_4444to8888(c)	((((c)&0xF000)<<16)|(((c)&0x0F00)<<12)|(((c)&0xF0)<<8)|((uint8_t)((c)<<4)))

/// 8�r�b�g�l��a,r,g,b�̊e���4�r�b�g���q����16�r�b�g�����ɕϊ�
#define ARGB4444(a,r,g,b)	(((((a)<<8) | (g)) & 0xF0F0) | (((((r)<<8)|(b)) & 0xF0F0) >> 4))
	//#define ARGB4444(a,r,g,b) ((((a)&0xF0)<<8) | (((r)&0xF0)<<4) | ((g)&0xF0) | (((b)>>4)&0xF))

/// a8r8g8b8 �� a1r5g5b5 ��16�r�b�g�l�ɕϊ�
#define ARGB_8888to1555(c)	((((c)>>16)&0x8000)|(((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((uint8_t)((c)>>3)))

/// a1r5g5b5 �� a8r8g8b8 �ɕϊ�
#define ARGB_1555to8888(c)	((((c)&0x8000)<<16)|(((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((uint8_t)(c)<<3)))

/// 8�r�b�g�l��a,r,g,b �̏�� 1,5,5,5�r�b�g���擾���� 16�r�b�g�l�ɕϊ�
#define ARGB1555(a,r,g,b)	(((a>>7)<<15)|(((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))

/// 8�r�b�g�l��r,g,b �̏�� r5,g6,b5�r�b�g���擾���� 16�r�b�g�l�ɕϊ�
#define RGB565(r,g,b)		((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|(((b)>>3)&0x1f))

//#define COLORPACK(r,g,b,a)	((((uint32_t)((a) * 255)) << 24) | (((uint32_t)((r) * 255)) << 16) | (((uint32_t)((g) * 255)) << 8) | (uint32_t)((b) * 255))

#define ARGB_FA(argb)		(((uint8_t)((uint32_t)(argb)>>24))* (1/255.f))	///< argb�l���Aalpha�� 0�`1.0F��Float�l�Ƃ��Ď擾
#define ARGB_FR(argb)		(((uint8_t)((uint32_t)(argb)>>16))* (1/255.f))	///< argb�l���Ared  �� 0�`1.0F��Float�l�Ƃ��Ď擾
#define ARGB_FG(argb)		(((uint8_t)((uint32_t)(argb)>>8)) * (1/255.f))	///< argb�l���Agreen�� 0�`1.0F��Float�l�Ƃ��Ď擾
#define ARGB_FB(argb)		(((uint8_t)(argb))              * (1/255.f))	///< argb�l���Ablue �� 0�`1.0F��Float�l�Ƃ��Ď擾

#define ARGB_ALP(a)			ARGB((a),0xFF,0xFF,0xFF)					///< ���̂݊|�����킹��萔�𐶐�
#define ARGB_DFLT			ARGB(0xFF,0xFF,0xFF,0xFF)					///< �F���|�����킹��Ƃ��́A�f�t�H���g�l
#define ARGB_WHITE			ARGB(0xFF,0xFF,0xFF,0xFF)					///< ����ARGB�l
#define ARGB_BLACK			ARGB(0xFF,0x00,0x00,0x00)					///< ����ARGB�l

/// PS2��ABGR�l��(win��)�ʏ��ARGB�l�ɕϊ�
#define ARGB_FROM_PS2COL(c)	(((((uint32_t)(c)>>24)*255>>7)<<24)|((c)&0xFF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// (win��)�ʏ��ARGB�l��PS2��ABGR�ɕϊ�
#define ARGB_TO_PS2COL(c)	(((((uint32_t)(c)>>24)*128/255)<<24)|((c)&0xFF00)|((uint8_t)((c)>>16))|(((uint8_t)(c))<<16))

/// ARGB���т�RGBA���тɕϊ�
#define ARGB_TO_RGBA(c)		((uint8_t)((c) >> 24)|(uint32_t)((c)<<8))

/// RGBA���т�ARGB���тɕϊ�
#define ARGB_FROM_RGBA(c)	(((uint8_t)(c) << 24)|((uint32_t)(c)>>8))


#endif	// MEM_MAC_H
