/**
 *  @file   StrExpr.cpp
 *  @brief  文字列を式として計算（簡易な一行電卓)
 *  @author Masashi KITAMURA
 *
 *  @note
 *
 *  int StrExpr(char *s, char **s_nxt, long *val)
 *      文字列 sをC似の式として計算して、*valに計算結果値をいれて返す.
 *      また, s_nxtがNULLでなければ使用した文字列の次のアドレスを
 *      *s_nextに入れて返す.
 *      関数の戻り値は 0なら正常。以外はエラーがあった
 *          1:想定していない式だった
 *          2:括弧が閉じていない
 *          3:0で割ろうとした
 *          4:(値でない)名前がある
 *
 *  　式は long で計算。演算子は以下の通り
 *      単項+ 単項- ( ) ~ !
 *      * / %
 *      + -
 *      << >>
 *      > >= < <=
 *      == !=
 *      &
 *      ^
 *      |
 *      && ||
 *
 *
 *  void StrExpr_SetNameChkFunc(int (*name2valFnc)(char *name, long *valp))
 *      式中に名前が現れたとき、その名前を値に変換するルーチンを登録する。
 *
 *      int name2valFnc(char *name, long *valp)
 *
 *      のような関数をStrExpr利用者が作成し、StrExpr_SetNameChkFuncで登録する.
 *      利用者が作る関数は、nameを受け取り、値にするならば、その値を *valp
 *      にいれ、0を返す。値に出来ないならば 非0を返すこと。
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "subr.h"



#define ISKANJI(c)  (((unsigned char)(c) >= 0x81 && (unsigned char)(c) <= 0x9F) || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC))
//x #define ISKANJI(c)  ((unsigned)((c)^0x20) - 0xA1 < 0x3C)


//#define DCASM

#if 0 //def DCASM
    extern int g_adrLinTop;     // ここはコメントアウト中.
#endif

//typedef long  val_t;
typedef double  val_t;

#define SYM_NAME_LEN    1030
#define isNAMETOP(ch)   (isalpha(ch) || (ch) == '_' || (ch) == '@' /*||(ch) == '.'*/)
#define isNAMECHR(ch)   (isNAMETOP(ch) || isdigit(ch) || (ch) == '$')

static int      expr_err = 0;

static int      ch      = 0;
static char     *ch_p   = NULL;

static unsigned sym     = 0;
static val_t    sym_val = 0;
static char     sym_name[SYM_NAME_LEN];
static int      (*name2valFunc)(char *name, val_t *valp) = NULL;

#undef  CC
#define CC(a,b) (((a)<<8) | (b))


/// 一文字取得.
static inline void ch_get(void)
{
    ch = (unsigned char)*ch_p;
    if (ch)
        ch_p++;
}


/// 名前の取得.
static char *getName(char *name, char *s)
{
    int i = 0;

    for (;;) {
        if (ISKANJI(*s) && s[1]) {
            if (i < SYM_NAME_LEN-1) {
                i+=2;
                *name++ = *s++;
                *name++ = *s;
            }
        } else if (isNAMECHR(*s)) {
            if (i < SYM_NAME_LEN) {
                i++;
                *name++ = *s;
            }
        } else {
            break;
        }
        s++;
    }
    *name = 0;
    return s;
}


