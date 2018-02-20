/**
 *  @file   def.h
 *  @brief  �G���Ȋ�{��`�B�R�}���h���C����Win�p�̊ȈՂ�1�w�b�_�t�@�C����.
 *  @author Masashi KITAMURA
 *  @note
 *      �����̃t�@�C���ɕ�����Ă���ƌ��ǎg���܂킵�ɂ����̂ŁA
 *      �ŏ����̃R���p�C�����덇�킹�̒�`�A�ƁA�f�o�b�O�p�}�N���̂ݐݒ�.
 */

#ifndef DEF_H
#define DEF_H
#pragma once


// ===========================================================================
// �R���p�C���ʂ̍ŏ����̒���.


#if defined(_M_IX86) || defined(_X86_) || defined(_M_AMD64)
  #define ENABLE_BYTE_ALIGN     // �A���C�����g���o�C�g�P�ʂł������ÂɃ������A�N�Z�X�\.
  #undef  BIG_ENDIAN
  #ifndef _X86_
  #define _X86_
  #endif
  #if defined(_MSC_VER)         // vc 7.1(.net 2003)�ȍ~��Ώ�.
    #define _USE_MATH_DEFINES   // math.h �Œ�`����郉�x�������g���ꍇ�͐�ɒ�`����.
  #elif defined(__GNUC__)       // mingw gcc v3.?�ȍ~.
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
  #define ENABLE_BYTE_ALIGN     // �A���C�����g���o�C�g�P�ʂł������ÂɃ������A�N�Z�X�\.
  #define BIG_ENDIAN

#else   // �^�[�Q�b�g����
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
// ��{ ��`�� ���C�u�����E�w�b�_�� include.

// C C++ �΍�
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
// stdint �͖����� vc �͖��T�|�[�g�ŕs�ւȂ̂ŁA�Œ���̌^���`.
// (boost��SDL���g���ق��������)

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
#else   // �U��

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

// offsetof�}�N����ansi�K�i(c94)�����A����`�̃��C�u���������邽��.
#if defined(offsetof) == 0
 #define offsetof(t,m)      ((unsigned)&(((t*)0)->m))
#endif


// C99������C����inline���Ȃ�...��Win�p�R���p�C���ł�__inline�͑���̂�.
#if (defined(__cplusplus) == 0) && (defined __STDC_VERSION__ == 0 || __STDC_VERSION__ < 199901L)
 #define inline     __inline
#endif




// =====================================================
// win���̂Ƃ��́Awindows�p�̒�`��ǂݍ���ł���

#if defined _WINDOWS
#include <windows.h>
#endif







// ===========================================================================
// ===========================================================================

// ===========================================================================
// �f�o�b�O�E���O�o�̓}�N��


#if (defined DBG_TARGET)
// DBG_TARGET��ݒ肵����A�ʓr�A�^�[�Q�b�g�p�̉��L3�֐���ʃt�@�C���ɗp�ӂ��邱��

EXTERN_C int err_abort();
EXTERN_C int err_printf(const char *fmt, ...);

#else   // DBG_TARGET �̎w�肪�Ȃ���΁A���̏�Őݒ�

#if defined _WINDOWS

#include <stdio.h>
#include <stdarg.h>

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

/// �����ȃv���O�����I��(��Ƀf�o�b�O����)
static inline int err_abort() {
  #if defined __cplusplus && defined USE_DBG_EXIT_IS_THROW
    throw std::exception();
  #else
    // *(char*)0 = 0;
    DebugBreak();
  #endif
    return 0;
}


/// Win�̃f�o�b�O���O���֏o��&�I��
static inline int err_abortMsg(const char *fmt, ...) {
    char    buf[0x1000];
    va_list     args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    OutputDebugString(buf);
    return err_abort();
}


/// Win�̃f�o�b�O���O���֏o��
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
// �R���\�[���p

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define ERR_LOG_FP      stdout
//#define ERR_LOG_FP    stderr


#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4702)
#endif

/// �����ȃv���O�����I��(��Ƀf�o�b�O����)
static inline int err_abort() {
  #if defined __cplusplus && defined USE_DBG_EXIT_IS_THROW
    throw std::exception();
  #else
    // *(char*)0 = 0;
    exit(1);
  #endif
    return 0;
}


/// �W���o�͂փf�o�b�O���O���o��
static inline int err_printf(const char *fmt, ...) {
    int n;
    va_list     args;
    va_start(args, fmt);
    n = vfprintf(ERR_LOG_FP, fmt, args);
    va_end(args);
    fflush(ERR_LOG_FP);
    return n;
}


