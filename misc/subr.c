/**
 *	@file	subr.c
 *	@biref	各種雑多なルーチン群
 *	@author	Masashi Kitamura
 *	@date	199?-??-??
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>

#include "subr.h"


#ifdef __cplusplus
extern "C" {
#endif



/*--------------------------------------------------------------------------*/

int		dbgExLog_sw__ = 0;		///< [直接使用しちゃダメ] dbg_printf等でのログ出力の有無を保持. 0:off 1:on




/*--------------------------------------------------------------------------*/
/* 文字列処理関係															*/


static inline char *strNCpyZ(char *dst, const char *src, size_t size)
{
	strncpy(dst, src, size);
	dst[size-1] = 0;
	return dst;
}


#if 0
char *strSkipSpc(const char *s)
{
	while ((*s && *(unsigned char *)s <= ' ') || *s == 0x7f) {
		s++;
	}
	return (char*)s;
}


char *strDelLf(char *s)
{
	char *p;
	p = STREND(s);
	if (p != s && p[-1] == '\n') {
		p[-1] = 0;
	}
	return s;
}


long strtolKM(const char *s, char **d, int r)
{
	long l;

	l = strtol(s, (char**)&s, r);
	if (*s == 'k' || *s == 'K') {
		s++;
		l *= 1024;
	} else if (*s == 'm' || *s == 'M') {
		s++;
		l *= 1024*1024;
	} else if (*s == 'g' || *s == 'G') {
		s++;
		l *= 1024*1024*1024;
	}
	*d = (char*)s;
	return l;
}
#endif



/*--------------------------------------------------------------------------*/
/* エラー処理付きの標準関数													*/

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



#if 0
/** エラーがあれば即exitの calloc() */
void *reallocE(void *m, size_t a)
{
	void *p;

	if (a == 0)
		a = 1;
	p = realloc(m, a);
	//printf("realloc(0x%x,0x%x)\n",m,a);
	if (p == NULL) {
		err_abortMsg("メモリが足りない(%d byte(s))\n",a);
	}
	return p;
}
#endif



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



#if 0

void *mallocMaE(int sz, int minSz, int aln)
{
	void *p;

	p = mallocMa(sz, minSz, aln);
	if (p == NULL) {
		err_abortMsg("メモリが足りない(%d byte(s))\n",sz);
	}
	return p;
}


/** 指定したsz でmalloc して失敗したら、minSz ～sz の範囲で適当に試してメモリを確保する.
 * aln が 0以上であれば 2のaln乗でサイズをアライメントします
 */
void *mallocMa(int sz, int minSz, int aln)
{
	void *p;
	int  a;

	/* アライメント用のマスクを生成 */
	if (aln <= 0)
		a = 1;
	else
		a = (1<<aln);
	a = a - 1;

	/* サイズを調整 */
	sz = (sz + a) & ~a;
	minSz = (minSz + a) & ~a;

	p = malloc(sz);
	if (p)
		return p;

	/* 最小サイズが確保できるかチェック */
	if (minSz <= 0)
		return NULL;
	p = malloc(minSz);
	if (p == NULL)
		return NULL;
	free(p);

	/* 確保できるサイズを探す*/
	do {
		sz = (sz/2 + a) & ~a;
		if (sz < minSz)
			sz = minSz;
		p = malloc(sz);
	} while (p == NULL && sz > minSz);

	return p;
}


#endif




#if 0

/*--------------------------------------------------------------------------*/
/* エラー処理、デバッグ用関数												*/

/*volatile*/void err_abortMsg(const char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	vfprintf(stdout, fmt, app);
	va_end(app);
	exit(1);
}


void dbg_printf(const char *fmt, ...)
{
 #ifndef NDEBUG
  #ifdef DBG_FILE_NAME
	va_list app;
	FILE *fp;

	if (debugflag == 0)
		return;
	fp = fopen(DBG_FILE_NAME, "at");
	if (fp) {
		va_start(app, fmt);
		vfprintf(fp, fmt, app);
		va_end(app);
		fclose(fp);
	}
  #else
	va_list app;
	if (debugflag == 0)
		return;
	va_start(app, fmt);
	vfprintf(stdout, fmt, app);
	va_end(app);
  #endif
 #endif
}

