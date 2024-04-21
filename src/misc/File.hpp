/**
  @file     File.h
  *brief    api�̃t�@�C���֌W�����b�v�����N���X��֐�.
  *author   Masashi Kitamura.
  *date     2004,2007
  @note

  - C�W����FILE�x�[�X�Aio.h/unistd.h�x�[�X�Awin-API�x�[�X�A�R�̎���������.
  - size64()��seek64()��64�r�b�g�l�ł� win-API�ňȊO�� ����32�r�b�g�l��.
  - �w�b�_�t�@�C���݂̂ɂ��邽�߂ɁA������template�����Ď������Ă�����̂�����.
  - ��{�I�� vc �ł̃`�F�b�N�̂�. ���܂�win�̑��̃R���p�C���ł��邪,
    mingw�͂悢����ǁAbcc,dmc,watcom�͂��܂������肵�ĂȂ��͗l.
    win�ȊO�́A�C��������.
 */
/*
    // ��`����Ă���N���X���֐��̃v���g�^�C�v. (����`��inline�Ō��Â炢�̂Ŕ���)
    class File {
    public:
        enum RW_T { R,W,RP,WP,} // "rb", "wb", "rb+", "wb+",�ɑΉ�.���̒l�͎��ۂɂ�0,1,2,3�łȂ��ʂ̒l.
        enum      { seek_set, seek_cur, seek_end };
        File();
        File(const char* fname, RW_T rw);
        ~File();
        bool        is_open() const;
        bool        open  (const char* fname, RW_T rw);
        bool        open_e(const char* fname, RW_T rw);     // �G���[���������烁�b�Z�[�W�I��.
        void        close();
        size_t      read(void *buf, size_t size);
        size_t      read_e(void *buf, size_t size);         // �G���[���������烁�b�Z�[�W�I��.
        size_t      write(const void *buf, size_t size);
        size_t      write_e(const void *buf, size_t size);  // �G���[���������烁�b�Z�[�W�I��.
        bool        flush();

        size_t      seek(size_t ofs, int seekMode);
        uint64_t    seek64(uint64_t ofs, int seekMode);
        size_t      tell();
        uint64_t    tell64();
        size_t      size();
        uint64_t    size64();

        bool    getTime(uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
        bool    setTime(uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0); // win�n�̂�.

        const char* fname() const;
    };

    size_t   file_size(const char* fname);
    uint64_t file_size64(const char* fname);
    bool     file_getTime(const char* fname, uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
    bool     file_setTime(const char* fname, uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
    int      file_stat(const char* fname, file_stat_t* st);             // 32�r�b�g�ł̂��Ƃ�����.
    int      file_utime(const char* fname, file_utimbuf_t* utimbuf);    // 32�r�b�g�ł̂��Ƃ�����.

    bool     file_moveName(const char* oldName, const char* newName);
    bool     file_delete(const char* fname);
    bool     file_createDir(const char* fname);
    bool     file_deleteDir(const char* fname);
    bool     file_setCurrentDir(const char* nameBuf);
    bool     file_getCurrentDir(char* nameBuf, size_t nameBufSize);
    bool     file_getSystemDir(char* nameBuf, size_t nameBufSize);  // win�v�̂�.
    bool     file_getWindowsDir(char* nameBuf, size_t nameBufSize); // win�v�̂�.
    struct File_LocalTime;  // ���ԕϊ��p.

    unsigned file_save(const char* name, const void* buf, unsigned byteSize);
    unsigned file_load(const char* name, VECTOR& buf);
    unsigned file_load(const char* name, void* buf, unsigned byteSize);
    void*    file_loadAlloc(const char* name, size_t* pSize=0);
    void     file_loadFree(void* p);
    bool     file_copy(const char* srcName, const char* dstName);

*/


#ifndef FILE_H
#define FILE_H


// ========================================================================
// ��{�I��include.
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <climits>

#if defined( __BORLANDC__)
 #include <sys\stat.h>
 #include <utime.h>
#else
 #include <sys/stat.h>
 #include <sys/utime.h>
#endif


// open_e, read_e, write_e�ŗ�O�𓊂���FileEH��L���ɂ������ꍇ.
// #define MISC_FILE_USE_EH
#if defined MISC_FILE_USE_EH
// #include <exception>
#include <stdexcept>
#endif



// ------------------------------------------------------------------------
// �R���p�C���ʂ̒��날�킹�p
// file_size64_t �� vc ��uint64_t���`���Ă���Ă���s�v�������̂�����...

#if defined(_MSC_VER) || (defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601)
 #ifndef MISC_FILE_KIND
  #if defined _WINDOWS
   #define MISC_FILE_KIND       2   // Win-API�Ȃ�WinAPI��.
  #else
   #define MISC_FILE_KIND       0   // �R���\�[���Ȃ� FILE ��.
  #endif
 #endif
 #define MISC_FILE_WIN_CC           // Win�p�̃R���p�C���̎�(VC�݊�������)
 #define MISC_FILE_GMTIME64(x)      _gmtime64((const __time64_t *)(x))
 #define MISC_FILE_LOCALTIME64(x)   _localtime64((const __time64_t *)(x))
 #define MISC_FILE_MKTIME64         _mktime64
 #define MISC_FILE_FUTIME           _futime64
 #define MISC_FILE_FSTAT            _fstat64
 #define MISC_FILE_LENGTH64         _filelengthi64
 #define MISC_FILE_LSEEK64          _lseeki64
 #define MISC_FILE_STRDUP           _strdup
 namespace MISC {
    typedef unsigned __int64        file_size64_t;
    typedef struct __stat64         file_stat_t;
    typedef struct __utimbuf64      file_utimbuf_t;

    inline int  file_stat(const char* fname, file_stat_t* st) { return _stat64(fname, st); }
    inline int  file_utime(const char* fname, file_utimbuf_t* utimbuf) { return _utime64(fname, utimbuf); }
 }


#else   // (64bit�ł�stat,utime���Ȃ��ꍇ) -----------
 #ifndef MISC_FILE_KIND
  #if defined _WINDOWS || defined(__DMC__) || defined(__BORLANDC__) || defined(__WATCOM_C__) || defined(__MINGW32__)
   #define MISC_FILE_KIND       2   // WinAPI��.
  #elif defined(__CYGWIN__)
   #define MISC_FILE_KIND       1   // cygwin�Ȃ�unix���ǂ��Ƃ���.
  #else
   #define MISC_FILE_KIND       0   // �R���\�[���Ȃ� FILE ��.
  #endif
 #endif

 #if defined(__MINGW32__)                       // -------------
    #include <windows.h>
    #include <stdint.h>
    #define MISC_FILE_WIN_CC
    #define MISC_FILE_STRDUP    strdup

 #elif defined(__DMC__)                         // -------------
    #include <windows.h>
    #include <stdint.h>
    #define MISC_FILE_WIN_CC
    #define MISC_FILE_STRDUP    strdup

 #elif defined( __BORLANDC__)                   // -------------
    #include <windows.h>
    #if __BORLANDC__ <= 0x0551
     typedef __int64            uint64_t;
    #else
     #include <stdint.h>
    #endif
    #define MISC_FILE_WIN_CC
    #define MISC_FILE_STRDUP    strdup
 #elif 0    // defined(__WATCOM_C__)            // -------------
    // �F�X�s�s������悤�Ȃ̂ŁA������߂�.
    #include <windows.h>
    #include <stdint.h>
    #define MISC_FILE_WIN_CC
    #define MISC_FILE_FUTIME    _futime
    #define MISC_FILE_STRDUP    strdup
 #else                                          // -------------
    #include <stdint.h>
    #define MISC_FILE_STRDUP    strdup
 #endif
 #define MISC_FILE_FSTAT        fstat

 namespace MISC {
    typedef uint64_t            file_size64_t;
    typedef struct stat         file_stat_t;
    typedef struct utimbuf      file_utimbuf_t;

    inline int  file_stat(const char* fname, file_stat_t* st) { return ::stat(fname, st); }
    inline int  file_utime(const char* fname, file_utimbuf_t* utimbuf) { return ::utime(fname, utimbuf); }
 }
