/**
 *  @file   FixedClut256.hpp
 *  @brief  clutに従って32ビット色画を8ビット色画に変換. デフォルトclutの作成.
 *  @author Masashi KITAMURA
 */

#ifndef FIXEDCLUT256_H
#define FIXEDCLUT256_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cassert>



/// clutに従って32ビット色画を8ビット色画に変換. デフォルトclutの作成.
template<class A=int>
class FixedClut256 {
public:
    /// デフォルトパレットの作成.必ず256色分のメモリを用意のこと. bpp=3,4,6,8   typ:0=jpn 1=win.
    static void getFixedClut256(unsigned *clut, unsigned clutSize, unsigned clutBpp, unsigned typ=0);

	/// clutに従って32ビット色画を8ビット色画に変換.
    static bool decreaseColor(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                    , const unsigned* pClut, unsigned clutSize, unsigned idx=0, int alp1=0, int alp2=0);

	/// グレイ画像か?
	static bool isGrey(const unsigned* pSrc, unsigned w, unsigned h);

	/// グレイに近い画像か?
	static bool isNearGrey(const unsigned* pSrc, unsigned w, unsigned h, int threshold=12);

	/// グレイ(モノクロ)パレットの作成. 必ず256色分のメモリを用意のこと. bpp=3,4,6,8
    static void getFixedGreyClut(unsigned *clut, unsigned clutSize, unsigned clutBpp);

	/// 入力がグレイ画像前提で 8bit画像化
	static bool fromGreyToBpp8(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h);

public:
    /// g1r1b1 デジタル8色. (古い日本のパソコンのパレット番号に合わせたもの)
    static const unsigned* clutGRB111() {
        static const unsigned jpn8[] = {
            0xFF000000, 0xFF0000ff, 0xFFff0000, 0xFFff00ff, 0xFF00ff00, 0xFF00ffff, 0xFFffff00, 0xFFffffff,
        };
        return jpn8;
    }

    /// 古い日本のパソコンのデフォルトパレット(じゃないかもだけど、それに近いもの)
    static const unsigned* clutJpn16() {
        static const unsigned jpn16[] = {
            0xFF000000, 0xFF0000ff, 0xFFff0000, 0xFFff00ff, 0xFF00ff00, 0xFF00ffff, 0xFFffff00, 0xFFffffff,
            0xFF808080, 0xFF000080, 0xFF800000, 0xFF800080, 0xFF008000, 0xFF008080, 0xFF808000, 0xFFc0c0c0,
        };
        return jpn16;
    }

    /// dos/winな8色(じゃないかもだけど)
    static const unsigned* clutBGR111() {
        static const unsigned win8[] = {
            0xFF000000, 0xFFff0000, 0xFF00ff00, 0xFFffff00, 0xFF0000ff, 0xFFff00ff, 0xFF00ffff, 0xFFffffff,
        };
        return win8;
    }

    /// dos/winな16色.
    static const unsigned* clutWin16() {
        static const unsigned win16[] = {
            0xFF000000, 0xFF800000, 0xFF008000, 0xFF808000, 0xFF000080, 0xFF800080, 0xFF008080, 0xFFc0c0c0,
            0xFF808080, 0xFFff0000, 0xFF00ff00, 0xFFffff00, 0xFF0000ff, 0xFFff00ff, 0xFF00ffff, 0xFFffffff,
        };
        return win16;
    }

