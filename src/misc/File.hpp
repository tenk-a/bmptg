/**
  @file     File.h
  *brief    apiのファイル関係をラップしたクラスや関数.
  *author   Masashi Kitamura.
  *date     2004,2007
  @note

  - C標準のFILEベース、io.h/unistd.hベース、win-APIベース、３つの実装がある.
  - size64()やseek64()等64ビット値版は win-API版以外は 現状32ビット値版.
  - ヘッダファイルのみにするために、無理にtemplate化して実装しているものがある.
  - 基本的に vc でのチェックのみ. たまにwinの他のコンパイラでするが,
    mingwはよいけれど、bcc,dmc,watcomはいまいち安定してない模様.
    win以外は、気が向けば.
 */
/*
    // 定義されているクラス＆関数のプロトタイプ. (実定義はinlineで見づらいので抜粋)
    class File {
    public:
        enum RW_T { R,W,RP,WP,} // "rb", "wb", "rb+", "wb+",に対応.この値は実際には0,1,2,3でなく別の値.
        enum      { seek_set, seek_cur, seek_end };
        File();
        File(const char* fname, RW_T rw);
        ~File();
        bool        is_open() const;
        bool        open  (const char* fname, RW_T rw);
        bool        open_e(const char* fname, RW_T rw);     // エラーがあったらメッセージ終了.
        void        close();
        size_t      read(void *buf, size_t size);
        size_t      read_e(void *buf, size_t size);         // エラーがあったらメッセージ終了.
        size_t      write(const void *buf, size_t size);
        size_t      write_e(const void *buf, size_t size);  // エラーがあったらメッセージ終了.
        bool        flush();

        size_t      seek(size_t ofs, int seekMode);
        uint64_t    seek64(uint64_t ofs, int seekMode);
        size_t      tell();
        uint64_t    tell64();
        size_t      size();
        uint64_t    size64();

        bool    getTime(uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
        bool    setTime(uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0); // win系のみ.

        const char* fname() const;
    };

    size_t   file_size(const char* fname);
    uint64_t file_size64(const char* fname);
    bool     file_getTime(const char* fname, uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
    bool     file_setTime(const char* fname, uint64_t* pLastWrt, uint64_t* pLastAcs=0, uint64_t* pCreat=0);
    int      file_stat(const char* fname, file_stat_t* st);             // 32ビット版のことが多い.
    int      file_utime(const char* fname, file_utimbuf_t* utimbuf);    // 32ビット版のことが多い.

    bool     file_moveName(const char* oldName, const char* newName);
    bool     file_delete(const char* fname);
    bool     file_createDir(const char* fname);
    bool     file_deleteDir(const char* fname);
    bool     file_setCurrentDir(const char* nameBuf);
    bool     file_getCurrentDir(char* nameBuf, size_t nameBufSize);
    bool     file_getSystemDir(char* nameBuf, size_t nameBufSize);  // win計のみ.
    bool     file_getWindowsDir(char* nameBuf, size_t nameBufSize); // win計のみ.
    struct File_LocalTime;  // 時間変換用.

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
// 基本的なinclude.
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


// open_e, read_e, write_eで例外を投げるFileEHを有効にしたい場合.
// #define MISC_FILE_USE_EH
#if defined MISC_FILE_USE_EH
// #include <exception>
#include <stdexcept>
#endif



// ------------------------------------------------------------------------
// コンパイラ別の辻褄あわせ用.
// file_size64_t は vc がuint64_tを定義してくれてたら不要だったのだけど...

#if defined(_MSC_VER) || (defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601)
 #ifndef MISC_FILE_KIND
  #if defined _WINDOWS
   #define MISC_FILE_KIND       2   // Win-APIならWinAPI版.
  #else
   #define MISC_FILE_KIND       0   // コンソールなら FILE 版.
  #endif
 #endif
 #define MISC_FILE_WIN_CC           // Win用のコンパイラの時(VC互換が高め)
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


#else   // (64bit版のstat,utimeがない場合) -----------
 #ifndef MISC_FILE_KIND
  #if defined _WINDOWS || defined(__DMC__) || defined(__BORLANDC__) || defined(__WATCOM_C__) || defined(__MINGW32__)
   #define MISC_FILE_KIND       2   // WinAPI版.
  #elif defined(__CYGWIN__)
   #define MISC_FILE_KIND       1   // cygwinならunixもどきとして.
  #else
   #define MISC_FILE_KIND       0   // コンソールなら FILE 版.
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
    // 色々不都合あるようなので、あきらめる.
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
// 実装の種類別にinclude

#if   MISC_FILE_KIND == 2   // Win-API を使う時.

 #include <windows.h>
 #include <io.h>

#else   // io.h,unistd.h版  stdio.h 版.

 #include <cstdio>
 #include <stdio.h>         //  コンパイラによってstdに入ったり入らなかったりなので、あきらめ...
 #include <cerrno>
 #include <errno.h>         // errnoは変数でなくマクロの場合もあり.
 #include <ctime>

 #ifdef MISC_FILE_WIN_CC    //  dos,win系コンパイラのとき.
  #include <io.h>
  #include <fcntl.h>
  #include <direct.h>
 #else                      //  linux(unix系)の時.
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

class File_NouseFname;      // ファイル名を保持しない版.
class File_UseFname;        // ファイル名を保持する版.

template<class DMY>
class File_ChkErrMsgExit;   // open_e, read_e, write_eでエラーがあった時,メッセージを表示して終了.




/* ======================================================================== */
/*                                                                          */
/* (バイナリ)ファイルアクセスのためのファイルハンドル管理クラス             */
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
    // オープンモード. fopenの rb,rb+,wb,wb+相当のみ.
    enum RW_T { R=0, W=1, RP=2, WP=3 };

    FileT();
    FileT(const char* fname, RW_T rw);
    ~FileT() { this->close(); }

    /// オープンしているか否か.
    bool    is_open() const;

    bool open  (const char* fname, RW_T rw);
    bool open_e(const char* fname, RW_T rw) {
        return ERR::checkOpen(this->open(fname, rw), fname);
    }

    void close();

    /// 読み込み.
    std::size_t read(void *buf, std::size_t size);
    std::size_t read_e(void *buf, std::size_t size) {
        return ERR::checkRead( this->read(buf, size), size, NAME::fname() );
    }

    /// 書き込み.
    std::size_t write(const void *buf, std::size_t size);
    std::size_t write_e(const void *buf, std::size_t size) {
        return ERR::checkWrite( this->write(buf, size), size, NAME::fname() );
    }

    /// バッファリング内容をフラッシュ.
    bool        flush();

    enum { seek_set, seek_cur, seek_end };

    /// 指定の位置へファイルポインタを移動.
    std::size_t     seek(std::size_t ofs, int seekMode);
    file_size64_t   seek64(file_size64_t ofs, int seekMode);

    /// 現在位置を返す.
    std::size_t     tell();
    file_size64_t   tell64();

    /// ファイルサイズのバイト数を返す. 実行した後はseek位置は不定とする.
    std::size_t size();
    file_size64_t size64();

    /// 時間の取得. ※ Win-API とは引数が逆順なので注意.
    bool    getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0);

  #if MISC_FILE_KIND == 2 || defined MISC_FILE_FUTIME
    /// 時間の設定. ※ Win-API とは引数が逆順なので注意.
    /// win-api版以外は、pCreatは無視するので注意.
    /// コンパイラがfutimeを未サポートの場合は全くダミーなので注意.
    bool    setTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0);
  #endif


