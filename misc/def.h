/**
 *  @file   def.h
 *  @brief  雑多な基本定義。コマンドライン＆Win用の簡易な1ヘッダファイル版.
 *  @author Masashi KITAMURA
 *  @note
 *      複数のファイルに分かれていると結局使いまわしにくいので、
 *      最小限のコンパイラ辻褄合わせの定義、と、デバッグ用マクロのみ設定.
 */

#ifndef DEF_H
#define DEF_H
#pragma once


// ===========================================================================
// コンパイラ別の最小限の調整.


#if defined(_M_IX86) || defined(_X86_) || defined(_M_AMD64)
  #define ENABLE_BYTE_ALIGN     // アライメントがバイト単位でも落ちづにメモリアクセス可能.
  #undef  BIG_ENDIAN
  #ifndef _X86_
  #define _X86_
  #endif
  #if defined(_MSC_VER)         // vc 7.1(.net 2003)以降を対象.
    #define _USE_MATH_DEFINES   // math.h で定義されるラベル名を使う場合は先に定義する.
  #elif defined(__GNUC__)       // mingw gcc v3.?以降.
    #ifndef _M_IX86
    #define _M_IX86
    #endif
    #ifndef __inline
      #define __inline  inline
    #endif
    #ifndef __cdecl
      #define __cdecl
    #endif
  #elif defined(__BORLANDC__)   // bcc5.5
  #elif defined(__DMC__)        // v8.48
  #elif defined(__WATCOMC__)    // OpenWatcom 1.5
  #else
  #endif

#elif defined(_PPC_)
  #define ENABLE_BYTE_ALIGN     // アライメントがバイト単位でも落ちづにメモリアクセス可能.
  #define BIG_ENDIAN

#else   // ターゲット向け
  #ifndef __inline
    #define __inline    inline
  #endif
  #ifndef __cdecl
    #define __cdecl
  #endif
#endif


// 64bit-cpu
#if (defined _WIN64) ||(defined __WORDSIZE && __WORDSIZE == 64) || (defined _M_AMD64) || (defined _IA64_)
#define CPU64
#endif



// =====================================================
// 基本 定義の ライブラリ・ヘッダの include.

// C C++ 対策
#ifdef __cplusplus
 #define EXTERN_C       extern "C"
 #include <cstddef>
 #include <cmath>
 #include <cctype>
 #include <cstring>
 #include <climits>
 //#include <cassert>
 #include <exception>
#else
 #define EXTERN_C       extern
#endif

#include <stddef.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <assert.h>



// =====================================================
// stdint は未だに vc は未サポートで不便なので、最低限の型を定義.
// (boostやSDLを使うほうが無難かも)

#if (defined _MSC_VER && _MSC_VER < 1600) || (defined __BORLANDC__ && __BORLANDC__ <= 0x0551)
typedef __int8              int8_t;
typedef unsigned __int8     uint8_t;
typedef __int16             int16_t;
typedef unsigned __int16    uint16_t;
typedef __int32             int32_t;
typedef unsigned __int32    uint32_t;
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;

typedef __int64             intmax_t;
typedef uint64_t            uintmax_t;

 #if defined(_WIN64)
  typedef int64_t           intptr_t;
  typedef uint64_t          uintptr_t;
 #elif defined _MSC_VER
  typedef __w64 int32_t     intptr_t;
  typedef __w64 uint32_t    uintptr_t;
 #else
  typedef int32_t           intptr_t;
  typedef uint32_t          uintptr_t;
 #endif
#else
  #include <stdint.h>
#endif



#if defined(HAVE_INT128)
typedef long long          __INT128;
typedef unsigned long long __UINT128;
#else   // 偽者

typedef union __UINT128 {
    double      align;
    struct {
      #if defined BIG_ENDIAN
        uint64_t    hi;
        uint64_t    lo;
      #else
        uint64_t    lo;
        uint64_t    hi;
      #endif
    } val;
} __UINT128;

typedef union __INT128 {
    double      align;
    struct {
      #if defined BIG_ENDIAN
        int64_t     hi;
        uint64_t    lo;
      #else
        uint64_t    lo;
        int64_t     hi;
      #endif
    } val;
} __INT128;

#endif



// =====================================================

