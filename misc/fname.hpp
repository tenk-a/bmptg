/**	ファイル名処理
 */
#ifndef FNAME_H
#define FNAME_H

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#if defined _WIN32 || defined _WIN64
#include <windows.h>
//x #include <mbctype.h>
#else	// linux
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#define stricmp		strcasecmp
#endif


#if defined _DOS
#define MISC_FNAME_DIRSEP_CHR			'\\'
#define MISC_FNAME_DIRSEP_STR			"\\"
#else	// WinNT系(2K?以降)なら、ディレクトリセパレータとして / で問題ない模様.
#define MISC_FNAME_DIRSEP_CHR			'/'
#define MISC_FNAME_DIRSEP_STR			"/"
#endif




#if defined _MAX_PATH
enum { FNAME_MAX_PATH = ((_MAX_PATH*2 > 4096) ? _MAX_PATH*2 : 4096) };
#elif defined PATH_MAX
enum { FNAME_MAX_PATH = ((PATH_MAX * 2 > 4096) ? PATH_MAX * 2 : 4096) };
#else
enum { FNAME_MAX_PATH = 4096 };
#endif



// ===========================================================================

/** 文字 C が MS全角の１バイト目か否か. (utf8やeucは \ 問題は無いので 0が帰ればok)
 */
inline int fname_ismbblead(unsigned char c)
{
  #if defined _WIN32 || defined _WIN64
	return IsDBCSLeadByte(c) != 0;
  #elif defined HAVE_MBCTYPE_H
	return _ismbblead(c) != 0;
  #elif defined USE_SJIS
  	return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFE);
  #else
	return 0;
  #endif
}



template<typename CHR_T>
void	fname_strLcpy(CHR_T* d, CHR_T* s, unsigned l)
{
	assert(d != 0 && s != 0 && l > 0);
	unsigned n = std::strlen(s);
	if (n > l-1)
		n = l-1;
	std::memcpy(d, s, n);
	d[n] = 0;
}


// ===========================================================================

/** ファイルパス名中のディレクトリを除いたファイル名の位置を返す.
 */
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
char *fname_chgExt(CHR_T* filename, const CHR_T *ext)
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



template<class STRING>
STRING fname_chgExt(const STRING& fname, const char* ext) {
	char buf[4096];
	unsigned l = fname_strLcpy(buf, fname.c_str(), 4096);
	return STRING( fname_chgExt( buf, ext ) );
}



/**	拡張子がなければ、ext を追加する.
 */