public:
 #if MISC_FILE_KIND == 2    // Win-API版の実装のみ.
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

 #elif MISC_FILE_KIND == 1      // io.h, unistd.h 版のみ.
    int handle() const { return handle_; }

    /** オープン.
     *  flags   O_RDONLY, O_WRONLY, O_RDWR,   O_CREAT,O_EXCL, O_TRUNC, O_APPEND,
     *          O_DIRECT, O_DIRECTORY, O_ASYNC, O_SYNC, O_NOATIME, O_NOFOLLOW, O_NONBLOCK, O_NDELAY...
     *  mode    作成時のアクセス許可(パーミッション) (例: 0766 とか)
     */
    bool open(const char* fname, int flags, int mode);
    bool open_e(const char* fname, unsigned flags, int mode);

    int stat(file_stat_t* st);

   #if defined MISC_FILE_FUTIME
    int utime(file_utimbuf_t* utimbuf);
   #endif

 #else                      // FILE 系の実装のみ.
    std::FILE*       fp()       { return fp_; }
    const std::FILE* fp() const { return fp_; }

    // FileT変数にstdin,stdout,stderr を設定するために用意.
    // デストラクタでcloseするので必要なら再度NULLを設定する必要がある.
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
// Windows-API 版の実装.
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



// size()を実行した後はseek位置は不定とする.
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