#endif


/*--------------------------------------------------------------------------*/
/* ファイル名文字列処理														*/

int  fname_sjisFlag = 1;



int	  fname_setSjisMode(int sw)
{
	int n;

	n = fname_sjisFlag;
	fname_sjisFlag = sw;
	return n;
}



char *fname_baseName(const char *adr)
{
	const char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
		if (ISKANJI((*(unsigned char *)p)) && *(p+1) )
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
		s 	= fname_baseName(dir);
		l 	= strlen(s);
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
						if (ISKANJI(*s) && s[1]) {
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



#if 0
/** 全角２バイト目を考慮した strupr
 */
char *fname_strUpr(char *s0)
{
	char *s = s0;

	while (*s) {
		if (FNAM_ISKANJI(*s) && s[1]) {
			s += 2;
		} else if (islower(*s)) {
			*s = toupper(*s);
			s++;
		} else {
			s++;
		}
	}
	return s0;
}
#endif



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



/** ディレクトリ名とファイル名をくっつけたものをstrdupする
 *  拡張子の付け替えができるよう、+5バイトは余分にメモリは確保する
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
		strcat(m, "\\");
	}
	if (n) {
		strcat(m, name);
	}
	return m;
}



#if 1
char *fname_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext)
{
	if (onam == NULL || name == NULL || strcmp(name,".") == 0)
		return NULL;
	onam[0] = 0;
	if (dir && dir[0])
		strcpy(onam, dir);
	if (mdir && mdir[0]) {		// ソースディレクトリが指定されていれば、元の指定名のディレクトリも付ける
		if (onam[0])
			strcat(onam, "\\");
		strcat(onam, mdir);
	}
	/*if (name)*/ {
		if (onam[0])
			strcat(onam, "\\");
		strcat(onam, name);
		fname_chgExt(onam, chgext);
	}
	return onam;
}


char *fname_dirNameChgExt(char *nam, const char *dir, const char *name, const char *chgext)
{
	if (name == NULL || strcmp(name,".") == 0)
		return NULL;
	if (dir && *dir && name[0] != '\\' && name[1] != ':') {
		sprintf(nam, "%s\\%s", dir, name);
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
	if (dir && *dir && name[0] != '\\' && name[1] != ':') {
		sprintf(nam, "%s\\%s", dir, name);
	} else {
		sprintf(nam, "%s", name);
	}
	fname_addExt(nam, addext);
	//strupr(nam);
	return nam;
}
#endif


/// name 中のドライブ名とベース名を抜いたディレクトリ名をmdir[]に入れて返す
char *fname_getMidDir(char mdir[], const char *name)
{
	char *d;

	if (mdir == NULL || name == NULL)
		return NULL;
	if (name[1] == ':')		// ドライブ名付きだった
		name += 2;
	if (name[0] == '\\')	// ルート指定されてた
		name += 1;
	strcpy(mdir, name);
	d = fname_baseName(mdir);
	if (d <= mdir) {
		mdir[0] = 0;
	} else if (d[-1] == '\\') {
		*d = '\0';
	}
	return mdir;
}


/* ------------------------- */

#if 0
/// ファイルをrename. newname がすでにあれば .bak に予め変名.
int fil_rename(const char *oldname, const char *newname)
{
	char bak[FIL_NMSZ];

	strcpy(bak, newname);
	fname_chgExt(bak, "bak");
	remove(bak);
	rename(newname, bak);
	return rename(oldname, newname);
}
#endif


#if 0

static int fil_getTmpDirSub(char *t, int f);

int fil_getTmpDirE(char *t)
{
	return fil_getTmpDirSub(t, 1);
}


int fil_getTmpDir(char *t)
{
	return fil_getTmpDirSub(t, 0);
}


static int fil_getTmpDirSub(char *t, int f)
{
	char *p;
	char nm[FIL_NMSZ+2];

	if (*t) {
		strcpy(nm, t);
		p = STREND(nm);
	} else {
		p = getenv("TMP");
		if (p == NULL) {
			p = getenv("TEMP");
			if (p == NULL) {
				if (f == 0)
					p = ".\\";
				else
					err_abortMsg("環境変数TMPかTEMPでテンポラリ・ディレクトリを指定してください\n");
			}
		}
		strcpy(nm, p);
		p = STREND(nm);
	}
	if (p[-1] != '\\' && p[-1] != ':' && p[-1] != '/')
		strcat(nm,"\\");
	strcat(nm,"*.*");
	_fullpath(t, nm, FIL_NMSZ);
	p = fname_baseName(t);
	*p = 0;
	if (p[-1] == '\\')
		p[-1] = 0;
	return 0;
}

#endif



/*--------------------------------------------------------------------------*/

int fil_isWildC(const char *onam)
{
	return (strpbrk(onam, "*?") != NULL);
}


#if defined(__SC__) || defined(__LCC__)	// dmc, lcc
// _finddatai64_t が無い場合用...
// なんだが windows 系だとむしろ、このほうが汎用的だろう...
// が、コンパイル通しただけで、変換テスト等していないので、とりあえず暫定物。

static WIN32_FIND_DATA 	fil_findData;
static char 			fil_findDir[FIL_NMSZ];
static char *			fil_findBase;
static HANDLE 			fil_findHdl;

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



/** 二つのファイルの日付の大小を比較する.
 *  tgt が新しければ 1(正), 同じならば 0, tgt が古ければ -1(負)
 */
int fil_fdateCmp(const char *tgt, const char *src)
{
	WIN32_FIND_DATA 	srcData;
	WIN32_FIND_DATA 	tgtData;
	HANDLE 				srcFindHdl;
	HANDLE				tgtFindHdl;
	uint64_t 				srcTm;
	uint64_t				tgtTm;

	srcFindHdl = FindFirstFile((char *)src, &srcData);
	srcTm = ((uint64_t)srcData.ftLastWriteTime.dwHighDateTime<<32) | (uint64_t)srcData.ftLastWriteTime.dwLowDateTime;
	if (srcFindHdl == INVALID_HANDLE_VALUE)
		srcTm = 0;

	tgtFindHdl = FindFirstFile((char *)tgt, &tgtData);
	tgtTm = ((uint64_t)tgtData.ftLastWriteTime.dwHighDateTime<<32) | (uint64_t)tgtData.ftLastWriteTime.dwLowDateTime;
	if (tgtFindHdl == INVALID_HANDLE_VALUE)
		tgtTm = 0;

    //x DBG_F(("%x - %x = %d\n", tgtTm, srcTm, tgtTm - srcTm));

	if (tgtTm < srcTm)
		return -1;
	else if (tgtTm > srcTm)
		return 1;
	return 0;
}



#else	// VC BC GCC WatcomC

static struct _finddatai64_t fil_findData;
static char 				 fil_findDir[FIL_NMSZ];
static char *				 fil_findBase;
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

#if 0
void fil_makeDmyFileE(const char *oname, size_t fsiz, int fh)
{
	int c;

	c = fil_makeDmyFile(oname, fsiz, fh);
	if (c < 0) {
		switch (c) {
		case -1:
			err_abortMsg("ファイル %s をオープンできませんでした\n", oname);
			break;
		case -2:
			err_abortMsg("ファイル %s の書き込み中にエラー発生\n", oname);
			break;
		case -3:
			err_abortMsg("ファイル %s の作成時、メモリが不足した.\n", oname);
			break;
		default:
			assert(0);
		}
	}
}


int fil_makeDmyFile(const char *oname, size_t fsiz, int fh)
{
	FILE *fp;
	int  sz, rsz, n;
	char *b;

	sz  = 0x10000;
	n   = fsiz / sz;
	rsz = fsiz % sz;

	b   = (char*)malloc(sz);
	if (b == NULL) {
		return -3;
	}
	memset(b, fh, sz);
	fp = fopen(oname, "wb");
	if (fp == NULL)
		return -1;
	while (n--) {
		fwrite(b, 1, sz, fp);
		if (ferror(fp)) {
			fclose(fp);
			if (ferror(fp))
				fclose(fp);
			return -2;
		}
	}
	if (rsz) {
		fwrite(b, 1, rsz, fp);
		if (ferror(fp)) {
			fclose(fp);
			if (ferror(fp))
				fclose(fp);
			return -2;
		}
	}
	free(b);
	fclose(fp);
	return 0;
}
#endif




/* ------------------------------------------------------------------------ */
#if 0
void *fil_saveE(const char *name, void *buf, int size)
{
	void *p;

	p = fil_save(name, buf, size);
	if (p == NULL) {
		err_abortMsg("%sのセーブに失敗しました\n", name);
	}
	return p;
}
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
	if (strpbrk(mode, "wa") == NULL)	// 読み込みモードならばフォルダを作成しない
		return NULL;
	strcpy(nm, name);
	e = nm + strlen(nm);
	do {
		s = fname_baseName(nm);
		if (s <= nm)
			return NULL;
		--s;
		*s = '\0';
	} while (mkdir(nm) != 0);
	do {
		*s = '\\';
		s += strlen(s);
		if (s >= e)
			return fopen(name, mode);
	} while (mkdir(nm) == 0);
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


void *fil_loadE(const char *name, void *buf, int bufsz, int *rdszp)
{
	void *p;

	p = fil_load(name, buf, bufsz, rdszp);
	if (p == NULL) {
		err_abortMsg("%sのロードに失敗しました\n", name);
	}
	return p;
}



/** ファイル・ロード
 *	@return bufのアドレスかmallocされたアドレス. エラー時はNULLを返す
 */
void *fil_load(
	const char  *name,	///< 読みこむファイル
	void 		*buf,	///< 読みこむメモリ。NULLが指定されれば mallocし、16バイト余分に確保する
	int 		bufsz,	///< bufのサイズ。0が指定されれば ファイルサイズとなる
	int 		*rdszp)	///< NULLでなければ、読みこんだファイルサイズを入れて返す
{
	FILE *fp;
	int  l;

	fp = fopen(name, "rb");
	if (fp == NULL)
		return NULL;
	l = filelength(fileno(fp));
	if (rdszp)
		*rdszp = l;
	if (bufsz == 0)
		bufsz = l;
	if (l > bufsz)
		l = bufsz;
	if (buf == NULL) {
		bufsz = (bufsz + 15 + 16) & ~15;
		buf = calloc(1, bufsz);
		if (buf == NULL)
			return NULL;
	}
	fread(buf, 1, l, fp);
	if (ferror(fp)) {
		fclose(fp);
		buf = NULL;
	}
	fclose(fp);
	return buf;
}




#if 0

/*--------------------------------------------------------------------------*/
/* ファイルの日付を取得														*/

#ifdef __BORLANDC__

FIL_TIME fil_timeGet(const char *srcname)
{
	FILE		*fp;
	int    		f;
	FIL_TIME	fdt;

	fp = fopen(srcname, "rb");
	if (fp == NULL) {
		memset(&fdt, 0, sizeof fdt);
		return fdt;
	}
	f = getftime(fileno(fp), &fdt);
	fclose(fp);
	if (f < 0) {
		memset(&fdt, 0, sizeof fdt);
		return fdt;
	}
	return fdt;
}


int fil_timeSet(const char *dstname, FIL_TIME fdt)
{
	FILE *fp;
	int    f;

	fp = fopen(dstname, "ab+");
	if (fp == NULL) {
		return -1;
	}
	f = setftime(fileno(fp), &fdt);
	fclose(fp);
	if (f < 0) {
		return -1;
	}
	return 0;
}


int fil_timeCmp(FIL_TIME t1, FIL_TIME t2)
{
	int f;

	f = t1.ft_year  - t2.ft_year;	if (f)	return f;
	f = t1.ft_month - t2.ft_month;	if (f)	return f;
	f = t1.ft_day   - t2.ft_day;	if (f)	return f;
	f = t1.ft_hour  - t2.ft_hour;	if (f)	return f;
	f = t1.ft_min   - t2.ft_min;	if (f)	return f;
	f = t1.ft_tsec  - t2.ft_tsec;
	return f;
}

#endif /* __BORLANDC__ */

#endif





/* ------------------------------------------------------------------------ */
/* エラーexitする ファイル関数												*/


FILE *fopenE(const char *name, const char *mod)
{
	/* エラーがあれば即exitの fopen() */
	FILE *fp;

	fp = fopen(name,mod);
	if (fp == NULL) {
		err_abortMsg("ファイル %s をオープンできません\n",name);
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



#if 0
/* ファイルサイズを求める */
size_t	fil_length(FILE *fp)
{
	return filelength(fileno(fp));
}
#endif




/* ------------------------------------------------------------------------ */
/* エラーexitする fgetc,fputcファイル関数									*/

#if 0

/** fpより 1バイト(0..255) 読み込む. エラーがあれば即終了 */
int fgetcE(FILE *fp)
{
	static uint8_t buf[1];

	freadE(buf, 1, 1, fp);
	return (uint8_t)buf[0];
}


/** fpより ﾘﾄﾙｴﾝﾃﾞｨｱﾝで 2バイト読み込む. エラーがあれば即終了 */
int fgetc2iE(FILE *fp)
{
	int c;
	c = fgetcE(fp);
	return (uint16_t)(c + (fgetcE(fp)<<8));
}


/** fpより リトルエンディアンで 4バイト読み込む. エラーがあれば即終了 */
int fgetc4iE(FILE *fp)
{
	int c;

	c = fgetc2iE(fp);
	return c + (fgetc2iE(fp)<<16);
}


/** fpより bigエンディアンで 2バイト読み込む. エラーがあれば即終了 */
int fgetc2mE(FILE *fp)
{
	int c;

	c = fgetcE(fp);
	return (uint16_t)((c<<8) + fgetcE(fp));
}


/** fpより bigエンディアンで 4バイト読み込む. エラーがあれば即終了 */
int fgetc4mE(FILE *fp)
{
	int c;

	c = fgetc2mE(fp);
	return (c<<16) + fgetc2mE(fp);
}


/** fpに 1バイト(0..255) 書き込む. エラーがあれば即終了 */
void fputcE(int c, FILE *fp)
{
	static uint8_t buf[1];

	buf[0] = (uint8_t)c;
	fwriteE(buf, 1, 1, fp);
}


/** fpに ﾋﾞｯｸﾞｴﾝﾃﾞｨｱﾝで 2バイト書き込む. エラーがあれば即終了 */
void fputc2mE(int c, FILE *fp)
{
	static uint8_t buf[4];

	buf[0] = (uint8_t)(c>> 8);
	buf[1] = (uint8_t)(c);
	fwriteE(buf, 1, 2, fp);
}


/** fpに ﾋﾞｯｸﾞｴﾝﾃﾞｨｱﾝで 4バイト書き込む. エラーがあれば即終了 */
void fputc4mE(int c, FILE *fp)
{
	static uint8_t buf[4];

	buf[0] = (uint8_t)(c>>24);
	buf[1] = (uint8_t)(c>>16);
	buf[2] = (uint8_t)(c>> 8);
	buf[3] = (uint8_t)(c);
	fwriteE(buf, 1, 4, fp);
}


void *fputsE(char *s, FILE *fp)
{
	int n;

	n = strlen(s);
	fwriteE(s, 1, n, fp);
	return s;
}


/** fpに ﾘﾄﾙｴﾝﾃﾞｨｱﾝで 2バイト書き込む. エラーがあれば即終了 */
void fputc2iE(int c, FILE *fp)
{
	static uint8_t buf[4];

	buf[0] = (uint8_t)(c);
	buf[1] = (uint8_t)(c>> 8);
	fwriteE(buf, 1, 2, fp);
}


/** fpに ﾘﾄﾙｴﾝﾃﾞｨｱﾝで 4バイト書き込む. エラーがあれば即終了 */
void fputc4iE(int c, FILE *fp)
{
	static uint8_t buf[4];

	buf[0] = (uint8_t)(c);
	buf[1] = (uint8_t)(c>> 8);
	buf[2] = (uint8_t)(c>> 16);
	buf[3] = (uint8_t)(c>> 24);
	fwriteE(buf, 1, 4, fp);
}

#endif




/*--------------------------------------------------------------------------*/

#if 0
/** BM法を用いた複数バイトデータのサーチ
 * ※ C言語によるアルゴリズム辞典のを改造
 */
void *memSearch(
	const void *area0,		///< 検索開始アドレス
	int 		asiz,		///< 検索範囲(byte)
	const void *ptn0,		///< 検索するパターン
	int ptnlen)				///< パターンの長さ(byte)
{
	int skp[256];
	int i, j, k;
	int c, tail;
	unsigned char *area = (unsigned char *)area0, *ptn = (unsigned char*)ptn0;

	if (ptnlen <= 0 || asiz <= 0) {
		return NULL;
	}
	tail = ptn[ptnlen - 1];				/* 最後の文字(1byte) */
	if (ptnlen == 1) {					/* 長さ1なら簡単! */
		for (i = 0; i < asiz; i++) {	/* 単純に同じ1byteが見つかるまで検索 */
			if (area[i] == tail)
				return area + i;		/* みつかった */
		}
	} else {							/* 長さ2以上のとき表引き */
										/* skp[]に、その文字が検索文字外のときに次の検索開始位置へスキップするバイト数を設定する */
		for (i = 0; i < 256; i++)		/* まず256文字すべて不一致の場合として検索データサイズ分のスキップ値を設定 */
			skp[i] = ptnlen;
		for (i = 0; i < ptnlen-1; i++)	/* 次に検索データを構成する文字の場合は、その文字より後ろの検索データのバイト数分をスキップにする */
			skp[ptn[i]] = ptnlen - 1 - i;
		/* i = ptnlen - 1; */			/* for文の結果がこの式に等しいから不要 */
		/* 指定範囲の検索開始！ */
		while (i < asiz) {
			c = area[i];				/* 検索データの尾穴の1byteをチェックする. */
			if (c == tail) {			/* ここで違えば、(最大)検索データバイト数は、比較せずスキップできる */
				j = ptnlen - 1;			/* 同じならば、さらに検索データを後ろから比較する */
				k = i;
				while (ptn[--j] == area[--k]) {
					if (j == 0)			/* 検索データの先頭ならば */
						return area + k;/* 見つかった */
				}
			}
			i += skp[c];				/* その位置でマッチしなかったので、次の位置までスキップ */
		}
	}

	/* 見つからなかった */
	return NULL;
}
#endif




/* ------------------------------------------------------------------------ */
/* 文字列のリストを作成		*/

/** 文字列のリストを追加		*/
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


/** 文字列のリストを削除	*/
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



/* ------------------------------------------------------------------------ */
/* ファイル名 の取得 */
#if 0

static flist_mode = 0;
static flist_srcDir[FIL_NMSZ];
static flist_dstDir[FIL_NMSZ];
static flist_srcExt[FIL_NMSZ];
static flist_dstExt[FIL_NMSZ];
static flist_outName[FIL_NMSZ];


void flist_setMode(int mode)
{
	flist_mode = mode;
}

void flist_setSrcDir(const char *src)
{
	strcpy(flist_srcDir, src);
}

void flist_setSrcExt(const char *src)
{
	strcpy(flist_srcExt, src);
}


void flist_setDstDir(const char *dst)
{
	strcpy(flist_dstDir, dst);
}


void flist_setDstExt(const char *dst)
{
	strcpy(flist_dstExt, dst);
}


void flist_setOutName(const char *dst)
{
	strcpy(flist_outName, dst);
	fname_addExt(flist_outName, flist_dstExt);
}



static flist_t *flist_add1(flist_t **p0, const char *nm)
{
	/* 文字列のリストを追加		*/
	char buf[FIL_NMSZ], mdir[FIL_NMSZ];
	flist_t* p;

	p = *p0;
	if (p == NULL) {
		p = callocE(1, sizeof(flist_t));
		*p0 = p;
	} else {
		while (p->link != NULL) {
			p = p->link;
		}
		p->link = callocE(1, sizeof(flist_t));
		p = p->link;
	}

	fname_dirNameAddExt(buf, flist_srcDir, nm, flist_srcExt);
	p->s = strdupE(buf);
	mdir[0] = 0;
	if (flist_srcDir[0])
		fname_getMidDir(mdir, nm);
	if (flist_outName[0]) {
		p->d = strdupE(flist_outName);
		flist_outName[0] = 0;
	} else {
		fname_dirDirNameChgExt(buf, flist_dstDir, mdir, nm, flist_dstExt);
		p->d = strdupE(buf);
	}
	return p;
}


flist_t *flist_add(flist_t **p0, const char *s)
{
	char nm[FIL_NMSZ];

	if (fil_isWildC(s) == 0) {
		return flist_add1(p0, s);
	}
	if (fil_findFirstName(nm, s)) {
		do {
			slist_add1(p0, nm);
		} while (fil_findNextName(nm));
	}
	return NULL;
}

#endif



#if 0
/* ------------------------------------------------------------------------ */
/* ソート済み文字列ポインタの配列より、文字列を検索してその位置を返す		*/

static int STBL_strCmp(const char *a, const char *b)
{
	return strcmp(a, b);
}

static STBL_CMP STBL_cmp = (STBL_CMP)STBL_strCmp;


STBL_CMP STBL_setFncCmp(STBL_CMP cmp)
{
	if (cmp)
		STBL_cmp = cmp;
	return STBL_cmp;
}


/** 文字列テーブルに一つ登録
 *  @param t     : 文字列へのポインタをおさめた配列
 *  @param tblcnt: 登録済個数
 *  @param key   : 追加する文字列
 *  @return 0:追加 -1:すでに登録済
 */
int STBL_add(void *t[], int *tblcnt, void *key)
{
	int  low, mid, f, hi;

	hi = *tblcnt;
	mid = low = 0;
	while (low < hi) {
		mid = (low + hi - 1) / 2;
		if ((f = STBL_cmp(key, t[mid])) < 0) {
			hi = mid;
		} else if (f > 0) {
			mid++;
			low = mid;
		} else {
			return -1;	/* 同じものがみつかったので追加しない */
		}
	}
	(*tblcnt)++;
	for (hi = *tblcnt; --hi > mid;) {
		t[hi] = t[hi-1];
	}
	t[mid] = key;
	return 0;
}


int STBL_search(void *tbl[], int nn, void *key)
{
   /*
	*  key:さがす文字列へのポインタ
	*  tbl:文字列へのポインタをおさめた配列
	*  nn:配列のサイズ
	*  復帰値:見つかった文字列の番号(0より)  みつからなかったとき-1
	*/
	int     low, mid, f;

	low = 0;
	while (low < nn) {
		mid = (low + nn - 1) / 2;
		if ((f = STBL_cmp(key, tbl[mid])) < 0)
			nn = mid;
		else if (f > 0)
			low = mid + 1;
		else
			return mid;
	}
	return -1;
}
#endif




/*--------------------------------------------------------------------------*/
/* 一つのテキストファイル読みこみ											*/

unsigned int    TXT1_line;
char	TXT1_name[FIL_NMSZ];
FILE	*TXT1_fp;


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




#if 0
/* ------------------------------------------------------------------------ */
/* 一つのテキストファイル出力												*/

uint32_t	OTXT1_line;
char	OTXT1_name[FIL_NMSZ];
FILE	*OTXT1_fp;


void OTXT1_error(const char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(stdout, "%-12s %5d : ", OTXT1_name, OTXT1_line);
	vfprintf(stdout, fmt, app);
	va_end(app);
	return;
}


void OTXT1_errorE(const char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(stdout, "%-12s %5d : ", OTXT1_name, OTXT1_line);
	vfprintf(stdout, fmt, app);
	va_end(app);
	exit(1);
}


int OTXT1_open(const char *name)
{
	OTXT1_fp = fopenMD(name,"wt");
	if (OTXT1_fp == 0)
		return -1;
	strcpy(OTXT1_name, name);
	OTXT1_line = 0;
	return 0;
}


void OTXT1_openE(const char *name)
{
	OTXT1_fp = fopenE(name,"rt");
	strcpy(OTXT1_name, name);
	OTXT1_line = 0;
}


int OTXT1_putsE(const char *buf)
{
	OTXT1_line++;
	fputs(buf, OTXT1_fp);
	if (ferror(OTXT1_fp)) {
		OTXT1_error("file write error\n");
		exit(1);
	}
	return 0;
}


void OTXT1_close(void)
{
	fcloseE(OTXT1_fp);
}
#endif




/*--------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