    /// winのシステム256色.
    static const unsigned* clutWin256() {
        static const unsigned win256[] = {
            0xFF000000,0xFF800000,0xFF008000,0xFF808000,0xFF000080,0xFF800080,0xFF008080,0xFF808080,0xFFc0dcc0,0xFFa6caf0,0xFF2a3faa,0xFF2a3fff,0xFF2a5f00,0xFF2a5f55,0xFF2a5faa,0xFF2a5fff,
            0xFF2a7f00,0xFF2a7f55,0xFF2a7faa,0xFF2a7fff,0xFF2a9f00,0xFF2a9f55,0xFF2a9faa,0xFF2a9fff,0xFF2abf00,0xFF2abf55,0xFF2abfaa,0xFF2abfff,0xFF2adf00,0xFF2adf55,0xFF2adfaa,0xFF2adfff,
            0xFF2aff00,0xFF2aff55,0xFF2affaa,0xFF2affff,0xFF550000,0xFF550055,0xFF5500aa,0xFF5500ff,0xFF551f00,0xFF551f55,0xFF551faa,0xFF551fff,0xFF553f00,0xFF553f55,0xFF553faa,0xFF553fff,
            0xFF555f00,0xFF555f55,0xFF555faa,0xFF555fff,0xFF557f00,0xFF557f55,0xFF557faa,0xFF557fff,0xFF559f00,0xFF559f55,0xFF559faa,0xFF559fff,0xFF55bf00,0xFF55bf55,0xFF55bfaa,0xFF55bfff,
            0xFF55df00,0xFF55df55,0xFF55dfaa,0xFF55dfff,0xFF55ff00,0xFF55ff55,0xFF55ffaa,0xFF55ffff,0xFF7f0000,0xFF7f0055,0xFF7f00aa,0xFF7f00ff,0xFF7f1f00,0xFF7f1f55,0xFF7f1faa,0xFF7f1fff,
            0xFF7f3f00,0xFF7f3f55,0xFF7f3faa,0xFF7f3fff,0xFF7f5f00,0xFF7f5f55,0xFF7f5faa,0xFF7f5fff,0xFF7f7f00,0xFF7f7f55,0xFF7f7faa,0xFF7f7fff,0xFF7f9f00,0xFF7f9f55,0xFF7f9faa,0xFF7f9fff,
            0xFF7fbf00,0xFF7fbf55,0xFF7fbfaa,0xFF7fbfff,0xFF7fdf00,0xFF7fdf55,0xFF7fdfaa,0xFF7fdfff,0xFF7fff00,0xFF7fff55,0xFF7fffaa,0xFF7fffff,0xFFaa0000,0xFFaa0055,0xFFaa00aa,0xFFaa00ff,
            0xFFaa1f00,0xFFaa1f55,0xFFaa1faa,0xFFaa1fff,0xFFaa3f00,0xFFaa3f55,0xFFaa3faa,0xFFaa3fff,0xFFaa5f00,0xFFaa5f55,0xFFaa5faa,0xFFaa5fff,0xFFaa7f00,0xFFaa7f55,0xFFaa7faa,0xFFaa7fff,
            0xFFaa9f00,0xFFaa9f55,0xFFaa9faa,0xFFaa9fff,0xFFaabf00,0xFFaabf55,0xFFaabfaa,0xFFaabfff,0xFFaadf00,0xFFaadf55,0xFFaadfaa,0xFFaadfff,0xFFaaff00,0xFFaaff55,0xFFaaffaa,0xFFaaffff,
            0xFFd40000,0xFFd40055,0xFFd400aa,0xFFd400ff,0xFFd41f00,0xFFd41f55,0xFFd41faa,0xFFd41fff,0xFFd43f00,0xFFd43f55,0xFFd43faa,0xFFd43fff,0xFFd45f00,0xFFd45f55,0xFFd45faa,0xFFd45fff,
            0xFFd47f00,0xFFd47f55,0xFFd47faa,0xFFd47fff,0xFFd49f00,0xFFd49f55,0xFFd49faa,0xFFd49fff,0xFFd4bf00,0xFFd4bf55,0xFFd4bfaa,0xFFd4bfff,0xFFd4df00,0xFFd4df55,0xFFd4dfaa,0xFFd4dfff,
            0xFFd4ff00,0xFFd4ff55,0xFFd4ffaa,0xFFd4ffff,0xFFff0055,0xFFff00aa,0xFFff1f00,0xFFff1f55,0xFFff1faa,0xFFff1fff,0xFFff3f00,0xFFff3f55,0xFFff3faa,0xFFff3fff,0xFFff5f00,0xFFff5f55,
            0xFFff5faa,0xFFff5fff,0xFFff7f00,0xFFff7f55,0xFFff7faa,0xFFff7fff,0xFFff9f00,0xFFff9f55,0xFFff9faa,0xFFff9fff,0xFFffbf00,0xFFffbf55,0xFFffbfaa,0xFFffbfff,0xFFffdf00,0xFFffdf55,
            0xFFffdfaa,0xFFffdfff,0xFFffff55,0xFFffffaa,0xFFccccff,0xFFffccff,0xFF33ffff,0xFF66ffff,0xFF99ffff,0xFFccffff,0xFF007f00,0xFF007f55,0xFF007faa,0xFF007fff,0xFF009f00,0xFF009f55,
            0xFF009faa,0xFF009fff,0xFF00bf00,0xFF00bf55,0xFF00bfaa,0xFF00bfff,0xFF00df00,0xFF00df55,0xFF00dfaa,0xFF00dfff,0xFF00ff55,0xFF00ffaa,0xFF2a0000,0xFF2a0055,0xFF2a00aa,0xFF2a00ff,
            0xFF2a1f00,0xFF2a1f55,0xFF2a1faa,0xFF2a1fff,0xFF2a3f00,0xFF2a3f55,0xFFfffbf0,0xFFa0a0a4,0xFF808080,0xFFff0000,0xFF00ff00,0xFFffff00,0xFF0000ff,0xFFff00ff,0xFF00ffff,0xFFffffff,
        };
        return win256;
    }

