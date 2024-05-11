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
/* 文字列処理関係                                                           */

#if 0
static inline char *strNCpyZ(char *dst, const char *src, size_t size)
{
    strncpy(dst, src, size);
    dst[size-1] = 0;
    return dst;
}
#endif

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



int   fname_setSjisMode(int sw)
{
    int n = fname_sjisFlag;
    fname_sjisFlag = sw;
    return n;
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



char *fname_addExt(char filename[], const char *ext)
{
    if (strrchr(fname_baseName(filename), '.') == NULL) {
        strcat(filename,".");
        strcat(filename, ext);
    }
    return filename;
}




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

/** 全角２バイト目を考慮した strupr
 */
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


#if 0
/** ディレクトリ名とファイル名をくっつけたものをstrdupする
 *  拡張子の付け替えができるよう、+5バイトは余分にメモリは確保する
 */
char *fname_dirNameDupE(const char *dir, const char *name)
{
    const char *m;

    m = fname_dirNameDup(dir,name);
    if (m == NULL) {
        err_abortMsg("メモリが足りません\n");
    }
    return (char*)m;
}



/** ディレクトリ名とファイル名をくっつけたものをstrdupする.
 *  拡張子の付け替えができるよう、+5バイトは余分にメモリは確保する.
 */
char *fname_dirNameDup(const char *dir, const char *name)
{
    size_t  l,n;
    char *m;

    l = (dir) ? strlen(dir) : 0;
    n = (name) ? strlen(name) : 0;
    m = (char*)calloc(1, l+n+1+7);
    if (m == NULL)
        return NULL;
    if (l) {
        strcpy(m, dir);
        fname_delLastDirSep(m);
        strcat(m, "/");
    }
    if (n) {
        strcat(m, name);
    }
    return m;
}
#endif


#if 0
char *fname_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext)
{
    if (onam == NULL || name == NULL || strcmp(name,".") == 0)
        return NULL;
    onam[0] = 0;
    if (dir && dir[0])
        strcpy(onam, dir);
    if (mdir && mdir[0]) {      // ソースディレクトリが指定されていれば、元の指定名のディレクトリも付ける.
        if (onam[0])
            strcat(onam, "/");
        strcat(onam, mdir);
    }
    /*if (name)*/ {
        if (onam[0])
            strcat(onam, "/");
        strcat(onam, name);
        fname_chgExt(onam, chgext);
    }
    return onam;
}


char *fname_dirNameChgExt(char *nam, const char *dir, const char *name, const char *chgext)
{
    if (name == NULL || strcmp(name,".") == 0)
        return NULL;
    if (dir && *dir && name[0] != '/' && name[1] != ':') {
        sprintf(nam, "%s/%s", dir, name);
    } else {
        sprintf(nam, "%s", name);
    }
    fname_chgExt(nam, chgext);
    //strupr(nam);
    return nam;
}



char *fname_dirNameAddExt(char *nam, const char *dir, const char *name, const char *addext)
{
    if (name == NULL || strcmp(name,".") == 0)
        return NULL;
    if (dir && *dir && name[0] != '/' && name[1] != ':') {
        sprintf(nam, "%s/%s", dir, name);
    } else {
        sprintf(nam, "%s", name);
    }
    fname_addExt(nam, addext);
    //strupr(nam);
    return nam;
}


/// name 中のドライブ名とベース名を抜いたディレクトリ名をmdir[]に入れて返す.
char *fname_getMidDir(char mdir[], const char *name)
{
    char *d;

    if (mdir == NULL || name == NULL)
        return NULL;
    if (name[1] == ':')     // ドライブ名付きだった.
        name += 2;
    if (name[0] == '/')    // ルート指定されてた.
        name += 1;
    strcpy(mdir, name);
    d = fname_baseName(mdir);
    if (d <= mdir) {
        mdir[0] = 0;
    } else if (d[-1] == '/') {
        *d = '\0';
    }
    return mdir;
}
#endif


/*--------------------------------------------------------------------------*/

#if 0
int fil_isWildC(const char *onam)
{
    return (strpbrk(onam, "*?") != NULL);
}
#endif

#if 1 //defined(__SC__) || defined(__LCC__) // dmc, lcc
// _finddatai64_t が無い場合用...
// なんだが windows 系だとむしろ、このほうが汎用的だろう...
// が、コンパイル通しただけで、変換テスト等していないので、とりあえず暫定物.

#if 0
static WIN32_FIND_DATA  fil_findData;
static char             fil_findDir[FIL_NMSZ];
static char *           fil_findBase;
static HANDLE           fil_findHdl;

char *fil_findFirstName(char dst[], const char *src)
{
    fil_findHdl = FindFirstFile(src, &fil_findData);
    if (fil_findHdl == INVALID_HANDLE_VALUE) {
        fil_findDir[0] = 0;
        return NULL;
    }
    strNCpyZ(fil_findDir, src, FIL_NMSZ);
    fil_findBase   = fname_baseName(fil_findDir);
    *fil_findBase  = 0;

    //
    strcpy(dst, fil_findDir);
    strNCpyZ(dst+strlen(dst), fil_findData.cFileName, FIL_NMSZ-strlen(dst));

    if (fil_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return fil_findNextName(dst);
    }
    return dst;
}



