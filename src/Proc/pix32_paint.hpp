/**
 *  @file   Pix32_Paint.hpp
 *  @brief  32ビット画の、(x,y)の位置にある色を、指定した色で塗りつぶす.
 *  @note
 *      どこだったかのサイトを参考
 */
#ifndef PIX32_PAINT_HPP
#define PIX32_PAINT_HPP

/// (x,y)の色が続く領域を newCol で塗りつぶす.
/// x,yいづれかが負が指定されると、左上の点の色を元に、画像の四隅を塗りつぶす.
class Pix32_Paint {
    enum { PAINT_BUF_MAX = 0x4000 };
public:
    typedef int             xy_t;
    typedef unsigned        pix_t;
    typedef unsigned char   ch_t;

    Pix32_Paint(unsigned *pix, unsigned w, unsigned h, pix_t newCol=0, xy_t tgtX=xy_t(-1), xy_t tgtY=xy_t(-1), xy_t dif=8)
        : pix_(pix), w_(w), h_(h), x0_(0), y0_(0), x1_(xy_t(w-1)), y1_(xy_t(h-1)), ck_(dif*dif)
    {
        // 開始位置が負なら、左上の点の色を元に、四隅を塗りつぶす.
        if ((tgtX == xy_t(-1)) || (tgtY == xy_t(-1))) {
            pix_t   c = getPix(0, 0);
            paint(x0_, y0_, newCol);
            if (c == getPix(x1_, y0_))  paint(x1_, y0_, newCol);
            if (c == getPix(x0_, y1_))  paint(x0_, y1_, newCol);
            if (c == getPix(x1_, y1_))  paint(x1_, y1_, newCol);

        } else {
            paint(tgtX, tgtY, newCol);
        }
    }

    ~Pix32_Paint() {
    }

private:
    void paint(xy_t x, xy_t y, pix_t fillCol) {
        paintCur_           = paintBuf_;
        paintEnd_           = paintBuf_ + 1;

        paintCur_->leftX_   = paintCur_->rightX_  = x;
        paintCur_->curY_    = paintCur_->parentY_ = y;

        pix_t   tgtCol      = getPix(x, y);

        do {
            xy_t    lx      = paintCur_->leftX_;
            xy_t    rx      = paintCur_->rightX_;
            xy_t    uy      = paintCur_->curY_;
            xy_t    dy      = paintCur_->curY_;
            xy_t    oy      = paintCur_->parentY_;
            xy_t    lxsav   = lx - 1;
            xy_t    rxsav   = rx + 1;

            ++paintCur_;
            if (paintCur_ == &paintBuf_[PAINT_BUF_MAX])
                paintCur_   = &paintBuf_[0];

            // 処理済のシードなら無視.
            if (getPix(lx, uy) == fillCol)
                continue;

            // 右方向の境界を探す.
            while (rx < x1_) {
                if (isSamePix(rx + 1, uy, tgtCol) == false)
                    break;
                ++rx;
            }

            // 左方向の境界を探す.
            while (lx > x0_) {
                if (isSamePix(lx - 1, uy, tgtCol) == false)
                    break;
                --lx;
            }

            // lx-rxの線分を描画.
            for (xy_t i = lx; i <= rx; ++i)
                setPix(i, uy, fillCol);

            // 真上のスキャンラインを走査する.
            if (--uy >= y0_) {
                if (uy == oy) {
                    scanLine(   lx, lxsav, uy, uy + 1, tgtCol);
                    scanLine(rxsav,    rx, uy, uy + 1, tgtCol);
                } else {
                    scanLine(   lx,    rx, uy, uy + 1, tgtCol);
                }
            }

            // 真下のスキャンラインを走査する.
            if (++dy <= y1_) {
                if (dy == oy) {
                    scanLine(   lx, lxsav, dy, dy - 1, tgtCol);
                    scanLine(rxsav,    rx, dy, dy - 1, tgtCol);
                } else {
                    scanLine(   lx,    rx, dy, dy - 1, tgtCol);
                }
            }

        } while (paintCur_ != paintEnd_);
    }


    void scanLine(xy_t lx, xy_t rx, xy_t y, xy_t oy, pix_t tgtCol) {
        while (lx <= rx) {
            // 非領域色を飛ばす.
            for (; lx < rx; ++lx) {
                if (isSamePix(lx, y,tgtCol))
                    break;
            }

            if (isSamePix(lx, y,tgtCol) == false)
                break;

            paintEnd_->leftX_ = lx;

            // 領域色を飛ばす.
            for (; lx <= rx; ++lx) {
                if (isSamePix(lx, y, tgtCol) == false)
                    break;
            }

            paintEnd_->rightX_  = lx - 1;
            paintEnd_->curY_    = y;
            paintEnd_->parentY_ = oy;

            ++paintEnd_;
            if (paintEnd_ == &paintBuf_[PAINT_BUF_MAX]) {
                paintEnd_ = paintBuf_;
            }
        }
    }

    bool isSamePix(xy_t x, xy_t y, pix_t c) const {
        return isSameCol(c, pix_[y * w_ + x]);
    }

    bool isSameCol(pix_t c, pix_t c2) const {
        int     a  = ch_t(c >> 24);
        int     r  = ch_t(c >> 16);
        int     g  = ch_t(c >>  8);
        int     b  = ch_t(c      );
        int     a2 = ch_t(c2 >> 24);
        int     r2 = ch_t(c2 >> 16);
        int     g2 = ch_t(c2 >>  8);
        int     b2 = ch_t(c2      );
        a -= a2;
        r -= r2;
        g -= g2;
        b -= b2;
        int     ck = int(ck_);
        return r*r <= ck && g*g <= ck && b*b <= ck && a*a <= ck;
    }

    pix_t getPix(xy_t x, xy_t y) const {
        return  pix_[y * w_ + x];
    }

    void setPix(xy_t x, xy_t y, pix_t c) {
        pix_[y * w_ + x] = c;
    }

private:
    struct PaintBuf {
        xy_t    leftX_;         ///< 領域左端のX座標
        xy_t    rightX_;        ///< 領域右端のX座標
        xy_t    curY_;          ///< 領域のY座標
        xy_t    parentY_;       ///< 親ラインのY座標
    };
    pix_t*      pix_;
    xy_t        w_;
    xy_t        h_;
    xy_t        x0_;
    xy_t        y0_;
    xy_t        x1_;
    xy_t        y1_;
    xy_t        ck_;

    PaintBuf    paintBuf_[ PAINT_BUF_MAX ];
    PaintBuf*   paintCur_;
    PaintBuf*   paintEnd_;
};


#endif