#endif




// ------------------------------------------------------------------------
// �����̎�ޕʂ�include

#if   MISC_FILE_KIND == 2   // Win-API ���g����.

 #include <windows.h>
 #include <io.h>

#else   // io.h,unistd.h��  stdio.h ��

 #include <cstdio>
 #include <stdio.h>         //  �R���p�C���ɂ����std�ɓ����������Ȃ�������Ȃ̂ŁA�������...
 #include <cerrno>
 #include <errno.h>         // errno�͕ϐ��łȂ��}�N���̏ꍇ������.
 #include <ctime>

 #ifdef MISC_FILE_WIN_CC    //  dos,win�n�R���p�C���̂Ƃ�.
  #include <io.h>
  #include <fcntl.h>
  #include <direct.h>
 #else                      //  linux(unix�n)�̎�.
  #include <sys/unistd.h>
  #include <sys/fcntl.h>
 #endif

#endif




/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
namespace MISC {



#if defined(MISC_FILE_WIN_CC)
enum { FILE_MAX_PATH        = _MAX_PATH };
#else
enum { FILE_MAX_PATH        =  NAME_MAX > PATH_MAX ?  NAME_MAX  : PATH_MAX };
#endif

class File_NouseFname;      // �t�@�C������ێ����Ȃ���.
class File_UseFname;        // �t�@�C������ێ������.

template<class DMY>
class File_ChkErrMsgExit;   // open_e, read_e, write_e�ŃG���[����������,���b�Z�[�W��\�����ďI��.




/* ======================================================================== */
/*                                                                          */
/* (�o�C�i��)�t�@�C���A�N�Z�X�̂��߂̃t�@�C���n���h���Ǘ��N���X             */
/*                                                                          */
/* ======================================================================== */


template<class ERR=File_ChkErrMsgExit<void>, class NAME=File_NouseFname >
class FileT : public NAME, public ERR {
  #if   MISC_FILE_KIND == 2     // WIN-API
    HANDLE      handle_;
  #elif MISC_FILE_KIND == 1     // io.h
    int         handle_;
  #else                         // stdio.h
    std::FILE*  fp_;
  #endif

public:
    // �I�[�v�����[�h. fopen�� rb,rb+,wb,wb+�����̂�.
    enum RW_T { R=0, W=1, RP=2, WP=3 };

    FileT();
    FileT(const char* fname, RW_T rw);
    ~FileT() { this->close(); }

    /// �I�[�v�����Ă��邩�ۂ�.
    bool    is_open() const;

    bool open  (const char* fname, RW_T rw);
    bool open_e(const char* fname, RW_T rw) {
        return ERR::checkOpen(this->open(fname, rw), fname);
    }

    void close();

    /// �ǂݍ���.
    std::size_t read(void *buf, std::size_t size);
    std::size_t read_e(void *buf, std::size_t size) {
        return ERR::checkRead( this->read(buf, size), size, NAME::fname() );
    }

    /// ��������.
    std::size_t write(const void *buf, std::size_t size);
    std::size_t write_e(const void *buf, std::size_t size) {
        return ERR::checkWrite( this->write(buf, size), size, NAME::fname() );
    }

    /// �o�b�t�@�����O���e���t���b�V��.
    bool        flush();

    enum { seek_set, seek_cur, seek_end };

    /// �w��̈ʒu�փt�@�C���|�C���^���ړ�.
    std::size_t     seek(std::size_t ofs, int seekMode);
    file_size64_t   seek64(file_size64_t ofs, int seekMode);

    /// ���݈ʒu��Ԃ�.
    std::size_t     tell();
    file_size64_t   tell64();

    /// �t�@�C���T�C�Y�̃o�C�g����Ԃ�. ���s�������seek�ʒu�͕s��Ƃ���.
    std::size_t size();
    file_size64_t size64();

    /// ���Ԃ̎擾. �� Win-API �Ƃ͈������t���Ȃ̂Œ���.
    bool    getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0);

  #if MISC_FILE_KIND == 2 || defined MISC_FILE_FUTIME
    /// ���Ԃ̐ݒ�. �� Win-API �Ƃ͈������t���Ȃ̂Œ���.
    /// win-api�ňȊO�́ApCreat�͖�������̂Œ���.
    /// �R���p�C����futime�𖢃T�|�[�g�̏ꍇ�͑S���_�~�[�Ȃ̂Œ���.
    bool    setTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0);
  #endif


public:
 #if MISC_FILE_KIND == 2    // Win-API�ł̎����̂�.
    HANDLE   handle() const { return handle_; }
    unsigned winFileType() { return (unsigned)::GetFileType(handle_); }

    bool open(const char*           fname,
              DWORD                 access,
              DWORD                 share               = FILE_SHARE_READ,
              PSECURITY_ATTRIBUTES  psa                 = 0,
              DWORD                 createDisposition   = OPEN_EXISTING,
              DWORD                 flagsAndAttr        = FILE_ATTRIBUTE_NORMAL,
              HANDLE                hTemplate           = 0                     );

    bool open_e(const char* fname, DWORD acs, DWORD share=FILE_SHARE_READ,
                PSECURITY_ATTRIBUTES psa=0, DWORD createDisposition=OPEN_EXISTING,
                DWORD flagsAndAttr =FILE_ATTRIBUTE_NORMAL, HANDLE hTemplate=0);

 #elif MISC_FILE_KIND == 1      // io.h, unistd.h �ł̂�.
    int handle() const { return handle_; }

    /** �I�[�v��.
     *  flags   O_RDONLY, O_WRONLY, O_RDWR,   O_CREAT,O_EXCL, O_TRUNC, O_APPEND,
     *          O_DIRECT, O_DIRECTORY, O_ASYNC, O_SYNC, O_NOATIME, O_NOFOLLOW, O_NONBLOCK, O_NDELAY...
     *  mode    �쐬���̃A�N�Z�X����(�p�[�~�b�V����) (��: 0766 �Ƃ�)
     */
    bool open(const char* fname, int flags, int mode);
    bool open_e(const char* fname, unsigned flags, int mode);

    int stat(file_stat_t* st);

   #if defined MISC_FILE_FUTIME
    int utime(file_utimbuf_t* utimbuf);
   #endif

 #else                      // FILE �n�̎����̂�.
    std::FILE*       fp()       { return fp_; }
    const std::FILE* fp() const { return fp_; }

    // FileT�ϐ���stdin,stdout,stderr ��ݒ肷�邽�߂ɗp��.
    // �f�X�g���N�^��close����̂ŕK�v�Ȃ�ēxNULL��ݒ肷��K�v������.
    void    setFp(std::FILE* fp) { fp_ = fp; }
    bool    open_e(const char* fname, const char *mode);
    bool    open(const char* fname, const char *mode);
    void    rewind();
    bool    eof();
    bool    error();
    void    clearerr();
    int     getpos(std::fpos_t *pos);
    int     fsetpos(const std::fpos_t *pos);

    enum EIobf {
        ioNbf=_IONBF,       // unbuffered
        ioLbf=_IOLBF,       // line buffered
        ioFbf=_IOFBF,       // fully buffered
    };
    void setvbuf(void *buf, int iobf, std::size_t size);
    void setBufferSize(std::size_t size);

    int stat(file_stat_t* st);
   #if defined MISC_FILE_FUTIME
    int utime(file_utimbuf_t* utimbuf);
   #endif
 #endif
};