char *fil_findNextName(char dst[])
{
    if (fil_findHdl == INVALID_HANDLE_VALUE)
        return NULL;
    do {
        if (FindNextFile(fil_findHdl, &fil_findData) == 0) {
            FindClose(fil_findHdl);
            fil_findHdl = INVALID_HANDLE_VALUE;
            return NULL;
        }
        strcpy(dst, fil_findDir);
        strNCpyZ(dst+strlen(dst), fil_findData.cFileName, FIL_NMSZ-strlen(dst));
    } while (fil_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    return dst;
}

#endif

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



#else   // VC BC GCC WatcomC

#if 0
static struct _finddatai64_t fil_findData;
static char                  fil_findDir[FIL_NMSZ];
static char *                fil_findBase;
static ptrdiff_t             fil_findHdl;

char *fil_findFirstName(char dst[], const char *src)
{
    fil_findHdl = _findfirsti64((char *)src, &fil_findData);
    if (fil_findHdl == -1) {
        fil_findDir[0] = 0;
        return NULL;
    }
    strNCpyZ(fil_findDir, src, FIL_NMSZ);
    fil_findBase   = fname_baseName(fil_findDir);
    *fil_findBase  = 0;
    //

    strcpy(dst, fil_findDir);
    strNCpyZ(dst+strlen(dst), fil_findData.name, FIL_NMSZ-strlen(dst));

    if (fil_findData.attrib & _A_SUBDIR) {
        return fil_findNextName(dst);
    }
    return dst;
}


char *fil_findNextName(char dst[])
{
    if (fil_findHdl == -1)
        return NULL;
    do {
        if (_findnexti64(fil_findHdl, &fil_findData)) {
            _findclose(fil_findHdl);
            fil_findHdl = -1;
            return NULL;
        }
        strcpy(dst, fil_findDir);
        strNCpyZ(dst+strlen(dst), fil_findData.name, FIL_NMSZ-strlen(dst));
    } while (fil_findData.attrib & _A_SUBDIR);
    return dst;
}

#endif

/** 二つのファイルの日付の大小を比較する.
 *  tgt が新しければ 1(正), 同じならば 0, tgt が古ければ -1(負)
 */
int fil_fdateCmp(const char *tgt, const char *src)
{
    struct _finddatai64_t srcData;
    struct _finddatai64_t tgtData;
    ptrdiff_t   srcFindHdl, tgtFindHdl;
    time_t srcTm, tgtTm;

    srcFindHdl = _findfirsti64((char *)src, &srcData);
    srcTm = (srcFindHdl == -1) ? 0 : srcData.time_write;

    tgtFindHdl = _findfirsti64((char *)tgt, &tgtData);
    tgtTm = (tgtFindHdl == -1) ? 0 : tgtData.time_write;

    // DBG_PRINTF(("%x - %x = %d\n", tgtTm, srcTm, tgtTm - srcTm));

    if (tgtTm < srcTm)
        return -1;
    else if (tgtTm > srcTm)
        return 1;
    return 0;
}

#endif



/* ------------------------- */
/*--------------------------------------------------------------------------*/

#if defined(_WIN32)
#undef mkdir
#define mkdir(a,b)	mkdir(a)
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


/* ------------------------------------------------------------------------ */
/* エラーexitする fgetc,fputcファイル関数                                   */


/* ------------------------------------------------------------------------ */
/* 文字列のリストを作成     */
#if 0
/** 文字列のリストを追加        */
slist_t *slist_add(slist_t **p0, char *s)
{
    slist_t* p;

    p = *p0;
    if (p == NULL) {
        p = (slist_t*)callocE(1, sizeof(slist_t));
        p->s = strdupE(s);
        *p0 = p;
    } else {
        while (p->link != NULL) {
            p = p->link;
        }
        p->link = (slist_t*)callocE(1, sizeof(slist_t));
        p = p->link;
        p->s = strdupE(s);
    }
    return p;
}


/** 文字列のリストを削除    */
void slist_free(slist_t **p0)
{
    slist_t *p, *q;

    for (p = *p0; p; p = q) {
        q = p->link;
        freeE(p->s);
        freeE(p);
    }
    *p0 = NULL;
}

#endif


#if 0
/*--------------------------------------------------------------------------*/
/* 一つのテキストファイル読みこみ                                           */

unsigned int    TXT1_line;
char    TXT1_name[FIL_NMSZ];
FILE    *TXT1_fp;


void TXT1_error(const char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s(%5d): ", TXT1_name, TXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    return;
}


void TXT1_errorE(const char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s(%5d): ", TXT1_name, TXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    exit(1);
}


int TXT1_open(const char *name)
{
    TXT1_fp = fopen(name,"rt");
    if (TXT1_fp == 0)
        return -1;
    strcpy(TXT1_name, name);
    TXT1_line = 0;
    return 0;
}


void TXT1_openE(const char *name)
{
    TXT1_fp = fopenE(name,"rt");
    strcpy(TXT1_name, name);
    TXT1_line = 0;
}


char *TXT1_getsE(char *buf, int sz)
{
    char *p;

    p = fgets(buf, sz, TXT1_fp);
    if (ferror(TXT1_fp)) {
        TXT1_error("file read error\n");
        exit(1);
    }
    TXT1_line++;
    return p;
}


void TXT1_close(void)
{
    fcloseE(TXT1_fp);
}
#endif


/*--------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