// 時間の取得.  ※ Win-API とは引数が逆順なので注意.
template<class E, class N>
bool    FileT<E,N>::getTime(file_size64_t* pLastWrt, file_size64_t* pLastAcs, file_size64_t* pCreat) {
    assert(handle_ != INVALID_HANDLE_VALUE);
    return ::GetFileTime(handle_, (FILETIME*)pCreat, (FILETIME*)pLastAcs, (FILETIME*)pLastWrt) != 0;
}



// 時間の設定.  ※ Win-API とは引数が逆順なので注意.
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
        fname = "";     // NULLだとハングする環境もあるがopenをしないとerrnoが設定されないので代用.
    handle_ = ::CreateFileA((PCSTR)fname, access, share, psa, createDisposition, flagsAndAttr, hTemplate);
    return handle_ != INVALID_HANDLE_VALUE;
}




#elif MISC_FILE_KIND == 1       // linuxなgccを想定.
// -------------------------------------------------------------------------
// unistd.h(io.h) ベース. gcc(linux環境)向.
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
    // ダミー.
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



// size()を実行した後はseek位置は不定とする.
template<class E, class N>
std::size_t   FileT<E,N>::size() {
    file_stat_t st; return this->stat(&st)==0 ? (std::size_t)st.st_size : 0;
}



template<class E, class N>
file_size64_t FileT<E,N>::size64() {
    file_stat_t st; return this->stat(&st)==0 ? st.st_size : 0;
}



/// 時間の取得. 値はシステムに依存.
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
/// 時間の設定. pCreatは無視するので注意!.
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


/** オープン.
 *  flags   O_RDONLY, O_WRONLY, O_RDWR,   O_CREAT,O_EXCL, O_TRUNC, O_APPEND,
 *          O_DIRECT, O_DIRECTORY, O_ASYNC, O_SYNC, O_NOATIME, O_NOFOLLOW, O_NONBLOCK, O_NDELAY...
 *  mode    作成時のアクセス許可(パーミッション) (例: 0766 とか)
 */
template<class E, class N>
bool FileT<E,N>::open(const char* fname, int flags, int mode) {
    if (handle_ != -1)
        this->close();
    N::setFname(fname);
    if (fname == 0)
        fname = "";     // FILEと違いerrnoに設定されるかは不明だが、一応openしとくために""化.
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






#else   // stdio.h 版.
// -------------------------------------------------------------------------
// 標準 Cライブラリ(FILE)版.
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



// FILE版では 64ビットサイズのシークは非対応. 32ビットで処理.
template<class E, class N>
file_size64_t   FileT<E,N>::seek64(file_size64_t ofs, int seekMode) {
    assert(ofs <= (file_size64_t)0xFFFFFFFFU);
    return seek((std::size_t)ofs, seekMode);
}



// FILE版では 64ビットサイズのtell()は非対応. 32ビットで処理.
template<class E, class N>
file_size64_t   FileT<E,N>::tell64() {
    return tell();
}



// size()を実行した後はseek位置は不定とする.
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



/// 時間の取得. 値はシステムに依存.
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
/// 時間の設定. pCreatは無視するので注意!.
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
    // fname=NULLはエラー. errnoに設定するためfopenするがnullだとハングもありえすので""化.
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
  #if defined _MSC_VER || defined __BORLANDC__  // マクロゆえstdにない.
    return fp_ == 0 || feof(fp_) != 0;
  #else
    return fp_ == 0 || std::feof(fp_) != 0;
  #endif
}



