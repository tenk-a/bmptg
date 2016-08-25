/**
 *  @file   gen_clmp.c
 *  @brief  �Z��/�}�b�v��
 *  @author Masashi Kitamura
 *  @date   2001-01-28
 *  @note
 *      ��{�� 32�r�b�g�F�p�����ACLMP_BPP_8 ���`���ăR���p�C������� 8�r�b�g�F�p�ɂȂ�B
 */

#include "subr.h"
#include "mem_mac.h"
#include "gen.h"


#ifdef CLMP_BPP_8   // 8bit�F�ł́A�}�N���ł��܂����Đ���
#define pix_t       uint8_t
#define gen_clmp    gen_clmp8
#else
#define pix_t       uint32_t
#define gen_clmp    gen_clmp32
#endif

#define PIX_BYT     sizeof(pix_t)

#define CEL_NUM_MAX (1<<14)
#define MAP_HDR_SZ  32

static void genMap(uint8_t *dst, int *map, int flags, int bpp, int cnum, int mw, int mh, int cw, int ch, int w0, int h0, int x0, int y0, int fuchiTyp);
static void genTex(pix_t *pix, int texW, int th, int cw, int ch, int cnum, const pix_t *fnt, int bw, int bh);
//static void genTexSp(pix_t *pix, int texW, int tw, int th, int cw, int ch, int cnum, const pix_t *fnt /*, int bw, int bh*/);
static pix_t *resizePixBuf(pix_t *src, int srcW, int srcH, int mw, int mh, int cw, int ch);
static pix_t *genCelFuchiTex(pix_t *src, int srcW, int srcH, int mw, int mh, int cw, int ch);
static void cels_init(int cw, int ch, int mutoumeiFlg, int numMax, int nukiCol, uint32_t *clut, int nocmpMode);
static void cels_term(void);
static pix_t *cels_fntAdr(void);
//static int  cels_getNum(void);
static int  cels_pix2map(pix_t *pix, int *map, int xsz, int ysz, int cw, int ch, int flag);
static int  cels_add(void *fnt);



