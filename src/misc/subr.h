/**
 *  @file   subr.h
 *  @biref  各種雑多なルーチン群.
 *  @author Masashi Kitamura
 *  @date   199?-??-??
 */

#ifndef SUBR_H
#define SUBR_H

#include "def.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#ifdef __BORLANDC__
#include <memory.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TOUPPER(c)      (((unsigned)(c) < 0x80) ? toupper(c) : (c))

#if defined(_WIN32) || defined(_MSC_VER)
#define strcasecmp		_stricmp
#define strncasecmp		_strnicmp
#else
#endif

static inline int strStartsWith(char const* str, char const* prefix) {
	return strncmp(str, prefix, strlen(prefix)) == 0;
}

static inline int strCaseStartsWith(char const* str, char const* prefix) {
	return strncasecmp(str, prefix, strlen(prefix)) == 0;
}

/*--------------------------------------------------------------------------*/

void *mallocE(size_t a);
void *callocE(size_t a, size_t b);
char *strdupE(const char *p);
int freeE(void *p);


/*--------------------------------------------------------------------------*/
/* ファイル関係 */

extern int  fname_sjisFlag;

//int   fname_setSjisMode(int sw);			// ファイル名はMS全角(SJIS)を対応する(1)か否(0)か.

static inline int fname_isDirSep(char c) {
 #if defined(_WIN32) || defined(_MSC_VER)
    return c == '/' || c == '\\';
 #else
    return c == '/';
 #endif
}

int fname_startsWith(char const* a, char const* prefix);
int fname_isAbsolutePath(char const* s);

char *fname_baseName(const char *adr);
char *fname_delLastDirSep(char *dir);

char *fname_getExt(const char *adr);
char *fname_chgExt(char filename[], const char *ext);
//char *fname_addExt(char filename[], const char *ext);
char *fname_strLwr(char *s0);
char *fname_backslashToSlash(char filePath[]);

void *fil_loadMalloc(const char *name, size_t *rdszp);
void *fil_loadMallocE(const char *name,size_t *rdszp);

void *fil_save(const char *name, void *buf, int size);
void *fil_saveE(const char *name, void *buf, int size);

FILE *fopenMD(const char *name, char *mode);

int fil_fdateCmp(const char *tgt, const char *src);

/*--------------------------------------------------------------------------*/

#define FIL_NMSZ        4096

FILE*	fopenE(const char *name, const char *mod);
void 	fcloseE(FILE *fp);
size_t  fwriteE(const void *buf, size_t sz, size_t num, FILE *fp);
size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp);

/*--------------------------------------------------------------------------*/
/* 式計算 */

int strExpr(const char *s_old, const char **s_new, double *val);        /* 戻り値0:no error  !0:error */
void strExpr_setNameChkFunc(int (*name2valFnc)(char *name, double *valp));
    /* name2valFnc は、名前が渡され、正常なら0を返しその名前の値を *valpにいれる. 異常なら-1を返す関数を設定すること */


/*--------------------------------------------------------------------------*/
/* デバッグログ出力 */

// -------
// デバッグ.

extern int  dbgExLog_sw__;                              ///< [直接使用しちゃダメ] dbg_printf等でのログ出力の有無を保持. 0:off 1:on
#define dbgExLog_setSw(sw)      (dbgExLog_sw__ = (sw))  ///< ログ出力を行うか否かを設定.
#define dbgExLog_getSw()        (dbgExLog_sw__)         ///< 現在のログ出力の有無を取得.
#define dbgExLog_revSw()        (dbgExLog_sw__ ^= 1)    ///< ログ出力の有無を、反転する.


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
#if defined(_WIN32)
struct scoped_console_output_utf8 {
	scoped_console_output_utf8() : cp_(GetConsoleOutputCP()) { SetConsoleOutputCP(65001); }
	~scoped_console_output_utf8() { SetConsoleOutputCP(cp_); }
private:
	int cp_;
};
#else
#include <locale.h>
struct scoped_console_output_utf8 {
	scoped_console_output_utf8() { setlocale(LC_ALL, nullptr); }
};
#endif
#endif

#endif  /* SUBR_H */