template<class E, class N>
bool    FileT<E,N>::error() {
    assert(fp_);
  #if defined _MSC_VER || defined __BORLANDC__  // マクロゆえstdにない.
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
// エラーメッセージ関係.
// ========================================================================


/// 直前のエラーを返す.
inline int file_lastError() {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return (unsigned)::GetLastError();
  #else
    return errno;
  #endif
}


/** 直前のエラーのメッセージ文字列をbufに入れて返す.
 *  bufがNULLならstd::strdupしてそのアドレスを返す.
 */
template<typename CHR_T>
char* file_lastErrorStr(CHR_T* buf, std::size_t bufSize=0) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    LPVOID lpMsgBuf;
    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 既定の言語.
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
// Fileのメンバー関数 ???_e でのエラーチェック用クラス.
// ========================================================================


//----------------------------------------------------
// エラーがあったらメッセージを出してexit()する版(コマンドライン用).


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
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    DWORD sz;
    ::WriteConsole(::GetStdHandle(STD_OUTPUT_HANDLE), s, std::strlen(s), &sz, NULL);
  #elif MISC_FILE_KIND == 1 // io.h,unistd.h版.
    ::write(2, s, std::strlen(s) );
  #else
    std::fputs(s, stderr);
  #endif
}




//----------------------------------------------------
// open_e,read_e,write_eで、エラーがあったら例外を投げる版.

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
// ファイル名を保持するかしないかの選択用クラス.
// ========================================================================


/// ファイル名を保持しない場合用.
class File_NouseFname {
public:
    const char* fname() const { return ""; }

protected:
    bool setFname(const char* /*fname*/) { return true;}
    void releaseFname() {;}
};



/// ファイル名を保持する場合.
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
// 実際にFileTをデフォルトで使う時の名をtypedefで設定.
// ========================================================================


typedef FileT<>     File;


#if defined MISC_FILE_USE_EH
typedef FileT<File_ChkErrThrow>     FileEH;
#endif




// ========================================================================
// ディレクトリ・エントリ関係.
// ========================================================================


/// ファイル名の変更.パス移動.  返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
inline int file_rename(const char* oldName, const char* newName) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return ::MoveFile( oldName, newName ) ? 0 : -1;
  #else
    return ::rename( oldName, newName );
  #endif
}


/// ファイル削除.     返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
inline int file_remove(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return ::DeleteFile( fname ) ? 0 : -1;
  #else
    return ::remove(fname);
  #endif
}


/// ディレクトリ作成.   返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
inline int file_mkdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return ::CreateDirectory( fname, NULL ) ? 0 : -1;
  #elif defined(MISC_FILE_WIN_CC)
    return ::_mkdir(fname);
  #else
    return ::mkdir(fname, 0777);
  #endif
}


/// ディレクトリ削除.     返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
inline int file_rmdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return ::RemoveDirectory( fname ) ? 0 : -1;
  #elif defined(MISC_FILE_WIN_CC)
    return ::_rmdir(fname);
  #else
    return ::rmdir(fname);
  #endif
}


/// カレントディレクトリの設定.   返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
inline int file_chdir(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    return ::SetCurrentDirectory(fname) ? 0 : -1;
  #else
    return ::chdir(fname);
  #endif
}


/// カレントディレクトリの取得.   返値 nameBufを返す. 失敗したらNULLを返す.
inline char* file_getcwd(char* nameBuf, std::size_t nameBufSize) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
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


#if MISC_FILE_KIND == 2 // Win-APIアプリの時.

/// システムディレクトリの取得.   返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
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


/// windowsディレクトリの取得.    返値0:ok  負:エラー. 詳細はfile_lastError()から取得.
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



/** フルパス生成.
 *  fullName=NULLならmallocしたアドレスを返す. エラーがあればNULLを返す.
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
// ファイル情報関係.
// ========================================================================

/// ファイルの属性を変更.
inline int file_chmod(const char* fname, unsigned mod) {
  #if defined(MISC_FILE_WIN_CC)
    return ::_chmod(fname, mod);
  #else
    return ::chmod(fname, mod);
  #endif
}



/// ファイルサイズを返す.
inline std::size_t file_size(const char* fname) {
    File file;
    if (file.open(fname, File::R))
        return file.size();
    return 0;
}



/// uint64_t型でファイルサイズを返す.
inline file_size64_t  file_size64(const char* fname) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    File file;
    return (file.open(fname, File::R)) ? file.size64() : 0;
  #else
    file_stat_t st;
    return file_stat(fname, &st)==0 ? st.st_size : 0;
  #endif
}



// ========================================================================
// ファイル時間関係.
// ========================================================================

/** ファイルの日付を取得.
 *  win-api版は64ビット値なので file_stat()よりこちらを利用のこと.
 */