int gen_clmp(uint8_t **a_map, int *a_mapSz, int *a_celNum, uint8_t **a_pix, int *a_w, int *a_h, int x0, int y0, int texW, int texH, int cw, int ch, int md, int bpp, int nukiCol, int flags, void *clut, int w0, int h0)
{
    //int   mtFlg    = flags & 2;       // ���Ƃ�
    int   t256x256Flg = (flags>>3) & 1;
    int   allNoCmpFlg = (flags>>2) & 1;
    int   nocmpFlg    = (flags>>1) & 1;
    int   fuchiTyp    = flags & 1;
    int   w = *a_w;
    int   h = *a_h;
    pix_t *pix = (pix_t*)*a_pix;
    int mw, mh, tw, th, cnum,cnumMax;
    //uint8_t *mapDt;
    int   *map;
    int w2,h2,cw2,ch2;

    if (cw <= 0 || cw > 4096 || ch <= 0 || ch > 4096) {
        err_abortMsg("�Z���T�C�Y���s��\n");
        return 0;
    }

    mw = (w+cw-1) / cw;
    mh = (h+ch-1) / ch;

    w2 = w, h2 = h, cw2 = cw, ch2 = ch;
    if (fuchiTyp) {
        pix = genCelFuchiTex(pix, w, h, mw, mh, cw, ch);
        cw2 = cw + 2, ch2 = ch + 2;
        w2 = mw * cw2, h2 = mh * ch2;
    } else if (w % cw || h % ch) {	// �[��������Ȃ�pix�T�C�Y����
        pix = resizePixBuf(pix, w, h, mw, mh, cw, ch);
		cw2 = cw      , ch2 = ch;
        w2  = mw * cw2, h2  = mh * ch2;
	}

    tw = texW / cw2;
    if (tw <= 0) {
        err_abortMsg("�e�N�X�`���T�C�Y���Z���T�C�Y��菬����\n");
        return 0;
    }
    //printf("%d, %d*%d, %d*%d, %d*%d, (%d*%d)\n", bpp, mw,mh,cw,ch,w,h,texW,texH);

    /* �}�b�v�쐬�̏��� */
    map = (int*)callocE(4/*PIX_BYT*/, mw * mh + 64);

    cnumMax = mw * mh + 4;
    if (cnumMax >= CEL_NUM_MAX) {
        cnumMax = CEL_NUM_MAX;
    }
    /* �}�b�v���Z���t�H���g�쐬 */
    cels_init(cw2, ch2, md & 1, cnumMax, nukiCol, (uint32_t*)clut, nocmpFlg | (allNoCmpFlg << 1));
    cnum = cels_pix2map(pix, map, w2, h2, cw2, ch2, t256x256Flg);
    freeE(pix);
    if (cnum <= 1)
        cnum = 2;
    th = (cnum-1 + tw-1) / tw;
    if (dbgExLog_getSw()) printf("   cel[%d*%d] map%d*%d  cel��:0x%x��\n", cw, ch, mw, mh, cnum);
    if (cnum <= 256 && md == 2) {
        *a_mapSz = (MAP_HDR_SZ+mw*mh*1);
    } else {
        *a_mapSz = (MAP_HDR_SZ+mw*mh*2);
    }
    // �}�b�v�f�[�^�𐮂���
    *a_map = (uint8_t*)callocE(1, *a_mapSz + 64);
    genMap(*a_map, map, md, bpp, cnum, mw, mh, cw, ch, w0, h0, x0, y0, fuchiTyp);

    // �e�N�X�`������
    DBG_M();
    {
        int thch2;
        if (th == 0)
            th = 1;
        thch2 = th * ch2;
        if (thch2 == 0)
            thch2 = texH;
        if (texH == 0) {
            texH = thch2;
        } else {
            texH = ((thch2 + texH - 1) / texH) * texH;
        }
    }
    *a_pix = (uint8_t*)callocE(PIX_BYT, texW*texH);
    genTex((pix_t*)*a_pix, texW, th, cw2, ch2, cnum, cels_fntAdr(), w2,h2);

    /* �I������ */
    DBG_M();
    *a_celNum = cnum;
    *a_w = texW;
    *a_h = texH;

    cels_term();
    freeE(map);
    return 1;
}


static void genMap(uint8_t *dst, int *map, int flags, int bpp, int cnum, int mw, int mh, int cw, int ch, int w0, int h0, int x0, int y0, int fuchiTyp)
{
    int n,x,y;
    //printf("%x,%d,%d, %d*%d, %d*%d, %d*%d, (%d,%d)\n", flags, bpp, cnum, mw,mh,cw,ch,w0,h0,x0,y0);
    if (fuchiTyp == 0)  // �ʏ�
        memcpy(dst, "MAP\0", 4);
    else                // ���L��Z��
        memcpy(dst, "MAP\1", 4);
    POKEiD(dst+0x04, 0);
    POKEiD(dst+0x08, 0);
    POKEB (dst+0x0c, flags);
    POKEB (dst+0x0d, bpp);
    POKEiW(dst+0x0e, cnum);
    POKEiW(dst+0x10, mw);
    POKEiW(dst+0x12, mh);
    POKEiW(dst+0x14, cw);
    POKEiW(dst+0x16, ch);
    POKEiW(dst+0x18, w0);
    POKEiW(dst+0x1a, h0);
    POKEiW(dst+0x1c, x0);
    POKEiW(dst+0x1e, y0);
    dst += 0x20;
    n = 0;
    for (y = 0; y < mh; y++) {
        for (x = 0; x < mw; x++) {
            if (flags == 2)
                POKEB(dst+n, map[n]);
            else
                POKEiW(dst+n*2, map[n]);
            n++;
        }
    }
}