    /// 固定GRB 64色. (g2r2b2)
    static const unsigned* clutGRB222() {
        static const unsigned grb222[] = {
            0xff000000,0xff000055,0xff0000aa,0xff0000ff,0xff550000,0xff550055,0xff5500aa,0xff5500ff,0xffaa0000,0xffaa0055,0xffaa00aa,0xffaa00ff,0xffff0000,0xffff0055,0xffff00aa,0xffff00ff,
            0xff005500,0xff005555,0xff0055aa,0xff0055ff,0xff555500,0xff555555,0xff5555aa,0xff5555ff,0xffaa5500,0xffaa5555,0xffaa55aa,0xffaa55ff,0xffff5500,0xffff5555,0xffff55aa,0xffff55ff,
            0xff00aa00,0xff00aa55,0xff00aaaa,0xff00aaff,0xff55aa00,0xff55aa55,0xff55aaaa,0xff55aaff,0xffaaaa00,0xffaaaa55,0xffaaaaaa,0xffaaaaff,0xffffaa00,0xffffaa55,0xffffaaaa,0xffffaaff,
            0xff00ff00,0xff00ff55,0xff00ffaa,0xff00ffff,0xff55ff00,0xff55ff55,0xff55ffaa,0xff55ffff,0xffaaff00,0xffaaff55,0xffaaffaa,0xffaaffff,0xffffff00,0xffffff55,0xffffffaa,0xffffffff,
        };
        return grb222;
    }