/// �W���o�͂փf�o�b�O���O���o��&�����I��
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
// �f�o�b�O�����}�N��

// ���l��"���l"������
#define DBG_I2STR(a)        DBG_I2STR_2(a)
#define DBG_I2STR_2(a)      #a
#define DBG_I2I(a)          a

// 2�̈�����A�����ĂP�̃��x���𐶐�
#define DBG_STR_CAT(a,b)    DBG_STR_CAT_2(a,b)
#define DBG_STR_CAT_2(a,b)  a##b

/// �R���p�C����assert.
#define DBG_STATIC_ASSERT(cc)   struct DBG_STR_CAT(STATIC_ASSERT_CHECK_ST,__LINE__) { char dmy[2*((cc)!=0) - 1];}; enum { DBG_STR_CAT(STATIC_ASSERT_CHECK,__LINE__) = sizeof( DBG_STR_CAT(STATIC_ASSERT_CHECK_ST,__LINE__) ) }



// __FUNCTION__������΂�����g��(����C99�ɂȂ��ĂȂ������̂˂�...)
#if defined(__FUNCTION__) == 0
    #if (defined __STDC_VERSION__ == 0 || __STDC_VERSION__ < 199901L)
        #define __FUNCTION__    ""              // __FILE__ "(" DBG_I2STR(__LINE__) ")"
    #else
        #define __FUNCTION__    __func__
    #endif
#endif




#if defined( NDEBUG )   // defined(NDEBUG) // �����[�X�� - - - - - -
// �����[�X���͂����̃}�N���͎��̂������Ȃ�
// �}�N���̂��߁Aprintf�X�^�C���̎w��́A
//      ERR_PRINTF(("err=%d\n",err));
//  �̂悤��(( ))��p����.

#if (defined DBG_TARGET)            // �^�[�Q�b�g���̃����[�X�ł́A�G���[�͖������đ��点��...
#define ERR_PRINTF(s)               ///< �G���[���O�o��
#define ERR_ABORT()                 ///< �G���[�I��
#define ERR_ABORTMSG(s)             ///< printf�`���Ń��b�Z�[�W�w�肠��̃G���[�I��
#define ERR_EXIT(s)                 ///< printf�`���Ń��b�Z�[�W�w�肠��̃G���[�I��
#else                               // �^�[�Q�b�g�̎w��̖����ꍇ�́A�R���\�[���A�v���Ƃ��ē��삳����B
#define ERR_PRINTF(s)   (err_printf s)
#define ERR_ABORT()     (err_abort())
#define ERR_ABORTMSG(s) (err_abortMsg s)
#define ERR_EXIT(s)     ERR_ABORTMSG(s)
#endif
#define ERR_ROUTE()                 ///< ��΂ɗ��Ȃ����[�g�ɒu��assert(0)���}�N��

#define DBG_M()                     ///< �J�����̃��[�g�ʉ߃`�F�b�N�p�}�N��
#define DBG_PRINTF(s)               ///< �G���[���O�o��
#define DBG_ABORT()                 ///< �G���[�I��
#define DBG_ABORTMSG(s)             ///< printf�`���Ń��b�Z�[�W�w�肠��̃G���[�I��
#define DBG_BAD_ROUTE()             ///< ��΂ɗ��Ȃ����[�g�ɒu��assert(0)���}�N��

#define DBG_ASSERT(x)               ///< assert(x)�̌�㕨. x���U�Ȃ�G���[�I��
#define DBG_ASSERT_THROW(x)         ///< ������O���A���Ă�����^�A�Ȃ���΋U�ŃG���[�I��

#define DBG_NOCHK(a)                ///< a�̓`�F�b�N���Ȃ��A�Ƃ����R�����g�̕ς��.
#define DBG_LIM_I(a, mi, ma)        ///< ���� a �� [mi,ma]�͈̔͊O�Ȃ�G���[�I��
#define DBG_LIM_U(a, ma)            ///< ���������� a �� [0 ,ma]�͈̔͊O�Ȃ�G���[�I��
#define DBG_LIM_BOOL(a)             ///< a �� 0 or 1 �Ȃ�ok�A�Ⴆ�΃G���[�I��
#define DBG_LIM_F(a, mi, ma)        ///< a �� [mi,ma]�͈̔͊O�Ȃ�G���[�I��
#define DBG_LIM_D(a, mi, ma)        ///< a �� [mi,ma]�͈̔͊O�Ȃ�G���[�I��
#define DBG_LIM_CSTR(s, sz)         ///< s �� sz�o�C�g�ȉ��̕�����ւ̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_LIM_CSTR0(s, sz)        ///< s �� NULL ��sz�o�C�g�ȉ��̕�����ւ̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.

