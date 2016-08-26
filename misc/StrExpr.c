/**
 *  @file   StrExpr.cpp
 *  @brief  ����������Ƃ��Čv�Z�i�ȈՂȈ�s�d��)
 *  @author Masashi KITAMURA
 *
 *  @note
 *
 *  int StrExpr(char *s, char **s_nxt, long *val)
 *      ������ s��C���̎��Ƃ��Čv�Z���āA*val�Ɍv�Z���ʒl������ĕԂ�.
 *      �܂�, s_nxt��NULL�łȂ���Ύg�p����������̎��̃A�h���X��
 *      *s_next�ɓ���ĕԂ�.
 *      �֐��̖߂�l�� 0�Ȃ琳��B�ȊO�̓G���[��������
 *          1:�z�肵�Ă��Ȃ���������
 *          2:���ʂ����Ă��Ȃ�
 *          3:0�Ŋ��낤�Ƃ���
 *          4:(�l�łȂ�)���O������
 *
 *  �@���� long �Ōv�Z�B���Z�q�͈ȉ��̒ʂ�
 *      �P��+ �P��- ( ) ~ !
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
 *      �����ɖ��O�����ꂽ�Ƃ��A���̖��O��l�ɕϊ����郋�[�`����o�^����B
 *
 *      int name2valFnc(char *name, long *valp)
 *
 *      �̂悤�Ȋ֐���StrExpr���p�҂��쐬���AStrExpr_SetNameChkFunc�œo�^����.
 *      ���p�҂����֐��́Aname���󂯎��A�l�ɂ���Ȃ�΁A���̒l�� *valp
 *      �ɂ���A0��Ԃ��B�l�ɏo���Ȃ��Ȃ�� ��0��Ԃ����ƁB
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
    extern int g_adrLinTop;     // �����̓R�����g�A�E�g��
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


/// �ꕶ���擾
static inline void ch_get(void)
{
    ch = (unsigned char)*ch_p;
    if (ch)
        ch_p++;
}


/// ���O�̎擾
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


/// 10, 2, 8, 16�i���̎擾
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
  #ifndef DCASM /* 8�i���͎g��Ȃ� */
    } else if (c == '0' && isdigit(*s)) {
        /* �Ƃ肠�����A2,8,16�i�`�F�b�N */
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
        if (bf < 0) {   /* 2�i�������� */
            val = bv;
        } else if (*s == 'H' || *s == 'h') {    /* 16�i�������� */
            val = xv;
            s++;
        } else /*if (of == 0)*/ {   /* 8�i�������� */
            val = ov;
        }
  #endif
    } else {
        /* �Ƃ肠�����A2,10,16�i�`�F�b�N */
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
        if (bf < 0) {   /* 2�i�������� */
            val = bv;
        } else if (*s == 'H' || *s == 'h') {    /* 16�i�������� */
            s++;
            val = xv;
        } else /*if (df == 0)*/ {   /* 10�i�������� */
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
/// 2�i���̎擾
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


/// 16�i���̎擾
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

/// �V���{���擾
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
            expr_err = 4;       /*printf("�l�łȂ����O������\n");*/
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
                    else {expr_err = 1; sym = CC('=','=');}     // = �����Ȃ̂́A�A�E�g
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
            //expr_err = 1;     /*printf("�z�肵�Ă��Ȃ������������ꂽ\n");*/
            break;
        }
    }
}


static val_t expr(void);


/// ���l��(��)�� �P���� - + ~ ! �̏���
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
            expr_err = 1;       //printf("�z�肵�Ă��Ȃ�����\n");
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
                expr_err = 2;   //printf("���ʂ����Ă��Ȃ�\n");
        } else {
            sym_get();
        }
    } else {
        if (expr_err == 0)
            expr_err = 1;       //printf("�z�肵�Ă��Ȃ�����\n");
    }
    return l;
}


/// * / % �̏���
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
                    expr_err = 3;//printf("0�Ŋ��낤�Ƃ���\n");
            } else {
                l = l / n;
            }
        } else if (sym == '%') {
            n = expr0();
            if (n == 0) {
                l = 0;
                if (expr_err == 0)
                    expr_err = 3;//printf("0�Ŋ��낤�Ƃ���\n");
            } else {
                l = (long)l % (long)n;
            }
        } else {
            break;
        }
    }
    return l;
}


/// + - �̏���
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


/// << >> �̏���
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


/// < <= > >= �̏���
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


/// ==, != �̏���
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


/// & �̏���
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


/// ^ �̏���
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


/// | �̏���
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



/// && �̏���
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


/// || �̏���
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


/** ������ s��C���̎��Ƃ��Čv�Z���āA*val�Ɍv�Z���ʒl������ĕԂ�.
 *  �܂�, s_nxt��NULL�łȂ���Ύg�p����������̎��̃A�h���X��
 *  *s_next�ɓ���ĕԂ�.
 *  @retval 0   ����I��
 *  @retval 1   �z�肵�Ă��Ȃ���������
 *  @retval 2   ���ʂ����Ă��Ȃ�
 *  @retval 3   0�Ŋ��낤�Ƃ���
 *  @retval 4   (�l�łȂ�)���O������
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


/** �����ɖ��O�����ꂽ�Ƃ��A���̖��O��l�ɕϊ����郋�[�`����o�^����B<br>
 *      int name2valFnc(char *name, long *valp)<br>
 *  �̂悤�Ȋ֐���StrExpr���p�҂��쐬���AStrExpr_SetNameChkFunc�œo�^����.
 *  ���p�҂����֐��́Aname���󂯎��A�l�ɂ���Ȃ�΁A���̒l�� *valp
 *  �ɂ���A0��Ԃ��B�l�ɏo���Ȃ��Ȃ�� ��0��Ԃ����ƁB
 */
void StrExpr_SetNameChkFunc(int (*name2valFnc)(char *name, val_t *valp))
{
    name2valFunc = name2valFnc;
}