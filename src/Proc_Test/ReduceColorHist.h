/**
 *  @file   ReduceColorHist.h
 *  @brief  適当に頻度数えたりしての減色.
 *  @author Masashi KITAMURA
 *  @note
 *      いろいろ破綻してるけど、とりあえず動いてるので残す.
 */

#ifndef REDUCECOLORHIST_H
#define REDUCECOLORHIST_H

#pragma once

#include "def.h"



/// 適当に頻度数えたりしての減色処理.
class ReduceColorHist {
public:
    ReduceColorHist() {; }
    ~ReduceColorHist(){;}

    /** clut化向けに、画像の色数をcolNumになるまでカウント.
     *  clut0が指定されていたら、色を入れて返す.
     */
    static unsigned countColor(const unsigned *pix, unsigned w, unsigned h, unsigned colNum=257, bool transparentFlg=false, unsigned* clut0=NULL) {
        //
        if (colNum <= 0)
            return 0;
        unsigned*   clut = (clut0) ? clut0 : new unsigned[ colNum ];
        memset(clut, 0xFF, (sizeof clut[0]) * colNum);

        unsigned n = 0;

        if (transparentFlg) // 0番に透明色を必ず割り当てる場合.
            n = 1;

        for (unsigned i = 0; i < w * h && n < colNum; ++i) {
            insert_tbl_n(clut, n, pix[i]);
        }

        if (clut0 == NULL)
            delete clut;
        return n;
    }


    /** 32ビット色を clut に従って近い色に置換. αも単純比較.
     *  ※ 32ビット色画が、実際にはclutSize色以内の場合の変換のために用意.
     */
    static void convPix32to8(unsigned char *dst, unsigned *src, int w, int h, unsigned *clut, unsigned clutSize) {
        unsigned *s     = (unsigned*)src;
        unsigned char  *d     = (unsigned char*)dst;

        for (unsigned n = 0; n < w * h; ++n) {
            unsigned ii = 0;
            unsigned c  = s[n];
            unsigned k  = 0xFFFFFFFF;
            for (unsigned i = 0; i < clutSize; ++i) {
                unsigned    ic = clut[i];
                int         a  = ARGB_A(ic) - ARGB_A(c);
                int         r  = ARGB_R(ic) - ARGB_R(c);
                int         g  = ARGB_G(ic) - ARGB_G(c);
                int         b  = ARGB_B(ic) - ARGB_B(c);
                unsigned    ik = a*a + r*r + g*g + b*b;
                if (ik < k) {
                    k  = ik;
                    ii = i;
                }
            }
            d[n] = ii;
        }
    }



    /** 32ビット色を clut に従って近い色に置換.
     *  αは、抜き色１色のみ考慮. minAの上下で、不透明、透明、のどちらかに強制。
     *  ※ ReduceColorMCでα対応したので、こちらは、単純な処理のみに。
     */
    static int convPix32to8_na(unsigned char *dst, unsigned *src, int w, int h, unsigned *clut, unsigned clutSize, unsigned clutOfs, int minA) {
        unsigned *s     = (unsigned*)src;
        unsigned char  *d     = (unsigned char*)dst;

        // 透明(α)がclutにあるか探す.
        unsigned indA   = 0;
        unsigned n;
        for (n = 0; n < clutSize; ++n) {
            if (ARGB_A(clut[n]) == 0) {
                indA = n;
                break;
            }
        }

        bool no_transparent = (n == clutSize);  // 透明がなかったかどうか

        for (n = 0; n < w * h; ++n) {
            unsigned ii = indA;
            unsigned c  = s[n];
            unsigned a  = ARGB_A(c);
            if (a > minA || no_transparent) {   // 透明以外のピクセルだったとき.
                unsigned k = 0xFFFFFFFF;
                for (unsigned i = clutOfs; i < clutSize; ++i) {
                    unsigned    ic = clut[i];
                    if (ARGB_A(ic) <= minA) // clutのαが透明なら、それは対象外！
                        continue;
                    int         r = ARGB_R(ic) - ARGB_R(c);
                    int         g = ARGB_G(ic) - ARGB_G(c);
                    int         b = ARGB_B(ic) - ARGB_B(c);
                    unsigned    ik = /* a*a+ */ r*r + g*g + b*b;
                    if (ik < k) {
                        k  = ik;
                        ii = i;
                    }
                }
            }
            d[n] = ii;
        }
        return 1;
    }