#define DBG_CHK_PTR(p, asz)         ///< p �� �A���C�����gasz�𖞂�����ok. ng�Ȃ�G���[�I��
#define DBG_CHK_PTR0(p, asz)        ///< p �� NULL ���A���C�����gasz�𖞂�����ok. ng�Ȃ�G���[�I��

#define DBG_CHK_MPTR(p)             ///< p �� malloc���ꂽ�|�C���^�̉\���������ok. �Ȃ���΃G���[�I��.
#define DBG_CHK_MPTR0(p)            ///< p �� NULL ��malloc���ꂽ�|�C���^�̉\���������ok. �Ȃ���΃G���[�I��.

#define DBG_CHK_PTRPTR(p)           ///< p �� sizeof(void*) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_PTRPTR0(p)          ///< p �� NULL �� sizeof(void*) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.

#define DBG_CHK_SHORTPTR(p)         ///< p �� sizeof(short) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_SHORTPTR0(p)        ///< p �� NULL �� sizeof(short) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_INTPTR(p)           ///< p �� sizeof(int) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_INTPTR0(p)          ///< p �� NULL �� sizeof(int) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_LONGPTR(p)          ///< p �� sizeof(long) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_LONGPTR0(p)         ///< p �� NULL �� sizeof(long) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_LONGLONGPTR(p)      ///< p �� sizeof(long long) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.
#define DBG_CHK_LONGLONGPTR0(p)     ///< p �� NULL �� sizeof(long long) �o�C�g�A���C�����g�̃|�C���^�Ȃ�ok. ng�Ȃ�G���[�I��.

#define DBG_ARY_INVARIANT(clsAry,num)   ///< class�z��clsAry[num] �� 0�`num-1�͈̔͂�invariant()���Ăяo��.

// CppUnit �ɂ���`�F�b�N�̗ގ��i(���ʓI�Ɍ��\��������...)
#define DBG_ASSERT_MESSAGE(msg, cond)                       ///< cond���U��������msg���o�͂��ďI��.
#define DBG_FAIL( msg )                                     ///< msg���o�͂��ĕK���I��.
#define DBG_ASSERT_EQUAL( expected, actual )                ///< expected == actual �Ȃ�^�A�U�Ȃ�G���[�I��
#define DBG_ASSERT_EQUAL_MESSAGE( message,expected,actual ) ///< expected == actual �Ȃ�^�A�U�Ȃ�message���o�͂��ăG���[�I��
#define DBG_ASSERT_INTS_EQUAL( expected, actual )           ///< int �l�̔�r�� expected == actual �Ȃ�^�A�U�Ȃ�G���[�I��
#define DBG_ASSERT_INT64S_EQUAL( expected, actual )         ///< int64_t �l�̔�r�� expected == actual �Ȃ�^�A�U�Ȃ�G���[�I��
#define DBG_ASSERT_FLOATS_EQUAL( expected, actual, delta )  ///< float �l�̔�r�� expected == actual(��delta�ȓ�) �Ȃ�^�A�U�Ȃ�G���[�I��
#define DBG_ASSERT_DOUBLES_EQUAL( expected, actual, delta ) ///< double �l�̔�r�� expected == actual(��delta�ȓ�) �Ȃ�^�A�U�Ȃ�G���[�I��

#define DBG_ASSERT_MEMS_EQUAL( s1, s2, sz )                 ///< �A�h���X s1��s2�����sz�o�C�g�������Ȃ�ok. NG �Ȃ�G���[�I��.
#define DBG_ASSERT_CSTRS_EQUAL(s1, s2 )                     ///< C������s1��s2�������Ȃ�ok. NG�Ȃ�G���[�I��.

/// �N���X�����񐔂𐧌����邽�߂̃}�N��
#define DBG_COUNT_CLASS(NMAX)       typedef char Dbg_Count_Class_##NMAX##_



#else   // �f�o�b�O�� - - - - - - - - - - - - - - -

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

