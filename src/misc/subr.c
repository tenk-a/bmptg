/**
 *  @file   subr.c
 *  @biref  各種雑多なルーチン群
 *  @author Masashi Kitamura
 *  @date   199?-??-??
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "ExArgv.h"

#include "subr.h"


#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define FNAM_ISKANJI(c) (0) 
#else
#define FNAM_ISKANJI(c) (fname_sjisFlag && ISKANJI(c))
#define ISKANJI(c)      (((unsigned char)(c) >= 0x81 && (unsigned char)(c) <= 0x9F) || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC))
#endif

/*--------------------------------------------------------------------------*/

int     dbgExLog_sw__ = 0;      ///< [直接使用しちゃダメ] dbg_printf等でのログ出力の有無を保持. 0:off 1:on



/*--------------------------------------------------------------------------*/
/* エラー処理付きの標準関数                                                 */

/** エラーがあれば即exitの malloc() */
void *mallocE(size_t a)
{
    void *p;

    if (a == 0)
        a = 1;
    p = calloc(1,a);
    //printf("malloc(0x%x)\n",a);
    if (p == NULL) {
        err_abortMsg("メモリが足りない(%d byte(s))\n",a);
    }
    return p;
}



/** エラーがあれば即exitの calloc() */
void *callocE(size_t a, size_t b)
{
    void *p;

    if (a== 0 || b == 0)
        a = b = 1;
    p = calloc(a,b);
    //printf("calloc(0x%x,0x%x)\n",a,b);
    if (p == NULL) {
        err_abortMsg("メモリが足りない(%d*%d byte(s))\n",a,b);
    }
    return p;
}



/** エラーがあれば即exitの strdup() */
char *strdupE(const char *s)
{
    char *p;

    //printf("strdup('%s')\n",s);
    if (s == NULL)
        return (char*)callocE(1,1);
    p = (char*)calloc(1,strlen(s)+8);
    if (p)
        strcpy(p, s);
    if (p == NULL) {
        err_abortMsg("メモリが足りない(長さ%d+1)\n",strlen(s));
    }
    return p;
}


int freeE(void *p)
{
    if (p)
        free(p);
    return 0;
}


/*--------------------------------------------------------------------------*/
/* ファイル名文字列処理                                                     */

int  fname_sjisFlag = 0;


#if 0
int   fname_setSjisMode(int sw)
{
    int n = fname_sjisFlag;
    fname_sjisFlag = sw;
    return n;
}
#endif


int fname_startsWith(char const* a, char const* prefix)
{
 #if defined(_WIN32) || defined(_MSC_VER)
	return strncasecmp(a, prefix, strlen(prefix)) == 0;
 #else
	return strncmp(a, prefix, strlen(prefix)) == 0;
 #endif
}


int fname_isAbsolutePath(char const* s)
{
 #if defined(_WIN32) || defined(_MSC_VER)
    if (fname_isDirSep(*s))
        return 1;
    if (*s && s[1] == ':' && fname_isDirSep(s[2])
        && (*s >= 'A' && *s <= 'Z' || *s >= 'a' && *s <= 'z')
    ) {
        return 1;
    }
    return 0;
  #else
    return fname_isDirSep(*s);
  #endif
}


char *fname_baseName(const char *adr)
{
    const char *p = adr;
    while (*p != '\0') {
        if (*p == ':' || *p == '/' || *p == '\\')
            adr = p + 1;
        if (FNAM_ISKANJI((*(unsigned char *)p)) && *(p+1) )
            p++;
        p++;
    }
    return (char*)adr;
}


char *fname_getExt(const char *name)
{
    const char *p;
    name = fname_baseName(name);
    p = strrchr(name, '.');
    if (p) {
        return (char*)(p+1);
    }
    return (char*)(name + strlen(name));
}



char *fname_chgExt(char filename[], const char *ext)
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


#if 0
char *fname_addExt(char filename[], const char *ext)
{
    if (strrchr(fname_baseName(filename), '.') == NULL) {
        strcat(filename,".");
        strcat(filename, ext);
    }
    return filename;
}
#endif



/** 文字列の最後に \ か / があれば削除 */
char *fname_delLastDirSep(char *dir)
{
    char *p, *s;

    if (dir) {
        size_t l;
        s   = fname_baseName(dir);
        l   = strlen(s);
        if (l > 1) {
            p = s + l;
            if (p[-1] == '/') {
                p[-1] = 0;
            } else if (p[-1] == '\\') {
                if (fname_sjisFlag == 0) {
                    p[-1] = 0;
                } else {
                    int f = 0;
                    while (*s) {
                        f = 0;
                        if (FNAM_ISKANJI(*s) && s[1]) {
                            s++;
                            f = 1;
                        }
                        s++;
                    }
                    if (f == 0)
                        p[-1] = 0;
                }
            }
        }
    }
    return dir;
}


/** 全角２バイト目を考慮した strlwr
 */
char *fname_strLwr(char *s0)
{
    char *s = s0;

    while (*s) {
        if (FNAM_ISKANJI(*s) && s[1]) {
            s += 2;
        } else if (isupper(*s)) {
            *s = tolower(*s);
            s++;
        } else {
            s++;
        }
    }
    return s0;
}

/** filePath中の \ を / に置換.
 */