    /// 固定GRB 256色. (g3r3b2)
    static const unsigned* clutGRB332() {
        static const unsigned grb332[] = {
            0xff000000,0xff000055,0xff0000aa,0xff0000ff,0xff240000,0xff240055,0xff2400aa,0xff2400ff,0xff490000,0xff490055,0xff4900aa,0xff4900ff,0xff6d0000,0xff6d0055,0xff6d00aa,0xff6d00ff,
            0xff920000,0xff920055,0xff9200aa,0xff9200ff,0xffb60000,0xffb60055,0xffb600aa,0xffb600ff,0xffdb0000,0xffdb0055,0xffdb00aa,0xffdb00ff,0xffff0000,0xffff0055,0xffff00aa,0xffff00ff,
            0xff002400,0xff002455,0xff0024aa,0xff0024ff,0xff242400,0xff242455,0xff2424aa,0xff2424ff,0xff492400,0xff492455,0xff4924aa,0xff4924ff,0xff6d2400,0xff6d2455,0xff6d24aa,0xff6d24ff,
            0xff922400,0xff922455,0xff9224aa,0xff9224ff,0xffb62400,0xffb62455,0xffb624aa,0xffb624ff,0xffdb2400,0xffdb2455,0xffdb24aa,0xffdb24ff,0xffff2400,0xffff2455,0xffff24aa,0xffff24ff,
            0xff004900,0xff004955,0xff0049aa,0xff0049ff,0xff244900,0xff244955,0xff2449aa,0xff2449ff,0xff494900,0xff494955,0xff4949aa,0xff4949ff,0xff6d4900,0xff6d4955,0xff6d49aa,0xff6d49ff,
            0xff924900,0xff924955,0xff9249aa,0xff9249ff,0xffb64900,0xffb64955,0xffb649aa,0xffb649ff,0xffdb4900,0xffdb4955,0xffdb49aa,0xffdb49ff,0xffff4900,0xffff4955,0xffff49aa,0xffff49ff,
            0xff006d00,0xff006d55,0xff006daa,0xff006dff,0xff246d00,0xff246d55,0xff246daa,0xff246dff,0xff496d00,0xff496d55,0xff496daa,0xff496dff,0xff6d6d00,0xff6d6d55,0xff6d6daa,0xff6d6dff,
            0xff926d00,0xff926d55,0xff926daa,0xff926dff,0xffb66d00,0xffb66d55,0xffb66daa,0xffb66dff,0xffdb6d00,0xffdb6d55,0xffdb6daa,0xffdb6dff,0xffff6d00,0xffff6d55,0xffff6daa,0xffff6dff,
            0xff009200,0xff009255,0xff0092aa,0xff0092ff,0xff249200,0xff249255,0xff2492aa,0xff2492ff,0xff499200,0xff499255,0xff4992aa,0xff4992ff,0xff6d9200,0xff6d9255,0xff6d92aa,0xff6d92ff,
            0xff929200,0xff929255,0xff9292aa,0xff9292ff,0xffb69200,0xffb69255,0xffb692aa,0xffb692ff,0xffdb9200,0xffdb9255,0xffdb92aa,0xffdb92ff,0xffff9200,0xffff9255,0xffff92aa,0xffff92ff,
            0xff00b600,0xff00b655,0xff00b6aa,0xff00b6ff,0xff24b600,0xff24b655,0xff24b6aa,0xff24b6ff,0xff49b600,0xff49b655,0xff49b6aa,0xff49b6ff,0xff6db600,0xff6db655,0xff6db6aa,0xff6db6ff,
            0xff92b600,0xff92b655,0xff92b6aa,0xff92b6ff,0xffb6b600,0xffb6b655,0xffb6b6aa,0xffb6b6ff,0xffdbb600,0xffdbb655,0xffdbb6aa,0xffdbb6ff,0xffffb600,0xffffb655,0xffffb6aa,0xffffb6ff,
            0xff00db00,0xff00db55,0xff00dbaa,0xff00dbff,0xff24db00,0xff24db55,0xff24dbaa,0xff24dbff,0xff49db00,0xff49db55,0xff49dbaa,0xff49dbff,0xff6ddb00,0xff6ddb55,0xff6ddbaa,0xff6ddbff,
            0xff92db00,0xff92db55,0xff92dbaa,0xff92dbff,0xffb6db00,0xffb6db55,0xffb6dbaa,0xffb6dbff,0xffdbdb00,0xffdbdb55,0xffdbdbaa,0xffdbdbff,0xffffdb00,0xffffdb55,0xffffdbaa,0xffffdbff,
            0xff00ff00,0xff00ff55,0xff00ffaa,0xff00ffff,0xff24ff00,0xff24ff55,0xff24ffaa,0xff24ffff,0xff49ff00,0xff49ff55,0xff49ffaa,0xff49ffff,0xff6dff00,0xff6dff55,0xff6dffaa,0xff6dffff,
            0xff92ff00,0xff92ff55,0xff92ffaa,0xff92ffff,0xffb6ff00,0xffb6ff55,0xffb6ffaa,0xffb6ffff,0xffdbff00,0xffdbff55,0xffdbffaa,0xffdbffff,0xffffff00,0xffffff55,0xffffffaa,0xffffffff,
        };
        return grb332;
    }