/// 10, 2, 8, 16進数の取得.
val_t get_dig(char **sp)
{
  #ifndef DCASM
    int /*of=0,*/ov=0;
  #endif
    int c, bf=0,bv=0,dv=0,xv=0;
    int flf = 0, flnum=0;
    val_t val;
    char *s;

    s = *sp;
    c = *s++;
    val = 0;
    if (c == '0' && *s == 'x') {
        for (; ;) {
            c = *++s;
            if (isdigit(c))
                val = val * 16 + c-'0';
            else if (c >= 'A' && c <= 'F')
                val = val * 16 + 10+c-'A';
            else if (c >= 'a' && c <= 'f')
                val = val * 16 + 10+c-'a';
            else if (c == '_')
                ;
            else
                break;
        }
    } else if (c == '0' && *s == 'b') {
        for (; ;) {
            c = *++s;
            if (c == '0' || c == '1')
                val = val * 2 + c-'0';
            else if (c == '_')
                ;
            else
                break;
        }
  #ifndef DCASM /* 8進数は使わない */
    } else if (c == '0' && isdigit(*s)) {
        /* とりあえず、2,8,16進チェック */
        for (; ;) {
            c = *s;
            if (c == '0' || c == '1') {
                bv = bv*2 + (c-'0');
                ov = ov*8 + (c-'0');
                xv = xv*16+ (c-'0');
            } else if (c >= '0' && c <= '7') {
                bf = 1;
                ov = ov*8 + (c-'0');
                xv = xv*16+ (c-'0');
            } else if (c == '8' || c == '9') {
                bf = 1;
                /*of = 1;*/
                xv = xv*16+ (c-'0');
            } else if (c >= 'A' && c <= 'F') {
                if (bf == 0 && c == 'B')
                    bf = -1;
                /*of = 1;*/
                xv = xv*16+ (c-'A'+10);
            } else if (c >= 'a' && c <= 'f') {
                if (bf == 0 && c == 'b')
                    bf = -1;
                /*of = 1;*/
                xv = xv*16+ (c-'a'+10);
            } else if (c == '_') {
                ;
            } else {
                break;
            }
            s++;
        }
        if (bf == 0 && (*s == 'B' || *s == 'b')) {
            bf = -1;
            s++;
        }
        if (bf < 0) {   /* 2進数だった */
            val = bv;
        } else if (*s == 'H' || *s == 'h') {    /* 16進数だった */
            val = xv;
            s++;
        } else /*if (of == 0)*/ {   /* 8進数だった */
            val = ov;
        }
  #endif
    } else {
        /* とりあえず、2,10,16進チェック */
        --s;
        for (; ;) {
            c = *s;
            if (c == '0' || c == '1') {
                bv = bv*2 + (c-'0');
                dv = dv*10 + (c-'0');
                xv = xv*16+ (c-'0');
                if (flf)
                    flnum++;
            } else if (c >= '0' && c <= '9') {
                bf = 1;
                dv = dv*10+ (c-'0');
                xv = xv*16+ (c-'0');
                if (flf)
                    flnum++;
            } else if (c >= 'A' && c <= 'F') {
                if (bf == 0 && c == 'B')
                    bf = -1;
                //df = 1;
                xv = xv*16+ (c-'A'+10);
                if (flf)
                    flnum++;
            } else if (c >= 'a' && c <= 'f') {
                if (bf == 0 && c == 'b')
                    bf = -1;
                //df = 1;
                xv = xv*16+ (c-'a'+10);
                if (flf)
                    flnum++;
            } else if (c == '_') {
                ;
            } else if (c == '.') {
                flf = 1;
            } else {
                break;
            }
            s++;
        }
        if (bf == 0 && (*s == 'B' || *s == 'b')) {
            bf = -1;
            s++;
        }
        if (bf < 0) {   /* 2進数だった */
            val = bv;
        } else if (*s == 'H' || *s == 'h') {    /* 16進数だった */
            s++;
            val = xv;
        } else /*if (df == 0)*/ {   /* 10進数だった */
            val = dv;
            if (flf && flnum > 0)
                val = val / pow(10.0, flnum);
        }
    }
    while (*s && (isalnum(*s) || *s == '_'))
        s++;
    *sp = s;
    return val;
}


#if 0 //def DCASM
/// 2進数の取得.
static int  get_dig2(char **sp)
{
    char *s;
    int v=0,c;

    s = *sp;
    for (;;) {
        c = *s;
        if (c == '0' || c == '1') {
            v = v*2 + (c-'0');
        } else if (c == '_') {
            ;
        } else {
            break;
        }
        s++;
    }
    while (*s && (isalnum(*s) || *s == '_'))
        s++;
    *sp = s;
    return v;
}


/// 16進数の取得.
static int  get_dig16(char **sp)
{
    char *s;
    int v=0,c;

    s = *sp;
    for (;;) {
        c = *s;
        if (isdigit(c)) {
            v = v*16 + (c-'0');
        } else if (c >= 'A' && c <= 'F') {
            v = v*16 + (c-'A')+10;
        } else if (c >= 'a' && c <= 'f') {
            v = v*16 + (c-'a')+10;
        } else if (c == '_') {
            ;
        } else {
            break;
        }
        s++;
    }
    while (*s && (isalnum(*s) || *s == '_'))
        s++;
    *sp = s;
    return v;
}
#endif

