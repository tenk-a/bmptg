/**
 *  @file   intrusive_ptr_ref_count.hpp
 *  @brief  intrusive_ptrで指すクラスの参照カウンタ実装用クラス(継承して使う)
 *  @note
 *  -   delete (T*)this; がちゃんと実行できるように継承する必要がある.
 *      → intrusive_ptr_ref_count<T,C>を継承の1番目にする.
 *  -   deleteで開放するので、newで確保される必要ある
 */
#ifndef INTRUSIVE_PTR_REF_COUNT_HPP_INCLUDED
#define INTRUSIVE_PTR_REF_COUNT_HPP_INCLUDED

#ifdef _WIN32
#pragma once
#endif

#include "intrusive_ptr.hpp"

#ifndef assert
#include <cassert>
#endif

/// 通常のnew に対する delete
template<class T>
struct intrusive_ptr_ref_count_default_deleter {
    void operator()(T* p) {
        delete p;
    }
};

/// 通常のnew[] に対する delete
template<class T>
struct intrusive_ptr_ref_count_default_array_deleter {
    void operator()(T* p) {
        delete[] p;
    }
};

/// intrusive_ptrで指すクラスの参照カウンタ実装用クラス(継承して使う)
template< class T, class D=intrusive_ptr_ref_count_default_deleter<T>, typename C=unsigned>
class intrusive_ptr_ref_count : private D {
public:
    typedef intrusive_ptr<T>    ptr_type;

    intrusive_ptr_ref_count() : cnt_(0) {;}

    C    use_count() const { return this ? cnt_ : 0; }

    friend void intrusive_ptr_add_ref(intrusive_ptr_ref_count* t) {
        if (t) {
            ++t->cnt_;
            assert(t->cnt_ > 0);
        }
    }

    friend void intrusive_ptr_release(intrusive_ptr_ref_count* t) {
        if (t && t->cnt_ && --t->cnt_ == 0)
            t->D::operator()( reinterpret_cast<T*>(t) );
    }

private:
    C    cnt_;
};


#endif  // ARA_INTRUSIVE_PTR_REF_COUNT_HPP_INCLUDED