// offsetofマクロはansi規格(c94)だが、未定義のライブラリもあるため.
#if defined(offsetof) == 0
 #define offsetof(t,m)      ((unsigned)&(((t*)0)->m))
#endif


// C99未満のCだとinlineがない...がWin用コンパイラでは__inlineは大抵あるので.
#if (defined(__cplusplus) == 0) && (defined __STDC_VERSION__ == 0 || __STDC_VERSION__ < 199901L)
 #define inline     __inline
#endif




// =====================================================
// win環境のときは、windows用の定義を読み込んでおく

#if defined _WINDOWS
#include <windows.h>
#endif







// ===========================================================================
// ===========================================================================

// ===========================================================================
// デバッグ・ログ出力マクロ


#if (defined DBG_TARGET)
// DBG_TARGETを設定したら、別途、ターゲット用の下記3関数を別ファイルに用意すること

EXTERN_C int err_abort();
EXTERN_C int err_printf(const char *fmt, ...);

#else   // DBG_TARGET の指定がなければ、この場で設定

#if defined _WINDOWS

#include <stdio.h>
#include <stdarg.h>

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

/// 強引なプログラム終了(主にデバッグ向け)
static inline int err_abort() {
  #if defined __cplusplus && defined USE_DBG_EXIT_IS_THROW
    throw std::exception();
  #else
    // *(char*)0 = 0;
    DebugBreak();
  #endif
    return 0;
}


/// Winのデバッグログ窓へ出力&終了
static inline int err_abortMsg(const char *fmt, ...) {
    char    buf[0x1000];
    va_list     args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    OutputDebugString(buf);
    return err_abort();
}


/// Winのデバッグログ窓へ出力
static inline int err_printf(const char *fmt, ...) {
    char    buf[0x1000];
    va_list     args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    OutputDebugString(buf);
    return n;
}

#if defined _MSC_VER
#pragma warning(pop)
#endif


#else   // ------------------------------------------------------
// コンソール用

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define ERR_LOG_FP      stdout
//#define ERR_LOG_FP    stderr


#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

/// 強引なプログラム終了(主にデバッグ向け)
static inline int err_abort() {
  #if defined __cplusplus && defined USE_DBG_EXIT_IS_THROW
    throw std::exception();
  #else
    // *(char*)0 = 0;
    exit(1);
  #endif
    return 0;
}


/// 標準出力へデバッグログを出力
static inline int err_printf(const char *fmt, ...) {
    int n;
    va_list     args;
    va_start(args, fmt);
    n = vfprintf(ERR_LOG_FP, fmt, args);
    va_end(args);
    fflush(ERR_LOG_FP);
    return n;
}


/// 標準出力へデバッグログを出力&強制終了
static inline int err_abortMsg(const char *fmt, ...) {
    va_list     args;
    va_start(args, fmt);
    vfprintf(ERR_LOG_FP, fmt, args);
    va_end(args);
    fflush(ERR_LOG_FP);
    return err_abort();
}


#if defined _MSC_VER
#pragma warning(pop)
#endif

#undef ERR_LOG_FP

#endif  // _WINDOWS

#endif  // DBG_TARGET



// ===========================================================================
// デバッグ向けマクロ

// 数値を"数値"文字列化
#define DBG_I2STR(a)        DBG_I2STR_2(a)
#define DBG_I2STR_2(a)      #a
#define DBG_I2I(a)          a

// 2つの引数を連結して１つのラベルを生成
#define DBG_STR_CAT(a,b)    DBG_STR_CAT_2(a,b)
#define DBG_STR_CAT_2(a,b)  a##b

/// コンパイル時assert.
#define DBG_STATIC_ASSERT(cc)   struct DBG_STR_CAT(STATIC_ASSERT_CHECK_ST,__LINE__) { char dmy[2*((cc)!=0) - 1];}; enum { DBG_STR_CAT(STATIC_ASSERT_CHECK,__LINE__) = sizeof( DBG_STR_CAT(STATIC_ASSERT_CHECK_ST,__LINE__) ) }



// __FUNCTION__があればそれを使う(結局C99になってなかったのねん...)
#if defined(__FUNCTION__) == 0
    #if (defined __STDC_VERSION__ == 0 || __STDC_VERSION__ < 199901L)
        #define __FUNCTION__    ""              // __FILE__ "(" DBG_I2STR(__LINE__) ")"
    #else
        #define __FUNCTION__    __func__
    #endif