/// シンボル取得.
static void sym_get(void)
{
    do {
        ch_get();
    } while (0 < ch && ch <= 0x20);
    if (isdigit(ch)) {
        ch_p--;
        sym_val = get_dig(&ch_p);
        sym = '0';
    } else if (isNAMETOP(ch) || ISKANJI(ch)) {
        ch_p = getName(sym_name, ch_p-1);
        sym = 'A';
        sym_val = 0;
        if (name2valFunc && name2valFunc(sym_name, &sym_val) == 0) {
            sym = '0';
        } else {
            sym = '0';
            sym_val = 0;
            expr_err = 4;       /*printf("値でない名前がある\n");*/
        }
    } else {
        sym = ch;
        switch(ch) {
        case '>':   if (*ch_p == '>')       {ch_p++; sym = CC('>','>');}
                    else if (*ch_p == '=')  {ch_p++; sym = CC('>','=');}
                    break;
        case '<':   if (*ch_p == '<')       {ch_p++; sym = CC('<','<');}
                    else if (*ch_p == '=')  {ch_p++; sym = CC('<','=');}
                    break;
        case '=':   if (*ch_p == '=') {ch_p++; sym = CC('=','=');}
                    else {expr_err = 1; sym = CC('=','=');}     // = だけなのは、アウト.
                    break;
        case '!':   if (*ch_p == '=') {ch_p++; sym = CC('!','=');}
                    break;
        case '&':   if (*ch_p == '&') {ch_p++; sym = CC('&','&');}
                    break;
        case '|':   if (*ch_p == '|') {ch_p++; sym = CC('|','|');}
                    break;
        case '+':
        case '-':
        case '^':
        case '~':
        case '/':
        case '%':
        case '*':
        case '(':
        case ')':
        case '\0':
        case '$':
            break;
        default:
            //expr_err = 1;     /*printf("想定していない文字があらわれた\n");*/
            break;
        }
    }
}


static val_t expr(void);


/// 数値や(式)や 単項の - + ~ ! の処理.
static val_t expr0(void)
{
    val_t l;

    sym_get();
    l = 0;
    if (sym == '0') {
        l = sym_val;
        sym_get();
  #if 0 //def DCASM
    } else if (sym == '*') {
        l = sym_val = g_adrLinTop;
        sym = '0';
        sym_get();
    } else if (sym == '$') {
        if (isxdigit(*ch_p)) {
            l = sym_val = get_dig16(&ch_p);
            sym = '0';
        } else {
            extern int g_adrLinTop;
            l = sym_val = g_adrLinTop;
            sym = '0';
        }
        sym_get();
    } else if (sym == '%') {
        if (*ch_p == '0' || *ch_p == '1') {
            l = sym_val = get_dig2(&ch_p);
            sym = '0';
        } else if (expr_err == 0) {
            expr_err = 1;       //printf("想定していない式だ\n");
        }
        sym_get();
  #endif
    } else if (sym == '-') {
        l = -expr0();
    } else if (sym == '+') {
        l = expr0();
    } else if (sym == '~') {
        l = ~(long)expr0();
    } else if (sym == '!') {
        l = !(long)expr0();
    } else if (sym == '(') {
        l = expr();
        if (sym != ')') {
            if (expr_err == 0)
                expr_err = 2;   //printf("括弧が閉じていない\n");
        } else {
            sym_get();
        }
    } else {
        if (expr_err == 0)
            expr_err = 1;       //printf("想定していない式だ\n");
    }
    return l;
}


/// * / % の処理.
static val_t expr1(void)
{
    val_t l,n;

    l = expr0();
    for (; ;) {
        if (sym == '*') {
            l = l * expr0();
        } else if (sym == '/') {
            n = expr0();
            if (n == 0) {
                l = 0;
                if (expr_err == 0)
                    expr_err = 3;//printf("0で割ろうとした\n");
            } else {
                l = l / n;
            }
        } else if (sym == '%') {
            n = expr0();
            if (n == 0) {
                l = 0;
                if (expr_err == 0)
                    expr_err = 3;//printf("0で割ろうとした\n");
            } else {
                l = (long)l % (long)n;
            }
        } else {
            break;
        }
    }
    return l;
}


