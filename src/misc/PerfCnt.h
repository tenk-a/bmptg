/**
 *  @file   perfcnt.h
 *  @brief  時間計測関係(Perfomance Counter)
 *  @author tenk*
 */
#ifndef PERFCNT_H
#define PERFCNT_H

#ifdef _WIN32
#include <windows.h>
typedef unsigned __int64    PerfCnt_t;
static inline PerfCnt_t     PerfCnt_get()    { PerfCnt_t c; QueryPerformanceCounter((LARGE_INTEGER*)&c); return c; }
static inline PerfCnt_t     PerfCnt_per_sec() { static PerfCnt_t s = 0; if (!s) QueryPerformanceFrequency((LARGE_INTEGER*)&s); return s; }

#elif defined LINUX || defined __LINUX || defined __LINUX__
#include <sys/resource.h>
typedef unsigned long long  PerfCnt_t;
static inline PerfCnt_t     PerfCnt_get()    { struct rusage t; getrusage(RUSAGE_SELF, &t); return t.ru_utime.tv_sec * 1000000ULL + t.ru_utime.tv_usec; }
#define                     PerfCnt_per_sec()   1000000ULL

#else   // 標準ライブラリ.
#include <ctime.h>
typedef       clock_t       PerfCnt_t;
static inline PerfCnt_t     PerfCnt_get()    { return clock();   }
#define                     PerfCnt_per_sec()   CLOCKS_PER_SEC
#endif

// 浮動小数点で結果が欲しい場合は、引数のほうで型を double なりなんなりにしておくこと.
#define PERFCNT_TO_SEC(t)       ((t) / PerfCnt_per_sec())
#define PERFCNT_TO_MILLI_SEC(t) ((t) * PerfCnt_t(1000) / PerfCnt_per_sec())
#define PERFCNT_TO_MICRO_SEC(t) ((t) * PerfCnt_t(1000000) / PerfCnt_per_sec())


#endif  // TARGET_DIR
