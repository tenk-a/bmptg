/**	�t�@�C��������
 */

#include "fname.h"
#include <string.h>
#include <stdlib.h>


// #define FNAME_CMDLINE_ERR



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
#define DIRSEP_CHR			'\\'
#define DIRSEP_STR			"\\"
#else	// WinNT�n(2K?�ȍ~)�Ȃ�A�f�B���N�g���Z�p���[�^�Ƃ��� / �Ŗ��Ȃ��͗l.
#define DIRSEP_CHR			'/'
#define DIRSEP_STR			"/"
#endif




/// ���� C �� MS�S�p�̂P�o�C�g�ڂ��ۂ�. (utf8��euc�� \ ���͖����̂� 0���A���ok)
int fname_ismbblead(unsigned char c)
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



/// �t�@�C���p�X�����̃f�B���N�g�����������t�@�C�����̈ʒu��Ԃ�.
char *fname_baseName(const char *adr)
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
char *fname_getExt(const char *name)
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



/**	�g���q���Ȃ���΁Aext ��ǉ�����.
 */
char *fname_addExt(char filename[], const char *ext)
{
	if (strrchr(fname_baseName(filename), '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}




/** �S�p�Q�o�C�g�ڂ��l������ strupr
 */
char *fname_strUpr(char *s0)
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
char *fname_strLwr(char *s0)
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
char 	*fname_backslashToSlash(char filePath[])
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
char 	*fname_slashToBackslash(char filePath[])
{
	char *p;
	for (p = filePath; *p != '\0'; ++p) {
		if (*p == '/') {
			*p = '\\';
		}
	}
	return filePath;
}



/** ������̍Ō�� \ �� / ������΍폜 */
char *fname_delLastDirSep(char *dir)
{
	if (dir) {
		unsigned int l;
		char*		 s 	= fname_baseName(dir);
		l 	= strlen(s);
		if (l > 1) {
			char* 	p = s + l;
			if (p[-1] == '/') {
				p[-1] = 0;
			}
		  #if defined _WIN32 || defined _WIN64 || defined _DOS
			 else if (p[-1] == '\\') {
				// if (fname_sjisFlag == 0) {
				//		p[-1] = 0;
				// } else
				{
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
						p[-1] = 0;
				}
			}
		  #endif
		}
	}
	return dir;
}



/// �R�}���h���C���Ŏw�肳�ꂽ�t�@�C�����Ƃ��āA""���l������,
/// �󔒂ŋ�؂�ꂽ������(�t�@�C����)���擾.
/// @return �X�L�����X�V��̃A�h���X��Ԃ��Bstr��EOS��������NULL��Ԃ�.
char *fname_scanArgStr(const char *str, char arg[], int argSz)
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



/**	���ϐ�tmp �� temp ����e���|�����f�B���N�g�������擾. �Ȃ���΃J�����g"./"��Ԃ�.
 *  @return		0:tmp,temp����������.  1:������.
 */
int fname_getTmpDir(char *dirName, size_t dirNameLen)
{
	int	 	f = 0;
	char 	*p;
	char 	nm[FNAME_SIZE+2];

	if (*dirName) {
		strncpy(nm, dirName, FNAME_SIZE);
		nm[FNAME_SIZE] = '\0';
		p = nm + strlen(nm);
	} else {
		p = getenv("TMP");
		if (p == NULL) {
			p = getenv("TEMP");
			if (p == NULL) {
				p = "." DIRSEP_STR;
				f = 0;
			}
		}
		strcpy(nm, p);
		p = nm + strlen(nm);
	}

	if (p[-1] != '\\' && p[-1] != ':' && p[-1] != '/')
		strcat(nm, DIRSEP_STR);

	strcat(nm,"*.*");

	fname_fullpath(dirName, nm, dirNameLen);

	p = fname_baseName(dirName);
	*p = 0;
	if (p[-1] == DIRSEP_CHR)
		p[-1] = 0;

	return f;
}



/// name ���̃h���C�u���ƃx�[�X���𔲂����f�B���N�g������mdir[]�ɓ���ĕԂ�
char *fname_getMidDir(char mdir[], const char *name)
{
	char *d;

	if (mdir == NULL || name == NULL)
		return NULL;
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
	return mdir;
}



// ===========================================================================

/// malloc
static void* fname_malloc(unsigned size)
{
	void* p = malloc(size);
	if (p == NULL) {
	  #if defined FNAME_CMDLINE_ERR
		fprintf(STDERR, "not enough memory.\n");
		exit(1);
	  #else
		return NULL;
	  #endif
	}
	memset(p, 0, size);
	return p;
}



/// strdup
static char* fname_strdup(const char* s)
{
	size_t	 sz = strlen(s) + 1;
	char*    p  = (char*)fname_malloc(sz);
	if (p == NULL)
		return NULL;
	return (char*)memcpy(p, s, sz);
}



/// free
static void fname_free(void* s)
{
	if (s)
		free(s);
}




// ===========================================================================

/** �t���p�X����.
 *  fullName=NULL�Ȃ�malloc�����A�h���X��Ԃ�. �G���[�������NULL��Ԃ�.
 */
char*	fname_fullpath(char* fullName, const char* path, size_t fullNameSize)
{
  #if defined _WIN32 || defined _WIN64 || defined _DOS
	unsigned sz = 0 < fullNameSize ?  fullNameSize : FNAME_SIZE ;
	if (fullName == NULL) {
		fullName = fname_malloc(sz + 16);
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
	sz = strlen(buf) + 1;
	if (sz > fullNameSize) {
		free(buf);
		return NULL;
	}
	memcpy(fullname, buf, sz);
	free(buf);
	return fullname;
  #endif
}



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
		strcat(m, DIRSEP_STR);
	}
	if (n) {
		strcat(m, name);
	}
	return m;
}




// ===========================================================================

#define FNAME_ARY_BASE_SIZE		2048


///	���������񃊃X�g���Ǘ����鍪�����쐬.
///
fname_ary_t* fname_ary_create(unsigned size)
{
	fname_ary_t* pVec = fname_malloc( sizeof(fname_ary_t) );
	if (pVec == NULL)
		return NULL;
	size 				= ((size + FNAME_ARY_BASE_SIZE) / FNAME_ARY_BASE_SIZE) * FNAME_ARY_BASE_SIZE;
	pVec->capa 			= size;
	pVec->size			= 0;
	pVec->ptr 			= fname_malloc(sizeof(void*) * size);
	if (pVec->ptr == NULL) {
		fname_free(pVec);
		return NULL;
	}
	return pVec;
}



///	���������񃊃X�g�ɁA�������ǉ�.
///
void fname_ary_push(fname_ary_t* pVec, const char* pStr)
{
	assert(pVec != 0);
	assert(pStr  != 0);
	if (pStr && pVec) {
		unsigned 	capa = pVec->capa;
		assert(pVec->ptr != 0);
		if (pVec->size >= capa) {	// �L���p�𒴂��Ă�����A���������m�ۂ��Ȃ���.
			char**		ptr;
			//x printf("!  %p: %p %d %d ::%s\n", pVec, pVec->ptr, pVec->capa, pVec->size, pStr);
			assert(pVec->size == capa);
			pVec->capa 	= capa + FNAME_ARY_BASE_SIZE;
			ptr			= fname_malloc(sizeof(void*) * pVec->capa);
			if (pVec->ptr)
				memcpy(ptr, pVec->ptr, capa*sizeof(void*));
			memset(ptr+capa, 0, FNAME_ARY_BASE_SIZE*sizeof(void*));
			fname_free(pVec->ptr);
			pVec->ptr  	= ptr;
		}
		assert(pVec->size < pVec->capa);
		pVec->ptr[ pVec->size ] = fname_strdup(pStr);
		++ pVec->size;
		//x printf("!!  %p: %p %d %d ::%s\n", pVec, pVec->ptr, pVec->capa, pVec->size, pStr);
	}
}



/** fname_matchs �Ŏ擾���ꂽargc,argv �̃��������J������.
 */
void fname_argv_release(int argc, char** argv)
{
	unsigned i;
	for (i = 0; i < argc; ++i) {
		fname_free(argv[i]);
	}
	fname_free(argv);
}



/**	fname_ary_t �̕ێ����郁�������J������.
 */
void fname_ary_release(fname_ary_t* pVec)
{
	unsigned i;
	for (i = 0; i < pVec->size; ++i) {
		fname_free(pVec->ptr[i]);
	}
	fname_free(pVec->ptr);
	fname_free(pVec);
}



/**	srchName�Ƀ}�b�`����t�@�C�����̈ꗗ��Ԃ�.
 */
char** fname_matchs(const char* srchName, int recFlag, int* pArgc)
{
	fname_ary_t* pVec = fname_ary_create(1);
	char**	 ppArgv;
	assert(pArgc != 0);
	fname_ary_matchs(pVec, srchName, recFlag);
	*pArgc	 = pVec->size;
	ppArgv   = pVec->ptr;
	fname_Free(pVec);
	return ppArgv;
}




/** srchName�Ŏw�肳�ꂽ�p�X��(���C���h�J�[�h�����Ή�) �Ƀ}�b�`����p�X����S�� pVec �ɓ���ĕԂ�.
 *	recFlag ���^�Ȃ�ċA�������s��.
 */
int 	fname_ary_matchs(fname_ary_t* pVec, const char* srchName, int recFlag)
{
  #if defined _WIN32 || defined _WIN64
	unsigned 			num       	= 0;
	WIN32_FIND_DATA*	pFindData 	= (WIN32_FIND_DATA*)fname_malloc(sizeof(WIN32_FIND_DATA));
	HANDLE 				hdl;
	char*				pathBuf;
	char*				baseName;
	size_t				baseNameSz;

	if (pFindData == NULL)
		return -1;
	hdl 		= FindFirstFile(srchName, pFindData);
	pathBuf  	= (char*)fname_malloc(FNAME_SIZE);
	if (pathBuf == NULL)
		return -1;

	strncpy(pathBuf, srchName, FNAME_SIZE);
	pathBuf[ FNAME_SIZE-1 ] = '\0';

	baseName 	= fname_baseName(pathBuf);
	*baseName  	= 0;
	baseNameSz 	= FNAME_SIZE - strlen(pathBuf);
	assert(baseNameSz >= MAX_PATH);

	if (hdl != INVALID_HANDLE_VALUE) {
		// �t�@�C�������擾.
		do {
			strncpy(baseName, pFindData->cFileName, baseNameSz);
			pathBuf[ FNAME_SIZE-1 ] = '\0';	//x baseName[baseNameSz-1] = '\0';
			if ((pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				fname_ary_push( pVec, pathBuf );
				++num;
			}
		} while (FindNextFile(hdl, pFindData) != 0);
		FindClose(hdl);
	}

	// �t�@�C�������擾.
	if (recFlag && baseNameSz >= 16) {
		const char* srch = fname_baseName(srchName);
		strcpy(baseName, "*.*");
		hdl = FindFirstFile(pathBuf, pFindData);
		if (hdl != INVALID_HANDLE_VALUE) {
			do {
				strncpy(baseName, pFindData->cFileName, baseNameSz);
				pathBuf[ FNAME_SIZE-1 ] = '\0';	//x baseName[baseNameSz-1] = '\0';
				if ((pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					if (strcmp(baseName, ".") == 0 || strcmp(baseName, "..") == 0) {
						;
					} else {
                        strncat(baseName, DIRSEP_STR, baseNameSz);
                        strncat(baseName, srch, baseNameSz);
						num += fname_ary_matchs(pVec, pathBuf, 1);
					}
				}
			} while (FindNextFile(hdl, pFindData) != 0);
			FindClose(hdl);
		}
	}

	fname_free(pathBuf);
	fname_free(pFindData);
	return num;
  #else
	struct dirent**	namelist = 0;
	unsigned 		num      = 0;
	char*			pathBuf  = (char*)fname_malloc(FNAME_SIZE);
	int				dirNum;
	char*			srchBase;
	char*			baseName;
	size_t			baseNameSz;
	int				flag = 0;

	if (pathBuf == 0)
		return -1;

	srchBase 	= fname_baseName(srchName);
	strncpy(pathBuf, srchName, FNAME_SIZE);
	pathBuf[ FNAME_SIZE-1 ] = '\0';

	baseName 	= fname_baseName(pathBuf);

	if (baseName == pathBuf) {	// �f�B���N�g�����������ꍇ.
		strcpy(pathBuf, "./");	// �J�����g�w����ꎞ�I�ɐݒ�.
		baseName = pathBuf+2;
		flag	 = 1;
	}
	*baseName  	= 0;
	baseNameSz 	= FNAME_SIZE - strlen(pathBuf);
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
				strncpy(baseName, d->d_name, baseNameSz);
				pathBuf[ FNAME_SIZE-1 ] = '\0';	//x baseName[baseNameSz-1] = '\0';
				if (stat(pathBuf, &statBuf) >= 0) {
					if ((statBuf.st_mode & S_IFMT) != S_IFDIR) {
						fname_ary_push( pVec, pathBuf );
						++num;
					}
				}
			}
		}

		// �f�B���N�g��������΍ċA.
		if (recFlag && baseNameSz >= 16) {
			const char* srch = fname_baseName(srchName);
			for (i = 0; i < dirNum; ++i) {
				struct dirent* d = namelist[i];
				strncpy(baseName, d->d_name, baseNameSz);
				pathBuf[ FNAME_SIZE-1 ] = '\0';	//x baseName[baseNameSz-1] = '\0';
				if (stat(pathBuf, &statBuf) >= 0 && strcmp(baseName,".") != 0 && strcmp(baseName,"..") !=0 ) {
					if ((statBuf.st_mode & S_IFMT) == S_IFDIR) {
	                    strncat(baseName, "/", baseNameSz);
						strncat(baseName, srch, baseNameSz);
						num += fname_ary_matchs(pVec, pathBuf, 1);
					}
				}
			}
		}

		// �g�������������J��.
		for (i = 0; i < dirNum; ++i)
			free( namelist[i] );
		free( namelist );
	}
	fname_free( pathBuf );
	return num;
  #endif
}



// ===========================================================================

