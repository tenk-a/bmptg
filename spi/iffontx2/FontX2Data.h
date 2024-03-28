/**
 *	@file	FontX2Data.h
 *	@brief	fontX2形式のフォントデータの取得.
 *	@author	Masashi KITAMURA.
 *	@note
 */
#ifndef FONTX2DATA_H
#define FONTX2DATA_H

#include <stddef.h>
#include <assert.h>


template<class DMY=void>
class FontX2Data {
public:
	FontX2Data() : data_(0) { }
	~FontX2Data() { }

	bool setData(const unsigned char* data);

	static bool	isSupported(const void* buf) { return buf && memcmp(buf, "FONTX2", 6) == 0; }

	bool		haveData() const { return this && isSupported(data_); }
	bool		zenkakuFlag() const { return zenkakuFlag_!=0; }

	unsigned	chrByte()      const { return chByte_; }
	unsigned	chrWidth()     const { return chW_; }
	unsigned	chrHeight()    const { return chH_; }
	unsigned	chrWidthByte() const { return wb_; }

	// 文字コードに対応するデータのアドレスを返す.
	const unsigned char* chrImage(int c) const;

	int 		chrCodeToInrNo(int c) const;

private:
	const unsigned char*	data_;
	unsigned short			chW_;
	unsigned short			chH_;
	unsigned short			chByte_;
	unsigned short			wb_;
	unsigned short			top_;
	unsigned char			zenkakuFlag_;
	unsigned short			tblNum_;
	unsigned short			kcode_rng_[256][3];
};



template<class DMY>
bool FontX2Data<DMY>::setData(const unsigned char* data)
{
	if (isSupported(data) == false)
		return false;
	data_	 		= data;
	chW_ 			= data[14];
	chH_ 			= data[15];
	wb_				= ((chW_ + 7) / 8);
	chByte_			= wb_ * chH_;
	zenkakuFlag_	= data[16];

	if (zenkakuFlag_ != 0) {	//2バイトコードのとき
		unsigned	n   = 0;
		tblNum_ 		= data[17];

		unsigned short*  tbl = (unsigned short*)&data[18];

		for (unsigned i = 0; i < tblNum_; ++i) {
			kcode_rng_[i][0] = *tbl++;
			kcode_rng_[i][1] = *tbl++;
			kcode_rng_[i][2] = n;
			n += kcode_rng_[i][1] - kcode_rng_[i][0] + 1;
		}
		top_ = 18 + tblNum_ * 4;
	} else {		// 1バイト文字のとき
		//tblNum = 1;
		//kcode_rng_[0][0] =   0;
		//kcode_rng_[0][1] = 255;
		top_ = 17;
	}

	return (chW_ > 0 && chH_ > 0);
}



template<class DMY>
int FontX2Data<DMY>::chrCodeToInrNo(int c) const
{
	int m = c;
	if (zenkakuFlag_) {
		// テーブルを検索
		m = -1;	//見つからない場合は先頭を代用
		for (unsigned i = 0; i < tblNum_; ++i) {
			if (kcode_rng_[i][0] <= c && c <= kcode_rng_[i][1]) {
				//m = c - kcode_rng_[i][0];
				m = kcode_rng_[i][2] + c - kcode_rng_[i][0];
				break;
			}
		}
	} 
	return m;
}



template<class DMY>
const unsigned char* FontX2Data<DMY>::chrImage(int c) const
{
	int m = chrCodeToInrNo(c);
	int ofs = top_;
	if (m > 0)
		ofs = top_ + m * chByte_;
	return &data_[ofs];
}




// =====================================================================
// フォントデータを 16x16 または 94x94 文字敷き詰めた画像に変換.


template<class DMY=void>
class FontX2SampleImage {
public:
	FontX2SampleImage() : w_(0), h_(0), wb_(0),wbA_(0), type_(0)
						, revY_(0), chr1Mode_(0), dst_(0) {;}
	FontX2SampleImage(const void* data, bool chr1Mode=0) {setData(data, chr1Mode);}
	~FontX2SampleImage() {;}

	static bool isSupported(const void* data) { return FontX2Data<>::isSupported(data); }

	bool 	 setData(const void* data, bool chr1Mode=0);
	unsigned width() 	const { return w_; }
	unsigned height() 	const { return h_; }
	unsigned bpp() 		const { return 1; }
	unsigned clutSize() const { return 2; }
	void 	 getClut(unsigned* clut, unsigned clutSize=2);

	unsigned imageByte(unsigned algn=1) const { return widthByte(algn) * h_; }

	/// アライメントは1,2,4,8のみのこと.
	unsigned widthByte(unsigned algn=1) const {
		assert(algn == 1 || algn == 2 || algn == 4 || algn == 8);
		return (wb_+algn-1) & ~(algn-1);
	}

	bool 	read(unsigned char* data, int algn, bool revY);

	bool 	read1(unsigned char* data, int algn, bool revY);


private:
	unsigned jis2sjis(unsigned jis);

