/**
 *	@file	misc.h
 *	@brief	コマンドラインツールなサンプルルーチンのお供用に、適当に集めたサブルーチン
 *	@author	Masashi KITAMURA
 */
#ifndef MISC_H
#define MISC_H

// -------------------------------------------------------------------
// 適当に標準ライブラリをinclude

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <string>
#include <algorithm>
#include <mbctype.h>


#if (defined _MSC_VER && _MSC_VER < 1600) || (defined __BORLANDC__ && __BORLANDC__ <= 0x0550)
typedef	__int8				int8_t;
typedef unsigned __int8		uint8_t;
typedef __int16				int16_t;
typedef unsigned __int16	uint16_t;
typedef __int32				int32_t;
typedef unsigned __int32	uint32_t;
typedef __int64				int64_t;
typedef unsigned __int64	uint64_t;

typedef __int64 			intmax_t;
typedef uint64_t 			uintmax_t;

 #if defined(_WIN64)
  typedef int64_t			intptr_t;
  typedef uint64_t			uintptr_t;
 #elif defined _MSC_VER
  typedef __w64 int32_t		intptr_t;
  typedef __w64 uint32_t	uintptr_t;
 #else
  typedef int32_t			intptr_t;
  typedef uint32_t			uintptr_t;
 #endif
#else
  #include <stdint.h>
#endif



// -------------------------------------------------------------------
#define CSTR_CAT(a,b)		CSTR_CAT_2(a,b)
#define CSTR_CAT_2(a,b)		a##b
#define STATIC_ASSERT(cc)	struct CSTR_CAT(STATIC_ASSERT_CHECK_FAIL,__LINE__) 			\
									{ char dmy[2*((cc)!=0) - 1];};						\
							enum { CSTR_CAT(STATIC_ASSERT_CHECK,__LINE__)				\
								= sizeof( CSTR_CAT(STATIC_ASSERT_CHECK_ST,__LINE__) ) }	\




// -------------------------------------------------------------------

/** ファイル・ロード
 */
template<typename T>
bool file_load(const char* name, std::vector<T>& buf)
{
	FILE* fp = fopen(name, "rb");
	if (fp == NULL)
		return false;

	//x int		sz = filelength(fileno(fp));
	fseek(fp, 0L, SEEK_END);
	int	sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	if (sz <= 0)
		return false;

	buf.resize( (sz+sizeof(T)-1) / sizeof(T) );
	fread((void*)&buf[0], 1, sz, fp);

	bool 	rc = (ferror(fp) == 0);
	fclose(fp);
	return 	rc;
}


/** ファイル・セーブ
 */
template<typename T>
bool file_save(const char* name, const T* buf, unsigned byteSize)
{
	FILE* fp = fopen(name, "wb");
	if (fp == NULL)
		return false;
	if (byteSize)
		fwrite((void*)buf, 1, byteSize, fp);
	bool 	rc = (ferror(fp) == 0);
	fclose(fp);
	return 	rc;
}




// ------------------------------------------------
// ヘッダに書くために無理やり templateに..CHR_T=charのみ!


/// 文字 C が MS全角の１バイト目か否か. (utf8やeucは \ 問題は無いので 0が帰ればok)
int fname_ismbblead(unsigned char c)
{
  #if defined _WIN32 || defined _WIN64
 //	return IsDBCSLeadByte(c) != 0;
 // #elif defined HAVE_MBCTYPE_H
	return _ismbblead(c) != 0;
  #elif defined USE_SJIS
  	return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFE);
  #else
	return 0;
  #endif
}


/// ファイルパス名中のディレクトリを除いたファイル名の位置を返す.
template<typename CHR_T>
char *fname_baseName(const CHR_T *adr)
{
	const char *p;
	for (p = adr; *p != '\0'; ++p) {
		if (*p == ':' || *p == '/'
		  #if defined _WIN32 || defined _WIN64 || defined _DOS
			|| *p == '\\'
		  #endif
		) {
			adr = p + 1;
		}
		if (fname_ismbblead((*(unsigned char *)p)) && *(p+1) )
			++p;
	}
	return (char*)adr;
}



/**	拡張子の位置を返す.
 */
template<typename CHR_T>
char *fname_getExt(const CHR_T *name)
{
	const char *p;

	name = fname_baseName(name);
	p = strrchr(name, '.');
	if (p) {
		return (char*)(p+1);
	}
	return (char*)name + strlen(name);
}



/**	拡張子を、ext に変更する.
 */
template<typename CHR_T>
char *fname_chgExt(CHR_T filename[], const char *ext)
{
	char *p;

	p = fname_baseName(filename);
	p = strrchr( p, '.');
	if (p == NULL) {
		if (ext) {
			strcat(filename,".");
			strcat( filename, ext);
		}
	} else {
		if (ext == NULL)
			*p = 0;
		else
			strcpy(p+1, ext);
	}
	return filename;
}



/**	拡張子がなければ、ext を追加する.
 */
template<typename CHR_T>
char *fname_addExt(CHR_T filename[], const char *ext)
{
	if (strrchr(fname_baseName(filename), '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}


// ------------------------------------------------
// 非常に手抜きなstd::string対応...



inline std::string fname_chgExt(std::string fname, const char* ext) {
	char buf[2048];
	unsigned l = fname.size() < 2047 ? fname.size() : 2047;
	memcpy(buf, fname.c_str(), l);
	buf[l] = 0;
	return std::string( fname_chgExt( buf, ext ) );
}


inline std::string fname_addExt(std::string fname, const char* ext) {
	char buf[2048];
	unsigned l = fname.size() < 2047 ? fname.size() : 2047;
	memcpy(buf, fname.c_str(), l);
	buf[l] = 0;
	return std::string( fname_addExt( buf, ext ) );
}



#endif
