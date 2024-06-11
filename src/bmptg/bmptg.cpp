/**
 *  @file   bmptg.cpp
 *  @brief  画像コンバータ.
 *  @author Masashi Kitamura
 *  @date   2000-2024
 *  @note
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#endif

//#include "fsys.hpp"
#include "subr.h"
#include "ExArgv.h"
#include "ConvOne.hpp"
#include "../Proc/PaternDither.hpp"
#ifdef USE_MY_FMT
#include "my.h"
#else
#define MY_USAGE_1
#define MY_USAGE_2
#endif

char *g_appName;

using namespace std;


int usage(void)
{
    printf("https://github.com/tenk-a/bmptg/\n"
           "usage> %s [-opts] file(s)   // v2.40 " __DATE__ "  by tenk*\n", g_appName);
    printf(
       "  bmp tga jpg png 等画像を相互に変換.\n"
       "  α値は有効で00:透明～0xFF:不透明. α無画は0xFFとして抜色はα=0として処理.\n"
       MY_USAGE_1
       "\n"
       "  :FMT          出力フォーマットをFMT(bmp tga jpg png bin non)に.\n"
       "                bin はヘッダ&clut無ピクセルのみのベタ出力.\n"
       "                non は経過メッセージのみで、ファイル変換無.\n"
       "  -f[FMT]       :FMTに同じ\n"
       "  -b[N]         出力をN bit色に(1,4,8,12,15,16,24,32).\n"
       "                (下位ビット切捨なので、必要なら-xdでディザ減色を)\n"
       "  -v-           処理メッセージを減らす.\n"
       "\n"
       " [ファイル名]\n"
       "  -o[FILE]      1ファイル変換での出力名.\n"
       "  -d[DIR]       出力ディレクトリをDIRに.\n"
       "  -s[DIR]       入力ディレクトリをDIRに.\n"
       "  -ed[EXT]      出力の拡張子を EXT に.\n"
       "  -es[EXT]      入力の省略時拡張子をEXTに.\n"
       "  -u            日付が新しければ変換.\n"
       "\n"
       "  [バイナリ入力]\n"
       "  -ig[W:H:B:E]  Bビット色W*Hのベタ画(またはバイナリファイル)入力.\n"
       "  -is[N]        入力FILE先頭N byteスキップ.\n"
       "\n"
       " [色関係]\n"
       "  -if           clut画をフルカラー(24or32ビット色)にして入力.\n"
       "  -ib[N]        入力時(他の変換をする前)に、N(3～24) bit色に変換.\n"
       "                24 = R8G8B8    23 = R8G8B7   22 = R7G8B7\n"
       "                21 = R7G7B7    ...........    3 = R1G1B1\n"
       "  -xd[N:T]      ディザでN(3～24)bit色に減色. (T:0-7)\n"
       "                 N= 指定方法は -ib[N] に同じ\n"
       "                 T=  0:ディザ無し\n"
       "                     1:パターンディザ 2x2\n"
       "                     2:パターンディザ 4x4\n"
       "                     3:パターンディザ 8x8\n"
       //x "                    +4: 誤差拡散する(実験:現状バグってる模様?)\n"
       "                 +0x80:A,G と R,B とでディザマトリックスを対にする\n"
       "                +0x100:αもディザする. 有効ビット数はGに同じ\n"
       "\n"
       "  -cq[ARGB]     ARGB を、BGRA,AGRB 等の順に並び換.\n"
       "  -t[N]         RGB値を N%% にして出力.\n"
       "  -tt[N]        輝度を N%% にして出力.\n"
       "  -cm[C]        各ピクセルに色Cを乗ずる.\n"
       "  -cem          R,G,B,A値各々を二乗する.\n"
       "  -cs[A:R:G:B]  実数指定でrgbピクセルの各値を何倍するかを指定\n"
       "  -ct[A:Y:U:V]  実数指定でyuvピクセルの各値を何倍するかを指定\n"
       "  -cg           モノクロ化\n"
       "  -cgc[R|G|B|A] R,G,B,A いずれかのチャンネルを用いてモノクロ化.\n"
       "  -cf[C]        抜色を C (省略時000000)に.\n"
       "  (-ca[C]       -cf拡張で抜色周辺αぼかし. 実験物)\n"
       "  -cfn[C]       -cfにほぼ同じだが α=0にしない(-vvでの埋め色)\n"
       "  -an           入力画α値を無視(α無扱い).\n"
       "  -ao           αチャンネルをmask画像化したファイルを出力.\n"
       "  -ai[FILE]     FILEをαチャンネルに読み込む\n"
       "  -ari          入力α反転.\n"
       "  -aro          出力α反転.\n"
       "  -am[A:B]      減色α関係の変換で用いるα範囲[A,B]. A以下は透明,B以上は不透明.\n"
       "  -ac[C:C2]     ピクセルと色Cをαブレンドしα=0 or 0xFF. C2指定はα=0時のRGB値.\n"
       "  -ag[RATE:OFS] ピクセルRGBよりモノクロ値を求めそれをαとする.\n"
       "                 RATE,OFSがあれば色の変換 (r,g,b)*RATE+OFS を先に行う\n"
       "  -ad           抜き色と非抜き色の堺のαをぼかす簡易処理(仮BU作成向)\n"
       "                (v2.38以前は -ca -xca で機能だったのを分離)\n"
       "  -az           α=0ならRGBも0にする\n"
       "\n"
       " [clut関係]\n"
       "  -cpc[FILE]    clut内容をc用テキスト出力.\n"
       "  -cpf[FILE]    減色で用いる固定clut.\n"
       "  -cp[N]        固定clut設定or減色アルゴリズム指定.\n"
       "                1:jp固定clut             2:winシステムclut.\n"
       "                3:メディアンカット(yuv)  4:メディアンカット(rgb)\n"
       "                5:頻度順\n"
       "                Nの指定のない場合は-cp3\n"
       "                -cp3:M:L (-cp4も)\n"
       "                  M=0:中央値は単純に2で割ったもの. M=1:頻度を反映(デフォルト)\n"
       "                  L=1～4.0:Y,U,V,Aの選択で、Y値をL倍して行う(デフォルト1.2)\n"
       "  -cn[N]        減色時に1<<bpp未満の色数にしたい場合に指定\n"
       "\n"
       "  -ap           α付clutで出力. これか抜色指定(-ap)がないとα無clutになる\n"
       "  -ap[N]        N:減色時α数. N>=2は強制的に-cp3メディアンカット(yuv)になる\n"
       "  -ci[I]        clut画の抜色番号をIに.\n"
       "  -cc[I:C]      clut の i(0～255)番を色 C に置換. Iが違えば複数指定有効\n"
       "  -co[N]        clutの添え字/中身を N個ずらす. (-cis[N]も同じ)\n"
       "  -cb[N]        tgaでの出力clutのBPP.\n"
       "\n"
       " [サイズ変換]\n"
       "  -xrs[W:H:K]   W*Hに拡縮.\n"
       "  -xrp[N]       縦横の長さをN％に拡縮.\n"
       "  -xrc          拡大にバイキュービック法を用いる(デフォルト).\n"
       "  -xrb          拡大にバイリニア法を用いる.\n"
       "  -xre          拡大にspline36を用いる.\n"
       "  -xrl          拡縮にlanczos-3を用いる.\n"
       "  -xrn          拡縮にニアレストネイバー法を用いる.\n"
       "  -xn           不透明範囲のみにする.\n"
      #ifdef MY_H
       "                -mc[N:M]指定時はNxMチップ単位で狭める.\n"
      #endif
       "  -xv[W:H:X:Y:w:h:x:y]  元画像の(x,y)のw*hをW*H画の(X,Y)にロードした画像にする.\n"
       "                        W,Hは 0なら画像サイズ.\n"
       "                -xv+10:+5 のように+で記述すれば画像サイズに対する増分になる.\n"
       "  -xvc[W:H:X:Y:w:h:x:y] センタリング版-xv. X:Y,x:yはセンタリング後オフセット\n"
       "  -xvr[W:H:X:Y:w:h:x:y] 右(下)寄版-xv. X:Y,x:yは右寄後のオフセット\n"
       "  -xve[W:H:X:Y:w:h:x:y] ファイル名の最後が数字で奇数なら-xv,偶数なら-xvr\n"
       "  -xv?u[…],-xv?m[…],-xv?d[…] -xv の縦基準位置指定\n"
       "                ?はl|c|rの横指定, u|m|r は縦 上寄|中央寄|下寄 指定\n"
       //"  -xl[M:N]      -xv+M:+N:M:N に同じ\n"
      #ifdef MY_H
       "\n"
       " [チップ&マップ]\n"
       "  -mc[N:M]      N*N(N*M)ドット単位(チップ)化\n"
       "  -mh[W]        チップ&マップ化. W:テクスチャ横幅\n"
       "  -mp[W]        テクスチャを含めたファイルでのチップ&マップ化.\n"
       "  -mn           同チップ纏めを行わない(抜きチップ詰めは行う)\n"
       "  -mnn          抜チップ同チップを纏めない.\n"
       "  -mb           -mh,-mpにおいてチップの周囲１ドットを増殖したテクスチャを生成\n"
       "  -mq[TW:TH]    -mp と同様。ただし256x256内の画像なら1*1マップにする\n"
       "  -mi           情報テキスト生成.\n"
       "  -my           マップ化での走査順の特別版\n"
       "  -xnn          -xnと同様だが、-mc時に始点側はチップ単位化せず、ぎりぎりを選択.\n"
       "  -xo[X:Y]      画像フォーマットの始点情報を設定\n"
      #endif
       "\n"
       " [その他の変換]\n"
       "  -xe           圧縮して出力(tgaのみ).\n"
       "  -xq[N]        jpg圧縮時のクオリティ(％)\n"
       "  -xqg[N]       モノクロ画像用のjpg圧縮時のクオリティ(％)\n"
       "  -xx           x方向反転.\n"
       "  -xy           y方向反転.\n"
       "  -xj           右90°回転.\n"
       "  -xjl          左90°回転.\n"
       "  -xi[R]        右R°回転.\n"
       "  -xb           ピクセル値のビット反転(2値画での反転を想定).\n"
       "  -xg[N]        α<=Nの点を(0,0,0)に、出力bppで(0,0,0)になる点を近似の別色に.\n"
       "  -xca[N]       左上(0,0)の色を抜き色として4隅からα=0の色でペイント.\n"
       "                N=1-256適応範囲. (バストアップ仮絵作成を想定)\n"
       "  -cgn[R|G|B|A] モノクロ画像ぽければ -cg|-cgc の動作(実験中)\n"
       "  -xfa[A:N]     α<=A のピクセルをぼかす. N:回数\n"
       "  -xf[..]       ぼかしフィルタ処理.\n"
       "   -xf1:N                     ぼかし[3x3:4/2/1]をN回.\n"
       "   (-xf2:N:LeapARGB           実験:-xf1拡張でぼかし画像と通常画を合成)\n"
       "   (-xf2:N:LeapARGB:RGB1:RGB2 実験:さらに2色から生成したマスクも反映)\n"
       //"   -xf4:N                     α<255のピクセルをぼかす\n"
       "\n"
        MY_USAGE_2
       "\n"
    );
    return 1;
}



/* ------------------------------------------------------------------------ */

