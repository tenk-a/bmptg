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

/*--------------------------------------------------------------------------*/
/* 文字列操作等 */

#define ISKANJI(c)      (((unsigned char)(c) >= 0x81 && (unsigned char)(c) <= 0x9F) || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC))
#define ISKANJI2(c)     ((unsigned char)(c) >= 0x40 && (unsigned char)(c) <= 0xfc && (c) != 0x7f)

//x char *strNCpyZ(char *dst, const char *src, unsigned size);
//x long  strtolKM(const char *s, char **d, int r);
//x char *strSkipSpc(const char *s);
//x char *strDelLf(char *s);

//x void *memSearch(const void *area0, int asiz, const void *ptn0, int ptnlen);

/// 文字列リスト.
typedef struct slist_t {
    struct slist_t  *link;  ///< 次へのリンク.
    char            *s;     ///< 文字列.
} slist_t;

slist_t *slist_add(slist_t **root, char *s);
void     slist_free(slist_t **root);


//x int Vsprintf(char *dst, const char *fmt, void *args);   /* なぜか、ターゲットのライブラリに vsprintfが存在しなかった場合の交代として */
//x uint32_t memCrc32(void *dat, int sz);       /* CCITT 32ビット CRC の計算 */


/*--------------------------------------------------------------------------*/

void *mallocE(size_t a);
void *callocE(size_t a, size_t b);
char *strdupE(const char *p);
int freeE(void *p);

//#define MALLOC  mallocE
//#define CALLOC  callocE
//#define FREE    freeE


/*--------------------------------------------------------------------------*/
/* ファイル関係 */

extern int  fname_sjisFlag;
#define FNAM_ISKANJI(c) (fname_sjisFlag && ISKANJI(c))

int   fname_setSjisMode(int sw);                                                // ファイル名はMS全角(SJIS)を対応する(1)か否(0)か.
char *fname_baseName(const char *adr);
char *fname_getExt(const char *adr);
char *fname_chgExt(char filename[], const char *ext);
char *fname_addExt(char filename[], const char *ext);
char *fname_delLastDirSep(char *dir);
//x int fil_rename(const char *oldname, const char *newname);                   // rename改. newname.bakを作成してから、変名.
char *fname_dirNameDup(const char *dir, const char *name);
char *fname_dirNameDupE(const char *dir, const char *name);
char *fname_dirNameAddExt(char *nam, const char *dir, const char *name, const char *addext);
char *fname_dirNameChgExt(char *nam, const char *dir, const char *name, const char *chgext);
char *fname_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext);
char *fname_getMidDir(char mdir[], const char *name);

//x int   fil_getTmpDir(char *t);
//x int   fil_getTmpDirE(char *t);
void *fil_load(const char *name, void *buf, int bufsz, int *rdszp);
void *fil_loadE(const char *name, void *buf, int bufsz, int *rdszp);
void *fil_save(const char *name, void *buf, int size);
void *fil_saveE(const char *name, void *buf, int size);
//x int   fil_copy(const char *dstfile, const char *srcfile, void *buf, int bufsz);
//x int   fil_makeDmyFile(const char *oname, unsigned fsiz, int fh);
//x void  fil_makeDmyFileE(const char *oname, unsigned fsiz, int fh);

//x #ifdef __BORLANDC__
//x typedef struct ftime FIL_TIME;
//x FIL_TIME fil_timeGet(const char *srcname);
//x int      fil_timeCmp(FIL_TIME t1, FIL_TIME t2);
//x int      fil_timeSet(const char *dstname, FIL_TIME fdt);
//x #endif /* __BORLANDC__ */


FILE *fopenMD(const char *name, char *mode);

int fil_isWildC(const char *onam);
char *fil_findFirstName(char dst[], const char *src);
char *fil_findNextName(char dst[]);

int fil_fdateCmp(const char *tgt, const char *src);

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



/*--------------------------------------------------------------------------*/

FILE *fopenE(const char *name, const char *mod);
void fcloseE(FILE *fp);
size_t  fwriteE(const void *buf, size_t sz, size_t num, FILE *fp);
size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp);
//x unsigned fil_length(FILE *fp);


//x int fgetcE(FILE *fp);
//x int fgetc2iE(FILE *fp);
//x int fgetc4iE(FILE *fp);
//x int fgetc2mE(FILE *fp);
//x int fgetc4mE(FILE *fp);
//x void fputcE(int c, FILE *fp);
//x void fputc2mE(int c, FILE *fp);
//x void fputc4mE(int c, FILE *fp);
//x void *fputsE(char *s, FILE *fp);
//x void fputc2iE(int c, FILE *fp);
//x void fputc4iE(int c, FILE *fp);



/*--------------------------------------------------------------------------*/

//x typedef int (*STBL_CMP)(void *s0, void *s1);
//x STBL_CMP STBL_setFncCmp(STBL_CMP fncCmp);
//x int STBL_add(void *t[], int *tblcnt, void *key);
//x int STBL_search(void *tbl[], int nn, void *key);



/*--------------------------------------------------------------------------*/
/* 式計算 */

int strExpr(const char *s_old, const char **s_new, double *val);        /* 戻り値0:no error  !0:error */
void strExpr_setNameChkFunc(int (*name2valFnc)(char *name, double *valp));
    /* name2valFnc は、名前が渡され、正常なら0を返しその名前の値を *valpにいれる. 異常なら-1を返す関数を設定すること */



/*--------------------------------------------------------------------------*/
/* テキスト入出力 */

#define FIL_NMSZ        4096

int  TXT1_open(const char *name);
void TXT1_openE(const char *name);
void TXT1_close(void);
char *TXT1_getsE(char *buf, int sz);
void TXT1_error(const char *fmt, ...);
void TXT1_errorE(const char *fmt, ...);
extern unsigned int    TXT1_line;
extern char     TXT1_name[FIL_NMSZ];
extern FILE     *TXT1_fp;


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