#endif




#if defined( NDEBUG )   // defined(NDEBUG) // リリース時 - - - - - -
// リリース時はこれらのマクロは実体が無くなる
// マクロのため、printfスタイルの指定は、
//      ERR_PRINTF(("err=%d\n",err));
//  のように(( ))を用いる.

#if (defined DBG_TARGET)            // ターゲット環境のリリースでは、エラーは無視して走らせる...
#define ERR_PRINTF(s)               ///< エラーログ出力
#define ERR_ABORT()                 ///< エラー終了
#define ERR_ABORTMSG(s)             ///< printf形式でメッセージ指定ありのエラー終了
#define ERR_EXIT(s)                 ///< printf形式でメッセージ指定ありのエラー終了
#else                               // ターゲットの指定の無い場合は、コンソールアプリとして動作させる。
#define ERR_PRINTF(s)   (err_printf s)
#define ERR_ABORT()     (err_abort())
#define ERR_ABORTMSG(s) (err_abortMsg s)
#define ERR_EXIT(s)     ERR_ABORTMSG(s)
#endif
#define ERR_ROUTE()                 ///< 絶対に来ないルートに置くassert(0)交代マクロ

#define DBG_M()                     ///< 開発時のルート通過チェック用マクロ
#define DBG_PRINTF(s)               ///< エラーログ出力
#define DBG_ABORT()                 ///< エラー終了
#define DBG_ABORTMSG(s)             ///< printf形式でメッセージ指定ありのエラー終了
#define DBG_BAD_ROUTE()             ///< 絶対に来ないルートに置くassert(0)交代マクロ

#define DBG_ASSERT(x)               ///< assert(x)の交代物. xが偽ならエラー終了
#define DBG_ASSERT_THROW(x)         ///< 何か例外が帰ってきたら真、なければ偽でエラー終了

#define DBG_NOCHK(a)                ///< aはチェックしない、というコメントの変わり.
#define DBG_LIM_I(a, mi, ma)        ///< 整数 a が [mi,ma]の範囲外ならエラー終了
#define DBG_LIM_U(a, ma)            ///< 符号無整数 a が [0 ,ma]の範囲外ならエラー終了
#define DBG_LIM_BOOL(a)             ///< a が 0 or 1 ならok、違えばエラー終了
#define DBG_LIM_F(a, mi, ma)        ///< a が [mi,ma]の範囲外ならエラー終了
#define DBG_LIM_D(a, mi, ma)        ///< a が [mi,ma]の範囲外ならエラー終了
#define DBG_LIM_CSTR(s, sz)         ///< s が szバイト以下の文字列へのポインタならok. ngならエラー終了.
#define DBG_LIM_CSTR0(s, sz)        ///< s が NULL かszバイト以下の文字列へのポインタならok. ngならエラー終了.

#define DBG_CHK_PTR(p, asz)         ///< p が アライメントaszを満たせばok. ngならエラー終了
#define DBG_CHK_PTR0(p, asz)        ///< p が NULL かアライメントaszを満たせばok. ngならエラー終了

#define DBG_CHK_MPTR(p)             ///< p が mallocされたポインタの可能性があればok. なければエラー終了.
#define DBG_CHK_MPTR0(p)            ///< p が NULL かmallocされたポインタの可能性があればok. なければエラー終了.

#define DBG_CHK_PTRPTR(p)           ///< p が sizeof(void*) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_PTRPTR0(p)          ///< p が NULL か sizeof(void*) バイトアライメントのポインタならok. ngならエラー終了.

#define DBG_CHK_SHORTPTR(p)         ///< p が sizeof(short) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_SHORTPTR0(p)        ///< p が NULL か sizeof(short) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_INTPTR(p)           ///< p が sizeof(int) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_INTPTR0(p)          ///< p が NULL か sizeof(int) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_LONGPTR(p)          ///< p が sizeof(long) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_LONGPTR0(p)         ///< p が NULL か sizeof(long) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_LONGLONGPTR(p)      ///< p が sizeof(long long) バイトアライメントのポインタならok. ngならエラー終了.
#define DBG_CHK_LONGLONGPTR0(p)     ///< p が NULL か sizeof(long long) バイトアライメントのポインタならok. ngならエラー終了.