#if MISC_FILE_KIND == 2
// -------------------------------------------------------------------------
// Windows-API �ł̎���
// -------------------------------------------------------------------------


template<class E, class N> inline
FileT<E,N>::FileT()
    : handle_(INVALID_HANDLE_VALUE)
{
}


template<class E, class N> inline
FileT<E,N>::FileT(const char* fname, RW_T rw)
    : handle_(INVALID_HANDLE_VALUE)
{
    this->open(fname, rw);
}


template<class E, class N> inline
bool    FileT<E,N>::is_open() const {
    return handle_ != INVALID_HANDLE_VALUE;
}



template<class E, class N>
bool FileT<E,N>::open  (const char* fname, RW_T rw) {
    static const unsigned rwTbl[4] = {
        GENERIC_READ,                       // R  "rb"  //  = 0x80000000
        GENERIC_WRITE|1,                    // W  "wb"  //  = 0x40000000|1
        GENERIC_READ|GENERIC_WRITE,         // RP "rb+" //  = 0xC0000000
        GENERIC_READ|GENERIC_WRITE|1,       // WP "wb+" //  = 0xC0000000|1
    };
    int n = rwTbl[rw];
    return this->open(fname, n&~1, FILE_SHARE_READ, 0, (n&1) ?  CREATE_ALWAYS : OPEN_EXISTING);
}


template<class E, class N>
void FileT<E,N>::close() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        ::CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
    N::releaseFname();
}



template<class E, class N>
std::size_t FileT<E,N>::read(void *buf, std::size_t size) {
    assert(handle_ != INVALID_HANDLE_VALUE && buf != 0);
    DWORD sz = 0;
    if (size > 0) {
        if (::ReadFile(handle_, (LPVOID)buf, size, &sz, NULL) == 0)
            sz = 0;
    }
    return sz;
}



template<class E, class N>
std::size_t FileT<E,N>::write(const void *buf, std::size_t size) {
    assert(handle_ != INVALID_HANDLE_VALUE && buf != 0);
    DWORD sz = 0;
    if (size > 0) {
        if (::WriteFile(handle_, (LPVOID)buf, size, &sz, NULL) == 0)
            sz = 0;
    }
    return sz;
}



template<class E, class N>
bool    FileT<E,N>::flush() {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::FlushFileBuffers(handle_) != 0;
}



template<class E, class N>
std::size_t FileT<E,N>::seek(std::size_t ofs, int seekMode) {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::SetFilePointer(handle_, (LONG) ofs, NULL, seekMode);
}



template<class E, class N>
file_size64_t   FileT<E,N>::seek64(file_size64_t ofs, int seekMode) {
    assert(handle_ != INVALID_HANDLE_VALUE);
    DWORD       h = DWORD(ofs >> 32);
    DWORD       l = ::SetFilePointer(handle_, DWORD(ofs), (PLONG)&h, seekMode);
    return ((file_size64_t)h << 32) | l;
}



template<class E, class N>
std::size_t     FileT<E,N>::tell() {
    return seek(0, seek_cur);
}



template<class E, class N>
file_size64_t   FileT<E,N>::tell64() {
    return seek64(0, seek_cur);
}



// size()�����s�������seek�ʒu�͕s��Ƃ���.
template<class E, class N>
std::size_t FileT<E,N>::size() {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::GetFileSize(handle_, 0);
}



template<class E, class N>
file_size64_t FileT<E,N>::size64() {
    assert(handle_ != INVALID_HANDLE_VALUE);
    DWORD       h;
    std::size_t l = ::GetFileSize(handle_, &h);
    return ((file_size64_t)h << 32) | l;
}



// ���Ԃ̎擾.  �� Win-API �Ƃ͈������t���Ȃ̂Œ���.
template<class E, class N>
bool    FileT<E,N>::getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::GetFileTime(handle_, (FILETIME*)pCreat, (FILETIME*)pLastAcs, (FILETIME*)pLastWrt) != 0;
}



// ���Ԃ̐ݒ�.  �� Win-API �Ƃ͈������t���Ȃ̂Œ���.
template<class E, class N>
bool    FileT<E,N>::setTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::SetFileTime(handle_, (FILETIME*)pCreat, (FILETIME*)pLastAcs, (FILETIME*)pLastWrt) != 0;
}



template<class E, class N>
bool FileT<E,N>::open_e(
        const char*             fname,
        DWORD                   access,
        DWORD                   share,
        PSECURITY_ATTRIBUTES    psa,
        DWORD                   createDisposition,
        DWORD                   flagsAndAttr,
        HANDLE                  hTemplate)
{
    return E::checkOpen(open(fname, access, share, psa, createDisposition, flagsAndAttr, hTemplate), fname);
}


template<class E, class N>
bool FileT<E,N>::open(
        const char*             fname,
        DWORD                   access,
        DWORD                   share,
        PSECURITY_ATTRIBUTES    psa,
        DWORD                   createDisposition,
        DWORD                   flagsAndAttr,
        HANDLE                  hTemplate)
{
    if (handle_ != INVALID_HANDLE_VALUE)
        this->close();
    N::setFname(fname);
    if (fname == 0)
        fname = "";     // NULL���ƃn���O����������邪open�����Ȃ���errno���ݒ肳��Ȃ��̂ő�p.
    handle_ = ::CreateFileA((PCSTR)fname, access, share, psa, createDisposition, flagsAndAttr, hTemplate);
    return handle_ != INVALID_HANDLE_VALUE;
}




#elif MISC_FILE_KIND == 1       // linux��gcc��z��.
// -------------------------------------------------------------------------
// unistd.h(io.h) �x�[�X. gcc(linux��)��.
// -------------------------------------------------------------------------