/// + - の処理.
static val_t expr2(void)
{
    val_t l;

    l = expr1();
    for (; ;) {
        if (sym == '+') {
            l = l + expr1();
        } else if (sym == '-') {
            l = l - expr1();
        } else {
            break;
        }
    }
    return l;
}


/// << >> の処理.
static val_t expr3(void)
{
    val_t l;

    l = expr2();
    for (; ;) {
        if (sym == CC('<','<')) {
            l = (long)l << (int)expr2();
        } else if (sym == CC('>','>')) {
            l = (long)l >> (int)expr2();
        } else {
            break;
        }
    }
    return l;
}


/// < <= > >= の処理.
static val_t expr4(void)
{
    val_t l;

    l = expr3();
    for (; ;) {
        if (sym == '>') {
            l = (l > expr3());
        } else if (sym == '<') {
            l = (l < expr3());
        } else if (sym == CC('>','=')) {
            l = (l >= expr3());
        } else if (sym == CC('<','=')) {
            l = (l <= expr3());
        } else {
            break;
        }
    }
    return l;
}


/// ==, != の処理.
static val_t expr5(void)
{
    val_t l;

    l = expr4();
    for (; ;) {
        if (sym == CC('=','=')) {
            l = (l == expr4());
        } else if (sym == CC('!','=')) {
            l = (l != expr4());
        } else {
            break;
        }
    }
    return l;
}


/// & の処理.
static val_t expr6(void)
{
    val_t l;

    l = expr5();
    for (; ;) {
        if (sym == '&') {
            l = (long)l & (long)expr5();
        } else {
            break;
        }
    }
    return l;
}


/// ^ の処理.
static val_t expr7(void)
{
    val_t l;

    l = expr6();
    for (; ;) {
        if (sym == '^') {
            l = (long)l ^ (long)expr6();
        } else {
            break;
        }
    }
    return l;
}


/// | の処理.
static val_t expr8(void)
{
    val_t l;

    l = expr7();
    for (; ;) {
        if (sym == '|') {
            l = (long)l | (long)expr7();
        } else {
            break;
        }
    }
    return l;
}



/// && の処理.
static val_t expr9(void)
{
    val_t l,m;

    l = expr8();
    for (; ;) {
        if (sym == CC('&','&')) {
            m = expr8();
            if (l) {
                expr_err = 0;
            }
            l = (l && m);
        } else {
            break;
        }
    }
    return l;
}


/// || の処理.
static val_t expr(void)
{
    val_t l,m;

    l = expr9();
    for (; ;) {
        if (sym == CC('|','|')) {
            m = expr9();
            if (l) {
                expr_err = 0;
            }
            l = (l || m);
        } else {
            break;
        }
    }
    return l;
}


/** 文字列 sをC似の式として計算して、*valに計算結果値をいれて返す.
 *  また, s_nxtがNULLでなければ使用した文字列の次のアドレスを
 *  *s_nextに入れて返す.
 *  @retval 0   正常終了
 *  @retval 1   想定していない式だった
 *  @retval 2   括弧が閉じていない
 *  @retval 3   0で割ろうとした
 *  @retval 4   (値でない)名前がある
 */
int strExpr(const char *s, const char **s_nxt, val_t *val)
{
    expr_err = 0;
    ch_p = (char *)s;
    *val = expr();
    if (s_nxt) {
        //if (ch_p > s && *ch_p)
        //  --ch_p;
        if (*ch_p)
            *s_nxt = ch_p - 1;
        else
            *s_nxt = ch_p;
    }
    return expr_err;
}


/** 式中に名前が現れたとき、その名前を値に変換するルーチンを登録する。<br>
 *      int name2valFnc(char *name, long *valp)<br>
 *  のような関数をStrExpr利用者が作成し、StrExpr_SetNameChkFuncで登録する.
 *  利用者が作る関数は、nameを受け取り、値にするならば、その値を *valp
 *  にいれ、0を返す。値に出来ないならば 非0を返すこと。
 */
void StrExpr_SetNameChkFunc(int (*name2valFnc)(char *name, val_t *valp))
{
    name2valFunc = name2valFnc;
}