static void genTex(pix_t *pix, int texW, int th, int cw, int ch, int cnum, const pix_t *fnt, int bw, int bh)
{
    int tw = texW / cw;
    int n, tx,ty,x,y;

    n = 1;
    fnt += n*ch*cw;
    for (ty = 0; ty < th; ty++) {
        for (tx = 0; tx < tw; tx++) {
            pix_t *p = pix + ty * ch * texW + tx * cw;
            for (y = 0; y < ch; y++) {
                for (x = 0; x < cw; x++) {
                    if (x < bw && y < bh)
                        p[x] = *fnt;
                    fnt++;
                }
                p += texW;
            }
            if (++n >= cnum)
                return;
        }
    }
}


#if 0
static void genTexSp(pix_t *pix, int texW, int tw, int th, int cw, int ch, int cnum, const pix_t *fnt /*, int bw, int bh*/)
{
    // �o�C���j�A�t�B���^�΍�Ŏ��͂P�h�b�g�����B�����Z����~���l�߂�
    int n, tx,ty,x,y, ch2=ch+2, cw2=cw+2;

    n = 1;
    fnt += n*ch*cw;
    for (ty = 0; ty < th; ty++) {
        for (tx = 0; tx < tw; tx++) {
            pix_t *p = pix + ty * ch2 * texW + tx * cw2;

            //��ӂ̃R�s�[
            p[0] = fnt[0];              //p[0*texW + 0] = fnt[0*cw+0];
            for (x = 0; x < cw; x++)
                p[1+x] = fnt[x];        //  p[0*texW + 1+x] = fnt[0*cw+x];
            p[1+cw] = fnt[cw-1];        //p[0*texW + 1+cw] = fnt[0*cw+cw-1];

            //���ӂ̃R�s�[
            p[(1+ch)*texW + 0] = fnt[(ch-1)*cw+0];
            for (x = 0; x < cw; x++)
                p[(1+ch)*texW + 1+x] = fnt[(ch-1)*cw+x];
            p[(1+ch)*texW + 1+cw] = fnt[(ch-1)*cw+cw-1];

            // ���g
            for (y = 0; y < ch; y++) {
                p[(1+y)*texW + 0] = fnt[y*cw+0];        //����
                for (x = 0; x < cw; x++) {
                    p[(1+y)*texW + 1+x] = fnt[y*cw+x];
                }
                p[(1+y)*texW + 1+cw] = fnt[y*cw+cw-1];  //�E��
            }
            if (++n >= cnum)
                return;
            fnt += ch*cw;
        }
    }
}
#endif



static pix_t *resizePixBuf(pix_t *src, int srcW, int srcH, int mw, int mh, int cw, int ch)
{
	int 	dstW = mw * cw;
	int 	dstH = mh * ch;
    pix_t*	dst  = (pix_t*)callocE(PIX_BYT, dstW * dstH+0x1000);
	int		y;

    if (dbgExLog_getSw()) {
        printf("resizePixBuf(%p,  %d,%d,  %d,%d,  %d,%d)\n", src, srcW,srcH,mw,mh,cw,ch);
    }

	for (y = 0; y < srcH; ++y)
		memcpy(dst + y * dstW, src + y * srcW, srcW * PIX_BYT);

	free(src);
	return dst;
}