template<class E, class N> inline
FileT<E,N>::FileT() : handle_(-1) {}



template<class E, class N> inline
bool    FileT<E,N>::is_open() const {
    return handle_ != -1;
}



template<class E, class N> inline
FileT<E,N>::FileT(const char* fname, RW_T rw) : handle_(-1) {
    this->open(fname, rw);
}



template<class E, class N>
bool FileT<E,N>::open(const char* fname, RW_T rw) {
    static const int rwTbl[4] = {
        _O_RDONLY|_O_BINARY,                    // R  "rb"
        _O_WRONLY|_O_BINARY|_O_CREAT|_O_TRUNC,  // W  "wb"
        _O_RDWR|_O_BINARY,                      // RP "rb+"
        _O_RDWR|_O_BINARY|_O_CREAT,             // WP "wb+"
    };
    int n = rwTbl[rw];
    this->open(fname, n, 0766);
    return (handle_ >= 0);
}


template<class E, class N>
void FileT<E,N>::close() {
    if (handle_ != -1) {
        ::close(handle_);
        handle_ = -1;
    }
    N::releaseFname();
}



template<class E, class N>
std::size_t FileT<E,N>::read(void *buf, std::size_t size) {
    assert(handle_ != -1 && buf != 0);
    size_t sz = 0;
    if (size > 0)
        sz = ::read(handle_, buf, size);
    return sz;
}



template<class E, class N>
std::size_t FileT<E,N>::write(const void *buf, std::size_t size) {
    assert(handle_ != -1 && buf != 0);
    size_t sz = 0;
    if (size > 0)
        sz = ::write(handle_, buf, size);
    return sz;
}



template<class E, class N> inline
bool    FileT<E,N>::flush() {
    // �_�~�[.
    return true;
}



template<class E, class N>
std::size_t     FileT<E,N>::seek(std::size_t ofs, int seekMode) {
    assert(handle_ != -1);
    return ::lseek(handle_, ofs, seekMode);
}



template<class E, class N>
file_size64_t   FileT<E,N>::seek64(file_size64_t ofs, int seekMode) {
    assert(handle_ != -1);
  #if defined MISC_FILE_LSEEK64
    return MISC_FILE_LSEEK64(handle_, ofs, seekMode);
  #else
    return this->seek((std::size_t)ofs, seekMode);
  #endif
}



template<class E, class N>
std::size_t     FileT<E,N>::tell()   {
    return this->seek(0, seek_cur);
}



template<class E, class N>
file_size64_t   FileT<E,N>::tell64() {
    return this->seek64(0, seek_cur);
}



// size()�����s�������seek�ʒu�͕s��Ƃ���.
template<class E, class N>
std::size_t   FileT<E,N>::size() {
    file_stat_t st; return this->stat(&st)==0 ? (std::size_t)st.st_size : 0;
}



template<class E, class N>
file_size64_t FileT<E,N>::size64() {
    file_stat_t st; return this->stat(&st)==0 ? st.st_size : 0;
}



/// ���Ԃ̎擾. �l�̓V�X�e���Ɉˑ�.
template<class E, class N>
bool    FileT<E,N>::getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    file_stat_t st;
    bool rc = this->stat(&st)==0;
    if (rc) {
        if (pLastWrt) *pLastWrt = st.st_mtime;
        if (pLastAcs) *pLastAcs = st.st_atime;
        if (pCreat  ) *pCreat   = st.st_ctime;
    }
    return rc;
}



#if defined MISC_FILE_FUTIME
/// ���Ԃ̐ݒ�. pCreat�͖�������̂Œ���!.
template<class E, class N>
bool    FileT<E,N>::setTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* /*pCreat*/) {
    file_utimbuf_t  utimbuf;
    utimbuf.actime  = *pLastAcs;
    utimbuf.modtime = *pLastWrt;
    return this->utime(&utimbuf) == 0;
}
#endif




template<class E, class N>
bool FileT<E,N>::open_e(const char* fname, unsigned flags, int mode) {
    return E::checkOpen(this->open(fname, flags, mode), fname);
}


/** �I�[�v��.
 *  flags   O_RDONLY, O_WRONLY, O_RDWR,   O_CREAT,O_EXCL, O_TRUNC, O_APPEND,
 *          O_DIRECT, O_DIRECTORY, O_ASYNC, O_SYNC, O_NOATIME, O_NOFOLLOW, O_NONBLOCK, O_NDELAY...
 *  mode    �쐬���̃A�N�Z�X����(�p�[�~�b�V����) (��: 0766 �Ƃ�)
 */
template<class E, class N>
bool FileT<E,N>::open(const char* fname, int flags, int mode) {
    if (handle_ != -1)
        this->close();
    N::setFname(fname);
    if (fname == 0)
        fname = "";     // FILE�ƈႢerrno�ɐݒ肳��邩�͕s�������A�ꉞopen���Ƃ����߂�""��.
    handle_ = ::open(fname, flags, mode);
    //x printf("open%d %s %#x %#x\n", handle_, fname, flags, mode);
    return handle_ >= 0;
}


template<class E, class N>
int FileT<E,N>::stat(file_stat_t* st) {
    assert(handle_ != -1 && st != 0);
    return MISC_FILE_FSTAT(handle_, st);
}



#if defined MISC_FILE_FUTIME
template<class E, class N>
int FileT<E,N>::utime(file_utimbuf_t* utimbuf) {
    assert(handle_ != 0 && utimbuf != 0);
    return MISC_FILE_FUTIME(handle_, utimbuf);
}
#endif






#else   // stdio.h ��
// -------------------------------------------------------------------------
// �W�� C���C�u����(FILE)��.
// -------------------------------------------------------------------------


template<class E, class N>
FileT<E,N>::FileT() : fp_(0)
{
}



template<class E, class N>
bool    FileT<E,N>::is_open() const {
    return fp_ != NULL;
}



template<class E, class N>
FileT<E,N>::FileT(const char* fname, RW_T rw) : fp_(0) {
    this->open(fname, rw);
}



template<class E, class N>
bool FileT<E,N>::open(const char* fname, RW_T rw) {
    static const char* mode[] = { "rb", "wb", "rb+", "wb+" };
    return this->open(fname, mode[rw]);
}



template<class E, class N>
void FileT<E,N>::close() {
    if (fp_) {
        std::fclose(fp_);
        fp_ = 0;
    }
    N::releaseFname();
}



template<class E, class N>
std::size_t FileT<E,N>::read(void *buf, std::size_t size) {
    assert(fp_ && buf != 0);
    return size ? std::fread(buf, 1, size, fp_) : 0;
}



template<class E, class N>
std::size_t FileT<E,N>::write(const void *buf, std::size_t size) {
    assert(fp_ && buf != 0);
    return size ? std::fwrite(buf, 1, size, fp_) : 0;
}



template<class E, class N>
bool    FileT<E,N>::flush() {
    assert(fp_);
    return std::fflush(fp_) == 0;
}