#define DBG_ARY_INVARIANT(clsAry,num)   ///< class配列clsAry[num] の 0～num-1の範囲のinvariant()を呼び出す.

// CppUnit にあるチェックの類似品(結果的に結構ちがうが...)
#define DBG_ASSERT_MESSAGE(msg, cond)                       ///< condが偽だったらmsgを出力して終了.
#define DBG_FAIL( msg )                                     ///< msgを出力して必ず終了.
#define DBG_ASSERT_EQUAL( expected, actual )                ///< expected == actual なら真、偽ならエラー終了
#define DBG_ASSERT_EQUAL_MESSAGE( message,expected,actual ) ///< expected == actual なら真、偽ならmessageを出力してエラー終了
#define DBG_ASSERT_INTS_EQUAL( expected, actual )           ///< int 値の比較で expected == actual なら真、偽ならエラー終了
#define DBG_ASSERT_INT64S_EQUAL( expected, actual )         ///< int64_t 値の比較で expected == actual なら真、偽ならエラー終了
#define DBG_ASSERT_FLOATS_EQUAL( expected, actual, delta )  ///< float 値の比較で expected == actual(差delta以内) なら真、偽ならエラー終了
#define DBG_ASSERT_DOUBLES_EQUAL( expected, actual, delta ) ///< double 値の比較で expected == actual(差delta以内) なら真、偽ならエラー終了

#define DBG_ASSERT_MEMS_EQUAL( s1, s2, sz )                 ///< アドレス s1とs2からのszバイトが同じならok. NG ならエラー終了.
#define DBG_ASSERT_CSTRS_EQUAL(s1, s2 )                     ///< C文字列s1とs2が同じならok. NGならエラー終了.

/// クラス生成回数を制限するためのマクロ
#define DBG_COUNT_CLASS(NMAX)       typedef char Dbg_Count_Class_##NMAX##_



#else   // デバッグ中 - - - - - - - - - - - - - - -

#define ERR_PRINTF(s)           (err_printf s)
#define ERR_ABORT()             (err_abort())
#define ERR_ABORTMSG(s)         (err_abortMsg s)
#define ERR_EXIT(s)             ERR_ABORTMSG(s)
#define ERR_ROUTE()             DBG_BAD_ROUTE()

#define DBG_M()                 DBG_PRINTF(("%-14s(%5d):\n", __FILE__, __LINE__))
#define DBG_PRINTF(s)           (err_printf s)
#define DBG_ABORT()             (err_abort())
#define DBG_ABORTMSG(s)         (err_abortMsg s)
#define DBG_BAD_ROUTE()         DBG_ABORTMSG(("%s(%d):bad route! (in %s)\n", __FILE__, __LINE__, __FUNCTION__))

#define DBG_ASSERT(x)           ((x) || DBG_ABORTMSG(("%s(%d):%s> assert(%s) is false.\n", __FILE__, __LINE__, __FUNCTION__, #x)) )

#define DBG_ASSERT_THROW(x)     for(;;) { bool f = 0; try { x; } catch (...) { f = 1; } DBG_ASSERT_MESSAGE( #x " が例外を投げない\n", f); break;}