	static const unsigned* clutg6r6b6c40() {
        static const unsigned clut[] = {
			0xff000000,0xff00003f,0xff00006f,0xff00009f,0xff0000cf,0xff0000ff,0xff3f0000,0xff3f003f,0xff3f006f,0xff3f009f,0xff3f00cf,0xff3f00ff,0xff6f0000,0xff6f003f,0xff6f006f,0xff6f009f,
			0xff6f00cf,0xff6f00ff,0xff9f0000,0xff9f003f,0xff9f006f,0xff9f009f,0xff9f00cf,0xff9f00ff,0xffcf0000,0xffcf003f,0xffcf006f,0xffcf009f,0xffcf00cf,0xffcf00ff,0xffff0000,0xffff003f,
			0xffff006f,0xffff009f,0xffff00cf,0xffff00ff,0xff003f00,0xff003f3f,0xff003f6f,0xff003f9f,0xff003fcf,0xff003fff,0xff3f3f00,0xff3f3f3f,0xff3f3f6f,0xff3f3f9f,0xff3f3fcf,0xff3f3fff,
			0xff6f3f00,0xff6f3f3f,0xff6f3f6f,0xff6f3f9f,0xff6f3fcf,0xff6f3fff,0xff9f3f00,0xff9f3f3f,0xff9f3f6f,0xff9f3f9f,0xff9f3fcf,0xff9f3fff,0xffcf3f00,0xffcf3f3f,0xffcf3f6f,0xffcf3f9f,
			0xffcf3fcf,0xffcf3fff,0xffff3f00,0xffff3f3f,0xffff3f6f,0xffff3f9f,0xffff3fcf,0xffff3fff,0xff006f00,0xff006f3f,0xff006f6f,0xff006f9f,0xff006fcf,0xff006fff,0xff3f6f00,0xff3f6f3f,
			0xff3f6f6f,0xff3f6f9f,0xff3f6fcf,0xff3f6fff,0xff6f6f00,0xff6f6f3f,0xff6f6f6f,0xff6f6f9f,0xff6f6fcf,0xff6f6fff,0xff9f6f00,0xff9f6f3f,0xff9f6f6f,0xff9f6f9f,0xff9f6fcf,0xff9f6fff,
			0xffcf6f00,0xffcf6f3f,0xffcf6f6f,0xffcf6f9f,0xffcf6fcf,0xffcf6fff,0xffff6f00,0xffff6f3f,0xffff6f6f,0xffff6f9f,0xffff6fcf,0xffff6fff,0xff009f00,0xff009f3f,0xff009f6f,0xff009f9f,
			0xff009fcf,0xff009fff,0xff3f9f00,0xff3f9f3f,0xff3f9f6f,0xff3f9f9f,0xff3f9fcf,0xff3f9fff,0xff6f9f00,0xff6f9f3f,0xff6f9f6f,0xff6f9f9f,0xff6f9fcf,0xff6f9fff,0xff9f9f00,0xff9f9f3f,
			0xff9f9f6f,0xff9f9f9f,0xff9f9fcf,0xff9f9fff,0xffcf9f00,0xffcf9f3f,0xffcf9f6f,0xffcf9f9f,0xffcf9fcf,0xffcf9fff,0xffff9f00,0xffff9f3f,0xffff9f6f,0xffff9f9f,0xffff9fcf,0xffff9fff,
			0xff00cf00,0xff00cf3f,0xff00cf6f,0xff00cf9f,0xff00cfcf,0xff00cfff,0xff3fcf00,0xff3fcf3f,0xff3fcf6f,0xff3fcf9f,0xff3fcfcf,0xff3fcfff,0xff6fcf00,0xff6fcf3f,0xff6fcf6f,0xff6fcf9f,
			0xff6fcfcf,0xff6fcfff,0xff9fcf00,0xff9fcf3f,0xff9fcf6f,0xff9fcf9f,0xff9fcfcf,0xff9fcfff,0xffcfcf00,0xffcfcf3f,0xffcfcf6f,0xffcfcf9f,0xffcfcfcf,0xffcfcfff,0xffffcf00,0xffffcf3f,
			0xffffcf6f,0xffffcf9f,0xffffcfcf,0xffffcfff,0xff00ff00,0xff00ff3f,0xff00ff6f,0xff00ff9f,0xff00ffcf,0xff00ffff,0xff3fff00,0xff3fff3f,0xff3fff6f,0xff3fff9f,0xff3fffcf,0xff3fffff,
			0xff6fff00,0xff6fff3f,0xff6fff6f,0xff6fff9f,0xff6fffcf,0xff6fffff,0xff9fff00,0xff9fff3f,0xff9fff6f,0xff9fff9f,0xff9fffcf,0xff9fffff,0xffcfff00,0xffcfff3f,0xffcfff6f,0xffcfff9f,
			0xffcfffcf,0xffcfffff,0xffffff00,0xffffff3f,0xffffff6f,0xffffff9f,0xffffffcf,0xffffffff,0xff1f1f1f,0xff1f1f3f,0xff1f1f6f,0xff3f1f1f,0xff3f1f3f,0xff3f1f6f,0xff6f1f1f,0xff6f1f3f,
			0xff6f1f6f,0xff1f3f1f,0xff1f3f3f,0xff1f3f6f,0xff3f3f1f,0xff3f3f3f,0xff3f3f6f,0xff6f3f1f,0xff6f3f3f,0xff6f3f6f,0xff1f6f1f,0xff1f6f3f,0xff1f6f6f,0xff3f6f1f,0xff3f6f3f,0xff3f6f6f,
			0xff6f6f1f,0xff6f6f3f,0xff6f6f6f,0xff1f1f1f,0xff1f1f9f,0xff9f1f1f,0xff9f1f9f,0xff1f9f1f,0xff1f9f9f,0xff9f9f1f,0xff9f9f9f,0xff2f2f2f,0xff585858,0xff888888,0xffb8b8b8,0xffe8e8e8,
		};
        return clut;
    }
};