template<class E, class N>
std::size_t     FileT<E,N>::seek(std::size_t ofs, int seekMode) {
    assert(fp_);
    return std::fseek(fp_, ofs, seekMode);
}



template<class E, class N>
std::size_t     FileT<E,N>::tell() {
    assert(fp_);
    return std::ftell(fp_);
}



// FILE�łł� 64�r�b�g�T�C�Y�̃V�[�N�͔�Ή�. 32�r�b�g�ŏ���.
template<class E, class N>
file_size64_t   FileT<E,N>::seek64(file_size64_t ofs, int seekMode) {
    assert(ofs <= (file_size64_t)0xFFFFFFFFU);
    return seek((std::size_t)ofs, seekMode);
}



// FILE�łł� 64�r�b�g�T�C�Y��tell()�͔�Ή�. 32�r�b�g�ŏ���.
template<class E, class N>
file_size64_t   FileT<E,N>::tell64() {
    return tell();
}



// size()�����s�������seek�ʒu�͕s��Ƃ���.
template<class E, class N>
std::size_t FileT<E,N>::size() {
    assert(fp_);
  #if defined(MISC_FILE_WIN_CC)
    return ::filelength(fileno(fp_));
  #else
    this->rewind();
    this->seek(0,2);
    std::size_t l = this->tell();
    this->rewind();
    return l;
  #endif
}



template<class E, class N>
file_size64_t   FileT<E,N>::size64() {
    assert(fp_);
  #if defined(MISC_FILE_LENGTH64)
    return MISC_FILE_LENGTH64(fileno(fp_));
  #else
    file_stat_t st;
    return this->stat(&st)==0 ? st.st_size : 0;
  #endif
}



/// ���Ԃ̎擾. �l�̓V�X�e���Ɉˑ�.
template<class E, class N>
bool    FileT<E,N>::getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    file_stat_t st;
    bool rc = this->stat(&st)==0;
    if (rc) {
        if (pLastWrt) *pLastWrt = st.st_mtime;
        if (pLastAcs) *pLastAcs = st.st_atime;
        if (pCreat  ) *pCreat   = st.st_ctime;
    }
    return rc;
}



#if defined MISC_FILE_FUTIME
/// ���Ԃ̐ݒ�. pCreat�͖�������̂Œ���!.
template<class E, class N>
bool    FileT<E,N>::setTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    pCreat;
    file_utimbuf_t  utimbuf;
    utimbuf.actime  = *pLastAcs;
    utimbuf.modtime = *pLastWrt;
    return this->utime(&utimbuf)==0;
}
#endif



template<class E, class N>
bool FileT<E,N>::open_e(const char* fname, const char *mode) {
    return E::checkOpen(this->open(fname, mode), fname);
}



template<class E, class N>
bool FileT<E,N>::open(const char* fname, const char *mode) {
    if (fp_ != 0)
        this->close();
    N::setFname(fname);
    // fname=NULL�̓G���[. errno�ɐݒ肷�邽��fopen���邪null���ƃn���O�����肦���̂�""��.
    if (fname == 0)
        fname = "";
    fp_ = std::fopen(fname, mode);
    return fp_ != 0;
}



template<class E, class N>
void    FileT<E,N>::rewind() {
    assert(fp_); std::rewind(fp_);
}



template<class E, class N>
bool    FileT<E,N>::eof()   {
  #if defined _MSC_VER || defined __BORLANDC__  // �}�N���䂦std�ɂȂ�
    return fp_ == 0 || feof(fp_) != 0;
  #else
    return fp_ == 0 || std::feof(fp_) != 0;
  #endif
}



template<class E, class N>
bool    FileT<E,N>::error() {
    assert(fp_);
  #if defined _MSC_VER || defined __BORLANDC__  // �}�N���䂦std�ɂȂ�
    return ferror(fp_) != 0;
  #else
    return std::ferror(fp_) != 0;
  #endif
}



template<class E, class N>
void    FileT<E,N>::clearerr() {
    if (fp_)
        std::clearerr(fp_);
}



template<class E, class N>
int  FileT<E,N>::getpos(std::fpos_t *pos) {
    return std::fgetpos(fp_, pos);
}



template<class E, class N>
int  FileT<E,N>::fsetpos(const std::fpos_t *pos) {
    return std::fsetpos(fp_, pos);
}



template<class E, class N>
void FileT<E,N>::setvbuf(void *buf, int iobf, std::size_t size) {
    std::setvbuf(fp_, (char*)buf, iobf, size);
}



template<class E, class N>
void FileT<E,N>::setBufferSize(std::size_t size) {
    std::setvbuf(fp_, 0, size?ioFbf:ioNbf, size);
}



template<class E, class N>
int FileT<E,N>::stat(file_stat_t* st) {
    assert(fp_ != 0 && st != 0);
    int hdl = fileno(fp_);
    return MISC_FILE_FSTAT(hdl, st);
}



#if defined MISC_FILE_FUTIME
template<class E, class N>
int FileT<E,N>::utime(file_utimbuf_t* utimbuf) {
    assert(fp_ != 0 && utimbuf != 0);
    int hdl = fileno(fp_);
    return MISC_FILE_FUTIME(hdl, utimbuf);
}
#endif




#endif  // MISC_FILE_KIND ===================================================





// ========================================================================
// �G���[���b�Z�[�W�֌W.
// ========================================================================


/// ���O�̃G���[��Ԃ�.
inline int file_lastError() {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return (unsigned)::GetLastError();
  #else
    return errno;
  #endif
}


/** ���O�̃G���[�̃��b�Z�[�W�������buf�ɓ���ĕԂ�.
 *  buf��NULL�Ȃ�std::strdup���Ă��̃A�h���X��Ԃ�.
 */
template<typename CHR_T>
char* file_lastErrorStr(CHR_T* buf, std::size_t bufSize=0) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    LPVOID lpMsgBuf;
    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ����̌���
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    if (buf == 0) {
        buf = MISC_FILE_STRDUP((const char*)(lpMsgBuf));
    } else {
        assert(bufSize > 1);
        unsigned l = std::strlen((const char*)lpMsgBuf) + 1;
        if (l > bufSize)
            l = bufSize;
        std::memcpy(buf, (const char*)lpMsgBuf, l);
        buf[l-1] = '\0';
    }
    ::LocalFree(lpMsgBuf);
    return buf;
  #else
    char* p = std::strerror(errno);
    if (p == 0)
        return NULL;
    if (buf == 0) {
        buf = MISC_FILE_STRDUP(p);
    } else {
        assert(bufSize > 1);
        unsigned l = std::strlen(p) + 1;
        if (l > bufSize)
            l = bufSize;
        std::memcpy(buf, p, l);
        buf[l-1] = '\0';
    }
    return buf;
  #endif
}




// ========================================================================
// File�̃����o�[�֐� ???_e �ł̃G���[�`�F�b�N�p�N���X.
// ========================================================================


//----------------------------------------------------
// �G���[���������烁�b�Z�[�W���o����exit()�����(�R�}���h���C���p).