#define DBG_NOCHK(a)
#define DBG_LIM_I(a, mi, ma)    for(;;) {if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %d <= %s <= %d を満たさない(%d)\n", __FILE__, __LINE__, __FUNCTION__, (mi), #a, (ma),(a) ));break;}
#define DBG_LIM_U(a, ma)        for(;;) { if (! (unsigned(a) <= (ma))) DBG_ABORTMSG(("%s (%d): %s: %d <= %s <= %d を満たさない(%d)\n", __FILE__, __LINE__, __FUNCTION__, 0, #a, (ma),(a) )); break;}
#define DBG_LIM_BOOL(a)         for(;;) {if (int(a)!= 0 && int(a)!=1) DBG_ABORTMSG(("%s (%d): %s: 0 <= %s <= 1 を満たさない(%d)\n", __FILE__, __LINE__, __FUNCTION__, #a, (a) ));break;}
#define DBG_LIM_F(a, mi, ma)    for(;;) {const float *aa=&(a); if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %f <= %s <= %f を満たさない(%f)\n", __FILE__, __LINE__, __FUNCTION__, (double)(mi), #a, (double)(ma),(double)(a) )); break;}
#define DBG_LIM_D(a, mi, ma)    for(;;) {const double *aa=&(a); if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %f <= %s <= %f を満たさない(%f)\n", __FILE__, __LINE__, __FUNCTION__, (double)(mi), #a, (double)(ma),(a) ));break;}
#define DBG_LIM_CSTR(a, sz)     for(;;) {int l__ = strlen(a); if (l__ <= 0 || (sz) <= l__) DBG_ABORTMSG(("%s (%d): %s: %s{`%s'}の長さ%dは1～%dの範囲外\n", __FILE__, __LINE__, __FUNCTION__, #a, (a), l__, (sz)-1 ));   break;}
#define DBG_LIM_CSTR0(a, sz)    for(;;) {int l__ = strlen(a); if (l__ < 0 || (sz) <= l__) DBG_ABORTMSG(("%s (%d): %s: %s{`%s'}の長さ%dは0～%dの範囲外\n", __FILE__, __LINE__, __FUNCTION__, #a, (a), l__, (sz)-1 ));  break;}


// ポインタとしておかしな値なら真( 環境依存なので、ここでは大雑把に定義 )
#ifndef DBG_IS_BADPTR
 #if defined _WIN64
  #define DBG_IS_BADPTR(p)      ((p) != 0 && (size_t)(ptrdiff_t)(p) < 0x10000)
 #elif defined _WIN32
  //#define DBG_IS_BADPTR(p)        ((p) != 0 && ((size_t)(ptrdiff_t)(p) < 0x10000)||((size_t)(ptrdiff_t)(p) >= 0x80000000))
  #define DBG_IS_BADPTR(p)      ((p) != 0 && (size_t)(ptrdiff_t)(p) < 0x10000)
 #else
  #define DBG_IS_BADPTR(p)      (0)
 #endif
#endif

#define DBG_CHK_PTR(p, asz) for(;;) {                                       \
    enum {l0__ = (asz)}; int l__ = l0__;                                    \
    if ( ((p) == 0) || (((ptrdiff_t)(p) & (l__ - 1)) != 0) || DBG_IS_BADPTR(p) ) { \
        DBG_ABORTMSG(("%s (%d): %s: %s{%#x}はアドレスが不正",               \
                __FILE__,__LINE__,__FUNCTION__, #p, (int)(ptrdiff_t)(p) )); \
        if ((((ptrdiff_t)(p) & (l__ - 1)) != 0))                            \
            DBG_ABORTMSG(("(align %d でない)", l__ ));                      \
        DBG_ABORTMSG(("\n"));                                               \
    }                                                                       \
    break;                                                                  \
}

#define DBG_CHK_PTR0(p, asz) for(;;) {                                      \
    enum {l0__ = (asz)}; int l__ = l0__;                                    \
    if ( (((ptrdiff_t)(p) & (l__ - 1)) != 0) || DBG_IS_BADPTR(p) ) {        \
        DBG_ABORTMSG(("%s (%d): %s: %s{%#x}はアドレスが不正",               \
                __FILE__,__LINE__,__FUNCTION__, #p, (int)(ptrdiff_t)(p) )); \
        if ((((ptrdiff_t)(p) & (l__ - 1)) != 0))                            \
            DBG_ABORTMSG(("(align %d でない)", l__ ));                      \
        DBG_ABORTMSG(("\n"));                                               \
    }                                                                       \
    break;                                                                  \
}

#define DBG_CHK_PTRPTR(p)       DBG_CHK_PTR(p, sizeof(void*))
#define DBG_CHK_PTRPTR0(p)      DBG_CHK_PTR0(p, sizeof(void*))

#define DBG_CHK_SHORTPTR(p)     DBG_CHK_PTR(p, sizeof(short))
#define DBG_CHK_SHORTPTR0(p)    DBG_CHK_PTR0(p, sizeof(short))
#define DBG_CHK_INTPTR(p)       DBG_CHK_PTR(p, sizeof(int))
#define DBG_CHK_INTPTR0(p)      DBG_CHK_PTR0(p, sizeof(int))
#define DBG_CHK_LONGPTR(p)      DBG_CHK_PTR(p, sizeof(long))
#define DBG_CHK_LONGPTR0(p)     DBG_CHK_PTR0(p, sizeof(long))
#define DBG_CHK_LONGLONGPTR(p)  DBG_CHK_PTR(p, sizeof(long long))
#define DBG_CHK_LONGLONGPTR0(p) DBG_CHK_PTR0(p, sizeof(long long))

/// num個の配列ary の各 invariant() を実行.
#define DBG_ARY_INVARIANT( ary, num )   for(;;) {                           \
    unsigned int num__ = (unsigned int)(num);                               \
    for (unsigned int i__ = 0; i__ < num__; ++i__) {                        \
        (ary)[i__].invariant();                                             \
    }                                                                       \
    break;                                                                  \
}



// -----------------------------------------
// CppUnit にあるチェックの類似品

/// cond の値が偽のときメッセージを表示して失敗
#define DBG_ASSERT_MESSAGE(msg, cond)           ((cond) || (DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg)), 0))

/// 必ず失敗
#define DBG_FAIL( msg )                         (DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg)), 0)

/// 期待値expected と 結果actual が等しくないときメッセージを表示して失敗
#define DBG_ASSERT_EQUAL_MESSAGE( message, expected, actual )  for(;;) {    \
        if ((expected) != (actual))                                         \
            DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg));        \
        break;                                                              \
    }