/** デフォルトパレットの作成.
 * bpp : 3,4,6,8   typ : 0=jpn 1=pcat/win
 */
template<class A>
void FixedClut256<A>::getFixedClut256(unsigned *clut, unsigned clutSize, unsigned bpp, unsigned typ)
{
    assert(clutSize > 0);
    if (clutSize > 256)
        clutSize = 256;

    unsigned        l;
    const unsigned* s;
    if (bpp < 6) {
        if (bpp < 4) {
            l = 8;
            s = typ ? clutBGR111() : clutGRB111();
        } else {
            l = 16;
            s = typ ? clutWin16() : clutJpn16();
        }
    } else {
        if (bpp < 8) {
            l = 64;
            s = clutGRB222();
        } else {
            l = 256;
            switch (typ) {
			default:
			case 0: s = clutGRB332(); break;
			case 1: s = clutWin256(); break;
			case 2: s = clutg6r6b6c40(); break;
			}
        }
    }
    if (l > clutSize)
        l = clutSize;
    memset(clut, 0, clutSize * sizeof(clut[0]));
    memcpy(clut, s, l        * sizeof(clut[0]));
}



/// 32ビット色画を8ビット色画に変換.
template<class A>
bool FixedClut256<A>::decreaseColor(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h
                                , const unsigned* pClut, unsigned clutSize, unsigned idx, int alp1, int alp2)
{
    assert(clutSize <= 256 && clutSize >= idx+2);
    enum { ALP_D = 2 };

    // 指定のclutで、すべてのピクセルをclutSize色化.
    // 近い色の条件として、
    //   - a,r,g,bの値が近いモノ(色同士の差が小さいモノ)
    //   - 1色内のr,g,bの輝度の大小関係が、近いモノ
    // をチェック.
    for (unsigned j = 0; j < w * h; ++j) {
        unsigned ii = 0;
        unsigned c  = pSrc[j];
        int      a  = c >> 24;
        int      r  = (unsigned char)(c >> 16);
        int      g  = (unsigned char)(c >>  8);
        int      b  = (unsigned char)(c);
        int      o  = g - r;
        int      p  = r - b;
        int      q  = b - g;
        a = (a < alp1) ? 0 : (a >= alp2) ? 255 : a;
        unsigned k = 0xFFFFFFFF;
        for (unsigned i = idx; i < clutSize; ++i) {
            unsigned ic = pClut[i];
            int      ia = ic >> 24;
            ia = (ia < alp1) ? 0 : (ia > alp2) ? 255 : ia;
            int      ir = (unsigned char)(ic >> 16);
            int      ig = (unsigned char)(ic >>  8);
            int      ib = (unsigned char)(ic);
            int      io = ig - ir;
            int      ip = ir - ib;
            int      iq = ib - ig;

            int      aa = ia - a;
            int      gg = ig - g;
            int      rr = ir - r;
            int      bb = ib - b;
            int      oo = io - o;
            int      pp = ip - p;
            int      qq = iq - q;
            unsigned ik = 3*aa*aa + gg*gg + rr*rr + bb*bb + oo*oo + pp*pp + qq*qq;
            if (ik < k) {
                k  = ik;
                ii = i;
            }
        }
        pDst[j] = ii;
    }

    return true;
}