template<class DMY=void>
class File_ChkErrMsgExit {
protected:
    bool        checkOpen(bool e, const char* fname);
    std::size_t checkWrite(std::size_t l, std::size_t size, const char* fname);
    std::size_t checkRead(std::size_t l, std::size_t size, const char* fname);

private:
    void        errorMsg(const char* fname, const char* msg);
    void        errorPuts(const char *s);
};


template<class T>
bool File_ChkErrMsgExit<T>::checkOpen(bool e, const char* fname) {
    if (e == false) {
        errorMsg(fname, "File open error.");
    }
    return e;   //x is_open();
}


template<class T>
std::size_t File_ChkErrMsgExit<T>::checkWrite(std::size_t l, std::size_t size, const char* fname) {
    if (l < size)
        errorMsg(fname, "File write error.");
    return l;
}



template<class T>
std::size_t File_ChkErrMsgExit<T>::checkRead(std::size_t l, std::size_t size, const char* fname) {
    if (l < size)
        errorMsg(fname, "File read error.");
    return l;
}



template<class T>
void File_ChkErrMsgExit<T>::errorMsg(const char* fname, const char* msg) {
    char buf[0x1000];
    if (fname[0]) {
        errorPuts( fname );
        errorPuts(" : ");
    }
    const char* p = file_lastErrorStr(buf, sizeof buf);
    if (p == NULL || strlen(p) < 5)
        p = msg;
    errorPuts(p);
    errorPuts("\n");
    exit(1);
}


template<class T>
void File_ChkErrMsgExit<T>::errorPuts(const char *s) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    DWORD sz;
    ::WriteConsole(::GetStdHandle(STD_OUTPUT_HANDLE), s, std::strlen(s), &sz, NULL);
  #elif MISC_FILE_KIND == 1 // io.h,unistd.h��.
    ::write(2, s, std::strlen(s) );
  #else
    std::fputs(s, stderr);
  #endif
}




//----------------------------------------------------
// open_e,read_e,write_e�ŁA�G���[�����������O�𓊂����.

#if defined MISC_FILE_USE_EH

class File_OpenError : public std::runtime_error {
public:
    File_OpenError(const std::string& msg) : std::runtime_error(msg) {}
};

class File_ReadError : public std::runtime_error {
public:
    File_ReadError(const std::string& msg) : std::runtime_error(msg) {}
};

class File_WriteError : public std::runtime_error {
public:
    File_WriteError(const std::string& msg) : std::runtime_error(msg) {}
};


class File_ChkErrThrow {
protected:
    bool checkOpen(bool e, const char* fname) {
        if (e == false) {
            throw File_WriteError(mkMsg(fname, "file open error."));
        }
        return e;
    }

    std::size_t checkRead(std::size_t l, std::size_t size, const char* fname) {
        if (l < size) {
            throw File_WriteError(mkMsg(fname, "file read error."));
        }
        return l;
    }

    std::size_t checkWrite(std::size_t l, std::size_t size, const char* fname) {
        if (l < size) {
            throw File_WriteError(mkMsg(fname, "file write error."));
        }
        return l;
    }

private:
    std::string mkMsg(const char* fname, const char* msg) {
        std::string str(fname);
        if (fname[0])
            str += " : ";
        char buf[0x1000];
        const char* p = file_lastErrorStr(buf, sizeof buf);
        if (p == NULL || strlen(p) < 5)
            p = msg;
        str += p;
        return str;
    }
};

#endif




// ========================================================================
// �t�@�C������ێ����邩���Ȃ����̑I��p�N���X
// ========================================================================


/// �t�@�C������ێ����Ȃ��ꍇ�p.
class File_NouseFname {
public:
    const char* fname() const { return ""; }

protected:
    bool setFname(const char* /*fname*/) { return true;}
    void releaseFname() {;}
};



/// �t�@�C������ێ�����ꍇ.
class File_UseFname {
public:
    File_UseFname() : fname_(0) {;}
    ~File_UseFname() { releaseFname();}

    const char* fname() const {
        return fname_ ? fname_ : "";
    }

protected:
    bool setFname(const char* fname) {
        releaseFname();
        fname_ = MISC_FILE_STRDUP(fname);
        return fname_ != 0;
    }

    void releaseFname() {
        if (fname_) {
            std::free(fname_);
            fname_ = 0;
        }
    }

private:
    char*   fname_;
};




// ========================================================================
// ���ۂ�FileT���f�t�H���g�Ŏg�����̖���typedef�Őݒ�.
// ========================================================================


typedef FileT<>     File;


#if defined MISC_FILE_USE_EH
typedef FileT<File_ChkErrThrow>     FileEH;
#endif




// ========================================================================
// �f�B���N�g���E�G���g���֌W.
// ========================================================================


/// �t�@�C�����̕ύX.�p�X�ړ�.  �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_rename(const char* oldName, const char* newName) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::MoveFile( oldName, newName ) ? 0 : -1;
  #else
    return ::rename( oldName, newName );
  #endif
}


/// �t�@�C���폜.     �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_remove(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::DeleteFile( fname ) ? 0 : -1;
  #else
    return ::remove(fname);
  #endif
}


/// �f�B���N�g���쐬.   �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_mkdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::CreateDirectory( fname, NULL ) ? 0 : -1;
  #elif defined(MISC_FILE_WIN_CC)
    return ::_mkdir(fname);
  #else
    return ::mkdir(fname, 0777);
  #endif
}


/// �f�B���N�g���폜.     �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_rmdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::RemoveDirectory( fname ) ? 0 : -1;
  #elif defined(MISC_FILE_WIN_CC)
    return ::_rmdir(fname);
  #else
    return ::rmdir(fname);
  #endif
}


/// �J�����g�f�B���N�g���̐ݒ�.   �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_chdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::SetCurrentDirectory(fname) ? 0 : -1;
  #else
    return ::chdir(fname);
  #endif
}


/// �J�����g�f�B���N�g���̎擾.   �Ԓl nameBuf��Ԃ�. ���s������NULL��Ԃ�.
inline char* file_getcwd(char* nameBuf, std::size_t nameBufSize) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::GetCurrentDirectory(nameBufSize, nameBuf) ? nameBuf : 0;
  #else
    return ::getcwd(nameBuf, nameBufSize);
  #endif
}


template<class STRING>
STRING& file_getcwd(STRING& name) {
    char buf[FILE_MAX_PATH+4];
    char* p = file_getcwd(buf, sizeof buf);
    name.assign( p ? p : "" );
    return name;
}


#if MISC_FILE_KIND == 2 // Win-API�A�v���̎�.

/// �V�X�e���f�B���N�g���̎擾.   �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_getSystemDir(char* nameBuf, std::size_t nameBufSize) {
    return ::GetSystemDirectory(nameBuf, nameBufSize) ? 0 : -1;
}

template<class STRING>
STRING& file_getSystemDir(STRING& name) {
    char buf[FILE_MAX_PATH+4];
    int rc = file_getSystemDir(buf, sizeof buf);
    name.assign( rc == 0 ? buf : "" );
    return name;
}