static pix_t *genCelFuchiTex(pix_t *src, int srcW, int srcH, int mw, int mh, int cw, int ch)
{
    int cw2 = cw + 2, ch2 = ch + 2;
    int dstW = mw * cw2, dstH = mh * ch2;
    pix_t *dst = (pix_t*)callocE(PIX_BYT, dstW * dstH+0x1000);
    pix_t *d;
    int mx,my, x,y;

    if (dbgExLog_getSw()) {
        printf("genCelFuchiTex(%p,  %d,%d,  %d,%d,  %d,%d)\n", src, srcW,srcH,mw,mh,cw,ch);
        printf("  %d*%d %d*%d\n", cw2, ch2, dstW, dstH);
    }

    for (my = 0; my < mh; my++) {
        for (mx = 0; mx < mw; mx++) {
            d = &dst[(my * ch2) * dstW + mx*cw2];
            #undef SP
            #define SP(c,x,y)   do {\
                int x_ = (x)+mx*cw, y_ = (y)+my*ch;\
                if (x_ < 0) x_ = 0;\
                else if (x_ >= srcW) x_ = srcW-1;\
                if (y_ < 0) y_ = 0;\
                else if (y_ >= srcH) y_ = srcH-1;\
                (c) = src[y_ * srcW + x_];\
            } while(0)

            SP(d[0], -1, -1);
            for (x = 0; x < cw; x++) {
                SP(d[1+x], x, -1);
            }
            SP(d[1+cw], cw, -1);

            SP(d[(1+ch)*dstW+0], -1, ch);
            for (x = 0; x < cw; x++) {
                SP(d[(1+ch)*dstW+1+x], x, ch);
            }
            SP(d[(1+ch)*dstW+1+cw], cw, ch);

            for (y = 0; y < ch; y++) {
                SP(d[(1+y)*dstW+0], -1, y);
                for (x = 0; x < cw; x++) {
                    SP(d[(1+y)*dstW+1+x], x, y);
                }
                SP(d[(1+y)*dstW+1+cw], cw, y);
            }
            #undef SP
        }
    }
    freeE(src);
    return dst;
}




/*---------------------------------------------------------------------------*/

/// �Z��&�}�b�v���ł̃Z���f�[�^�̊Ǘ�
typedef struct cels_t {
    pix_t  *fnt;
    int     num;
    int     sz1;
    int     numMax;
    int     mutoumeiFlg;
    int     nocmpMode;
    int     *atrs;
    int     nukiCol;
    uint32_t    *clut;
} cels_t;

static cels_t cels;


/** �t�H���g�����p�ɕ��ѕς����t�H���g�e�[�u�����쐬 */
static void cels_init(int cw, int ch, int mutoumeiFlg, int numMax, int nukiCol, uint32_t *clut, int nocmpMode)
{
    cels.num    = 1;
    cels.sz1    = cw*ch;
    cels.numMax = numMax;
    cels.mutoumeiFlg = mutoumeiFlg;
    cels.nocmpMode = nocmpMode;
    cels.nukiCol= nukiCol;
    cels.fnt    = (pix_t*)callocE(PIX_BYT, cels.sz1 * cels.numMax);
    cels.atrs   = (int*)callocE(sizeof(int), cels.numMax);
    cels.clut   = clut;
}


static void cels_term(void)
{
    freeE(cels.fnt);
    freeE(cels.atrs);
}


static pix_t *cels_fntAdr(void)
{
    return (pix_t*)cels.fnt;
}


//static int  cels_getNum(void)
//{
//  return cels.num;
//}


