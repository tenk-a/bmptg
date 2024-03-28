#pragma once

#include "../Proc/BppCnvImg.hpp"
#include "../misc/intrusive_ptr_ref_count.hpp"
#include <string>


class NamedImg : public intrusive_ptr_ref_count<NamedImg> {
public:
    NamedImg() : width_(0), height_(0) {}
    // ~ NamedImg() {}
    NamedImg(std::string const& name) : name_(name), width_(0), height_(0) {}

    BppCnvImg&          img() { return img_; }
    BppCnvImg const&    img() const { return img_; }
    std::string const&  name() const { return name_; }
    char const*         nameptr() const { return name_.c_str(); }

    void            setOriginalSize(unsigned w, unsigned h) { width_ = w, height_ = h; }
    unsigned        width() const { return width_; }
    unsigned        height() const { return height_; }

private:
    std::string     name_;
    BppCnvImg       img_;
    unsigned        width_;
    unsigned        height_;
};


typedef intrusive_ptr<NamedImg>     NamedImgPtr;




