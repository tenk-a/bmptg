/**
 *  @file   file_time.h
 *  @brief  file time getter & setter
 *  @author tenk* (Masashi Kitamura)
 *  @note
 *  -   for windows & linux(unix)
 */
#ifndef FKS_FILE_TIME_H
#define FKS_FILE_TIME_H

#ifdef _WIN32
typedef unsigned __int64                    fks_file_time_t;
#define FKS_FILE_TIME_TO_SEC(t)             ((t) / 10000000)
#define FKS_FILE_TIME_FROM_SEC(sec)         ((sec) * 10000000)
#else
//#include <sys/time.h>
#include <stdint.h>
typedef uint64_t                            fks_file_time_t;
#define FKS_FILE_TIME_TO_SEC(t)             ((t) / 1000000)
#define FKS_FILE_TIME_FROM_SEC(sec)         ((sec) * 1000000)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int fks_fileTimeGet(char const* fname, fks_file_time_t* pCreat, fks_file_time_t* pLastAcs, fks_file_time_t* pLastWrt);
int fks_fileTimeSet(char const* fname, fks_file_time_t const* pCreat, fks_file_time_t const* pLastAcs, fks_file_time_t const* pLastWrt);
fks_file_time_t fks_fileTimeFromCurrentTime();

#ifdef __cplusplus
}
#endif


#endif  // FKS_FILE_TIME_H