template<typename CHR_T>
char *fname_addExt(CHR_T filename[], const CHR_T *ext)
{
	if (strrchr(fname_baseName(filename), '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}



template<class STRING>
STRING fname_addExt(const STRING& fname, const char* ext) {
	char buf[4096];
	unsigned l = fname_strLcpy(buf, fname.c_str(), 4096);
	return STRING( fname_addExt( buf, ext ) );
}




// ===========================================================================

/** 全角２バイト目を考慮した strupr
 */
template<typename CHR_T>
char *fname_strUpr(CHR_T *s0)
{
	char *s = s0;

	while (*s) {
		if (fname_ismbblead(*s) && s[1]) {
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



/** 全角２バイト目を考慮した strlwr
 */
template<typename CHR_T>
char *fname_strLwr(CHR_T *s0)
{
	char *s = s0;

	while (*s) {
		if (fname_ismbblead(*s) && s[1]) {
			s += 2;
		} else if (isupper(*s)) {
			*s = tolower(*s);
			++s;
		} else {
			++s;
		}
	}
	return s0;
}



/// filePath中の \ を / に置換.
template<typename CHR_T>
char 	*fname_backslashToSlash(CHR_T* filePath)
{
	char *p;
	for (p = filePath; *p != '\0'; ++p) {
		if (*p == '\\') {
			*p = '/';
		} else if (fname_ismbblead((*(unsigned char *)p)) && *(p+1) ) {
			++p;
		}
	}
	return filePath;
}



/// filePath中の / を \ に置換.
template<typename CHR_T>
char 	*fname_slashToBackslash(char* filePath)
{
	char *p;
	for (p = filePath; *p != '\0'; ++p) {
		if (*p == '/') {
			*p = '\\';
		}
	}
	return filePath;
}



// ===========================================================================

/// コマンドラインで指定されたファイル名として、""を考慮して,
/// 空白で区切られた文字列(ファイル名)を取得.
/// @return スキャン更新後のアドレスを返す。strがEOSだったらNULLを返す.
template<typename CHR_T>
char *fname_scanArgStr(const CHR_T *str, CHR_T *arg, int argSz)
{
	const unsigned char* s = (const unsigned char*)str;
	char*    			 d = arg;
	char*				 e = d + argSz;
	unsigned 			 f = 0;
	int					 c;

	assert( str != 0 && arg != 0 && argSz > 1 );

	// 空白をスキップ.
	while ( isspace(*s) )
		++s;

	if (*s == '\0')	// EOSだったら、見つからなかったとしてNULLを返す.
		return NULL;

	do {
		c = *s++;
		if (c == '"') {
			f ^= 1;						// "の対の間は空白をファイル名に許す.ためのフラグ.

			// ちょっと気持ち悪いが、Win(XP)のcmd.exeの挙動に合わせてみる. 
			// (ほんとにあってるか、十分には調べてない)
			if (*s == '"' && f == 0)	// 閉じ"の直後にさらに"があれば、それはそのまま表示する.
				++s;
			else
				continue;				// 通常は " は省いてしまう.
		}
		if (d < e) {
			*d++ = (char)c;
		}
	} while (c >= 0x20 && (c != ' ' || f != 0));
	*--d  = '\0';
	--s;
	return (char *)s;
}




// ===========================================================================


/** 文字列の最後に \ か / があればそのアドレスを無ければ終端\0のアドレスを返す.
 */
template<typename CHR_T>
inline char*	fname_endDirSep(CHR_T *dir)
{
	if (dir) {
		char*		s 	= fname_baseName(dir);
		unsigned	l 	= strlen(s);
		if (l > 0) {
			char* 	p = s + l;
			if (p[-1] == '/')
				return p-1;
		  #if defined _WIN32 || defined _WIN64 || defined _DOS
			 else if (p[-1] == '\\') {
				int f = 0;
				while (*s) {
					f = 0;
					if (fname_ismbblead(*s) && s[1]) {
						s++;
						f = 1;
					}
					s++;
				}
				if (f == 0)
					return p-1;
			}
		  #endif
			return p;
		}
	}
	return dir;
}



/** 文字列の最後に \ か / があれば削除
 */
inline char *fname_delLastDirSep(char *dir)
{
	*fname_endDirSep(dir);
	return dir;
}



// ===========================================================================

/**	環境変数tmp か temp からテンポラリディレクトリ名を取得. なければカレント"./"を返す.
 *  @return	0:パスに対してバッファが少なかった.  1:tmp,tempが無かった.  2:tmp|tempがあった.
 */
template<typename CHR_T>
int fname_getTmpDir(CHR_T *dirName, size_t dirNameLen)
{
	int	 	rc = 1;
	char 	*p;
	char 	nm[FNAME_MAX_PATH+32];

	p = std::getenv("TMP");
	if (p == NULL) {
		p = std::getenv("TEMP");
		if (p == NULL) {
			p  = "." MISC_FNAME_DIRSEP_STR;
			rc = 2;
		}
	}

	l = strlen(p);
	if (l > FNAME_MAX_PATH) {
		l  = FNAME_MAX_PATH;
		rc = 0;
	}

	std::memcpy(nm, p, l);
	nm += l;
	*nm = 0;

	fname_delLastDirSep(nm);
	if (p[-1] != ':')
		strcat(nm, MISC_FNAME_DIRSEP_STR);

	strcat(nm,"*.*");

	if (fname_fullpath(dirName, nm, dirNameLen) == 0)
		rc = 0;

	p = fname_baseName(dirName);
	*p = 0;
	if (p[-1] == MISC_FNAME_DIRSEP_CHR)
		p[-1] = 0;

	return rc;
}



template<class STRING>
int fname_getTmpDir(STRING& tmpDir)
{
	char buf[4096];
	int rc = fname_getTmpDir(buf, sizeof buf);
	tmpDir.assign(buf);
	return rc;
}



/// name 中のドライブ名とベース名を抜いたディレクトリ名をmdir[]に入れて返す
template<typename CHR_T>
char *fname_getMidDir(CHR_T* mdir, const CHR_T *name)
{
	char *d;
	if (mdir == NULL || name == NULL)
		return NULL;

  #if defined _WIN32 || defined _DOS
	if (name[1] == ':')		// ドライブ名付きだった
		name += 2;
	if (name[0] == '\\' || name[0] == '/')	// ルート指定されてた
		name += 1;
	strcpy(mdir, name);
	d = fname_baseName(mdir);
	if (d <= mdir) {
		mdir[0] = 0;
	} else if (d[-1] == '\\' || d[-1] == '/') {
		*d = '\0';
	}
  #else
	if (name[0] == '/')	// ルート指定されてた
		name += 1;
	strcpy(mdir, name);
	d = fname_baseName(mdir);
	if (d <= mdir) {
		mdir[0] = 0;
	} else if (d[-1] == '/') {
		*d = '\0';
	}
  #endif
	return mdir;
}




// ===========================================================================

/** フルパス生成.
 *  fullName=NULLならmallocしたアドレスを返す. エラーがあればNULLを返す.
 */
template<typename CHR_T>
inline char* fname_fullpath(CHR_T* fullName, const CHR_T* path, std::size_t fullNameSize)
{
  #if defined(MISC_FILE_WIN_CC)
	unsigned sz = fullNameSize;
	if (fullName == NULL) {
		sz = (sz > FILE_MAX_PATH) ? sz : FILE_MAX_PATH;
		fullName = (char*)std::malloc(sz + 4);
		if (fullName == NULL)
			return fullName;
	}
	_fullpath(fullName, path, sz);
	return fullName;
  #else	// gcc(linux)
	unsigned sz  = 0;
	char* 	 buf = realpath(path, NULL);
	if (buf == NULL)
		return NULL;
	if (fullName == NULL)
		return buf;
	sz = std::strlen(buf) + 1;
	if (sz > fullNameSize) {
		std::free(buf);
		return NULL;
	}
	std::memcpy(fullName, buf, sz);
	std::free(buf);
	return fullName;
  #endif
}



template<class STRING>
STRING fname_fullpath(const STRING& path)
{
	char	buf[FILE_MAX_PATH+4];
	char*	p = file_fullpath(buf, path.c_str(), sizeof buf);
	return STRING( p ? p : "" );
}




// ===========================================================================



/** ディレクトリ名とファイル名をくっつけたものをstrdupする
 *  拡張子の付け替えができるよう、+10バイトは余分にメモリは確保する
 */
char *fname_dirNameDup(const char *dir, const char *name)
{
	int l,n;
	char *m;

	l = (dir) ? strlen(dir) : 0;
	n = (name) ? strlen(name) : 0;
	m = (char*)fname_malloc(l+n+1+10);
	if (m == NULL)
		return NULL;
	if (l) {
		strcpy(m, dir);
		fname_delLastDirSep(m);
		strcat(m, MISC_FNAME_DIRSEP_STR);
	}
	if (n) {
		strcat(m, name);
	}
	return m;
}




// ===========================================================================


template<chass LISTVEC=std::vector<char*>, class ALC=std::allocator<unsigned char> >
class Fname_Matchs : private ALC {
public:
	~Fname_Matchs();
	Fname_Matchs() : recFlag_(0) {;}
	Fname_Matchs(const char* srch, bool recFlag) { run(srch, recFlag); }

	run(const char* srchName, int recFlag);

	const LISTVEC&	list() const { return list_; }
	std::size_t	size() const { return list_.size(); }

private:
	unsigned	run1();
	char*		strDup();
	char*		alloc(unsigned sz) { return ALC::allocator(sz); }
	void		dealloc(void* p)   { return ALC::deallocator(p, 1); }
private:
	LISTVEC		list_;
	bool		recFlag_;
};



template<class L, class A>
Fname_Matchs::~Fname_Matchs()
{
	for (typename VecList::iterator it = list_.begin(); it != list_.end(); ++it)
		dealloc(*it);
}



template<class L, class A>
char*	Fname_Matchs::strDup(const char* fname)
{
	size_t	 sz = std::strlen(s) + 1;
	char*    p  = alloc(sz);
	if (p == NULL)
		return NULL;
	return (char*)std::memcpy(p, s, sz);
}





/** srchNameで指定されたパス名(ワイルドカード文字対応) にマッチするパス名を全て取得.
 *	recFlag が真なら再帰検索を行う.
 *	取得後はlist()経由で、結果を取り出せる.
 */
template<class L, class A>
int		Fname_Matchs<L,A>::run(const char* srchName, bool recFlag)
{
	recFlag_ = recFlag;
	list_.clear();
	return run1(srcName);
}



/** srchNameで指定されたパス名(ワイルドカード文字対応) にマッチするパス名を全て取得.
 *	recFlag_ が真なら再帰検索を行う.
 */
template<class L, class A>
int		Fname_Matchs<L,A>::run1(const char* srchName)
{
  #if defined _WIN32 || defined _WIN64
	unsigned 			num       	= 0;
	WIN32_FIND_DATA*	pFindData 	= (WIN32_FIND_DATA*)alloc(sizeof(WIN32_FIND_DATA));
	HANDLE 				hdl;
	char*				pathBuf;
	char*				baseName;
	size_t				baseNameSz;

	if (pFindData == NULL)
		return -1;
	hdl 		= ::FindFirstFile(srchName, pFindData);
	pathBuf  	= alloc(FNAME_MAX_PATH);
	if (pathBuf == NULL)
		return -1;

	fname_strLcpy(pathBuf, srchName, FNAME_MAX_PATH);

	baseName 	= fname_baseName(pathBuf);
	*baseName  	= 0;
	baseNameSz 	= FNAME_MAX_PATH - std::strlen(pathBuf);
	assert(baseNameSz >= MAX_PATH);

	if (hdl != INVALID_HANDLE_VALUE) {
		// ファイル名を取得.
		do {
			fname_strLcpy(baseName, pFindData->cFileName, baseNameSz);
			if ((pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				list_.push_back( strDup(pathBuf) );
				++num;
			}
		} while (::FindNextFile(hdl, pFindData) != 0);
		::FindClose(hdl);
	}

	// ファイル名を取得.
	if (recFlag_ && baseNameSz >= 16) {
		const char* srch = fname_baseName(srchName);
		strcpy(baseName, "*.*");
		hdl = ::FindFirstFile(pathBuf, pFindData);
		if (hdl != INVALID_HANDLE_VALUE) {
			do {
				fname_strLcpy(baseName, pFindData->cFileName, baseNameSz);
				if ((pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					if (std::strcmp(baseName, ".") == 0 || std::strcmp(baseName, "..") == 0) {
						;
					} else {
                        std::strncat(baseName, MISC_FNAME_DIRSEP_STR, baseNameSz);
                        std::strncat(baseName, srch, baseNameSz);
						num += run1( pathBuf);
					}
				}
			} while (::FindNextFile(hdl, pFindData) != 0);
			::FindClose(hdl);
		}
	}

	dealloc(pathBuf);
	dealloc(pFindData);
	return num;
  #else
	struct dirent**	namelist = 0;
	unsigned 		num      = 0;
	char*			pathBuf  = alloc(FNAME_MAX_PATH);
	int				dirNum;
	char*			srchBase;
	char*			baseName;
	size_t			baseNameSz;
	int				flag = 0;

	if (pathBuf == 0)
		return -1;

	srchBase 	= fname_baseName(srchName);
	fname_strLcpy(pathBuf, srchName, FNAME_MAX_PATH);

	baseName 	= fname_baseName(pathBuf);

	if (baseName == pathBuf) {	// ディレクトリ部が無い場合.
		std::strcpy(pathBuf, "./");	// カレント指定を一時的に設定.
		baseName = pathBuf+2;
		flag	 = 1;
	}
	*baseName  	= 0;
	baseNameSz 	= FNAME_MAX_PATH - strlen(pathBuf);
	assert(baseNameSz >= MAX_PATH);

	// ディレクトリエントリの取得.
	baseName[-1] = 0;
	dirNum = scandir(pathBuf, &namelist, 0, alphasort);
	baseName[-1] = '/';

	if (flag) {	// 一時的なカレント指定だったならば、捨てる.
		baseName  = pathBuf;
		*baseName = '\0';
	}

	if (namelist) {
		struct stat statBuf;
		int			i;

		// ファイル名を取得.
		for (i = 0; i < dirNum; ++i) {
			struct dirent* d = namelist[i];
			if (fnmatch(srchBase, d->d_name, 0) == 0) {
				fname_strLcpy(baseName, d->d_name, baseNameSz);
				if (stat(pathBuf, &statBuf) >= 0) {
					if ((statBuf.st_mode & S_IFMT) != S_IFDIR) {
						list_.push_back( pathBuf );
						++num;
					}
				}
			}
		}

		// ディレクトリがあれば再帰.
		if (recFlag_ && baseNameSz >= 16) {
			const char* srch = fname_baseName(srchName);
			for (i = 0; i < dirNum; ++i) {
				struct dirent* d = namelist[i];
				fname_strLcpy(baseName, d->d_name, baseNameSz);
				if (stat(pathBuf, &statBuf) >= 0
						&& std::strcmp(baseName,".") != 0
						&& std::strcmp(baseName,"..") !=0
				){
					if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
	                    std::strncat(baseName, "/", baseNameSz);
						std::strncat(baseName, srch, baseNameSz);
						num += run1( pathBuf );
					}
				}
			}
		}

		// 使ったメモリを開放.
		for (i = 0; i < dirNum; ++i)
			std::free( namelist[i] );
		std::free( namelist );
	}
	dealloc( pathBuf );
	return num;
  #endif
}




// ===========================================================================


#endif
