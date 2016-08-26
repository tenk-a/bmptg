/**
 *	@file	DecreaseColorIfWithin256.h
 *	@brief	32�r�b�g�F��̐F����256�F(�w��F��)�Ɏ��܂�Ȃ�A256�F��ɕϊ�.
 *	@author	Masashi KITAMURA
 */

#ifndef DECREASECOLORIFWITHIN256_H
#define DECREASECOLORIFWITHIN256_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>


/**	32�r�b�g�F��̐F����256�F(�w��F��)�Ɏ��܂�Ȃ�A256�F��ɕϊ�. ���܂�Ȃ��ꍇ�͕ϊ����Ȃ�.
 */
template<class DMY=void>
class DecreaseColorIfWithin256 {
public:
	DecreaseColorIfWithin256() {}
	DecreaseColorIfWithin256(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false) {
		conv(pDst,pSrc,w,h,pClut,clutSize, transf);
	}

	static bool conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false);

	static unsigned countColor(const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf=false);

private:
	template<typename T> static unsigned binary_find_tbl_n(T* tbl, unsigned num, const T& key);
	template<typename T> static unsigned binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key);
};



/// �F�����J�E���g����. �`�F�b�N�̂��߂�1�F�]���ɕK�v�ŁAclut�̃������� clutSize+1 �m�ۂ��Ă�������.
template<class DMY>
unsigned DecreaseColorIfWithin256<DMY>::countColor(const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf)
{
	assert(clutSize > 0);
	pClut[0] = 0;
	unsigned num = unsigned(transf);
	for (unsigned i = 0; i < w*h && num <= clutSize; ++i) {
		unsigned c = pSrc[i];
		binary_insert_tbl_n(pClut, num, c);
	}
	return num;
}



/**	32�r�b�g�F��̐F����256�F(�w��F��)�Ɏ��܂�Ȃ�A256�F��ɕϊ�. ���܂�Ȃ��ꍇ�͕ϊ����Ȃ�.
 */
template<class DMY>
bool DecreaseColorIfWithin256<DMY>::conv(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h, unsigned* pClut, unsigned clutSize, bool transf)
{
	/// ���ۂ̐F�����AclutSize�ȓ��Ɏ��܂邩���`�F�b�N.
	unsigned clut[256+1];		// ��]���Ɋm��.
	unsigned num = 0;
	if (clutSize > 256)
		clutSize = 256;

	num = countColor(pSrc, w, h, clut, clutSize, transf);
	if (num > clutSize)
		return false;

	// clut���Ɏ��܂�F���������ꍇ.
	std::memcpy(pClut    , clut, num*sizeof(pClut[0]));
	if (num < clutSize)
		std::memset(pClut+num, 0, (clutSize-num)*sizeof(pClut[0]));
	for (unsigned i = 0; i < w*h; ++i)
		pDst[i] = binary_find_tbl_n(&clut[0], num, pSrc[i]);
	return true;
}



/** ���񂳂ꂽ�e�[�u������key������ʒu(�ԍ�:0 �` num-1)��Ԃ�. �Ȃ���΁Anum��Ԃ�.
 */
template<class DMY>
template<typename T>
unsigned DecreaseColorIfWithin256<DMY>::binary_find_tbl_n(T* tbl, unsigned num, const T& key)
{
	unsigned	low = 0;
	unsigned    hi  = num;
	while (low < hi) {
		unsigned	mid = (low + hi - 1) / 2;
		if (key < tbl[mid]) {
			hi = mid;
		} else if (tbl[mid] < key) {
			low = mid + 1;
		} else {
			return mid;
		}
	}
	return num;
}



/** �e�[�u��pTbl�ɒlkey��ǉ�. �͈̓`�F�b�N�͗\�ߍs���Ă��邱�ƑO��.
 *  @return �e�[�u������key�̈ʒu.
 */
template<class DMY>
template<typename T>
unsigned DecreaseColorIfWithin256<DMY>::binary_insert_tbl_n(T* pTbl, unsigned& rNum, const T& key) {
	unsigned 	hi  = rNum;
	unsigned 	low = 0;
	unsigned 	mid = 0;
	while (low < hi) {
		mid = (low + hi - 1) / 2;
		if (key < pTbl[mid]) {
			hi = mid;
		} else if (pTbl[mid] < key) {
			++mid;
			low = mid;
		} else {
			return mid;	/* �������̂��݂������̂Œǉ����Ȃ� */
		}
	}

	// �V�K�o�^
	++rNum;

	// �o�^�ӏ��̃��������󂯂�
	for (hi = rNum; --hi > mid;) {
		pTbl[hi] = pTbl[hi-1];
	}

	// �o�^
	pTbl[mid] = key;
	return mid;
}


#endif	// DECREASECOLORIFWITHIN256_H