    /** デフォルトパレットの作成.
     * bpp : 3,4,6,8   typ : 0=jpn 1=pcat/win
     */
    void getPcDfltClut(void *clut0, int bpp, int typ) {
        static const int jpn8[] = {
            0x000000,   0xff0000,   0x00ff00,   0xffff00,   0x0000ff,   0xff00ff,   0x00ffff,   0xffffff,
        };
        static const int jpn16[] = {
            0x000000,   0x0000ff,   0xff0000,   0xff00ff,   0x00ff00,   0x00ffff,   0xffff00,   0xffffff,
            0x808080,   0x000080,   0x800000,   0x800080,   0x008000,   0x008080,   0x808000,   0xc0c0c0,
        };
        static const int win8[] = {
            0x000000,   0xff0000,   0x00ff00,   0xffff00,   0x0000ff,   0xff00ff,   0x00ffff,   0xffffff,
        };
        static const int win16[] = {
            0x000000,   0x800000,   0x008000,   0x808000,   0x000080,   0x800080,   0x008080,   0xc0c0c0,
            0x808080,   0xff0000,   0x00ff00,   0xffff00,   0x0000ff,   0xff00ff,   0x00ffff,   0xffffff,
        };
        static const int win256[] = {
            0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0x808080, 0xc0dcc0, 0xa6caf0, 0x2a3faa, 0x2a3fff, 0x2a5f00, 0x2a5f55, 0x2a5faa, 0x2a5fff,
            0x2a7f00, 0x2a7f55, 0x2a7faa, 0x2a7fff, 0x2a9f00, 0x2a9f55, 0x2a9faa, 0x2a9fff, 0x2abf00, 0x2abf55, 0x2abfaa, 0x2abfff, 0x2adf00, 0x2adf55, 0x2adfaa, 0x2adfff,
            0x2aff00, 0x2aff55, 0x2affaa, 0x2affff, 0x550000, 0x550055, 0x5500aa, 0x5500ff, 0x551f00, 0x551f55, 0x551faa, 0x551fff, 0x553f00, 0x553f55, 0x553faa, 0x553fff,
            0x555f00, 0x555f55, 0x555faa, 0x555fff, 0x557f00, 0x557f55, 0x557faa, 0x557fff, 0x559f00, 0x559f55, 0x559faa, 0x559fff, 0x55bf00, 0x55bf55, 0x55bfaa, 0x55bfff,
            0x55df00, 0x55df55, 0x55dfaa, 0x55dfff, 0x55ff00, 0x55ff55, 0x55ffaa, 0x55ffff, 0x7f0000, 0x7f0055, 0x7f00aa, 0x7f00ff, 0x7f1f00, 0x7f1f55, 0x7f1faa, 0x7f1fff,
            0x7f3f00, 0x7f3f55, 0x7f3faa, 0x7f3fff, 0x7f5f00, 0x7f5f55, 0x7f5faa, 0x7f5fff, 0x7f7f00, 0x7f7f55, 0x7f7faa, 0x7f7fff, 0x7f9f00, 0x7f9f55, 0x7f9faa, 0x7f9fff,
            0x7fbf00, 0x7fbf55, 0x7fbfaa, 0x7fbfff, 0x7fdf00, 0x7fdf55, 0x7fdfaa, 0x7fdfff, 0x7fff00, 0x7fff55, 0x7fffaa, 0x7fffff, 0xaa0000, 0xaa0055, 0xaa00aa, 0xaa00ff,
            0xaa1f00, 0xaa1f55, 0xaa1faa, 0xaa1fff, 0xaa3f00, 0xaa3f55, 0xaa3faa, 0xaa3fff, 0xaa5f00, 0xaa5f55, 0xaa5faa, 0xaa5fff, 0xaa7f00, 0xaa7f55, 0xaa7faa, 0xaa7fff,
            0xaa9f00, 0xaa9f55, 0xaa9faa, 0xaa9fff, 0xaabf00, 0xaabf55, 0xaabfaa, 0xaabfff, 0xaadf00, 0xaadf55, 0xaadfaa, 0xaadfff, 0xaaff00, 0xaaff55, 0xaaffaa, 0xaaffff,
            0xd40000, 0xd40055, 0xd400aa, 0xd400ff, 0xd41f00, 0xd41f55, 0xd41faa, 0xd41fff, 0xd43f00, 0xd43f55, 0xd43faa, 0xd43fff, 0xd45f00, 0xd45f55, 0xd45faa, 0xd45fff,
            0xd47f00, 0xd47f55, 0xd47faa, 0xd47fff, 0xd49f00, 0xd49f55, 0xd49faa, 0xd49fff, 0xd4bf00, 0xd4bf55, 0xd4bfaa, 0xd4bfff, 0xd4df00, 0xd4df55, 0xd4dfaa, 0xd4dfff,
            0xd4ff00, 0xd4ff55, 0xd4ffaa, 0xd4ffff, 0xff0055, 0xff00aa, 0xff1f00, 0xff1f55, 0xff1faa, 0xff1fff, 0xff3f00, 0xff3f55, 0xff3faa, 0xff3fff, 0xff5f00, 0xff5f55,
            0xff5faa, 0xff5fff, 0xff7f00, 0xff7f55, 0xff7faa, 0xff7fff, 0xff9f00, 0xff9f55, 0xff9faa, 0xff9fff, 0xffbf00, 0xffbf55, 0xffbfaa, 0xffbfff, 0xffdf00, 0xffdf55,
            0xffdfaa, 0xffdfff, 0xffff55, 0xffffaa, 0xccccff, 0xffccff, 0x33ffff, 0x66ffff, 0x99ffff, 0xccffff, 0x007f00, 0x007f55, 0x007faa, 0x007fff, 0x009f00, 0x009f55,
            0x009faa, 0x009fff, 0x00bf00, 0x00bf55, 0x00bfaa, 0x00bfff, 0x00df00, 0x00df55, 0x00dfaa, 0x00dfff, 0x00ff55, 0x00ffaa, 0x2a0000, 0x2a0055, 0x2a00aa, 0x2a00ff,
            0x2a1f00, 0x2a1f55, 0x2a1faa, 0x2a1fff, 0x2a3f00, 0x2a3f55, 0xfffbf0, 0xa0a0a4, 0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff,
        };



        static const int clutSz[] = {
            8*4, 16*4, 64*4, 256*4
        };

        static const int *clutTbl[2][4] = {
            {jpn8, jpn16, NULL, NULL},
            {win8, win16, NULL, win256},
        };

        int *clut = (int*)clut0;
        int r,g,b;

        int i = (bpp == 3) ? 0 : (bpp == 4) ? 1 : (bpp == 6) ? 2 : 3;

        if (clutTbl[typ][i]) {
            for (int j = 0; j < 256; ++j)
                clut[j] = 0x00000000;   // 0xFF000000;
            for (int j = 0; j < clutSz[i]/4; j++) {
                clut[j] = clutTbl[typ][i][j] | 0xFF000000;
            }

        } else {
            if (bpp == 6) {
                for (i = 0; i < 256; ++i)
                    clut[i] = 0x00000000;   // 0xFF000000;
                for (i = 0; i < 64; i++) {
                    r = (i & 0xc) >> 2;
                    r = (r << 6) | (r << 4) | (r << 2) | r;
                    g = (i & 0x30) >> 4;
                    g = (g << 6) | (g << 4) | (g << 2) | g;
                    b = i & 3;
                    b = (b << 6) | (b << 4) | (b << 2) | b;
                    clut[i] = argb(0xff, r,g,b);
                }
            } else {    // bpp = 8
                for (i = 0; i < 256; i++) {
                    r = ((i & 0x1c) << 3);
                    r = r | (r >> 3) | (r >> 6);
                    g = i & 0xE0;
                    g = g | (g >> 3) | (g >> 6);
                    b = i & 3;
                    b = (b << 6) | (b << 4) | (b << 2) | b;
                    clut[i] = argb(0xff, r,g,b);
                }
            }
        }
        //clut[0] &= 0xFFFFFF;
    }