inline bool file_getTime(const char* fname, file_size64_t* pLastWrt, file_size64_t* pLastAcs=0, file_size64_t* pCreat=0) {
  #if MISC_FILE_KIND == 2 || defined MISC_FILE_FUTIME
    // winは FindFirstを使って実装可能だが、面倒なんでfileに任す.
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


/** ファイルの日付を設定.
 *  win-api版は64ビット値なので file_utime()よりこちらを利用のこと.
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



/// ファイルの時間をローカル時間の年月日字分秒に変換.
struct File_LocalTime {
    File_LocalTime() { std::memset(this,0,sizeof(*this)); }
    File_LocalTime(unsigned y, unsigned mo, unsigned d,
                    unsigned h=0, unsigned mi=0, unsigned s=0, unsigned ms=0)
                    {set(y,mo,d,h,mi,s,ms);}

    explicit File_LocalTime(file_size64_t time) {setTime(time);}

    void set(unsigned y, unsigned mo, unsigned d, unsigned h=0, unsigned mi=0, unsigned s=0, unsigned ms=0) {
        this->year          = (unsigned short)y,
        this->month         = (unsigned short)mo,
        this->dayOfWeek     = 0;        // 適当.
        this->day           = (unsigned short)d,
        this->hour          = (unsigned short)h,
        this->minute        = (unsigned short)mi,
        this->second        = (unsigned short)s;
        this->milliSecond   = (unsigned short)ms;
    }

    // ローカル時間のものを変換して、ファイルの時間情報にして取得.
    file_size64_t getTime() const { return getTime(0); }

    // ファイルの時間を、ローカル時間に変換して設定.
    bool setTime(file_size64_t tim) { return setTime(tim, 0); }

    /// ファイルの時間を、ローカル時間に変換せずに設定.
    /// getGmTime() は用意しないので注意!
    bool setGmTime(file_size64_t tim) { return setGmTime(tim, 0); }


private:    // ヘッダに置く為にテンプレート化.
    template<typename DMY_T> file_size64_t  getTime(DMY_T) const;
    template<typename DMY_T> bool           setTime(file_size64_t tim, DMY_T);
    template<typename DMY_T> bool           setGmTime(file_size64_t tim, DMY_T);

public: // メンバー変数は公開.  // 手抜きでwinに合わせる.
    unsigned short      year;
    unsigned short      month;
    unsigned short      dayOfWeek;
    unsigned short      day;
    unsigned short      hour;
    unsigned short      minute;
    unsigned short      second;
    unsigned short      milliSecond;
};



/// ローカル時間のものを変換して、ファイルの時間情報にして取得.
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
    tb.tm_mon           = this->month - (this->month > 0);  // -1 でよいが、万が一month=0だった時のためにちょっとガード.
    tb.tm_wday          = this->dayOfWeek;  // 週の中の日数.
    tb.tm_mday          = this->day;
    tb.tm_hour          = this->hour;
    tb.tm_min           = this->minute;
    tb.tm_sec           = this->second;
    tb.tm_yday          = 0;                // 年間中の日数.
    tb.tm_isdst         = 0;                // サマータイム.
   #if defined MISC_FILE_MKTIME64
    file_size64_t   t   = MISC_FILE_MKTIME64(&tb);
   #else
    file_size64_t   t   = std::mktime(&tb);
   #endif
    return (t == file_size64_t(-1)) ? 0 : t;
  #endif
}



// ファイルの時間を、ローカル時間に変換して設定.
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



/// ファイルの時間を、ローカル時間に変換せずに設定.
/// getGmTime() は用意しないので注意!
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
// ロード・セーブ.
// ========================================================================


/** ファイル・セーブ.
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


/** ファイル・セーブ.
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



/** ファイル・ロード
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



/** ファイル・ロード.
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



/** ファイル・ロード.
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
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
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



/** file_loadAllocで取得したポインタをfreeする.
 */
inline void file_loadFree(void* p) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
    ::LocalFree(p);
  #else
    std::free(p);
  #endif
}



/** ファイルのコピー.
 */
template<typename CHR_T>
bool file_copy(const CHR_T* srcName, const CHR_T* dstName) {
  #if MISC_FILE_KIND == 2   // Win-APIアプリの時.
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