#define DBG_ASSERT_THROW(x)     for(;;) { bool f = 0; try { x; } catch (...) { f = 1; } DBG_ASSERT_MESSAGE( #x " ����O�𓊂��Ȃ�\n", f); break;}

#define DBG_NOCHK(a)
#define DBG_LIM_I(a, mi, ma)    for(;;) {if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %d <= %s <= %d �𖞂����Ȃ�(%d)\n", __FILE__, __LINE__, __FUNCTION__, (mi), #a, (ma),(a) ));break;}
#define DBG_LIM_U(a, ma)        for(;;) { if (! (unsigned(a) <= (ma))) DBG_ABORTMSG(("%s (%d): %s: %d <= %s <= %d �𖞂����Ȃ�(%d)\n", __FILE__, __LINE__, __FUNCTION__, 0, #a, (ma),(a) )); break;}
#define DBG_LIM_BOOL(a)         for(;;) {if (int(a)!= 0 && int(a)!=1) DBG_ABORTMSG(("%s (%d): %s: 0 <= %s <= 1 �𖞂����Ȃ�(%d)\n", __FILE__, __LINE__, __FUNCTION__, #a, (a) ));break;}
#define DBG_LIM_F(a, mi, ma)    for(;;) {const float *aa=&(a); if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %f <= %s <= %f �𖞂����Ȃ�(%f)\n", __FILE__, __LINE__, __FUNCTION__, (double)(mi), #a, (double)(ma),(double)(a) )); break;}
#define DBG_LIM_D(a, mi, ma)    for(;;) {const double *aa=&(a); if ((a) < (mi) || (ma) < (a)) DBG_ABORTMSG(("%s (%d): %s: %f <= %s <= %f �𖞂����Ȃ�(%f)\n", __FILE__, __LINE__, __FUNCTION__, (double)(mi), #a, (double)(ma),(a) ));break;}
#define DBG_LIM_CSTR(a, sz)     for(;;) {int l__ = strlen(a); if (l__ <= 0 || (sz) <= l__) DBG_ABORTMSG(("%s (%d): %s: %s{`%s'}�̒���%d��1�`%d�͈̔͊O\n", __FILE__, __LINE__, __FUNCTION__, #a, (a), l__, (sz)-1 ));   break;}
#define DBG_LIM_CSTR0(a, sz)    for(;;) {int l__ = strlen(a); if (l__ < 0 || (sz) <= l__) DBG_ABORTMSG(("%s (%d): %s: %s{`%s'}�̒���%d��0�`%d�͈̔͊O\n", __FILE__, __LINE__, __FUNCTION__, #a, (a), l__, (sz)-1 ));  break;}


// �|�C���^�Ƃ��Ă������Ȓl�Ȃ�^( ���ˑ��Ȃ̂ŁA�����ł͑�G�c�ɒ�` )
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
        DBG_ABORTMSG(("%s (%d): %s: %s{%#x}�̓A�h���X���s��",               \
                __FILE__,__LINE__,__FUNCTION__, #p, (int)(ptrdiff_t)(p) )); \
        if ((((ptrdiff_t)(p) & (l__ - 1)) != 0))                            \
            DBG_ABORTMSG(("(align %d �łȂ�)", l__ ));                      \
        DBG_ABORTMSG(("\n"));                                               \
    }                                                                       \
    break;                                                                  \
}