	int		getSrcBit(unsigned x, unsigned y, const unsigned char* img);
	void 	setDstBit(unsigned x, unsigned y0);
	void 	putCh(unsigned tx, unsigned ty, const unsigned char* pix);

private:
	FontX2Data<>	data_;
	unsigned		w_;
	unsigned		h_;
	unsigned		wb_;
	unsigned 		wbA_;
	unsigned char	type_;
	unsigned char	revY_;
	unsigned char	chr1Mode_;
	unsigned char*	dst_;
};



template<class DMY>
bool FontX2SampleImage<DMY>::setData(const void* data, bool chr1Mode)
{
	w_ = h_ = wb_ = wbA_ = type_ = revY_ = 0;
	type_ = revY_ = chr1Mode_ = 0;
	dst_ = 0;
	if (data_.setData((unsigned char*)data) == false)
		return false;

	chr1Mode_ = chr1Mode;
	type_     = data_.zenkakuFlag();

	if (chr1Mode_ == 0) {
		unsigned lineChrs = (type_) ? 94 : 16;
		w_ = data_.chrWidth()  * lineChrs;
		h_ = data_.chrHeight() * lineChrs;
	} else {
		int cn = (type_) ? 94*94 : 16*16;
		w_ = data_.chrWidth();
		h_ = data_.chrHeight() * cn;
	}
	wb_ = (w_ + 7) / 8;
	return true;
}



template<class DMY> inline
void FontX2SampleImage<DMY>::getClut(unsigned* clut, unsigned clutSize) {
	if (clut && clutSize >= 2) {
		clut[0] = 0x00000000;
		clut[1] = 0xFFFFFFFF;
	}
}



template<class DMY>
bool FontX2SampleImage<DMY>::read(unsigned char* pix, int algn, bool revY)
{
	if (data_.haveData() == false)
		return false;
	revY_ 	= revY;
	dst_	= pix;
	wbA_ 	= widthByte(algn);
	memset(pix, 0, wbA_*h_);

	if (chr1Mode_ == 0) {	// 16x16 , 94x94 で敷き詰める時.
		if (type_ == 0) {
			int code = 0;
			for (unsigned y = 0; y < 16; ++y) {
				for (unsigned x = 0; x < 16; ++x) {
					const unsigned char* p = data_.chrImage(code++);
					if (p) {
						putCh(x,y,p);
					}
				}
			}
		} else {
			for (unsigned y = 0; y < 94; ++y) {
				for (unsigned x = 0; x < 94; ++x) {
					unsigned jis  = ((y + 0x21) << 8) | (x + 0x21);
					unsigned sjis = jis2sjis(jis);
					const unsigned char* p = data_.chrImage(sjis);
					if (p) {
						putCh(x,y,p);
					}
				}
			}
		}
	} else {				// 横1文字で縦にずらっと並べるとき.
		if (type_ == 0) {
			int code = 0;
			for (unsigned y = 0; y < 16; ++y) {
				for (unsigned x = 0; x < 16; ++x) {
					const unsigned char* p = data_.chrImage(code++);
					if (p) {
						putCh(0,y*16+x,p);
					}
				}
			}
		} else {
			for (unsigned y = 0; y < 94; ++y) {
				for (unsigned x = 0; x < 94; ++x) {
					unsigned jis  = ((y + 0x21) << 8) | (x + 0x21);
					unsigned sjis = jis2sjis(jis);
					const unsigned char* p = data_.chrImage(sjis);
					if (p) {
						putCh(0,y*94+x,p);
					}
				}
			}
		}
	}
	return true;
}



template<class DMY> inline
int	FontX2SampleImage<DMY>::getSrcBit(unsigned x, unsigned y, const unsigned char* img) {
	static const unsigned char msk[] = {0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1};
	unsigned wb = data_.chrWidthByte();
	unsigned c  = img[ y * wb + (x >> 3) ];
	unsigned m  = msk[x & 7];
	return (c & m) != 0;
}



template<class DMY> inline
void FontX2SampleImage<DMY>::setDstBit(unsigned x, unsigned y0) {
	static const unsigned char msk[] = {0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1};
	unsigned y = (revY_ == 0) ? y0 : (h_-1-y0);
	dst_[y * wbA_ + (x >> 3)] |= msk[x & 7];
}



template<class DMY>
void FontX2SampleImage<DMY>::putCh(unsigned tx, unsigned ty, const unsigned char* pix)
{
	unsigned cw = data_.chrWidth();
	unsigned ch = data_.chrHeight();
	unsigned ox = cw * tx;
	unsigned oy = ch * ty;
	for (unsigned y = 0; y < ch; ++y) {
		for (unsigned x = 0; x < cw; ++x) {
			if (getSrcBit(x, y, pix))
				setDstBit(ox+x,oy+y);
		}
	}
}


template<class DMY>
unsigned FontX2SampleImage<DMY>::jis2sjis(unsigned c)
{
	c -= 0x2121;
	if (c & 0x100)
		c += 0x9e;
	else
		c += 0x40;
	if ((unsigned char)c >= 0x7f)
		c++;
	c = (((c >> (8+1)) + 0x81)<<8) | ((unsigned char)c);
	if (c >= 0xA000)
		c += 0x4000;
	return c;
}





#endif	// FONTX2DRAW_H
