/**
 *  @file   pix_subr.h
 *  @brief
 */
#ifndef PIX_SUBR_H
#define PIX_SUBR_H


/** グリッドサイズ gw*gh として、グリッドに合うように矩形サイズを調整 */
inline void pix_gridRect(int width, int height, int *x_p, int *y_p, int *w_p, int *h_p)
{
    if (width > 1) {
        int x1   = ((*x_p + *w_p + width - 1) / width) * width;
        int x0   = (*x_p / width) * width;
        *x_p     = x0;
        *w_p     = x1 - x0;
    }
    if (height > 1) {
        int y1   = ((*y_p + *h_p + height - 1) / height) * height;
        int y0   = (*y_p / height) * height;
        *y_p     = y0;
        *h_p     = y1 - y0;
    }
}




#if defined __cplusplus
// 下記は、ピクセルをあらわす型をtemplateで与えている以外は、ほぼCルーチンのまま.



/** 背景色colKey以外の部分が入る最小の矩形を求める.
 */
template<typename pix_t>
int pix_getRectWithoutColKey(pix_t *pix, unsigned width, unsigned height, pix_t colKey
                            , int *x_p, int *y_p, int *w_p, int *h_p
                            , bool fuchi1=false)
{
    int w = (int)width;
    int h = (int)height;
    int x,y,x0,y0,x1,y1;

    if (pix == NULL || w <= 0 || h <= 0) {
        return 0;
    }

    x0 = y0 =  0x7fffffff;
    x1 = y1 = -0x7fffffff;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (pix[y*w+x] != colKey) {
                if (x0 > x) x0 = x;
                if (y0 > y) y0 = y;
                if (x1 < x) x1 = x;
                if (y1 < y) y1 = y;
            }
        }
    }

    if (x0 == 0x7fffffff || x1 == -0x7fffffff) {    /* 背景色のみのとき */
        *w_p = 0;
        *h_p = 0;
        *x_p = 0;
        *y_p = 0;
    } else {
      #if 1 // 必ず淵１ドットに抜きいろがあるように調整
        if (fuchi1) {
            x0--, y0--, x1++, y1++;
            if (x0 < 0) x0 = 0;
            if (y0 < 0) y0 = 0;
            if (x1 >= w) x1 = w-1;
            if (y1 >= h) y1 = h-1;
        }
      #endif
        *x_p = x0;
        *y_p = y0;
        *w_p = x1+1 - x0;
        *h_p = y1+1 - y0;
    }
    return 1;
}




/// 画像の左右反転
template<typename pix_t>
inline void pix_revX(pix_t* pix, unsigned w, unsigned h) {
    unsigned wb = w * sizeof(pix_t);
    pix_t    *p = (pix_t*)pix;
    unsigned x,y;
    assert(pix != 0 && w != 0 && h != 0);
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w/2; ++x) {
            pix_t c  = p[x];
            p[x]     = p[w-1-x];
            p[w-1-x] = c;
        }
        p = (pix_t*)((unsigned char*)p + wb);
    }
}



/// 画像の上下反転
template<typename pix_t>
inline void pix_revY(pix_t* pix, unsigned w, unsigned h) {
    int         wb = w * sizeof(pix_t);
    pix_t       *p = (pix_t*)pix;
    pix_t       *q = (pix_t*)((unsigned char*)p + (h - 1) * wb);
    unsigned    x,y;
    assert(pix != 0 && w != 0 && h != 0);
    for (y = 0; y < h/2; ++y) {
        for (x = 0; x < w; ++x) {
            pix_t c = p[x];
            p[x]    = q[x];
            q[x]    = c;
        }
        p = (pix_t*)((unsigned char*)p + wb);
        q = (pix_t*)((unsigned char*)q - wb);
    }
}



/// 画像の180°反転
template<typename pix_t>
inline void pix_revXY(pix_t* pix, unsigned w, unsigned h) {
    unsigned    wb = w * sizeof(pix_t);
    pix_t       *p = (pix_t*)pix;
    pix_t       *q = (pix_t*)((unsigned char*)p + (h - 1) * wb);
    unsigned    x,y;
    assert(pix != 0 && w != 0 && h != 0);
    for (y = 0; y < h/2; ++y) {
        for (x = 0; x < w; ++x) {
            pix_t   c = p[x];
            p[x]      = q[w-1-x];
            q[w-1-x]  = c;
        }
        p = (pix_t*)((unsigned char*)p + wb);
        q = (pix_t*)((unsigned char*)q - wb);
    }
    if (h & 1) {
        for (x = 0; x < w/2; ++x) {
            pix_t   c = p[x];
            p[x]      = p[w-1-x];
            p[w-1-x]  = c;
        }
    }
}




/** 上下左右反転する
 *  dir: bit0:x反転   bit1:y反転
 */