/// windows�f�B���N�g���̎擾.    �Ԓl0:ok  ��:�G���[. �ڍׂ�file_lastError()����擾.
inline int file_getWindowsDir(char* nameBuf, std::size_t nameBufSize) {
    return ::GetWindowsDirectory(nameBuf, nameBufSize) ? 0 : -1;
}

template<class STRING>
STRING& file_getWindowsDir(STRING& name) {
    char buf[FILE_MAX_PATH+4];
    int rc = file_getWindowsDir(buf, sizeof buf);
    name.assign( rc == 0 ? buf : "" );
    return name;
}
#endif



/** �t���p�X����.
 *  fullName=NULL�Ȃ�malloc�����A�h���X��Ԃ�. �G���[�������NULL��Ԃ�.
 */
template<typename CHR_T>
inline char* file_fullpath(CHR_T* fullName, const CHR_T* path, std::size_t fullNameSize)
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
  #else // gcc(linux)
    unsigned sz  = 0;
    char*    buf = realpath(path, NULL);
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
STRING& file_fullpath(STRING& fullName, const char* path)
{
    char    buf[FILE_MAX_PATH+4];
    char*   p = file_fullpath(buf, path, sizeof buf);
    fullName.assign( p ? p : "" );
    return fullName;
}




// ========================================================================
// �t�@�C�����֌W.
// ========================================================================

/// �t�@�C���̑�����ύX.
inline int file_chmod(const char* fname, unsigned mod) {
  #if defined(MISC_FILE_WIN_CC)
    return ::_chmod(fname, mod);
  #else
    return ::chmod(fname, mod);
  #endif
}



/// �t�@�C���T�C�Y��Ԃ�.
inline std::size_t file_size(const char* fname) {
    File file;
    if (file.open(fname, File::R))
        return file.size();
    return 0;
}



/// uint64_t�^�Ńt�@�C���T�C�Y��Ԃ�.
inline file_size64_t  file_size64(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    File file;
    return (file.open(fname, File::R)) ? file.size64() : 0;
  #else
    file_stat_t st;
    return file_stat(fname, &st)==0 ? st.st_size : 0;
  #endif
}



// ========================================================================
// �t�@�C�����Ԋ֌W.
// ========================================================================

/** �t�@�C���̓��t���擾.
 *  win-api�ł�64�r�b�g�l�Ȃ̂� file_stat()��肱����𗘗p�̂���.
 */
inline bool file_getTime(const char* fname, file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0) {
  #if MISC_FILE_KIND == 2 || defined MISC_FILE_FUTIME
    // win�� FindFirst���g���Ď����\�����A�ʓ|�Ȃ��file�ɔC��.
    File    file;
    return file.open(fname, File::R) && file.getTime(pLastWrt, pLastAcs, pCreat);
  #else
    file_stat_t st;
    bool rc = file_stat(fname, &st)==0;
    if (rc) {
        if (pLastWrt) *pLastWrt = st.st_mtime;
        if (pLastAcs) *pLastAcs = st.st_atime;
        if (pCreat  ) *pCreat   = st.st_ctime;
    }
    return rc;
  #endif
}


/** �t�@�C���̓��t��ݒ�.
 *  win-api�ł�64�r�b�g�l�Ȃ̂� file_utime()��肱����𗘗p�̂���.
 */
inline bool file_setTime(const char* fname, file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0) {
  #if MISC_FILE_KIND == 2 || defined MISC_FILE_FUTIME
    File    file;
    return file.open(fname, File::RP) && file.setTime(pLastWrt, pLastAcs, pCreat);
  #else
    pCreat;
    file_utimbuf_t  utimbuf;
    utimbuf.actime  = *pLastAcs;
    utimbuf.modtime = *pLastWrt;
    return file_utime(fname, &utimbuf)==0;
  #endif
}



/// �t�@�C���̎��Ԃ����[�J�����Ԃ̔N���������b�ɕϊ�.
struct File_LocalTime {
    File_LocalTime() { std::memset(this,0,sizeof(*this)); }
    File_LocalTime(unsigned y, unsigned mo, unsigned d,
                    unsigned h=0, unsigned mi=0, unsigned s=0, unsigned ms=0)
                    {set(y,mo,d,h,mi,s,ms);}

    explicit File_LocalTime(file_size64_t time) {setTime(time);}

    void set(unsigned y, unsigned mo, unsigned d, unsigned h=0, unsigned mi=0, unsigned s=0, unsigned ms=0) {
        this->year          = (unsigned short)y,
        this->month         = (unsigned short)mo,
        this->dayOfWeek     = 0;        // �K��.
        this->day           = (unsigned short)d,
        this->hour          = (unsigned short)h,
        this->minute        = (unsigned short)mi,
        this->second        = (unsigned short)s;
        this->milliSecond   = (unsigned short)ms;
    }

    // ���[�J�����Ԃ̂��̂�ϊ����āA�t�@�C���̎��ԏ��ɂ��Ď擾.
    file_size64_t getTime() const { return getTime(0); }

    // �t�@�C���̎��Ԃ��A���[�J�����Ԃɕϊ����Đݒ�.
    bool setTime(file_size64_t tim) { return setTime(tim, 0); }

    /// �t�@�C���̎��Ԃ��A���[�J�����Ԃɕϊ������ɐݒ�.
    /// getGmTime() �͗p�ӂ��Ȃ��̂Œ��ӁI
    bool setGmTime(file_size64_t tim) { return setGmTime(tim, 0); }


private:    // �w�b�_�ɒu���ׂɃe���v���[�g��.
    template<typename DMY_T> file_size64_t  getTime(DMY_T) const;
    template<typename DMY_T> bool           setTime(file_size64_t tim, DMY_T);
    template<typename DMY_T> bool           setGmTime(file_size64_t tim, DMY_T);

public: // �����o�[�ϐ��͌��J.  // �蔲����win�ɍ��킹��.
    unsigned short      year;
    unsigned short      month;
    unsigned short      dayOfWeek;
    unsigned short      day;
    unsigned short      hour;
    unsigned short      minute;
    unsigned short      second;
    unsigned short      milliSecond;
};



/// ���[�J�����Ԃ̂��̂�ϊ����āA�t�@�C���̎��ԏ��ɂ��Ď擾.
template<typename DMY_T>
file_size64_t File_LocalTime::getTime(DMY_T) const {
  #if MISC_FILE_KIND == 2 //|| defined(MISC_FILE_WIN_CC)
    file_size64_t  localTim;
    file_size64_t  tim;
    return (   SystemTimeToFileTime((const SYSTEMTIME*)this, (FILETIME*)&localTim)
            && LocalFileTimeToFileTime((const FILETIME*)&localTim, (FILETIME*)&tim)
           ) ? tim : 0;
  #else
    /*struct*/ std::tm tb;
    tb.tm_year          = this->year;
    tb.tm_mon           = this->month - (this->month > 0);  // -1 �ł悢���A������month=0���������̂��߂ɂ�����ƃK�[�h.
    tb.tm_wday          = this->dayOfWeek;  // �T�̒��̓���.
    tb.tm_mday          = this->day;
    tb.tm_hour          = this->hour;
    tb.tm_min           = this->minute;
    tb.tm_sec           = this->second;
    tb.tm_yday          = 0;                // �N�Ԓ��̓���
    tb.tm_isdst         = 0;                // �T�}�[�^�C��
   #if defined MISC_FILE_MKTIME64
    file_size64_t   t   = MISC_FILE_MKTIME64(&tb);
   #else
    file_size64_t   t   = std::mktime(&tb);
   #endif
    return (t == file_size64_t(-1)) ? 0 : t;
  #endif
}



