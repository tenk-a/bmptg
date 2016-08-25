// ---------------------------------------------------------------------------
// �e�L�X�g�o��

#ifdef FILE_HDL_NS
namespace FILE_HDL_NS {
#endif

// ������o��. �� win�ł� \n ��\r\n�ɂ��ďo��. ���ȈՔłƂ��ĂȂ̂�encode��os�ˑ�.
static inline size_t    file_puts(const TCHAR* str, FILE_HDL h) {
 #if defined _WIN32
  #ifdef UNICODE
   #if 0    // UTF-16�̂܂܏o�͂���Ƃ�.
    typedef TCHAR   Char;
    const Char* s   = str;
   #else    // CP932�֕ϊ����Ă���o�͂���Ƃ�.
    enum { N = 2048 };
    typedef char    Char;
    enum { CP = 0 /* CP_UTF8 */ };
    Char            buf[ N + 1 ];
    size_t          slen = wcslen(str);
    size_t          blen = WideCharToMultiByte(CP, 0, str, slen, NULL, 0, NULL, NULL);
    Char*           s    = buf;
    if (blen > N) {
        s = (Char*)alloca(blen+1);
        if (s == NULL) {
            blen = N;
            s    = buf;
        }
    }
    WideCharToMultiByte(CP, 0, str, slen, s, blen, NULL, NULL);
    s[blen] = 0;
   #endif
  #else
    typedef TCHAR   Char;
    const Char* s   = str;
  #endif
    {
        enum { L = 4096 };
        Char         buf[L + 4];
        Char*        d  = buf;
        Char*        e  = d + L;
        size_t   sz = 0;
        while (*s) {
            Char  c = *s++;
            *d++ = c;
            if (c == '\r' && *s == '\n') {
                *d++ = *s++;
            } else if (c == '\n') {
                d[-1] = '\r';
                *d++  = c;
            }
            if (d >= e) {
                size_t l = d - buf;
                size_t r = file_write(h, buf, l*sizeof(Char));
                if (r != l * sizeof(Char))
                    return 0;
                sz += l;
                d = buf;
            }
        }
        if (d > buf) {
            size_t l = d - buf;
            size_t r = file_write(h, buf, l*sizeof(Char));
            if (r != l * sizeof(Char))
                return 0;
            sz += l;
        }
        return sz;
    }
 #else
    return file_write(h, s, strlen(s));
 #endif
}


// printf�����̕�����o��.
static inline size_t    file_vprintf(FILE_HDL h, const TCHAR* fmt, va_list a) {
    TCHAR   buf[ 4096 ];
    TCHAR*  s = buf;
    TCHAR*  p = s;
    size_t  bufSz = 4096;
    int     n;
    assert(h!=FILE_ERR_HDL && fmt);
    do {
        s = p;
     #if defined UNICODE
       #ifdef _WIN32
        n = _vsnwprintf(s, bufSz, fmt, a);
       #else
        n = vsnwprintf (s, bufSz, fmt, a);
       #endif
     #else
       #ifdef _WIN32
        n = _vsnprintf (s, bufSz, fmt, a);
       #else
        n = vsnprintf (s, bufSz, fmt, a);
       #endif
     #endif
    } while (n >= (int)(bufSz)-1 && (p = (TCHAR*)alloca((bufSz * 2)*sizeof(TCHAR))) != 0 && (bufSz *= 2));
    n = (n < 0) ? 0 : (n > (int)bufSz-1) ? (int)bufSz-1 : n;
    s[n] = 0;
    return file_puts(s, h);
}


// printf�����̕�����o��.
static inline size_t    file_printf(FILE_HDL h, const TCHAR* fmt, ...) {
    size_t  n;
    va_list  a;
    va_start(a,fmt);
    n = file_vprintf(h, fmt, a);
    va_end(a);
    return n;
}


#ifdef _WIN32   // win-api�� wsprintf ��p�����ȈՂȕW���o��printf. ���ʂ̕�������1024�o�C�g�����̂���.
#define FILE_STDOUT_PRINTF(...) do { char bUf[1030]; wsprintfA(bUf, __VA_ARGS__); file_puts(bUf, FILE_HDL_STDOUT()); } while (0)
#else
#define FILE_STDOUT_PRINTF      printf
#endif
