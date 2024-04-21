/**
 *  @file   pix32_kyuv.h
 *  @brief  rgb <=> yuv 変換.
 *  @author Masashi KITAMURA
 *  @note
 *      r,g,b は 各8bit前提.
 *      pix32_kyuv_t は rgb<=>yuv変換で誤差が出ないように y,u,v各 13ビット(前後)にして扱うための型(short * 4);
 */

#ifndef PIX32_KYUV_H
#define PIX32_KYUV_H


#ifndef PIX32_ARGB
/// 8bit値のa,r,g,bを繋げて 32bit の色値にする.
#define PIX32_ARGB(a,r,g,b)     ((((unsigned char)(a))<<24)|(((unsigned char)(r))<<16)|(((unsigned char)(g))<<8)|((unsigned char)(b)))

#define PIX32_GET_B(argb)       ((unsigned char)(argb))         ///< argb値中の blueの値を取得.
#define PIX32_GET_G(argb)       ((unsigned char)((argb)>>8))    ///< argb値中の greenの値を取得.
#define PIX32_GET_R(argb)       ((unsigned char)((argb)>>16))   ///< argb値中の greenの値を取得.
#define PIX32_GET_A(argb)       (((unsigned)(argb))>>24)        ///< argb値中の alphaの値を取得.
#endif

#ifndef PIX32_CLAMP
#define PIX32_CLAMP(c,mi,ma)    (((c) < (mi)) ? (mi) : ((ma) < (c)) ? (ma) : (c))
#endif

// r,g,b値から 各y,u,v を求め 係数を掛けるマクロ. (割算より先に掛算をしたいため、マクロに係数も含めている)
#ifndef PIX32_RGB_TO_YUV_Y_x_VAL
#define PIX32_RGB_TO_YUV_Y_x_VAL(aR__, aG__, aB__, Val__)   ((Val__) * (38444U * (aG__) + 19589U * (aR__) +  7503U * (aB__)               ) / 65536)
#define PIX32_RGB_TO_YUV_U_x_VAL(aR__, aG__, aB__, Val__)   ((Val__) * (-21709 * (aG__) + -11059 * (aR__) +  32768 * (aB__) +  255*65536/2) / 65536)
#define PIX32_RGB_TO_YUV_V_x_VAL(aR__, aG__, aB__, Val__)   ((Val__) * (-27439 * (aG__) +  32768 * (aR__) +  -5329 * (aB__) +  255*65536/2) / 65536)
#endif

// y,u,v値から 各r,g,b を求めるマクロ. u,vは 予め -128 してあること.
#ifndef PIX32_YUV_TO_RGB_G
#define PIX32_YUV_TO_RGB_G(kY__, kU__, kV__)                (((int)( 65536 * (kY__) + -22554 * (kU__) + -46802 * (kV__)) + 65536 / 2) >> 16)
#define PIX32_YUV_TO_RGB_R(kY__, kU__, kV__)                (((int)( 65536 * (kY__)                   +  91881 * (kV__)) + 65536 / 2) >> 16)
#define PIX32_YUV_TO_RGB_B(kY__, kU__, kV__)                (((int)( 65536 * (kY__) + 116130 * (kU__)                  ) + 65536 / 2) >> 16)
#endif

// 複数yuv画素合計の平均を rgb 値にするマクロ.
#define PIX32_RGB_FROM_TOTAL_YUV(dstRgba__, srcKY__, srcKU__, srcKV__, srcA__, totalSz__, TYPE__) do { \
        TYPE__  wkY__   = (srcKY__) / (totalSz__);                          \
        TYPE__  wkU__   = (srcKU__) / (totalSz__) - 128;                    \
        TYPE__  wkV__   = (srcKV__) / (totalSz__) - 128;                    \
        int     iG__    = PIX32_YUV_TO_RGB_G(wkY__, wkU__, wkV__);          \
        int     iR__    = PIX32_YUV_TO_RGB_R(wkY__, wkU__, wkV__);          \
        int     iB__    = PIX32_YUV_TO_RGB_B(wkY__, wkU__, wkV__);          \
        int     iA__    = ((srcA__) + (totalSz__)/2) / (totalSz__);         \
        if (iA__ > 255) iA__ = 255;                                         \
        iG__ = PIX32_CLAMP(iG__, 0, 255);                                   \
        iR__ = PIX32_CLAMP(iR__, 0, 255);                                   \
        iB__ = PIX32_CLAMP(iB__, 0, 255);                                   \
        (dstRgba__) = PIX32_ARGB(iA__,iR__,iG__,iB__);                      \
    } while (0)

/// y,u,v 各 13bit(前後)にした値を保持するための型. (略称kyuv).
typedef union pix32_kyuv_t {
    //uint64_t  pack;
    short       at[4];      // [0-2] y,u,v 0.. K*255. [3] a 0..255
} pix32_kyuv_t;

enum {
    PIX32_KYUV_BIT     = 13,      // 11 .. 14 を想定 (8..10 bitだとrgbに戻す時の誤差がおきやすい)
    PIX32_KYUV_MAX     = (1 << PIX32_KYUV_BIT) - 1,
    PIX32_KYUV_SHIFT_K = PIX32_KYUV_BIT - 8,
    PIX32_KYUV_MUL_K   = 1 << PIX32_KYUV_SHIFT_K,
};