/// オプション情報.
struct Opts {
public:
    ConvOne_Opts*   convOne_opts;
    char*           oname;
    char*           dstDir;
    char*           srcDir;
    char const*     srcExt;
    char const*     dstExt;
    int             updateFlg;
    int             dispInfo;
	bool			dstRec;

public:
    Opts(ConvOne_Opts& coo);
    ~Opts() {;}

    int     scan(const char *a);

private:
    static double   strExprD(const char *p, const char ** a_p, int* a_err);
    static int      strToI (const char* &p, int base) { return (int)strtol(p, (char**)&p, base); }
    static unsigned strToUI(const char* &p, int base) { return (unsigned)strtoul(p, (char**)&p, base); }

    void            readClutBin(char const* name, int clutbpp);
};




/** オプションの初期化 */
Opts::Opts(ConvOne_Opts& convOne_opts)
{
    memset(this, 0, sizeof(*this));
    this->convOne_opts  = &convOne_opts;
    this->srcDir        = NULL;
    this->dstDir        = NULL;
    this->srcExt        = "bmp";
    this->dstExt        = "tga";
    this->convOne_opts->dstFmt  = BM_FMT_TGA;
}



/** オプションの処理 */
int Opts::scan(const char *a)
{
    ConvOne_Opts* o = this->convOne_opts;
    const char  *p;

    p = a;
    if (*p == '-')
        p++;

    int b = *p++;
    int c = TOUPPER(b);
    switch (c) {
    case 'A':
        c = *p++, c = TOUPPER(c);
        if (c == 'N') {         // -an
            o->alpNon = 1;
        } else if (c == 'O') {  // -ao
            o->genMaskFlg = 1;
        } else if (c == 'C') {  // -ac
            o->alpToCol   = strToUI(p,16);
            o->nukiumeRgb = o->alpToCol;
            if (*p != 0) {
                ++p;
                o->nukiumeRgb = strToUI(p, 16);
            }
        } else if (c == 'D') {  // -ad
            o->alpBokasi = (*p == '\0') ? 128 : (*p == '-') ? -1 : strToUI(p,0);
            if (o->alpBokasi < -1)
                o->alpBokasi = -1;
            else if (o->alpBokasi > 255)
                o->alpBokasi = 255;
            ++o->alpBokasi;

        } else if (c == 'G') {  // -ag
            o->monoToAlp = (*p != '-');
            if (*p == 0) {
                o->monoToAlp_rate = 1.0;
                o->monoToAlp_ofs  = 0;
            } else {
                o->monoToAlp_rate = strExprD(p,&p,0);
                if (*p != '\0') {
                    ++p;
                    o->monoToAlp_ofs = strToI(p, 0);
                }
            }
        } else if (c == 'M') {  // -am
            o->alpMin = strToUI(p,0);
            if (*p) {
                ++p;
                o->alpMax = strToUI(p,0);
            }
            if (o->alpMax <= o->alpMin || o->alpMax > 255)
                o->alpMax = 255;
        } else if (c == 'P') {  // -ap
            o->clutAlpFlg = 1;
            if (*p) {
                o->clutAlpNum = strToUI(p,0);
            } else {
                o->clutAlpNum = -1; // 後でclut数より適当に求める.
            }
            if (o->clutAlpNum >= 256)
                o->clutAlpNum = 256;
        } else if (c == 'R') {  // -ar
            if (*p == 'o' || *p == 'O') //-aro
                o->alpModeO = 1;
            else
                o->alpModeI = 1;
        } else if (c == 'I') {  // -ai
            o->alphaPlaneFileName = strdupE(p);
            o->fullColFlg         = 1;
        } else if (c == 'Z') {  // -az
            o->clearColIfAlp0 = (*p != '-');

      #if 0 //旧版.
        } else if (c == 'I') {
            if (*p == 0)
                o->alpModeI = 1;
            else
                o->alpModeI = strToUI(p,16);
        } else if (c == 'O') {
            if (*p == 0)
                o->alpModeO = 1;
            else
                o->alpModeO = strToUI(p,16);
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'B':
        if (isdigit(*p)) {
            o->bpp = c = strToUI(p, 0);
            if (strchr("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0c\x0f\x10\x18\x20", c) == NULL)
                err_abortMsg("-b の指定値が想定外(%d)\n", c);
        } else {
            c = 0;
            if      (strcasecmp(p, "A2I6") == 0) c = 2;
            else if (strcasecmp(p, "A3I5") == 0) c = 3;
            else if (strcasecmp(p, "A4I4") == 0) c = 4;
            else if (strcasecmp(p, "A5I3") == 0) c = 5;
            else if (strcasecmp(p, "A6I2") == 0) c = 6;
            if (c) {
                o->fullColFlg      = 1;
                o->bpp             = 8;
                o->alpBitForBpp8   = c;
                o->decreaseColorMode = DCM_MC_YUV;  // とりあえずyuvなメディアンカット法で減色することにする.
            }
        }
        break;

    case 'C':
        c = *p++, c = TOUPPER(c);
        switch (c) {
        case 'O':   //-co
            o->clutOfs = strToUI(p, 0);
            break;

        case 'F':   //-cf
            if (TOUPPER(*p) == 'N') {
                ++p;
                o->colKey   = strToUI(p, 16);
                o->colKeyNA = 1;
            } else {
                o->colKey = strToUI(p, 16);
            }
            break;

        case 'A':   //-ca
            o->colKey = strToUI(p, 16);
            o->alpBokasi = 1;
            break;

        case 'B':   //-cb
            o->clutBpp = strToI(p, 0);
            break;

        case 'E':   //-ce
            if (TOUPPER(*p) == 'M') {   // -cem
                o->colChSquare = *(p+1) != '-';
            }
            break;

        case 'G':   //-cg
            o->mono = (*p != '-');
            if (TOUPPER(*p) == 'C') {   // -cgc
                int t = TOUPPER(*(unsigned char*)(p+1));
                o->monoChRGB = (t == 'B') + (t == 'G')*2 + (t == 'R')*3 + (t == 'A')*4;
            } else if (TOUPPER(*p) == 'N') {    // -cgn
                int t = TOUPPER(*(unsigned char*)(p+1));
                o->mono      = 0;
                o->monoNear  = 1;
                o->monoChRGB = (t == 'B') + (t == 'G')*2 + (t == 'R')*3 + (t == 'A')*4;
            }
            if (*p != '\0') {
                ++p;
				o->monoCol = strToUI(p, 16);
	            if ((o->monoCol & 0xFF000000) == 0)
	                o->monoCol |= 0xFF000000;
			}
            break;

        case 'M':   //-cm
            o->colMul = strToUI(p, 16);
            if ((o->colMul & 0xFF000000) == 0)
                o->colMul |= 0xFF000000;
            break;

        case 'S':   //-cs
        case 'T':   //-ct
            o->pixScaleType = (c == 'T');
            o->pixScale[0]  = o->pixScale[1] = o->pixScale[2] = o->pixScale[3] = 1.0;
            o->pixScale[0] = strExprD(p,&p,0);
            if (*p) {
                o->pixScale[1] = strExprD(p+1,&p,0);
                if (*p) {
                    o->pixScale[2] = strExprD(p+1,&p,0);
                    if (*p) {
                        o->pixScale[3] = strExprD(p+1,&p,0);
                    }
                }
            }
            break;

        case 'N':   //-cn
            if (*p == 'x' || *p == 'X') {   // -cnx
                o->colNum = 0x7fffffff;
            } else {
                o->colNum = strToUI(p,0);
            }
            break;

        case 'C':   //-cc
            c = strToI(p, 0);
            if (c < 0 || c > 255)
                goto OPT_ERR;
            o->clutChgFlg[c] = 1;
            if (*p) {
                ++p;
                o->clutChg[c] = strToUI(p, 16);
            }
            break;

        case 'I':   //-ci
            if (TOUPPER(*p) == 'S') {   // -cis
                ++p;
                o->clutIndShft = strToUI(p, 0);
                o->clutOfs     = o->clutIndShft;
            } else { // -ci
                o->nukiClut    = strToUI(p, 0);
            }
            break;

        case 'L':   //-cl
            if (toupper(*p) == 'B') {   //-clb
                ++p;
                int cbpp = strToUI(p, 0);
                if (cbpp != 24 && cbpp != 32) {
                    goto OPT_ERR;
                }
                if (*p != '\0') {
                    ++p;
                    readClutBin(p, cbpp);
                }
            }
            break;

        case 'P':   //-cp
            if (TOUPPER(*p) == 'C') {   //-cpc
                o->clutTxtName = strdupE(p+1);
            } else if (TOUPPER(*p) == 'F') {    // -cpf 外部パレットファイルを用いて減色.
                o->readFixedClut(p+1);
                o->decreaseColorMode = DCM_FIX_FILE;
                o->fullColFlg        = 1;
            } else if (TOUPPER(*p) == 'M') {    // -cpm
                ++p;
                o->decreaseColorMode2 = strToUI(p,0);
            } else {
                o->fullColFlg      = 1;
                //x o->dfltClutCg = (*p == 0) ? 1 : strToUI(p, 0);
                o->decreaseColorMode = Dcm_t((*p == 0) ? 1 : strToUI(p,0));
                if (*p) {
                    o->decreaseColorParam[0] = strExprD(p+1,&p,0);
                    if (*p) {
                        o->decreaseColorParam[1] = strExprD(p+1,&p,0);
                        if (*p) {
                            o->decreaseColorParam[2] = strExprD(p+1,&p,0);
                            if (*p) {
                                o->decreaseColorParam[3] = strExprD(p+1,&p,0);
                            }
                        }
                    }
                }
            }
            break;

        case 'Q':   //-cq
            {
                static const char *tbl[] = {
                    "argb", "ragb", "agrb", "garb", "rgab", "grab",
                    "arbg", "rabg", "abrg", "barg", "rbag", "brag",
                    "agbr", "gabr", "abgr", "bagr", "gbar", "bgar",
                    "rgba", "grba", "rbga", "brga", "gbra", "bgra",
                };
                if (strcasecmp(p,"rgb") == 0) p = "argb";
                if (strcasecmp(p,"rbg") == 0) p = "arbg";
                if (strcasecmp(p,"grb") == 0) p = "agrb";
                if (strcasecmp(p,"gbr") == 0) p = "agbr";
                if (strcasecmp(p,"brg") == 0) p = "abrg";
                if (strcasecmp(p,"bgr") == 0) p = "abgr";
                for (c = 0; c < 24; c++) {
                    if (strcasecmp(p, tbl[c]) == 0) {
                        o->colrot = c;
                        break;
                    }
                }
                if (c == 24)
                    goto OPT_ERR;
            }
            break;

        //case 'Y':
        //
        //  break;

        default:
            goto OPT_ERR;
        }
        break;

    case 'E':   // -e
        c = *p++, c = TOUPPER(c);
        if (c == 'D') { // -ed
            this->dstExt = strdupE(p);
            o->exDstExt = strdupE(p);
        } else if (c == 'S') {  // -es
            this->srcExt = strdupE(p);
        } else {
            goto OPT_ERR;
        }
        break;

    case ':':
    case 'F':
        if (strcasecmp(p, "NON") == 0) {
            o->dstFmt    = BM_FMT_NON;
            this->dstExt = "";
        } else if (strcasecmp(p, "BMP") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "bmp";
        } else if (strcasecmp(p, "DIB") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "dib";
      #if 1
        } else if (strcasecmp(p, "RLE") == 0) {
            o->dstFmt    = BM_FMT_BMP;
            this->dstExt = "rle";
            o->encMode = 1;
      #endif
        } else if (strcasecmp(p, "TGA") == 0) {
            o->dstFmt    = BM_FMT_TGA;
            this->dstExt = "tga";
      #if defined USE_JPG
        } else if (strcasecmp(p, "JPG") == 0 || strcasecmp(p, "JPEG") == 0) {
            o->dstFmt    = BM_FMT_JPG;
            this->dstExt = "jpg";
      #endif
      #if defined USE_PNG
        } else if (strcasecmp(p, "PNG") == 0) {
            o->dstFmt    = BM_FMT_PNG;
            this->dstExt = "png";
      #endif
        } else if (strcasecmp(p, "BIN") == 0) {
            o->dstFmt    = BM_FMT_BETA;
            this->dstExt = "bin";
      #ifdef MY_H
        } else if (MY_opt_fmtCheck(p, o, &this->dstExt)) {
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'H':
        c = *p++, c = TOUPPER(c);
        if (c == 0) {
            usage();
      #ifdef MY_H
        } else if (c == 'R') {
            o->lvlY = strToI(p, 0);
            if (*p != 0) {
                p++;
                o->lvlUV = strToI(p, 0);
            }
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'I':
        c = *p++, c = TOUPPER(c);
        if (c == 'F') {                 //-if
            o->fullColFlg = (*p != '-');
        } else if (c == 'B') {          //-ib
            if (*p == 0)
                o->srcBpp = 24;
            else
                o->srcBpp = strToI(p, 0);
        } else if (c == 'S') {          //-is
            o->bytSkp = strToI(p, 0);
        } else if (c == 'G') {          //-ig
            o->binW = 256;
            o->binH = -1;
            o->binBpp = 8;
            o->binW = strToI(p, 0);
            if (*p) {
                ++p;
                o->binH = strToI(p, 0);
                if (*p) {
                    ++p;
                    o->binBpp = strToI(p, 0);
                    if (*p) {
                        ++p;
                        o->binEndian = strToI(p, 0);
                    }
                }
            }
            if (o->binBpp <= 0 || o->binBpp > 32 || o->binW <= 0 || o->binH == 0)
                goto OPT_ERR;
        } else if (c == 'N') {
            this->dispInfo = (*p != '-');
        } else {
            goto OPT_ERR;
        }
        break;

    case 'M':
        c = *p++, c = TOUPPER(c);
        if (c == 'C') { //-mc
            o->celSzW = strToI(p,0);
            if (o->celSzW <= 0)
                goto OPT_ERR;
            o->celSzH = o->celSzW;
            if (*p) {
                p++;
                o->celSzH = strToI(p,0);
                if (o->celSzH <= 0)
                    goto OPT_ERR;
            }
        } else if (c == 'B') {  //-mb
            o->celStyl = (*p != '-') ? 1 : 0;
        } else if (c == 'T') {  //-mt   とりあえず、廃止.
            o->mapStyl = (*p != '-') ? 1 : 0;
        } else if (c == 'N') {  //-mn   同セルチェックをしない. -mnn 抜きキャラも抜かない(スプライトハード向けのデータコンバート)
            o->mapNoCmp = (*p == 'n' || *p == 'N') ? 3 : (*p != '-') ? 1 : 0;
        } else if (c == 'Y') {  //-my   マップ化の走査順番の特別版.
            o->mapEx256x256 = (*p != '-') ? 1 : 0;
        } else if (c == 'H' || c == 'P' || c == 'Q') {  //-mh,-mp,-mq
            o->mapMode = (c == 'H') ? 1 : (c == 'P') ? 2 : 3;
            o->mapTexW = (*p) ? strToI(p, 0) : 256;
            o->mapTexH = 0;
            if (*p) {
                p++;
                o->mapTexH = (*p) ? strToI(p, 0) : 256;
                if (*p) {
                    ++p;
                    o->mapMode3_y_min = strToI(p, 0);
                }
            }
            if (o->celSzW == 0) {
                o->celSzW = o->celSzH = 32;
            }
            if (o->mapTexW < o->celSzW || (o->mapTexH && o->mapTexH < o->celSzH)) {
                err_abortMsg("マップ化時のテクスチャ横幅が小さすぎる(-mpの前に-mcでサイズを設定してください)\n");
                goto OPT_ERR;
            }
        } else if (c == 'I') {      //-mi
            o->saveInfFile = 1;
      #if 0
        } else if (c == 'O' && TOUPPER(p[0]) == 'j' && TOUPPER(p[1]) == 'i') {  //-moji
            p += 2;
            o->fntX = strToI(p, 10);
            if (*p) {
                o->fntY = strToI(p+1, 10);
                if (*p) {
                    o->fntC = strToUI(p+1, 16);
                    if (*p) {
                        ++p;
                        o->fntD = strToI(p, 10);
                        if (*p) {
                            o->fntStr = strdupE(p+1);
                        }
                    }
                }
            }
            if (o->fntD < 5)
                o->fntD = 16;
            break;
      #endif
        } else {
            goto OPT_ERR;
        }
        break;

    case 'O': // -o
        this->oname = strdupE(p);
        fname_backslashToSlash(this->oname);
        break;

    case 'D': // -d
    	this->dstRec = (b == 'D');
        this->dstDir = strdupE(p);
        fname_delLastDirSep(this->dstDir);
        fname_backslashToSlash(this->dstDir);
        break;

    case 'S': // -s
        this->srcDir = strdupE(p);
        fname_delLastDirSep(this->srcDir);
        fname_backslashToSlash(this->srcDir);
        break;

    case 'T':   // -t
        o->toneType = 0;
        if (*p == 0) {
            o->tone = 50;
        } else if (TOUPPER(*p) == 'T') {    // -tt
            o->toneType = 1;
            o->tone     = 50;
            if (p[1]) {
                ++p;
                o->tone = strToI(p,10);
            }
        } else {
            o->tone = strToI(p,10);
        }
        break;

    case 'U':   // -u
        this->updateFlg = (*p == '-') ? 0 : 1;
        break;

    case 'V':   // -v
        o->verbose = (*p == '-') ? 0 : 1;
        break;

    case 'X':
        c = *p++;
        c = TOUPPER(c);
        switch (c) {
        case 'D':   //-xd
            {
                o->fullColFlg   = 1;
                unsigned flag	= 0;
                if (*p == 'P' || *p == 'p' || *p == 'O' || *p == 'o') {   // -xdp
                    p++;
                } else
                if (*p == 'E' || *p == 'e') {   // -xde 誤差拡散.
                    flag |= PaternDither::F_ERRDIF;
                    p++;
                } else
                if (*p == 'H' || *p == 'h') {   // -xdh 誤差拡散 (ハーフ指定廃止)
                    flag |= PaternDither::F_ERRDIF;
                    p++;
                }
                if (*p == 'X' || *p == 'x') {
					flag |= PaternDither::F_RGB_2BIT_X;
                    p++;
				}
                if (*p == 'Y' || *p == 'y') {
					flag |= PaternDither::F_RGB_2BIT_Y;
                    p++;
				}
                if (*p == 'A' || *p == 'a') {   // -xda
                    o->ditAlpFlg = 1;
                    p++;
                }
                o->ditBpp = strToI(p,10);
                if (o->ditBpp <= 0)
                    o->ditBpp = -1;
                else if (o->ditBpp >= 24)
                    err_abortMsg("-xd[%d] だとディザを行えません\n");
                o->ditTyp = flag ? 0 /*none*/ : 2 /* 2x2 */;
                if (*p) {
                    p++;
                    o->ditTyp = strToI(p,10);
                }
                o->ditTyp |= flag;
            }
            break;

        case 'E':   //-xe
          #ifdef MY_H
            c = MY_opt_checkXE(p);
            if (c)
                o->encMode = c;
            else
          #endif
                o->encMode = (*p == '-') ? 0 : (*p == '\0') ? 1 : strToI(p,10);
            break;

        case 'G':   //-xg
            if (*p == 0)
                o->g555val = 0;
            else
                o->g555val = strToI(p,0);
            if (o->g555val < 0 || o->g555val > 255)
                goto OPT_ERR;
            break;

        case 'R':   //-xr
            {
                if (o->rszN > 2)
                    err_abortMsg("-xrsまたは-xrp が3つ以上指定されている.\n");
                c = *p++, c = TOUPPER(c);
                o->rszK[o->rszN] = 0;
                if (c == 'P') {         //-xrp
                    o->rszXpar[o->rszN] = strExprD(p,&p, 0);
                    if (*p) {
                        o->rszYpar[o->rszN] = strExprD(p+1,&p, 0);
                    } else {
                        o->rszYpar[o->rszN] = o->rszXpar[o->rszN];
                    }
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1, 10);
                    o->rszN++;
                } else if (c == 'S') {  //-xrs
                    o->rszXsz[o->rszN] = int(strExprD(p, &p, 0));
                    o->rszYsz[o->rszN] = 0;
                    if (*p) {
                        o->rszYsz[o->rszN] = int(strExprD(p+1, &p, 0));
                    }
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1, 10);
                    o->rszN++;
                } else if (c == 'Q') {  //-xrq
                    o->rszXsz[o->rszN] = int(strExprD(p, &p, 0));
                    o->rszYsz[o->rszN] = 0;
                    if (*p)
                        o->rszYsz[o->rszN] = int(strExprD(p+1, &p, 0));
                    //if (*p)
                    //  o->rszK[o->rszN] = strToI(p+1,10);
                    o->rszK[o->rszN] = 1;
                    o->rszN++;
                } else if (c == 'N') {  //-xrn ニアレストネイバー.
                    o->rszType = 0;
                } else if (c == 'B') {  //-xrb バイリニア.
                    o->rszType = 1;
                } else if (c == 'C') {  //-xrc バイキュービック.
                    o->rszType = 2;
                } else if (c == 'E') {  //-xre spline36
                    o->rszType = 3;
                } else if (c == 'L') {  //-xrl lanczos-3
                    o->rszType = 4;
                }
            }
            break;

        case 'Q':   // -xq
            if (TOUPPER(*p) == 'G') {   // -xqg
                ++p;
                o->quality_grey = strToI(p, 10);
            } else {
                o->quality = strToI(p, 10);
            }
            break;

        case 'X':   //-xx
            o->dir |= 1;
            break;

        case 'Y':   //-xy
            o->dir |= 2;
            break;

        case 'J':   //-xj
            o->rotR90 = 1;
            if (TOUPPER(*p) == 'L') {
                o->rotR90 = -1;
            }
            break;

        case 'I':   //-xi
            o->rotR = strtod(p, (char**)&p);
            break;

        case 'N':   //-xn
            o->nukiRctFlg = 1;
            if (*p == 'n' || *p == 'N') {
                o->nukiRctFlg = 2;
                ++p;
            }
            if (*p == 'x' || *p == 'X') {
                o->nukiRctFlg |= 0x10;
            } else if (*p == 'y' || *p == 'Y') {
                o->nukiRctFlg |= 0x20;
            }
            break;

        case 'V':   //-xv
            if (*p) {
                if (o->vvIdx >= 2)
                    goto OPT_ERR;
                ConvOne_Opts::vv_t* ov = &o->vv[o->vvIdx++];
                ov->flg = 1;
                ov->lcr = 0;
                ov->umd = -1;
                ov->lcr_ex = 0;
                for (;;) {
                    int c2 = TOUPPER(*p);
                    if (c2 == 'C') {        // -xvc
                        ov->lcr = 1;
                        ++p;
                    } else if (c2 == 'R') { // -xvr
                        ov->lcr = 2;
                        ++p;
                    } else if (c2 == 'L') { // -xvl
                        ov->lcr = 0;
                        ++p;
                    } else if (c2 == 'U') { // -xvu
                        ov->umd = 0;
                        ++p;
                    } else if (c2 == 'M') { // -xvm
                        ov->umd = 1;
                        ++p;
                    } else if (c2 == 'D') { // -xvd
                        ov->umd = 2;
                        ++p;
                    } else if (c2 == 'E') { // -xve
                        ov->lcr_ex = 1;
                        if (ov->umd < 0)
                            ov->umd = 1;    // umd未設定なら縦中央寄.
                        ++p;
                    } else {
                        break;
                    }
                }
                if (ov->umd < 0)
                    ov->umd = ov->lcr;
                // W
                ov->w = 0;
                ov->h = 0;
                ov->x = 0;
                ov->y = 0;
                ov->sw= 0;
                ov->sh= 0;
                ov->sx= 0;
                ov->sy= 0;
                if (*p != ':' && *p != ',') {
                    ov->wf = 0;
                    if (*p == '+') {
                        ov->wf = 1;
                        p++;
                    }
                    ov->w = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // H
                if (*p != ':' && *p != ',') {
                    ov->hf = 0;
                    if (*p == '+') {
                        ov->hf = 1;
                        p++;
                    }
                    ov->h = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // X
                if (*p != ':' && *p != ',') {
                    ov->xf = 0;
                    //if (*p == '+') {
                    //    ov->xf = 1;
                    //    p++;
                    //}
                    ov->x = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // Y
                if (*p != ':' && *p != ',') {
                    ov->yf = 0;
                    //if (*p == '+') {
                    //    ov->yf = 1;
                    //    p++;
                    //}
                    ov->y = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // w
                if (*p != ':' && *p != ',') {
                    ov->sw = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // h
                if (*p != ':' && *p != ',') {
                    ov->sh = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // x
                if (*p != ':' && *p != ',') {
                    ov->sx = strToI(p, 10);
                }
                if (*p == 0)
                    break;
                p++;
                // y
                if (*p != ':' && *p != ',') {
                    ov->sy = strToI(p, 10);
                }
            } else {
                goto OPT_ERR;
            }
            break;
      #if 0 // とうに現行の-xvソースと矛盾してバグってるので削除.
        case 'L':   //-xl
            if (*p) {
                if (o->vvIdx >= 2)
                    goto OPT_ERR;
                ConvOne_Opts::vv_t* ov = &o->vv[o->vvIdx++];
                ov->flg = 1;
                // W
                if (*p != ':' && *p != ',') {
                    ov->wf = 1;
                    ov->x  = ov->w = strToI(p, 10);
                }
                // H
                if (*p != 0) {
                    ov->hf = 1;
                    ++p;
                    ov->y  = ov->h = strToI(p, 10);
                }
            } else {
                goto OPT_ERR;
            }
            break;
      #endif
        case 'B':   //-xb
            o->bitCom = (*p == '-') ? 0 : 1;
            break;

        case 'F':   //-xf   フィルタ.
            if (TOUPPER(*p) == 'A') {   // -xfa閾値:回数.
                o->filterType = 4;
                o->bokashiCnt = 1;
                o->bokashiAlpSikii = 254;
                if (*p != 0) {
                    ++p;
                    o->bokashiAlpSikii = (*p == '\0') ? 254 : (*p == '-') ? -1 : strToUI(p,0);
                    if (o->bokashiAlpSikii < -1)
                        o->bokashiAlpSikii = -1;
                    else if (o->bokashiAlpSikii > 255)
                        o->bokashiAlpSikii = 255;
                    ++o->bokashiAlpSikii;
                    if (*p != 0) {
                        ++p;
                        o->bokashiCnt = strToI(p, 10);
                    }
                }
            } else {    // -xf
                o->filterType = strToI(p, 10);
                if (o->filterType == 1 || o->filterType == 2 || o->filterType == 3 || o->filterType == 4) {
                    o->bokashiCnt = 1;
                    o->bokashiAlpSikii = 254;
                    if (*p != 0) {
                        ++p;
                        o->bokashiCnt = strToI(p, 10);
                        if (*p != 0) {  // -xf2 -xf3 用.
                            //o->bokashiMergeRate = strToI(p+1, 10) / 100.0;
                            ++p;
                            o->bokashiMergeRateRGB = strToUI(p, 16);
                            o->bokashiMaskGenCol1  = 0x000000;
                            o->bokashiMaskGenCol2  = 0xffffff;
                            if (*p != 0) {  // xf3 用.
                                ++p;
                                o->bokashiMaskGenCol1 = strToUI(p, 16);
                                if (*p != 0) {
                                    ++p;
                                    o->bokashiMaskGenCol2 = strToUI(p, 16);
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 'C':   // -xca
            if (*p == 'a' || *p == 'A') {
                ++p;
                o->genAlpEx = 8;
                if (*p < 0x80U && isdigit(*p))
                    o->genAlpEx = strToUI(p, 10);
            } else {
                goto OPT_ERR;
            }
            break;

        case 'O':
            o->startX = strToI(p,0);
            o->startY = 0;
            if (*p) {
                ++p;
                o->startY = strToI(p,0);
            }
            break;

      #ifdef MY_H
        case 'K':   //-xk
            MY_setOptXK();
            break;
      #endif

        default:
            goto OPT_ERR;
        }
        break;

    case 'Z':
        dbgExLog_setSw(*p != '-');
        break;

    case '\0':
    case '?':
        return usage();

    default:
  OPT_ERR:
        err_abortMsg("Incorrect command line option : %s\n", a);
        return 1;
    }
    return 0;
}



double Opts::strExprD(const char *p, const char ** a_p, int* a_err)
{
    double val = 0;
    int err = strExpr(p, (const char **)a_p, &val);
    if (a_err)
        *a_err = err;
    return val;
}



void Opts::readClutBin(char const* fname, int clutbpp)
{
    ConvOne_Opts*   o = convOne_opts;
    size_t sz = 0;
    unsigned* src = (unsigned*)fil_loadMallocE(fname, &sz);
    if (src == 0 || sz == 0)
        return;

    int w=0, h=0, bpp=0, clutNum=0;
    int fmt = bm_getHdr(src, sz, &w, &h, &bpp, &clutNum);
    if (fmt > 0 && clutNum > 0) {
        if (clutNum > 256)
            clutNum = 256;
        unsigned n = bm_getClut(src, o->clutChg, clutNum);
        for (unsigned i = 0; i < n; ++i)
            o->clutChgFlg[i] = 1;
     #ifdef MY_H
        MY_readClutBinSub(src, fmt);
     #endif
    } else {
        unsigned n = sz / 4;
        if (n > 256)
            n = 256;
        for (unsigned i = 0; i < n; ++i) {
            o->clutChgFlg[i] = 1;
            o->clutChg[i] = src[i];
        }
    }
    free(src);
}


/* ------------------------------------------------------------------------ */

class App {
	ConvOne convOne_;
	Opts    opts_;
	char   	nameBuf_[ FIL_NMSZ  ];
	char   	tempBuf_[ FIL_NMSZ  ];

	enum { Ok = 0, Er = 1 };
public:
	App() : convOne_(), opts_(convOne_.opts()) {
        memset(nameBuf_, 0, sizeof nameBuf_);
        memset(tempBuf_, 0, sizeof tempBuf_);
    }

	int main(int argc, char *argv[]) {
        // アプリ名取得.
        g_appName = strdupE(fname_baseName(argv[0]));
	    if (g_appName == NULL)
	    	return Er;
     #if 0 //defined(_WIN32)
		if (g_appName)
	    	fname_strLwr(g_appName);
	 #endif

        // コマンド引数調整(レスポンスファイル展開).
        if (ExArgv_convEx(&argc, &argv, 0) == 0)
	    	return Er;

        if (argc < 2)
	        return usage();

		size_t	n = 0;
		bool 	optCk = true;
	    // 引数解析.
	    for (int i = 1; i < argc; i++) {
	        char* p = argv[i];
	        if (optCk && *p == '-') {
				if (*p == '-' && p[1] == '-' && p[2] == 0) {
					optCk = false;
					continue;
				}
	            opts_.scan(p);
	        } else if (*p == ':') {
	            opts_.scan(p);
	        } else {
				++n;
			}
	    }
	    if (n == 0) {
	        err_abortMsg("ファイル名を指定してください\n");
			return Er;
	    }

	    if (opts_.convOne_opts->mapMode >= 2) {  // 合体mapファイルの拡張子を設定する.
	        if (opts_.convOne_opts->exDstExt == NULL)
	            opts_.convOne_opts->exDstExt = "mp";
	        opts_.dstExt = opts_.convOne_opts->exDstExt;
	    }

		// -s 指定がある場合は、相対パスにソースディレクトリを付加.
		char const* srcDir = opts_.srcDir;
		size_t srcDirLen = 0;
		if (srcDir) {
			srcDirLen = strlen(srcDir);
			optCk = true;
			for (size_t idx = 1; idx < argc; ++idx) {
		        char* arg = argv[idx];
		        if (optCk && *arg == '-') {
					if (*arg == '-' && arg[1] == '-' && arg[2] == 0)
						optCk = false;
					continue;
				} else if (*arg == ':') {
					continue;
				}
				if (!fname_isAbsolutePath(arg)) {
					snprintf(nameBuf_, FIL_NMSZ, "%s/%s", srcDir, arg);
					free(arg);
					argv[idx] = arg = strdupE(nameBuf_);
					if (arg == NULL)
						return Er;
				}
				if (*fname_getExt(arg) == 0 && opts_.srcExt) {
					snprintf(nameBuf_, FIL_NMSZ, "%s.%s", arg, opts_.srcExt);
					free(arg);
					argv[idx] = arg = strdupE(nameBuf_);
					if (arg == NULL)
						return Er;
				}
			}
		}

        // ワイルドカード展開.
	    if (ExArgv_convEx(&argc, &argv, 1) == 0)
	    	return Er;

        optCk = true;
		for (size_t idx = 1; idx < argc; ++idx) {
	        char* arg = argv[idx];
	        if (optCk && *arg == '-') {
				if (*arg == '-' && arg[1] == '-' && arg[2] == 0)
					optCk = false;
				continue;
			} else if (*arg == ':') {
				continue;
            }

			char const* srcpath = arg;
            // 出力名を設定.
            if (opts_.oname) {  // -o 指定有.
                char*   tgtname  = opts_.oname;
                char*   basename = fname_baseName(tgtname);
                if (opts_.dstDir && (tgtname[0] == '/' || tgtname[1] == ':'))   // 出力フォルダ指定があればそっち優先.
                    tgtname = basename;
                if (strchr(basename, '.') == NULL && opts_.dstExt) {    // 拡張子がなければつける.
					snprintf(tempBuf_, FIL_NMSZ, "%s.%s", tgtname, opts_.dstExt);
					tgtname = tempBuf_;
                }
				if (opts_.dstDir && opts_.dstDir[0])
					snprintf(nameBuf_, FIL_NMSZ, "%s/%s", opts_.dstDir, tgtname);
				else
					snprintf(nameBuf_, FIL_NMSZ, "%s", tgtname);
            } else {
				char const* tgtname = srcpath;
				if (srcDirLen && fname_startsWith(srcpath, srcDir)) {    // -sソースディレクトリファイル?
					tgtname += srcDirLen + 1;
				} else {
					tgtname =  fname_baseName(srcpath);                 // 違えば入力のフォルダは無視.
				}
				char const* ext = opts_.dstExt;
				size_t      extSize = strlen(ext) + 1;
				if (opts_.dstDir && opts_.dstDir[0])    // 出力ディレクトリ有?
					snprintf(nameBuf_, FIL_NMSZ - 1 - extSize, "%s/%s", opts_.dstDir, tgtname);
				else
					snprintf(nameBuf_, FIL_NMSZ - 1 - extSize, "%s", tgtname);
                // 拡張子を付け替える.
                char* basename = fname_baseName(nameBuf_);
                char* e = strchr(basename, '.');
				if (e) {
					memcpy(e+1, ext, extSize);
				} else {
					strcat(nameBuf_, ext);
				}
            }

            //常に変換の指定か、onamBufの日付が古ければ変換を行う.
            if (opts_.updateFlg == 0 || fil_fdateCmp(nameBuf_, arg) < 0) {
				char* onm = (opts_.dispInfo) ? NULL : nameBuf_;
                // 実際の変換処理.
                convOne_.run(srcpath, onm);
            }

            if (opts_.oname) // 出力ファイル名が指定されていた場合は１ファイルのみ.
                break;
		}

	    return 0;
	}

};

#if !defined(EXARGV_USE_WCHAR)
int main(int argc, char* argv[])
{
	scoped_console_output_utf8 sav_cp;
	static App app;
    int rc = app.main(argc, argv);
    return rc;
}
#else
int wmain(int argc, wchar_t* wargv[])
{
	scoped_console_output_utf8 sav_cp;

	int rc = 1;
    char** argv = ExArgv_wargvToUtf8(argc, wargv);
	if (argv) {
		static App app;
    	rc = app.main(argc, argv);
    }
	ExArgv_release(&argv);
    return rc;
}
#endif