static int cels_pix2map(pix_t *pix, int *map, int xsz, int ysz, int cw, int ch, int flag)
{
    pix_t *buf;
    int   u,v,x,y, cx;
    int xx, yy;

    if (dbgExLog_getSw()) printf("cels_pix2map %p %p %d %d %d %d\n",pix,map,xsz,ysz,cw,ch);
    cx = xsz / cw;
    buf = (pix_t*)callocE(1, cw*ch*PIX_BYT+15);
    if (flag) {
        for (xx = 0; xx < xsz; xx += 256) {
            for (yy = 0; yy < ysz; yy += 256) {
                for (y = yy; y < yy+256 && y < ysz; y+=ch) {
                    for (x = xx; x < xx+256 && x < xsz; x+=cw) {
                        if (dbgExLog_getSw()) printf("MAP(%d,%d:%d,%d)\n", x,y, y/ch,x/cw);
                        for (v = 0; v < ch; v++) {
                            for (u = 0; u < cw; u++) {
                                //printf("%d,%d, %d,%d\n", x,y, u,v);
                                if ((x+u) < xsz && (y+v) < ysz)
                                    buf[v*cw + u] = pix[(y+v)*xsz+(x+u)];
                            }
                        }
                        map[(y/ch) * cx + (x/cw)] = cels_add(buf);
                    }
                }
            }
        }
    } else {
        for (y = 0; y < ysz; y+=ch) {
            for (x = 0; x < xsz; x+=cw) {
                if (dbgExLog_getSw()) printf("MAP(%d,%d:%d,%d)\n", x,y, y/ch,x/cw);
                for (v = 0; v < ch; v++) {
                    for (u = 0; u < cw; u++) {
                        //printf("%d,%d, %d,%d\n", x,y, u,v);
                        if ((x+u) < xsz && (y+v) < ysz)
                            buf[v*cw + u] = pix[(y+v)*xsz+(x+u)];
                    }
                }
                map[(y/ch) * cx + (x/cw)] = cels_add(buf);
            }
        }
    }
    freeE(buf);
    if (dbgExLog_getSw()) printf("cels_pix2map end %p %p %d %d %d %d\n",pix,map,xsz,ysz,cw,ch);
    return cels.num;
}



/** �o�^�ς݃Z���̃t�H���g��T��, ������΂��̔ԍ����A�����łȂ����
 * �V�K�o�^���āA���̔ԍ���Ԃ�
 */
static int cels_add(void *fnt)
{
    int f = 0, m = 1, af = 0;
    int n, c;
  #ifndef CLMP_BPP_8
    int a;
  #endif

    //DBG_F(("%d, %d\n", cels.num, cels.sz1));
    /* �蔲������ */
    if (cels.nocmpMode == 0) {
        for (n = 0; n < cels.num; n++) {
            if (memcmp(cels.fnt + n * cels.sz1, fnt, cels.sz1*PIX_BYT) == 0) {
                return cels.atrs[n];    //�����������炻�̔ԍ���Ԃ�
            }
        }
    }
    /* �\���s�N�Z�����A���ׂĔ����F��, ���ׂĖ��������A�ǂ����`�F�b�N */
    //DBG_M();
    for (n = 0; n < cels.sz1; n++) {
        c = ((pix_t*)fnt)[n];
      #ifdef CLMP_BPP_8
        if (c != cels.nukiCol) {
          #if 0
            c = cels.clut[c];
            a = (unsigned)c >> 24;
            if (a) {                // ��!=0
                if (a < 0xFF) {     // ��<0xFF�Ȃ̂Ŕ�������
                    m  = 0;
                    af = 1;
                }
                f = 1;
            } else {                // ��=0�Ȃ̂Ŕ����F��
                m = 0;
            }
          #else
            f = 1;
          #endif
        } else {
            m = 0;
        }
      #else
        a = (unsigned)c >> 24;
        if (a) {                // ��!=0
            if (a < 0xFF) {     // ��<0xFF�Ȃ̂Ŕ�������
                m  = 0;
                af = 1;
            }
            f = 1;
        } else {                // ��=0�Ȃ̂Ŕ����F��
            m = 0;
        }
      #endif
    }
//if (dbgExLog_getSw()) printf("cels_add[%d] %d %d\n", n, f, m);
  #if 1 // ���ׂĔ����F������
    if (f == 0 && (cels.nocmpMode & 2) == 0)
        return 0;
  #endif
    /* ������Ȃ������̂œo�^ */
    n = cels.num;
    if (cels.num < cels.numMax) {
        memcpy(cels.fnt + cels.num * cels.sz1, fnt, cels.sz1*PIX_BYT);
        cels.num++;
    } else {
        err_abortMsg("�Z���̐��������Ȃ肷���܂�(�ő� %d��)\n", cels.numMax);
    }
    cels.atrs[n] = n;
    if (cels.mutoumeiFlg) {
        cels.atrs[n] |= (m<<15)|(af<<14);
    }
    return cels.atrs[n];
}


/*--------------------------------------------------------------------------*/