#define DBG_CHK_PTR0(p, asz) for(;;) {                                      \
    enum {l0__ = (asz)}; int l__ = l0__;                                    \
    if ( (((ptrdiff_t)(p) & (l__ - 1)) != 0) || DBG_IS_BADPTR(p) ) {        \
        DBG_ABORTMSG(("%s (%d): %s: %s{%#x}�̓A�h���X���s��",               \
                __FILE__,__LINE__,__FUNCTION__, #p, (int)(ptrdiff_t)(p) )); \
        if ((((ptrdiff_t)(p) & (l__ - 1)) != 0))                            \
            DBG_ABORTMSG(("(align %d �łȂ�)", l__ ));                      \
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

/// num�̔z��ary �̊e invariant() �����s.
#define DBG_ARY_INVARIANT( ary, num )   for(;;) {                           \
    unsigned int num__ = (unsigned int)(num);                               \
    for (unsigned int i__ = 0; i__ < num__; ++i__) {                        \
        (ary)[i__].invariant();                                             \
    }                                                                       \
    break;                                                                  \
}



// -----------------------------------------
// CppUnit �ɂ���`�F�b�N�̗ގ��i

/// cond �̒l���U�̂Ƃ����b�Z�[�W��\�����Ď��s
#define DBG_ASSERT_MESSAGE(msg, cond)           ((cond) || (DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg)), 0))

/// �K�����s
#define DBG_FAIL( msg )                         (DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg)), 0)

/// ���Ғlexpected �� ����actual ���������Ȃ��Ƃ����b�Z�[�W��\�����Ď��s
#define DBG_ASSERT_EQUAL_MESSAGE( message, expected, actual )  for(;;) {    \
        if ((expected) != (actual))                                         \
            DBG_ABORTMSG(("%s(%d): %s\n", __FILE__, __LINE__, msg));        \
        break;                                                              \
    }


/// ���Ғlexpected �� ����actual ���������Ȃ��Ƃ����s
#define DBG_ASSERT_EQUAL( expected, actual )  for(;;) {                     \
        if ((expected) != (actual))                                         \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s != %s\n"   \
                , __FILE__, __LINE__, #expected, #actual));                 \
        break;                                                              \
    }


/// ���Ғlexpected �� ����actual ���������Ȃ��Ƃ����s
#define DBG_ASSERT_INTS_EQUAL( expected, actual )  for(;;) {                \
        if ((expected) != (actual)) {                                       \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s{%d} != %s{%d}\n" \
                , __FILE__, __LINE__, #expected,expected, #actual,actual)); \
        }                                                                   \
        break;                                                              \
    }


/// ���Ғlexpected �� ����actual ���������Ȃ��Ƃ����s
#define DBG_ASSERT_INT64S_EQUAL( expected, actual )  for(;;) {              \
        if ((expected) != (actual)) {                                       \
            DBG_ABORTMSG(("%s(%d): equality assertion failed. %s{%#x%08x} != %s{%#x%08x}\n"         \
                , __FILE__, __LINE__                                                                \
                , #expected, (unsigned int)((uint64_t)(expected)>>32), (unsigned int)(expected)     \
                , #actual,   (unsigned int)((uint64_t)(actual)  >>32), (unsigned int)(actual)   )); \
        }                                                                   \
        break;                                                              \
    }

/// ���Ғlexpected �� ����actual �̍��� delta ���傫���Ƃ����s
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

/// ���Ғlexpected �� ����actual �̍��� delta ���傫���Ƃ����s
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



/// ������ s1 �� s1 �� sz �o�C�g����v����ΐ����A�łȂ���Ύ��s
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


/// C������ s1 �� s1 ����v����ΐ����A�łȂ���Ύ��s
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
/** �N���X�����o�ɋL�q���邱�ƂŁA�N���X�̐����񐔂�NMAX��܂łɌ��肷��
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
                DBG_ABORTMSG(("%s �� %d ���(�ő� %d)��\n"                  \
                    , __FUNCTION__, n, NMAX));                              \
        }                                                                   \
        ~Dbg_Count_Class() {                                                \
            int n = dbg_count_class_add(-1);                                \
            if (n < 0)                                                      \
                DBG_ABORTMSG(("%s �Ő������ɂȂ���\n"                       \
                    , __FUNCTION__));                                       \
        }                                                                   \
    };                                                                      \
    Dbg_Count_Class DBG_STR_CAT(dbg_count_class_,DBG_I2I(NMAX))


#endif  // NDEBUG



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ����
//  �R���p�C������:
//      __GNUC__,
//      _MSC_VER,
//      __BORLANDC__,
//      __MWERKS__      {Codewarrior},
//      __DMC__, __SC__ {DigitalMars},
//      __WATCOMC__
//      __INTEL_COMPILER,
//  os����:
//      _WIN32,_WIN64
//      WINVER, _WINDOWS_, _WIN32_WINNT, _MAC
//      _WINDOWS (windows�A�v���쐬�̂Ƃ��Avc�̃E�B�U�[�h��vcproj�ɐݒ�)
//      _CONSOLE (�R���\�[���A�v���쐬�̂Ƃ��Avc�̃E�B�U�[�h���ݒ�)
//  CPU(vc):
//      _M_IX86, _M_AMD64, _M_IA64
//  vc(��?),bcc,mingw:
//      _X86_, _MIPS_, _ALPHA_, _PPC_, _68K_, _MPPC_, _IA64_
//  mwerks:
//      __INTEL__, _MC68K_, _MIPS_, _MIPS_ISA2�`4_, _POWERPC_
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif  // DEF_H
