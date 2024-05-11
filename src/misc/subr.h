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

#ifdef __cplusplus
extern "C" {
#endif

#define TOUPPER(c)      (((unsigned)(c) < 0x80) ? toupper(c) : (c))

#if defined(_WIN32) || defined(_MSC_VER)
#define strcasecmp		_stricmp
#define strncasecmp		_strnicmp
#else
#endif

/*--------------------------------------------------------------------------*/
static inline int fnameIsDirSep(char c) {
 #if defined(_WIN32) || defined(_MSC_VER)
    return c == '/' || c == '\\';
 #else
    return c == '/';
 #endif
}

static inline int fnameStartsWith(char const* a, char const* prefix) {
 #if defined(_WIN32) || defined(_MSC_VER)
	return strncasecmp(a, prefix, strlen(prefix)) == 0;
 #else
	return strncmp(a, prefix, strlen(prefix)) == 0;
 #endif
}

static inline int fnameIsAbsolutePath(char const* s) {
 #if defined(_WIN32) || defined(_MSC_VER)
    if (fnameIsDirSep(*s))
        return 1;
    if (*s && s[1] == ':' && fnameIsDirSep(s[2])
        && (*s >= 'A' && *s <= 'Z' || *s >= 'a' && *s <= 'z')
    ) {
        return 1;
    }
    return 0;
  #else
    return fnameIsDirSep(*s);
  #endif
}


/*--------------------------------------------------------------------------*/

void *mallocE(size_t a);
void *callocE(size_t a, size_t b);
char *strdupE(const char *p);
int freeE(void *p);

/*--------------------------------------------------------------------------*/
/* ファイル関係 */

extern int  fname_sjisFlag;

int   fname_setSjisMode(int sw);                                                // ファイル名はMS全角(SJIS)を対応する(1)か否(0)か.
char *fname_baseName(const char *adr);
char *fname_delLastDirSep(char *dir);

char *fname_getExt(const char *adr);

char *fname_chgExt(char filename[], const char *ext);
char *fname_addExt(char filename[], const char *ext);
#if 0
char *fname_dirNameDup(const char *dir, const char *name);
char *fname_dirNameDupE(const char *dir, const char *name);
char *fname_dirNameAddExt(char *nam, const char *dir, const char *name, const char *addext);
char *fname_dirNameChgExt(char *nam, const char *dir, const char *name, const char *chgext);
char *fname_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext);
char *fname_getMidDir(char mdir[], const char *name);
#endif
char *fname_strLwr(char *s0);
char *fname_backslashToSlash(char filePath[]);

void *fil_load(const char *name, void *buf, int bufsz, int *rdszp);
void *fil_loadE(const char *name, void *buf, int bufsz, int *rdszp);
void *fil_save(const char *name, void *buf, int size);
void *fil_saveE(const char *name, void *buf, int size);

FILE *fopenMD(const char *name, char *mode);

#if 0
int fil_isWildC(const char *onam);
char *fil_findFirstName(char dst[], const char *src);
char *fil_findNextName(char dst[]);
#endif

int fil_fdateCmp(const char *tgt, const char *src);

#if 0
/// ファイル名リスト.
typedef struct flist_t {
    struct flist_t *link;   ///< 次へのリンク.
    char *s;                ///< 入力名.
    char *d;                ///< 出力名.
} flist_t;

void     flist_setMode(int mode);
void     flist_setSrcDir(const char *src);
void     flist_setSrcExt(const char *src);
void     flist_setDstDir(const char *dst);
void     flist_setDstExt(const char *dst);
flist_t *flist_add(flist_t **p0, const char *nm);
#endif


/*--------------------------------------------------------------------------*/

FILE *fopenE(const char *name, const char *mod);
void fcloseE(FILE *fp);
size_t  fwriteE(const void *buf, size_t sz, size_t num, FILE *fp);
size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp);

/*--------------------------------------------------------------------------*/
/* 式計算 */

int strExpr(const char *s_old, const char **s_new, double *val);        /* 戻り値0:no error  !0:error */
void strExpr_setNameChkFunc(int (*name2valFnc)(char *name, double *valp));
    /* name2valFnc は、名前が渡され、正常なら0を返しその名前の値を *valpにいれる. 異常なら-1を返す関数を設定すること */

/*--------------------------------------------------------------------------*/
/* テキスト入出力 */

#define FIL_NMSZ        4096

#if 0
int  TXT1_open(const char *name);
void TXT1_openE(const char *name);
void TXT1_close(void);
char *TXT1_getsE(char *buf, int sz);
void TXT1_error(const char *fmt, ...);
void TXT1_errorE(const char *fmt, ...);
extern unsigned int    TXT1_line;
extern char     TXT1_name[FIL_NMSZ];
extern FILE     *TXT1_fp;
#endif


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

#endif  /* SUBR_H */