/// グレイ画像か?
template<class A>
bool FixedClut256<A>::isGrey(const unsigned* pSrc, unsigned w, unsigned h)
{
	unsigned a = ARGB_A(pSrc[0]);
	for (unsigned j = 0; j < w * h; ++j) {
		unsigned c = pSrc[j];
		unsigned r = ARGB_R(c);
		if (r != ARGB_G(c) || r != ARGB_B(c) || a != ARGB_A(c))
			return false;
	}
    return true;
}


/// グレイ画像に近いか?
template<class A>
bool FixedClut256<A>::isNearGrey(const unsigned* pSrc, unsigned w, unsigned h, int threshold)
{
	size_t	wh     = w * h;
	size_t  comp   = 0;
	size_t  near1  = 0;
	size_t  near2  = 0;
	size_t  ufar   = 0;
	size_t  vfar   = 0;
	int		a      = ARGB_A(pSrc[0]);
	for (size_t j = 0; j < wh; ++j) {
		unsigned c = pSrc[j];
		int		 r = ARGB_R(c);
		int      g = ARGB_G(c);
		int      b = ARGB_B(c);
		//int	 y = ( 58661*g +  29891*r + 11448*b) * (2048>>5) / ( 255 * (100000>>5) );
		int		 u = (-33126*g -  16874*r + 50000*b) * (1024>>5) / ( 255 * (100000>>5) ); // + 512;
		int		 v = (-41869*g +  50000*r -  8131*b) * (1024>>5) / ( 255 * (100000>>5) ); // + 512;
		int au = abs(u);
		int av = abs(v);
		if (au == 0 && av == 0) {
			++comp;
		} else if (au <= 64 && av <= 64) {
			++near1;
		} else if (au <= 128 && av <= 128) {
			++near2;
		} else {
			if (au > 128)
				++ufar;
			if (av > 128)
				++vfar;
		}
	}
 #if 0
	printf(" @chk comp:%%%5.2f", double(comp)*100/wh);
	printf(" n1:%%%5.2f", double(near1)*100/wh);
	printf(" n2:%%%5.2f", double(near2)*100/wh);
	printf(" uf=%%%5.2f(%6d)", double(ufar)*100/wh, ufar);
	printf(" vf=%%%5.2f(%6d)", double(vfar)*100/wh, vfar);
 #endif
	return (100 * comp / wh >= 80) /* && (1000 * near2 / wh == 0)*/ && (100000 * ufar / wh == 0) && (100000 * vfar / wh == 0);
}


/** グレイ(モノクロ)パレットの作成.
 * bpp : 3,4,6,8
 */
template<class A>
void FixedClut256<A>::getFixedGreyClut(unsigned *clut, unsigned clutSize, unsigned bpp)
{
    assert(clutSize > 0 && bpp > 0 && bpp <= 8);
    if (clutSize > 256)
        clutSize = 256;
	unsigned size = 1 << bpp;
	if (size > clutSize)
		size   = clutSize;
	int maxVal = size - 1;
	unsigned i;
	for (i = 0; i < size; ++i) {
		unsigned c = 255 * i / maxVal;
		clut[i] = (0xFF<<24)|(c << 16)|(c << 8)|c;
	}
	for (; i < clutSize; ++i) {
		clut[i] = 0;
	}
}


/// モノクロの32ビット色画を前提に8ビット色画に変換.
template<class A>
bool FixedClut256<A>::fromGreyToBpp8(unsigned char* pDst, const unsigned* pSrc, unsigned w, unsigned h)
{
    for (unsigned j = 0; j < w * h; ++j)
		pDst[j] = (uint8_t)pSrc[j];
    return true;
}

#endif
