/**
 *  @file   fks_file_time.c
 *  @brief  �t�@�C�����Ԃ̎擾�E�ݒ�.
 *  @author tenk* (Masashi Kitamura)
 *  @note
 *  -   win �� linux(unix)�n��Ώ�(�z��).
 */

#include <stddef.h>
#include <assert.h>
#include "fks_file_time.h"


#if defined _WIN32  // --------------------------------------------------------

#include <windows.h>
#ifdef _MSC_VER
 #pragma warning(push)
 #pragma warning(disable:4996)
 #pragma warning(disable:4189)
#endif

/** �t�@�C���̎��Ԃ̎擾. ���������� 0, ���Ԃ����畉��Ԃ�.
 */
int fks_fileTimeGet(char const* fname, fks_file_time_t* pCreat, fks_file_time_t* pLastAcs, fks_file_time_t* pLastWrt) {
    HANDLE h;
    assert(fname);
    h = CreateFileA(fname, GENERIC_READ , FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h != INVALID_HANDLE_VALUE) {
        int rc = GetFileTime(h, (FILETIME*)pCreat, (FILETIME*)pLastAcs, (FILETIME*)pLastWrt);
        CloseHandle(h);
        if (rc)
            return 0;
    }
    return -1;
}

/** �t�@�C���̎��Ԃ̐ݒ�. ���������� 0, ���Ԃ����畉��Ԃ�.
 */
int fks_fileTimeSet(char const* fname, fks_file_time_t const* pCreat, fks_file_time_t const* pLastAcs, fks_file_time_t const* pLastWrt) {
    HANDLE h;
    assert(fname);
    h = CreateFile(fname, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h != INVALID_HANDLE_VALUE) {
        int rc;
        SetFilePointer(h, 0, 0, FILE_END);
        rc = SetFileTime(h, (FILETIME*)pCreat, (FILETIME*)pLastAcs, (FILETIME*)pLastWrt);
        CloseHandle(h);
        if (rc)
            return 0;
    }
    return -1;
}

/** ��Ƀt�@�C�����ԑ���ƕ��p��������ɁA���ݎ��Ԃ��擾����
 */
fks_file_time_t fks_fileTimeFromCurrentTime() {
    fks_file_time_t t = 0;
    GetSystemTimeAsFileTime((FILETIME*)&t);
    return t;
}

#else   // linux(unix) //----------------------------------------------------

#define _LARGEFILE64_SOURCE        // include���Ԃ��C�����Ȃ��Ə�����Ȃ�...
#define _FILE_OFFSET_BITS  64      // include���Ԃ��C�����Ȃ��Ə�����Ȃ�...
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define USEC_OF_1SEC            1000000

/** �t�@�C���̎��Ԃ̎擾. ���������� 0, ���Ԃ����畉��Ԃ�.
 */
int fks_fileTimeGet(char const* fname, fks_file_time_t* pCreat, fks_file_time_t* pLastAcs, fks_file_time_t* pLastWrt) {
    struct stat st;
    int         rc;
    assert(fname);
    rc = stat(fname, &st);
    if (rc == 0) {
        if (pLastWrt) *pLastWrt = st.st_mtime * USEC_OF_1SEC;
        if (pLastAcs) *pLastAcs = st.st_atime * USEC_OF_1SEC;
        if (pCreat  ) *pCreat   = st.st_ctime * USEC_OF_1SEC;
    }
    return rc;
}

/** �t�@�C���̎��Ԃ̐ݒ�. ���������� 0, ���Ԃ����畉��Ԃ�.
 */
int fks_fileTimeSet(char const* fname, fks_file_time_t const* pCreat, fks_file_time_t const* pLastAcs, fks_file_time_t const* pLastWrt) {
    struct timeval tv[2];
    fks_file_time_t wkLastAcs, wkLastWrt;
    assert(fname != 0);
    if (pLastWrt) {
        if (!pLastAcs) {
            wkLastAcs = 0;
            pLastAcs  = &wkLastAcs;
            fks_fileTimeGet(fname, NULL, pLastAcs, NULL);
        }
    } else if (pLastAcs) {
        wkLastWrt = 0;
        pLastWrt  = &wkLastWrt;
        fks_fileTimeGet(fname, NULL, NULL, pLastWrt);
    } else {
        wkLastAcs = 0;
        pLastAcs  = &wkLastAcs;
        wkLastWrt = 0;
        pLastWrt  = &wkLastWrt;
        fks_fileTimeGet(fname, NULL, pLastAcs, pLastWrt);
    }

    tv[0].tv_sec  = *pLastAcs / USEC_OF_1SEC;
    tv[0].tv_usec = *pLastAcs % USEC_OF_1SEC;
    tv[1].tv_sec  = *pLastWrt / USEC_OF_1SEC;
    tv[1].tv_usec = *pLastWrt % USEC_OF_1SEC;

    return utimes(fname, tv);
}

/** ��Ƀt�@�C�����ԑ���ƕ��p��������ɁA���ݎ��Ԃ��擾����
 */
fks_file_time_t fks_fileTimeFromCurrentTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * USEC_OF_1SEC + t.tv_usec;
}

#endif  //  -------------------------------------------------------