// r,g,b値から 13bit(前後) な y,u,v値を求めるマクロ.
#define PIX32_RGB_TO_KYUV_Y(aR__, aG__, aB__)       ((PIX32_KYUV_MUL_K * 38444U * (aG__) + PIX32_KYUV_MUL_K * 19589U * (aR__) +  PIX32_KYUV_MUL_K * 7503U * (aB__)                                ) >> 16)
#define PIX32_RGB_TO_KYUV_U(aR__, aG__, aB__)       ((PIX32_KYUV_MUL_K * -21709 * (aG__) + PIX32_KYUV_MUL_K * -11059 * (aR__) +  PIX32_KYUV_MUL_K * 32768 * (aB__) +  PIX32_KYUV_MUL_K*255*65536/2) >> 16)
#define PIX32_RGB_TO_KYUV_V(aR__, aG__, aB__)       ((PIX32_KYUV_MUL_K * -27439 * (aG__) + PIX32_KYUV_MUL_K *  32768 * (aR__) +  PIX32_KYUV_MUL_K * -5329 * (aB__) +  PIX32_KYUV_MUL_K*255*65536/2) >> 16)

// 13bit(前後) な y,u,v値から r,g,b値を求めるマクロ.
#define PIX32_KYUV_TO_RGB_G(kY__, kU__, kV__)       (((int)( 65536 * (kY__) + -22554 * (kU__) + -46802 * (kV__)) + PIX32_KYUV_MUL_K * 65536 / 2) >> (PIX32_KYUV_SHIFT_K + 16))
#define PIX32_KYUV_TO_RGB_R(kY__, kU__, kV__)       (((int)( 65536 * (kY__)                   +  91881 * (kV__)) + PIX32_KYUV_MUL_K * 65536 / 2) >> (PIX32_KYUV_SHIFT_K + 16))
#define PIX32_KYUV_TO_RGB_B(kY__, kU__, kV__)       (((int)( 65536 * (kY__) + 116130 * (kU__)                  ) + PIX32_KYUV_MUL_K * 65536 / 2) >> (PIX32_KYUV_SHIFT_K + 16))

#define PIX32_KYUV_U_TO_Cb(u)                       ((u) - 128 * PIX32_KYUV_MUL_K)
#define PIX32_KYUV_V_TO_Cr(v)                       ((v) - 128 * PIX32_KYUV_MUL_K)


/// kyuv 値を rgb 値に変換して代入するマクロ. (誤差の扱いのため計算中で使う変数の型を引数で指定)
#define PIX32_RGB_FROM_KYUV(dstRgba__, srcKY__, srcKU__, srcKV__, srcA__, TYPE__) do {  \
        TYPE__  iKY__ = (srcKY__);                              \
        TYPE__  iKU__ = (srcKU__) - PIX32_KYUV_MUL_K * 128;     \
        TYPE__  iKV__ = (srcKV__) - PIX32_KYUV_MUL_K * 128;     \
        int iG__ = PIX32_KYUV_TO_RGB_G(iKY__, iKU__, iKV__);    \
        int iR__ = PIX32_KYUV_TO_RGB_R(iKY__, iKU__, iKV__);    \
        int iB__ = PIX32_KYUV_TO_RGB_B(iKY__, iKU__, iKV__);    \
        iG__ = PIX32_CLAMP(iG__, 0, 255);                       \
        iR__ = PIX32_CLAMP(iR__, 0, 255);                       \
        iB__ = PIX32_CLAMP(iB__, 0, 255);                       \
        (dstRgba__) = PIX32_ARGB((srcA__),iR__,iG__,iB__);      \
    } while (0)

/// 複数kyuv画素合計の平均を rgb 値に変換して代入するマクロ. (誤差の扱いのため計算中で使う変数の型を引数で指定)
#define PIX32_RGB_FROM_TOTAL_KYUV(dstRgba__, srcKY__, srcKU__, srcKV__, srcA__, totalSz__, TYPE__) do { \
        TYPE__  dblY__  = (srcKY__) / (totalSz__);                          \
        TYPE__  dblU__  = (srcKU__) / (totalSz__) - PIX32_KYUV_MUL_K * 128; \
        TYPE__  dblV__  = (srcKV__) / (totalSz__) - PIX32_KYUV_MUL_K * 128; \
        int     iG__    = PIX32_KYUV_TO_RGB_G(dblY__, dblU__, dblV__);      \
        int     iR__    = PIX32_KYUV_TO_RGB_R(dblY__, dblU__, dblV__);      \
        int     iB__    = PIX32_KYUV_TO_RGB_B(dblY__, dblU__, dblV__);      \
        int     iA__    = ((srcA__) + (totalSz__)/2) / (totalSz__);         \
        if (iA__ > 255) iA__ = 255;                                         \
        iG__ = PIX32_CLAMP(iG__, 0, 255);                                   \
        iR__ = PIX32_CLAMP(iR__, 0, 255);                                   \
        iB__ = PIX32_CLAMP(iB__, 0, 255);                                   \
        (dstRgba__) = PIX32_ARGB(iA__,iR__,iG__,iB__);                      \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif

void    pix32_kyuvFromRgb(pix32_kyuv_t *dst, unsigned dstW, unsigned dstH, unsigned ofsX, unsigned ofsY, const unsigned *src, unsigned srcW, unsigned srcH);

/// ratio=0.0～1.0 の率で、画像の輝度を変更.
void    pix32_changeTone(unsigned *pix, unsigned w, unsigned h, double ratio);


#ifdef __cplusplus
}
#endif

#endif