// �t�@�C���̎��Ԃ��A���[�J�����Ԃɕϊ����Đݒ�.
template<typename DMY_T>
bool File_LocalTime::setTime(file_size64_t tim, DMY_T) {
  #if MISC_FILE_KIND == 2 //|| defined(MISC_FILE_WIN_CC)
    file_size64_t  localTim;
    return ::FileTimeToLocalFileTime((const FILETIME*)&tim, (FILETIME*)&localTim)
        && ::FileTimeToSystemTime((const FILETIME*)&localTim, (SYSTEMTIME*)this);
  #else
   #if defined MISC_FILE_LOCALTIME64
    /*struct*/ std::tm  *t      = MISC_FILE_LOCALTIME64(&tim);
   #else
    std::time_t tim0    = (time_t)tim;
    /*struct*/ std::tm  *t      = std::localtime(&tim0);
   #endif
    if (t == 0)
        return false;
    this->year          = (unsigned short)t->tm_year,
    this->month         = (unsigned short)t->tm_mon+1,
    this->dayOfWeek     = (unsigned short)t->tm_wday,
    this->day           = (unsigned short)t->tm_mday,
    this->hour          = (unsigned short)t->tm_hour,
    this->minute        = (unsigned short)t->tm_min,
    this->second        = (unsigned short)t->tm_sec;
    this->milliSecond   = (unsigned short)0;
    return true;
  #endif
}



/// �t�@�C���̎��Ԃ��A���[�J�����Ԃɕϊ������ɐݒ�.
/// getGmTime() �͗p�ӂ��Ȃ��̂Œ��ӁI
template<typename DMY_T>
bool File_LocalTime::setGmTime(file_size64_t tim, DMY_T) {
  #if MISC_FILE_KIND == 2 //|| defined(MISC_FILE_WIN_CC)
    return FileTimeToSystemTime((const FILETIME*)&tim, (SYSTEMTIME*)this) != 0;
  #else
   #if defined MISC_FILE_GMTIME64
    /*struct*/ std::tm  *t      = MISC_FILE_GMTIME64(&tim);
   #else
    std::time_t tim0    = (time_t)tim;
    /*struct*/ std::tm  *t      = std::gmtime(&tim0);
   #endif
    if (t == 0)
        return false;
    this->year          = (unsigned short)t->tm_year,
    this->month         = (unsigned short)t->tm_mon+1,
    this->dayOfWeek     = (unsigned short)t->tm_wday,
    this->day           = (unsigned short)t->tm_mday,
    this->hour          = (unsigned short)t->tm_hour,
    this->minute        = (unsigned short)t->tm_min,
    this->second        = (unsigned short)t->tm_sec;
    this->milliSecond   = (unsigned short)0;
    return true;
  #endif
}





// ========================================================================
// ���[�h�E�Z�[�u.
// ========================================================================


/** �t�@�C���E�Z�[�u.
 */
template<typename CHR_T>
unsigned file_save(const CHR_T* name, const void* buf, unsigned byteSize)
{
    File    file;
    if (file.open(name, File::W) == false)
        return false;
    if (byteSize == 0)
        return true;
    return file.write( buf, byteSize );
}


/** �t�@�C���E�Z�[�u.
 */
template<typename VECTOR>
unsigned file_save(const char* name, const VECTOR& vec)
{
    File    file;
    if (file.open(name, File::W) == false)
        return false;
    std::size_t l = vec.size() * sizeof(vec[0]);
    if (l == 0)
        return true;
    return file.write( &vec[0], l );
}



/** �t�@�C���E���[�h
 */
template<class VECTOR>
unsigned file_load(const char* name, VECTOR& buf)
{
    File file;
    if (file.open(name, File::R) == false)
        return false;
    std::size_t sz = file.size();
    buf.resize( (sz+sizeof(buf[0])-1) / sizeof(buf[0]) );
    return file.read( &buf[0], sz) > 0;
}



/** �t�@�C���E���[�h.
 */
template<typename CHR_T>
unsigned file_load(const CHR_T* name, void* buf, unsigned byteSize)
{
    File    file;
    if (file.open(name, File::R) == false)
        return false;
    if (byteSize == 0)
        return true;
    return file.read( buf, byteSize );
}



/** �t�@�C���E���[�h.
 */
template<typename CHR_T>
void* file_loadAlloc(const CHR_T* name, std::size_t* pSize=0)
{
    if (pSize)
        *pSize = 0;
    File    file;
    if (file.open(name, File::R) == false)
        return false;
    std::size_t l = file.size();
    if (l == 0)
        l = 1;
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    void* m = ::LocalAlloc(LMEM_FIXED, l);
  #else
    void* m = std::malloc( l );
  #endif
    std::size_t sz = 0;
    if (m) {
        sz = file.read(m, l);
        if (sz < l) {
            std::free(m);
            m = 0;
        }
    }
    if (pSize)
        *pSize = sz;
    return m;
}



/** file_loadAlloc�Ŏ擾�����|�C���^��free����.
 */
inline void file_loadFree(void* p) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    ::LocalFree(p);
  #else
    std::free(p);
  #endif
}



/** �t�@�C���̃R�s�[.
 */
template<typename CHR_T>
bool file_copy(const CHR_T* srcName, const CHR_T* dstName) {
  #if MISC_FILE_KIND == 2   // Win-API�A�v���̎�.
    return ::CopyFile(srcName, dstName, false) != 0;
  #else
    File    src;
    File    dst;
    if (src.open(srcName, File::R) == false)
        return false;
    if (dst.open(dstName, File::W) == false)
        return false;

    file_size64_t sz = src.size64();
    if (sz > 0) {
        enum { BUF_SIZE = 4*0x100000 };
        //x enum { BUF_SIZE = 0x1000 };
        struct CharBuf {
            CharBuf(std::size_t l)  { buffer = new char[l]; }
            ~CharBuf() { delete[] buffer; }
            char* buffer;
        };
        CharBuf charBuf(BUF_SIZE);
        char*   b = charBuf.buffer;
        if (b == NULL)
            return false;
        for (file_size64_t byt = sz; byt > 0;) {
            std::size_t l = (byt < BUF_SIZE) ? std::size_t(byt) : BUF_SIZE;
            if (src.read(b, l) < l)
                return  false;
            if (dst.write(b, l) < l)
                return false;
            byt -= l;
        }
    }
    return true;
  #endif
}



}   // namespce MISC
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */


#endif  // FILE_H