char *fname_backslashToSlash(char filePath[])
{
    char *p = filePath;
    while (*p != '\0') {
        if (FNAM_ISKANJI(*p) && p[1]) {
            p += 2;
        } else if (*p == '\\') {
            *p = '/';
            ++p;
        } else {
			++p;
		}
    }
    return filePath;
}


/*--------------------------------------------------------------------------*/

/** 二つのファイルの日付の大小を比較する.
 *  tgt が新しければ 1(正), 同じならば 0, tgt が古ければ -1(負)
 */
int fil_fdateCmp(const char *tgt, const char *src)
{
 #if defined(_WIN32)
    WIN32_FIND_DATAA    srcData;
    WIN32_FIND_DATAA    tgtData;
    HANDLE              srcFindHdl;
    HANDLE              tgtFindHdl;
    uint64_t            srcTm;
    uint64_t            tgtTm;

    srcFindHdl = FindFirstFileA((char *)src, &srcData);
    srcTm = ((uint64_t)srcData.ftLastWriteTime.dwHighDateTime<<32) | (uint64_t)srcData.ftLastWriteTime.dwLowDateTime;
    if (srcFindHdl == INVALID_HANDLE_VALUE)
        srcTm = 0;

    tgtFindHdl = FindFirstFileA((char *)tgt, &tgtData);
    tgtTm = ((uint64_t)tgtData.ftLastWriteTime.dwHighDateTime<<32) | (uint64_t)tgtData.ftLastWriteTime.dwLowDateTime;
    if (tgtFindHdl == INVALID_HANDLE_VALUE)
        tgtTm = 0;

    //x DBG_F(("%x - %x = %d\n", tgtTm, srcTm, tgtTm - srcTm));

    if (tgtTm < srcTm)
        return -1;
    else if (tgtTm > srcTm)
        return 1;
    return 0;
 #else
	struct stat tgt_st = {0};
	struct stat src_st = {0};
	int    tgt_rc = stat(tgt, &tgt_st);
	int    src_rc = stat(src, &src_st);
	if (tgt_rc < 0) {
		return (src_rc < 0) ? 0 : -1;
	} else if (src_rc < 0) {
		return 1;
	}
	return tgt_st.st_mtime - src_st.st_mtime;
 #endif
}


/* ------------------------- */
/*--------------------------------------------------------------------------*/

#if defined(_WIN32)
#undef mkdir
#define mkdir(a,b)	_mkdir(a)
#endif

/// ディレクトリ掘り機能付きのfopen
FILE *fopenMD(const char *name, char *mode)
{
    FILE *fp;
    char nm[FIL_NMSZ];
    char *s, *e;

    fp = fopen(name, mode);
    if (fp)
        return fp;
    if (strpbrk(mode, "wa") == NULL)    // 読み込みモードならばフォルダを作成しない.
        return NULL;
    strcpy(nm, name);
    e = nm + strlen(nm);
    do {
        s = fname_baseName(nm);
        if (s <= nm)
            return NULL;
        --s;
        *s = '\0';
    } while (mkdir(nm, 755) != 0);
    do {
        *s = '/';
        s += strlen(s);
        if (s >= e)
            return fopen(name, mode);
    } while (mkdir(nm, 755) == 0);
    return NULL;
}


void *fil_save(const char *name, void *buf, int size)
{
    FILE *fp;

    fp = fopenMD(name,"wb");
    if (fp == NULL)
        return NULL;
    setvbuf(fp, NULL, _IOFBF, 1024*1024);
    if (size && buf) {
        fwrite(buf, 1, size, fp);
        if (ferror(fp)) {
            fclose(fp);
            buf = NULL;
        }
    }
    fclose(fp);
    return buf;
}


void *fil_loadMallocE(const char *name, size_t *rdszp)
{
    void *p = fil_loadMalloc(name, rdszp);
    if (p == NULL) {
        err_abortMsg("%sのロードに失敗しました\n", name);
    }
    return p;
}



/** ファイル・ロード.
 *  @return bufのアドレスかmallocされたアドレス. エラー時はNULLを返す.
 */
void *fil_loadMalloc(const char* name, size_t* rdszp)
{
	return ExArgv_fileLoadMalloc(name, rdszp);
}


/* ------------------------------------------------------------------------ */
/* エラーexitする ファイル関数                                              */


FILE *fopenE(const char *name, const char *mod)
{
    /* エラーがあれば即exitの fopen() */
    FILE *fp = fopen(name,mod);
    if (fp == NULL) {
        err_abortMsg("ファイル %s をオープンできません\n",name);
        return NULL;
    }
    setvbuf(fp, NULL, _IOFBF, 1024*1024);
    return fp;
}


size_t  fwriteE(const void *buf, size_t sz, size_t num, FILE *fp)
{
    /* エラーがあれば即exitの fwrite() */
    size_t l;

    l = fwrite(buf, sz, num, fp);
    if (ferror(fp)) {
        fcloseE(fp);
        err_abortMsg("ファイル書込みでエラー発生\n");
    }
    return l;
}


size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp)
{
    /* エラーがあれば即exitの fread() */
    size_t l;

    l = fread(buf, sz, num, fp);
    if (ferror(fp)) {
        fcloseE(fp);
        err_abortMsg("ファイル読込みでエラー発生\n");
    }
    return l;
}


void fcloseE(FILE *fp)
{
    if (fp) {
        fclose(fp);
        if (ferror(fp)) {
            fclose(fp);
        }
    }
}


/*--------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

