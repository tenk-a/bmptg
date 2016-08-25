/**
 *  @file   file_hdl.h
 *  @brief  file handle api wrapper
 *  @author tenk* (Masashi Kitamura)
 *  @note
 *  -   win と linux(unix)系を対象(想定).
 *  -   基本的に unix 系の関数仕様に似せている.
 *      このため成功/失敗は 0/負値 にしているので注意.
 */

#ifndef FKS_FILE_HDL_H
#define FKS_FILE_HDL_H

#include <stddef.h>


#if !defined(FKS_USE_OSAPI_INL)

#ifdef fks
namespace fks {
#endif

extern FILE_HDL  const FILE_ERR_HDL;
FILE_HDL    file_stdin();
FILE_HDL    file_stdout();
FILE_HDL    file_stderr();

// file open. r,w,rp,wp : fopen's "rb" "wb" "rb+" "wb+"
FILE_HDL    file_open_r (TCHAR const* nm);
FILE_HDL    file_open_w (TCHAR const* nm);
FILE_HDL    file_open_rp(TCHAR const* nm);
FILE_HDL    file_open_wp(TCHAR const* nm);

void        file_close(FILE_HDL hdl);
size_t      file_read(FILE_HDL h, void* b, size_t sz);
size_t      file_write(FILE_HDL h, const void* b, size_t sz);
int         file_flush(FILE_HDL h);
file_off_t  file_seek(FILE_HDL h, file_off_t ofs, int md);
file_size_t file_filelength(FILE_HDL h);
file_off_t  file_tell(FILE_HDL h);

#ifdef fks
}   // namespace fks
#endif

#elif defined _WIN32  // --------------------------------------------------------

#include <windows.h>
#include <assert.h>

 //#include <tchar.h>
 //#include <malloc.h>
#if !defined(__cplusplus) && defined(_MSC_VER)
  #define inline __inline
#endif

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable:4996)
  #pragma warning(disable:4189)
#endif

#ifdef fks
namespace fks {
#endif

typedef HANDLE              FILE_HDL;
typedef unsigned __int64    file_size_t;
typedef __int64             file_off_t;

static const  FILE_HDL      FILE_ERR_HDL = INVALID_HANDLE_VALUE;
static inline FILE_HDL      file_stdin()  { return GetStdHandle(STD_INPUT_HANDLE) ; }
static inline FILE_HDL      file_stdout() { return GetStdHandle(STD_OUTPUT_HANDLE); }
static inline FILE_HDL      file_stderr() { return GetStdHandle(STD_ERROR_HANDLE) ; }

// file open. r,w,rp,wp : fopen's "rb" "wb" "rb+" "wb+"
static inline FILE_HDL      file_open_r (char const* nm) { assert(nm); return CreateFileA(nm, GENERIC_READ , FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); }
static inline FILE_HDL      file_open_w (char const* nm) { assert(nm); return CreateFileA(nm, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0); }
static inline FILE_HDL      file_open_rp(char const* nm) { assert(nm); return CreateFileA(nm, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); }
static inline FILE_HDL      file_open_wp(char const* nm) { assert(nm); return CreateFileA(nm, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0); }

static inline void          file_close(FILE_HDL hdl)                         { if (hdl != FILE_ERR_HDL) CloseHandle(hdl); }
static inline size_t        file_read(FILE_HDL h, void* b, size_t sz)        { DWORD r=0; assert(h!=FILE_ERR_HDL&&b&&sz); if (!ReadFile(h,b,sz,&r,0)) r=0; return r; }
static inline size_t        file_write(FILE_HDL h, const void* b, size_t sz) { DWORD r=0; assert(h!=FILE_ERR_HDL&&b&&sz); if (!WriteFile(h,b,sz,&r,0)) r=0; return r; }
static inline int           file_flush(FILE_HDL h)                           { assert(h!=FILE_ERR_HDL); return FlushFileBuffers(h) ? 0 : -1; }
//#if (_WIN32_WINNT >= 0x0500)
static inline file_off_t    file_seek(FILE_HDL h, file_off_t ofs, int md)    { assert(h!=FILE_ERR_HDL); return SetFilePointerEx(h, *(LARGE_INTEGER*)&ofs, (LARGE_INTEGER*)&ofs, md) ? ofs : 0; }
static inline file_size_t   file_filelength(FILE_HDL h)                      { file_size_t l = 0; assert(h!=FILE_ERR_HDL); return GetFileSizeEx(h, (LARGE_INTEGER*)&l) ? l : 0; }
//#else
//static inline file_off_t  file_seek(FILE_HDL h, file_off_t ofs, int md)      { assert(h!=FILE_ERR_HDL); return SetFilePointer(h, (LONG)ofs, 0, md); }
//static inline file_size_t file_filelength(FILE_HDL h)                        { DWORD m = 0, l; assert(h!=FILE_ERR_HDL); l = GetFileSize(h, (DWORD*)&m); return ((file_size_t)m<<32) | l; }
//#endif
static inline file_off_t    file_tell(FILE_HDL h)                            { assert(h!=FILE_ERR_HDL); return file_seek(h, 0, FILE_CURRENT); }

#ifdef fks
}   // namespace fks
#endif