    /** 減色
     */
    int getClut(const void *pix0, int w, int h, void *clut0, int clutNum, int alpFlg, int minA) {
        //
        unsigned *clut        = (unsigned*)clut0;
        const unsigned *pix   = (const unsigned*)pix0;
        unsigned *col64    = NULL;
        unsigned *hst64    = NULL;
        unsigned *col64b   = NULL;
        unsigned *hst64b   = NULL;
        unsigned (*hst)[2] = NULL;
        int      sz;
        int      ctop = (alpFlg > 0);
        unsigned cn   = ctop;
        int      r,g,b;
        int      i;
        int      c;
        int      rc=0;

        if (alpFlg == 0)
            minA = -1;
        assert(alpFlg == 0 || alpFlg == 1);
        if (dbgExLog_getSw()) printf("dicol_getClut %d*%d clutNum:%d alpFlg:%d\n", w, h, clutNum, ctop);
        //if (dbgExLog_getSw()) for (i = 0; i < clutNum; i++) printf("clut[%d]=%#x\n", i, clut[i]);
        DBG_M();
        if (clutNum < 2) {  // 2色未満は処理できねー
            ERR_PRINTF(("2色未満には減色できない\n"));
            return 0;
        }

        // まず、素の色数をカウント
        DBG_M();
        //memset(clut, 0x00, (sizeof clut[0]) * clutNum);
        for (i = 0; i < clutNum; ++i)
            clut[i] = 0xFF000000;
        unsigned *clutTmp = (unsigned*)callocE(sizeof(unsigned), clutNum+16);
        for (i = 0; i < w * h && cn < clutNum+1; i++) {
            insert_tbl_n(clutTmp, cn, pix[i]);
        }

        DBG_PRINTF(("getClut: wh:%d,%d  clutNum:%d alpFlg:%d  colNum=%d\n", w,h, clutNum, ctop, cn));

        // 抜き色チェック
        DBG_M();

        if (cn <= clutNum) {    // 元より clutNum 以下の色数だった.
            rc = 0x101;
            memcpy(clut, clutTmp, sizeof(unsigned)*clutNum);
            DBG_PRINTF(("元より%d色以下の色数(%d)なので詰め合わせるだけ\n",clutNum, cn));
            goto RET;
        }

        //
        enum {LB=5, LN = 1<<LB, LM=LN-1, LR=8-LB};
        // 作業メモリ確保
        col64   = (unsigned*)callocE(sizeof(unsigned),64);
        hst64   = (unsigned*)callocE(sizeof(unsigned),64);
        col64b  = (unsigned*)callocE(sizeof(unsigned),64);
        hst64b  = (unsigned*)callocE(sizeof(unsigned),64);
        hst     = (unsigned(*)[2])callocE(2*sizeof(unsigned), LN*LN*LN+4);
        DBG_M();

        // 各ピクセルの数を数える
        for (i = 0; i < w*h; i++) {
            c = pix[i];
            if ((int)((unsigned)c >> 24) >= minA) {     // α >= m(0 or 1 or 0xf0) が成立すれば
                c = (((c>>(16+LR))&LM)<<LB) | (((c>>(8+LR))&LM)<<(2*LB)) | ((c>>LR)&LM);
                hst[c][0]++;
            }
        }
        DBG_M();

        // 基本色数を選ぶ
        if ((clutNum-ctop) < 8) { // 2～7色なら濃淡にする
            int d,y,iy=0xff, my=0;
            for (i = 0; i < LN*LN*LN; i++) {
                c = hst[i][0];
                if (c) {
                    g = (i>>(LB+LB))&LM, r =(i>>LB)&LM, b = (i)&LM;
                    y = ((9*g + r*5 + b*2) << LR) >> 4;
                    if (y < iy) iy = y;
                    if (y > my) my = y;
                }
            }
            if (iy > my)
                iy = 0, my = 0xff;
            cn = clutNum - ctop;
            d = (my - iy) / cn;
            for (i = 0; i < cn; i++) {
                c = iy + d * i;
                clut[ctop+i] = argb(0xFF,c,c,c);
            }
            clut[clutNum-1] = argb(0xFF,my,my,my);
            rc = clutNum;
            goto RET;

        } else if ((clutNum-ctop) < 64) {
            // 8～63色の減色ならば、まず基本 8色を選ぶ
            unsigned sr[8],sg[8],sb[8],sm[8];
            DBG_M();
            for (i = 0; i < 8; i++)
                sr[i]=sg[i]=sb[i]=sm[i]=0;
            for (i = 0; i < LN*LN*LN; i++) {
                hst[i][1] = i;
                int m = hst[i][0];
                if (m) {
                    //clut[cn++] = i;
                    g=(i>>(3*LB-1))&1, r=(i>>(2*LB-1))&1, b=(i>>(LB-1))&1;
                    c = g<<2|r<<1|b;
                    sg[c] += ((i>>2*LB)&LM)*m;
                    sr[c] += ((i>>1*LB)&LM)*m;
                    sb[c] += ((i      )&LM)*m;
                    sm[c] += m;
                }
            }
            cn = ctop;
            for (i = 0; i < 8; i++) {
                sz = sm[i];
                if (sz == 0)
                    sz = 1;
                g = (sg[i]<<LR) / sz; if (g > 0xff) g = 0xff;
                r = (sr[i]<<LR) / sz; if (r > 0xff) r = 0xff;
                b = (sb[i]<<LR) / sz; if (b > 0xff) b = 0xff;
                clut[cn] = argb(0xFF,r,g,b);
                DBG_PRINTF(("base clut[%d] = %x sz=0x%x s(%x,%x,%x)\n", cn, clut[cn],sz,r,g,b));
                cn++;
            }
            DBG_M();

        } else {
            // 64色以上への減色ならば、まず基本 64色を選ぶ
            cn = ctop;
            for (i = 0; i < LN*LN*LN; i++) {
                hst[i][1] = i;
                if (hst[i][0]) {
                    if (cn < clutNum)
                        clutTmp[cn] = i;
                    cn++;
                    g=(i>>(3*LB-2))&3, r=(i>>(2*LB-2))&3, b=(i>>(LB-2))&3;
                    c = g<<4|r<<2|b;
                    if (hst[i][0] >= hst64[c]) {
                        col64b[c] = col64[c];
                        hst64b[c] = hst64[c];
                        col64[c]  = i;
                        hst64[c]  = hst[i][0];
                    }
                }
            }
            DBG_M();
            // 色数が clutNum以下しかなければ、そのままclut化
            if (cn <= clutNum) {
                DBG_M();
                for (i = ctop; i < cn; i++) {
                    c = clutTmp[i];
                    g = (c >> (2*LB-LR)) & (LM<<LR); g |= g >> (LB);
                    r = (c >> (1*LB-LR)) & (LM<<LR); r |= r >> (LB);
                    b = (c << LR)        & (LM<<LR); b |= b >> (LB);
                    clut[i] = argb(0xFF,r,g,b);
                }
                DBG_M();
                rc = clutNum;
                goto RET;
            }
            DBG_M();

            // clutテーブルに登録
            cn = ctop;
            for (i = 0; i < 64; i++) {
                if (hst64[i]) {
                    c = col64[i];
                    g = (c >> (2*LB-LR)) & (LM<<LR); g |= g >> (LB);
                    r = (c >> (1*LB-LR)) & (LM<<LR); r |= r >> (LB);
                    b = (c << LR)        & (LM<<LR); b |= b >> (LB);
                    clut[cn++] = argb(0xFF,r,g,b);
                    hst[c][0] = 0;
                }
            }
            DBG_M();
            if ((clutNum-ctop) >= 196) {    // 196色以上あるなら、第二候補も登録しちゃう
                for (i = 0; i < 64; i++) {
                    if (hst64b[i]) {
                        c = col64b[i];
                        g = (c >> (2*LB-LR)) & (LM<<LR); g |= g >> (LB);
                        r = (c >> (1*LB-LR)) & (LM<<LR); r |= r >> (LB);
                        b = (c << LR)        & (LM<<LR); b |= b >> (LB);
                        clut[cn++] = argb(0xFF,r,g,b);
                        hst[c][0] = 0;
                    }
                }
            }
            DBG_M();
        }

        // clutがすべて埋まっていたら終了
        if (cn >= clutNum) {
            rc = clutNum;
            goto RET;
        }

        DBG_M();
        // 頻度の高い順に並べて、残りの色を選ぶ
        qsort(hst, LN*LN*LN, sizeof(hst[0]), dicol_cmp1);   // 多いもの順に並べる

        DBG_M();
        for (; cn < clutNum; cn++) {
            c = hst[cn][1];
            g = (c >> (2*LB-LR)) & (LM<<LR); g |= g >> (LB);
            r = (c >> (1*LB-LR)) & (LM<<LR); r |= r >> (LB);
            b = (c << LR)        & (LM<<LR); b |= b >> (LB);
            clut[cn] = argb(0xFF,r,g,b);
        }

      RET:
        // 終了
        qsort(clut+ctop, clutNum - ctop, 4, clutSortCmp);
        DBG_M();

        if (hst)     free(hst);
        if (hst64)   free(hst64);
        if (hst64b)  free(hst64b);
        if (col64)   free(col64);
        if (col64b)  free(col64b);
        if (clutTmp) free(clutTmp);
        return rc;
    }


private:

    /// 8bit数4つを上位から順につなげて32ビット数にする
    static unsigned argb(unsigned char a, unsigned char r, unsigned char g, unsigned char b) {
        return (a << 24) | (r << 16) | (g << 8) | (b);
    }


    /** clut のソートのための比較関数 */
    static int __cdecl clutSortCmp(const void *a0, const void *b0)
    {
        unsigned a = *(unsigned *)a0;
        unsigned b = *(unsigned *)b0;
        if (a < b)
            return -1;
        else if (a > b)
            return 1;
        return 0;
    }


    /// 減色で頻度の高いモノ順に並べるための比較関数
    static int __cdecl dicol_cmp1(const void *a0, const void *b0)
    {
        const unsigned *a = (unsigned*)a0;
        const unsigned *b = (unsigned*)b0;

        if (a[0] > b[0])
            return -1;
        else if (a[0] < b[0])
            return 1;
        if (a[1] > b[1])
            return -1;
        else if (a[1] < b[1])
            return 1;
        return 0;
    }



    /** テーブルpTblに値keyを追加. 範囲チェックは予め行っていること前提！
     *  @return テーブル中のkeyの位置.
     */
    template<typename T>
    static unsigned insert_tbl_n(T pTbl[], unsigned& rNum, const T& key) {
        unsigned    hi  = rNum;
        unsigned    low = 0;
        unsigned    mid = 0;
        while (low < hi) {
            mid = (low + hi - 1) / 2;
            if (key < pTbl[mid]) {
                hi = mid;
            } else if (pTbl[mid] < key) {
                ++mid;
                low = mid;
            } else {
                return mid; /* 同じものがみつかったので追加しない */
            }
        }

        // 新規登録
        ++rNum;

        // 登録箇所のメモリを空ける
        for (hi = rNum; --hi > mid;) {
            pTbl[hi] = pTbl[hi-1];
        }

        // 登録
        pTbl[mid] = key;
        return mid;
    }


};


#endif
