/**	�t�@�C��������
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
#else	// WinNT�n(2K?�ȍ~)�Ȃ�A�f�B���N�g���Z�p���[�^�Ƃ��� / �Ŗ��Ȃ��͗l.
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

/** ���� C �� MS�S�p�̂P�o�C�g�ڂ��ۂ�. (utf8��euc�� \ ���͖����̂� 0���A���ok)
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

/** �t�@�C���p�X�����̃f�B���N�g�����������t�@�C�����̈ʒu��Ԃ�.
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



/**	�g���q�̈ʒu��Ԃ�.
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



/**	�g���q���Aext �ɕύX����.
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



/**	�g���q���Ȃ���΁Aext ��ǉ�����.
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

/** �S�p�Q�o�C�g�ڂ��l������ strupr
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



/** �S�p�Q�o�C�g�ڂ��l������ strlwr
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



/// filePath���� \ �� / �ɒu��.
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



/// filePath���� / �� \ �ɒu��.
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

/// �R�}���h���C���Ŏw�肳�ꂽ�t�@�C�����Ƃ��āA""���l������,
/// �󔒂ŋ�؂�ꂽ������(�t�@�C����)���擾.
/// @return �X�L�����X�V��̃A�h���X��Ԃ��Bstr��EOS��������NULL��Ԃ�.
template<typename CHR_T>
char *fname_scanArgStr(const CHR_T *str, CHR_T *arg, int argSz)
{
	const unsigned char* s = (const unsigned char*)str;
	char*    			 d = arg;
	char*				 e = d + argSz;
	unsigned 			 f = 0;
	int					 c;

	assert( str != 0 && arg != 0 && argSz > 1 );

	// �󔒂��X�L�b�v.
	while ( isspace(*s) )
		++s;

	if (*s == '\0')	// EOS��������A������Ȃ������Ƃ���NULL��Ԃ�.
		return NULL;

	do {
		c = *s++;
		if (c == '"') {
			f ^= 1;						// "�̑΂̊Ԃ͋󔒂��t�@�C�����ɋ���.���߂̃t���O.

			// ������ƋC�����������AWin(XP)��cmd.exe�̋����ɍ��킹�Ă݂�. 
			// (�ق�Ƃɂ����Ă邩�A�\���ɂ͒��ׂĂȂ�)
			if (*s == '"' && f == 0)	// ��"�̒���ɂ����"������΁A����͂��̂܂ܕ\������.
				++s;
			else
				continue;				// �ʏ�� " �͏Ȃ��Ă��܂�.
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


/** ������̍Ō�� \ �� / ������΂��̃A�h���X�𖳂���ΏI�[\0�̃A�h���X��Ԃ�.
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



/** ������̍Ō�� \ �� / ������΍폜
 */
inline char *fname_delLastDirSep(char *dir)
{
	*fname_endDirSep(dir);
	return dir;
}



// ===========================================================================

/**	���ϐ�tmp �� temp ����e���|�����f�B���N�g�������擾. �Ȃ���΃J�����g"./"��Ԃ�.
 *  @return	0:�p�X�ɑ΂��ăo�b�t�@�����Ȃ�����.  1:tmp,temp����������.  2:tmp|temp��������.
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



/// name ���̃h���C�u���ƃx�[�X���𔲂����f�B���N�g������mdir[]�ɓ���ĕԂ�
template<typename CHR_T>
char *fname_getMidDir(CHR_T* mdir, const CHR_T *name)
{
	char *d;
	if (mdir == NULL || name == NULL)
		return NULL;

  #if defined _WIN32 || defined _DOS
	if (name[1] == ':')		// �h���C�u���t��������
		name += 2;
	if (name[0] == '\\' || name[0] == '/')	// ���[�g�w�肳��Ă�
		name += 1;
	strcpy(mdir, name);
	d = fname_baseName(mdir);
	if (d <= mdir) {
		mdir[0] = 0;
	} else if (d[-1] == '\\' || d[-1] == '/') {
		*d = '\0';
	}
  #else
	if (name[0] == '/')	// ���[�g�w�肳��Ă�
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

/** �t���p�X����.
 *  fullName=NULL�Ȃ�malloc�����A�h���X��Ԃ�. �G���[�������NULL��Ԃ�.
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



/** �f�B���N�g�����ƃt�@�C�����������������̂�strdup����
 *  �g���q�̕t���ւ����ł���悤�A+10�o�C�g�͗]���Ƀ������͊m�ۂ���
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





/** srchName�Ŏw�肳�ꂽ�p�X��(���C���h�J�[�h�����Ή�) �Ƀ}�b�`����p�X����S�Ď擾.
 *	recFlag ���^�Ȃ�ċA�������s��.
 *	�擾���list()�o�R�ŁA���ʂ����o����.
 */
template<class L, class A>
int		Fname_Matchs<L,A>::run(const char* srchName, bool recFlag)
{
	recFlag_ = recFlag;
	list_.clear();
	return run1(srcName);
}



/** srchName�Ŏw�肳�ꂽ�p�X��(���C���h�J�[�h�����Ή�) �Ƀ}�b�`����p�X����S�Ď擾.
 *	recFlag_ ���^�Ȃ�ċA�������s��.
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
		// �t�@�C�������擾.
		do {
			fname_strLcpy(baseName, pFindData->cFileName, baseNameSz);
			if ((pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				list_.push_back( strDup(pathBuf) );
				++num;
			}
		} while (::FindNextFile(hdl, pFindData) != 0);
		::FindClose(hdl);
	}

	// �t�@�C�������擾.
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

	if (baseName == pathBuf) {	// �f�B���N�g�����������ꍇ.
		std::strcpy(pathBuf, "./");	// �J�����g�w����ꎞ�I�ɐݒ�.
		baseName = pathBuf+2;
		flag	 = 1;
	}
	*baseName  	= 0;
	baseNameSz 	= FNAME_MAX_PATH - strlen(pathBuf);
	assert(baseNameSz >= MAX_PATH);

	// �f�B���N�g���G���g���̎擾.
	baseName[-1] = 0;
	dirNum = scandir(pathBuf, &namelist, 0, alphasort);
	baseName[-1] = '/';

	if (flag) {	// �ꎞ�I�ȃJ�����g�w�肾�����Ȃ�΁A�̂Ă�.
		baseName  = pathBuf;
		*baseName = '\0';
	}

	if (namelist) {
		struct stat statBuf;
		int			i;

		// �t�@�C�������擾.
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

		// �f�B���N�g��������΍ċA.
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

		// �g�������������J��.
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
