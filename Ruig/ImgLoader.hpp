#pragma once

#include "../ImgFmt/JpgDecoder.hpp"
#include "NamedImg.hpp"

//#include <memory>
//typedef std::shared_ptr<NamedImg> NamedImgPtr;

class ImgDecoder {
public:
    ImgDecoder() {}

    NamedImgPtr jpgDecode(const char* name, void* bin, size_t binSz, bool useThumbnail=true) {
        if (!JpgDecoder::isSupported(bin))
            return NamedImgPtr();
        NamedImgPtr     namedImgPtr  = NamedImgPtr(new NamedImg(name));
        BppCnvImg&      bppCnvImg    = namedImgPtr->img();

        JpgDecoder dec(bin, binSz, useThumbnail);
        bppCnvImg.create(dec.width(), dec.height(), dec.bpp());
        namedImgPtr->setOriginalSize(dec.originalWidth(), dec.originalHeight());
        bool rc = dec.read(bppCnvImg.image());
        if (!rc) {
            return NamedImgPtr();
        }
        return namedImgPtr;
    }
};

