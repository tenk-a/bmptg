/**
 *  @file   Pix32_Paint.h
 *  @brief  32�r�b�g��́A(x,y)�̈ʒu�ɂ���F���A�w�肵���F�œh��Ԃ�.
 *  @note
 *  net����p�N
 */
#ifndef PIX32_PAINT_H
#define PIX32_PAINT_H

//x #include <assert.h>


/// (x,y)�̐F�������̈�� newCol �œh��Ԃ�.
/// x,y���Âꂩ�������w�肳���ƁA����̓_�̐F�����ɁA�摜�̎l����h��Ԃ�.
class Pix32_Paint {
public:
    Pix32_Paint(unsigned *pix, unsigned int w, unsigned int h, unsigned newCol=0, int tgtX=-1, int tgtY=-1, unsigned colMsk=(unsigned)-1)
        : pix_(pix), w_(w), h_(h), x0_(0), y0_(0), x1_(int(w-1)), y1_(int(h-1)), colMsk_(colMsk)
    {
        //x assert( 0 < x1 && x1_ < 0x8000 && 0 < y1 && y1_ < 0x8000 );

        // �J�n�ʒu�����Ȃ�A����̓_�̐F�����ɁA�l����h��Ԃ�.
        if ((tgtX < 0) | (tgtY < 0)) {
            unsigned c = getPix(0, 0);
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
    void paint(int x, int y, unsigned fillCol) {
        Pix32_Paint *t      = this;
        unsigned    cmpCol  = fillCol & colMsk_; // & 0xffffff;
        unsigned    col     = getPix(x, y);

        t->paintCur_        = t->paintBuf_;
        t->paintEnd_        = t->paintBuf_ + 1;

        t->paintCur_->leftX_= t->paintCur_->rightX_  = x;
        t->paintCur_->curY_ = t->paintCur_->parentY_ = y;

        do {
            int     lx    = t->paintCur_->leftX_;
            int     rx    = t->paintCur_->rightX_;
            int     uy    = t->paintCur_->curY_;
            int     dy    = t->paintCur_->curY_;
            int     oy    = t->paintCur_->parentY_;
            int     lxsav = lx - 1;
            int     rxsav = rx + 1;

            ++t->paintCur_;
            if (t->paintCur_ == &t->paintBuf_[PAINT_BUF_MAX])
                t->paintCur_ = &t->paintBuf_[0];

            // �����ς̃V�[�h�Ȃ疳��.
            if (getPix(lx, uy) == cmpCol)
                continue;

            // �E�����̋��E��T��.
            while (rx < t->x1_) {
                if (getPix(rx + 1, uy) != col)
                    break;
                ++rx;
            }

            // �������̋��E��T��.
            while (lx > t->x0_) {
                if (getPix(lx - 1, uy) != col)
                    break;
                --lx;
            }

            // lx-rx�̐�����`��.
            {
                int     i;
                for (i = lx; i <= rx; ++i)
                    setPix(i, uy, fillCol);
            }

            // �^��̃X�L�������C���𑖍�����.
            if (--uy >= t->y0_) {
                if (uy == oy) {
                    scanLine(   lx, lxsav, uy, uy + 1, col);
                    scanLine(rxsav,    rx, uy, uy + 1, col);
                } else {
                    scanLine(   lx,    rx, uy, uy + 1, col);
                }
            }

            // �^���̃X�L�������C���𑖍�����.
            if (++dy <= t->y1_) {
                if (dy == oy) {
                    scanLine(   lx, lxsav, dy, dy - 1, col);
                    scanLine(rxsav,    rx, dy, dy - 1, col);
                } else {
                    scanLine(   lx,    rx, dy, dy - 1, col);
                }
            }

        } while (t->paintCur_ != t->paintEnd_);
    }


    void scanLine(int lx, int rx, int y, int oy, unsigned col) {
        Pix32_Paint *t      = this;
        while (lx <= rx) {
            // ��̈�F���΂�.
            for (; lx < rx; ++lx) {
                if (getPix(lx, y) == col)
                    break;
            }

            if (getPix(lx, y) != col)
                break;

            t->paintEnd_->leftX_ = lx;

            // �̈�F���΂�.
            for (; lx <= rx; ++lx) {
                if (getPix(lx, y) != col)
                    break;
            }

            t->paintEnd_->rightX_  = lx - 1;
            t->paintEnd_->curY_    = y;
            t->paintEnd_->parentY_ = oy;

            ++t->paintEnd_;
            if (t->paintEnd_ == &t->paintBuf_[PAINT_BUF_MAX]) {
                t->paintEnd_ = t->paintBuf_;
            }
        }
    }

    unsigned getPix(int x, int y) const {
        const Pix32_Paint   *t  = this;
        unsigned    c = t->pix_[y * t->w_ + x];
        // ������
        c  &= colMsk_;
        return c;
    }

    void setPix(int x, int y, unsigned c) {
        Pix32_Paint *t      = this;
        t->pix_[y * t->w_ + x] = c;
    }

private:
    struct PaintBuf {
        int     leftX_;         ///< �̈捶�[��X���W
        int     rightX_;        ///< �̈�E�[��X���W
        int     curY_;          ///< �̈��Y���W
        int     parentY_;       ///< �e���C����Y���W
    };
    unsigned    *pix_;
    unsigned    w_;
    unsigned    h_;
    int         x0_;
    int         y0_;
    int         x1_;
    int         y1_;
    unsigned    colMsk_;

    enum { PAINT_BUF_MAX = 0x4000 };
    PaintBuf    paintBuf_[ PAINT_BUF_MAX ];
    PaintBuf*   paintCur_;
    PaintBuf*   paintEnd_;
};


#endif