/// 期待値expected と 結果actual が等しくないとき失敗
#define DBG_ASSERT_EQUAL( expected, actual )  for(;;) {                     \
        if ((expected) != (actual))                                         \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s != %s\n"   \
                , __FILE__, __LINE__, #expected, #actual));                 \
        break;                                                              \
    }


/// 期待値expected と 結果actual が等しくないとき失敗
#define DBG_ASSERT_INTS_EQUAL( expected, actual )  for(;;) {                \
        if ((expected) != (actual)) {                                       \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s{%d} != %s{%d}\n" \
                , __FILE__, __LINE__, #expected,expected, #actual,actual)); \
        }                                                                   \
        break;                                                              \
    }


/// 期待値expected と 結果actual が等しくないとき失敗
#define DBG_ASSERT_INT64S_EQUAL( expected, actual )  for(;;) {              \
        if ((expected) != (actual)) {                                       \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s{%#x%08x} != %s{%#x%08x}\n"         \
                , __FILE__, __LINE__                                                                \
                , #expected, (unsigned int)((uint64_t)(expected)>>32), (unsigned int)(expected)     \
                , #actual,   (unsigned int)((uint64_t)(actual)  >>32), (unsigned int)(actual)   )); \
        }                                                                   \
        break;                                                              \
    }

/// 期待値expected と 結果actual の差が delta より大きいとき失敗
#define DBG_ASSERT_FLOATS_EQUAL( expected, actual, delta )  for(;;) {       \
        double eXPe__ = (double)(float)(expected);                          \
        double aCTu__ = (double)(float)(actual);                            \
        double dElt__ = (double)(delta);                                    \
        if ((eXPe__ < aCTu__ -dElt__) || (aCTu__+dElt__ < eXPe__)) {        \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. abs(%s{%e} - %s{%e}) > %s(%e)\n"   \
                , __FILE__, __LINE__                                        \
                , #expected, eXPe__, #actual, aCTu__, #delta, dElt__));     \
        }                                                                   \
        break;                                                              \
    }

/// 期待値expected と 結果actual の差が delta より大きいとき失敗
#define DBG_ASSERT_DOUBLES_EQUAL( expected, actual, delta )  for(;;) {      \
        double eXPe__ = (double)(expected);                                 \
        double aCTu__ = (double)(actual);                                   \
        double dElt__ = (double)(delta);                                    \
        double dIFf__ = eXPe__ - aCTu__;                                    \
        if ((dIFf__ < -dElt__) || (dElt__ < dIFf__)) {                      \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. abs(%s{%e} - %s{%e}) > %s{%e}\n"   \
                , __FILE__, __LINE__                                        \
                , #expected, (double)eXPe__, #actual, (double)aCTu__, #delta, (double)dElt__));  \
        }                                                                   \
        break;                                                              \
    }



/// メモリ s1 と s1 の sz バイトが一致すれば成功、でなければ失敗
#define DBG_ASSERT_MEMS_EQUAL(s1, s2, sz )  for(;;) {                       \
        const unsigned char *cucp1__ = (const unsigned char *)(s1);         \
        const unsigned char *cucp2__ = (const unsigned char *)(s2);         \
        int size__ = (sz);                                                  \
        DBG_ASSERT( (sz) >= 0 );                                            \
        if (size__ > 0) {                                                   \
            int n__;                                                        \
            for (n__ = 0; n__ < size__; ++n__, ++cucp1__, ++cucp2__) {      \
                if (*cucp1__ != *cucp2__) {                                 \
                    DBG_ABORTMSG(("%s(%d): equality assertion failed. offset %#x> %s{%#x} != %s{%#x}\n" \
                        , __FILE__, __LINE__, n__, #s1, *cucp1__, #s2, *cucp2__ )); \
                    break;                                                  \
                }                                                           \
            }                                                               \
        }                                                                   \
        break;                                                              \
    }


/// C文字列 s1 と s1 が一致すれば成功、でなければ失敗
#define DBG_ASSERT_CSTRS_EQUAL(s1, s2 ) for(;;) {                           \
        const unsigned char *cucp1__ = (const unsigned char *)(s1);         \
        const unsigned char *cucp2__ = (const unsigned char *)(s2);         \
        int n__;                                                            \
        for (n__ = 0; ; ++n__, ++cucp1__, ++cucp2__) {                      \
            if (*cucp1__ != *cucp2__) {                                     \
                DBG_ABORTMSG(("%s(%d): equality assertion failed. offset %#x> %s{%s} != %s{%s}\n" \
                    , __FILE__, __LINE__, n__, #s1, cucp1__, #s2, cucp2__));\
                break;                                                      \
            }                                                               \
            if (*cucp1__ == 0 || *cucp2__ == 0)                             \
                break;                                                      \
        }                                                                   \
        break;                                                              \
    }




// ------------------------------------------------
/** クラスメンバに記述することで、クラスの生成回数をNMAX回までに限定する
 */
#define DBG_COUNT_CLASS(NMAX)                                               \
    class Dbg_Count_Class {                                                 \
        static int dbg_count_class_add(int add) {                           \
            static int num = 0;                                             \
            num += add;                                                     \
            return num;                                                     \
        }                                                                   \
    public:                                                                 \
        Dbg_Count_Class() {                                                 \
            int n = dbg_count_class_add(1);                                 \
            if (n > NMAX)                                                   \
                DBG_ABORTMSG(("%s が %d 回目(最大 %d)だ\n"                  \
                    , __FUNCTION__, n, NMAX));                              \
        }                                                                   \
        ~Dbg_Count_Class() {                                                \
            int n = dbg_count_class_add(-1);                                \
            if (n < 0)                                                      \
                DBG_ABORTMSG(("%s で数が負になった\n"                       \
                    , __FUNCTION__));                                       \
        }                                                                   \
    };                                                                      \
    Dbg_Count_Class DBG_STR_CAT(dbg_count_class_,DBG_I2I(NMAX))


#endif  // NDEBUG



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// メモ
//  コンパイラ識別:
//      __GNUC__,
//      _MSC_VER,
//      __BORLANDC__,
//      __MWERKS__      {Codewarrior},
//      __DMC__, __SC__ {DigitalMars},
//      __WATCOMC__
//      __INTEL_COMPILER,
//  os識別:
//      _WIN32,_WIN64
//      WINVER, _WINDOWS_, _WIN32_WINNT, _MAC
//      _WINDOWS (windowsアプリ作成のとき、vcのウィザードがvcprojに設定)
//      _CONSOLE (コンソールアプリ作成のとき、vcのウィザードが設定)
//  CPU(vc):
//      _M_IX86, _M_AMD64, _M_IA64
//  vc(旧?),bcc,mingw:
//      _X86_, _MIPS_, _ALPHA_, _PPC_, _68K_, _MPPC_, _IA64_
//  mwerks:
//      __INTEL__, _MC68K_, _MIPS_, _MIPS_ISA2～4_, _POWERPC_
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif  // DEF_H
