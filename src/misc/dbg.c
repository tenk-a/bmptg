/**
 *  @file   dbg.c
 *  @breaf  コマンドライン用 エラー＆デバッグルーチン
 *  @author Masashi Kitamura
 */

#include "def.h"
//x #include "dbg.h"

//---------------------------------------------------------------------------
// コマンドライン処理向けの関数実体の定義.
// １箇所で、DBG_GEN_CMDLINE を定義してincludeすれば実体化。dbg.cをリンクしない用。

#include <stdarg.h>
#include <stdlib.h>
#ifdef _MSC_VER
//void _CrtDbgBreak(int);   // 引数嘘
//void DebugBreak(int);     // 引数嘘
//#define exit      DebugBreak
#endif

#ifndef NDEBUG
int         dbg_log_sw__ = 0/*1*/;
#else
int         dbg_log_sw__ = 0;
#endif
int         dbg_level__  = 0;
const char  *err_fname__;       ///< [直接使用しちゃダメ] エラーファイル名へのポインタ. ローカル変数不可!
int         err_line__;         ///< [直接使用しちゃダメ] エラー行番号.

/// 標準エラー出力か、標準出力かを設定
#define DBG_STDERR      stdout
//#define DBG_STDERR    stderr

int dbg_init(void) {
    err_fname__ = NULL;
    err_line__  = 0;
  #ifndef NDEBUG
    dbg_log_sw__ = 1;
  #else
    dbg_log_sw__ = 0;
  #endif
    return 1;
}

void dbg_term(void) {
    ;
}

void dbg_puts(const char *s) {
    if (dbg_log_sw__)
        fprintf(DBG_STDERR, "%s", s);
}

int __cdecl dbg_printf(const char *fmt, ...) {
    int n = 0;
    if (dbg_log_sw__) {
        va_list     args;
        if (fmt) {
            va_start(args, fmt);
            n = vfprintf(DBG_STDERR, fmt, args);
            va_end(args);
        }
    }
    return n;
}

int __cdecl err_printf(const char *fmt, ...) {
    int n;
    va_list     args;
    if (fmt) {
        va_start(args, fmt);
        n = vfprintf(DBG_STDERR, fmt, args);
        va_end(args);
    } else {
        n = fprintf(DBG_STDERR, "error!!!\n");
    }
    return n;
}

//int err_setFnameLine(const char *name, int l) {
    //err_fname_ = name;
    //err_line_  = l;
    //return 0;
//}

int __cdecl err_abortMsg(const char *fmt, ...) {
    va_list     args;

    if (err_fname__)
        fprintf(DBG_STDERR, "%-14s %6d : ", err_fname__, err_line__);

    if (fmt) {
        va_start(args, fmt);
        vfprintf(DBG_STDERR, fmt, args);
        va_end(args);
    } else {
        fprintf(DBG_STDERR, "error!!!\n");
    }
//  *(char *)0 = 0;
    exit(1);
    return 0;   // マクロで式に混ぜるとき楽なように0を返すフリ
}