template<typename pix_t>
inline void pix_revXY(pix_t *pix, unsigned w, unsigned h, unsigned dir)
{
    switch (dir & 3) {
    case 1: pix_revX (pix,w,h); break;
    case 2: pix_revY (pix,w,h); break;
    case 3: pix_revXY(pix,w,h); break;
    }
}


/** 左90°回転.
 */
template<typename pix_t>
inline void pix_rotL90(pix_t *dst, const pix_t* src, unsigned w, unsigned h)
{
    unsigned x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            dst[(w-1-x)*h+y] = src[y*w+x];
        }
    }
}



/** 右90°回転.
 */
template<typename pix_t>
inline void pix_rotR90(pix_t *dst, const pix_t* src, unsigned w, unsigned h)
{
    unsigned x, y;
    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x) {
            dst[x*h+(h-1-y)] = src[y*w+x];
        }
    }
}



/** ピクセル値を単純にビット反転.
 *  (２値画像向けに用意、だけど、他のビット色でも)
 */
template<typename pix_t>
void pix_bitCom(pix_t *pix, int w, int h)
{
    pix_t *p = (pix_t*)pix;
    int i;

    for (i = 0; i < w * h; ++i) {
        *p = ~(*p);
        p++;
    }
}




#endif // __cplusplus





//============================================================================
//============================================================================
//============================================================================

#if defined __cplusplus


/** 背景色colKey以外の部分が入る最小の矩形を求める.
 *  比較用ファンクタを使う版
 */
template<typename pix_t, class IsKey>
int pix_getRectWithoutCmpKey(pix_t *pix, unsigned width, unsigned height
                            , const IsKey& isKey
                            , int *x_p, int *y_p, int *w_p, int *h_p
                            , bool fuchi1=false)
{
    int w = (int)width;
    int h = (int)height;
    int x,y,x0,y0,x1,y1;

    if (pix == NULL || w <= 0 || h <= 0) {
        return 0;
    }

    x0 = y0 =  0x7fffffff;
    x1 = y1 = -0x7fffffff;

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if ( !isKey(pix[y*w+x]) ) {
                if (x0 > x) x0 = x;
                if (y0 > y) y0 = y;
                if (x1 < x) x1 = x;
                if (y1 < y) y1 = y;
            }
        }
    }

    if (x0 == 0x7fffffff) {     /* 背景色のみのとき */
        *w_p = 0;
        *h_p = 0;
        *x_p = 0;
        *y_p = 0;
    } else {
      #if 1 // 必ず淵１ドットに抜き色があるように調整
        if (fuchi1) {
            x0--, y0--, x1++, y1++;
            if (x0 < 0) x0 = 0;
            if (y0 < 0) y0 = 0;
            if (x1 >= w) x1 = w-1;
            if (y1 >= h) y1 = h-1;
        }
      #endif
        *x_p = x0;
        *y_p = y0;
        *w_p = x1+1 - x0;
        *h_p = y1+1 - y0;
    }
    return 1;
}



/// clut画の抜き色をフルカラーで指定したい場合用.
struct Pix_Cmp_Alpha0 {
    // α=0ならtrueを返す.
    bool operator()(unsigned argb) const {
        return (argb & 0xFF000000) == 0;
    }
};

/** α情報が0以外の矩形を求める.
 */
inline int pix32_getAlpRect(unsigned* pix, unsigned width, int height, int *x_p, int *y_p, int *w_p, int *h_p, bool fuchi1=false)
{
    return pix_getRectWithoutCmpKey(pix, width, height, Pix_Cmp_Alpha0(), x_p, y_p, w_p, h_p, fuchi1);
}



/// clut画の抜き色をフルカラーで指定したい場合用.
struct Pix_Cmp_Rgb_ClutNo {
    Pix_Cmp_Rgb_ClutNo(const unsigned* clut, unsigned colKeyRGB)
        : clut_(clut), colKeyRGB_(colKeyRGB & 0xFFFFFF) {;}

    // 抜き色と同じならばtrueを返す.
    bool operator()(unsigned clutNo) const {
        return colKeyRGB_ == (clut_[ clutNo ] & 0xFFFFFF);
    }

private:
    const unsigned* clut_;
    unsigned        colKeyRGB_;
};


/** 8bit色の画で、色colKeyRGBが抜き色として、抜き色以外の矩形を求める.
 */
inline int pix8_getRectWithoutColKeyRGB(unsigned char *pix, unsigned width, unsigned height
                , unsigned* clut, unsigned  colKeyRGB, int *x_p, int *y_p, int *w_p, int *h_p, bool fuchi1=false)
{
    return pix_getRectWithoutCmpKey(pix, width, height, Pix_Cmp_Rgb_ClutNo(clut, colKeyRGB) , x_p, y_p, w_p, h_p, fuchi1);
}




#endif // __cplusplus



#endif  // PIX_SUBR_H