#else   // linux(unix) //----------------------------------------------------

#include <assert.h>
#define _LARGEFILE64_SOURCE        // include順番を気をつけないと乗っ取れない...
#define _FILE_OFFSET_BITS  64      // include順番を気をつけないと乗っ取れない...
// #include <unistd.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>

#ifdef fks
namespace fks {
#endif

typedef int                 FILE_HDL;
typedef uint64_t            file_size_t;
typedef off64_t             file_off_t;
typedef time_t              file_time_t;

enum { FILE_ERR_HDL = -1 };
static inline FILE_HDL      file_stdin()  { return 0; }
static inline FILE_HDL      file_stdout() { return 1; }
static inline FILE_HDL      file_stderr() { return 2; }

// file open. r,w,rp,wp : fopen's "rb" "wb" "rb+" "wb+"
static inline FILE_HDL      file_open_r (char const* nm) { assert(nm); return open(nm, O_RDONLY, 0766); }
static inline FILE_HDL      file_open_w (char const* nm) { assert(nm); return open(nm, O_WRONLY|O_CREAT|O_TRUNC, 0766); }
static inline FILE_HDL      file_open_rp(char const* nm) { assert(nm); return open(nm, O_RDWR, 0766); }
static inline FILE_HDL      file_open_wp(char const* nm) { assert(nm); return open(nm, O_RDWR|O_CREAT, 0766); }

static inline void          file_close(FILE_HDL hdl)                           { if (hdl != FILE_ERR_HDL) close(hdl); }
static inline size_t        file_read(FILE_HDL h, void* buf, size_t sz)        { assert(h!=FILE_ERR_HDL && buf && sz); return read(h, buf, sz); }
static inline size_t        file_write(FILE_HDL h, const void* buf, size_t sz) { assert(h!=FILE_ERR_HDL && buf && sz); return write(h, buf, sz); }
static inline int           file_flush(FILE_HDL h)                             { return 0; } // dummy

static inline file_off_t    file_seek(FILE_HDL h, file_off_t ofs, int md)      { assert(h!=FILE_ERR_HDL); return lseek64(h,ofs,md); }
static inline file_off_t    file_tell(FILE_HDL h)                              { assert(h!=FILE_ERR_HDL); return file_seek(h, 0, 1/*seek_cur*/); }
static inline file_size_t   file_filelength(FILE_HDL h)                        { assert(h!=FILE_ERR_HDL); struct stat st; return fstat(h, &st)==0 ? st.st_size : 0; }

#ifdef fks
}   // namespace fks
#endif

#endif  //  -------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // FKS_FILE_HDL_H
