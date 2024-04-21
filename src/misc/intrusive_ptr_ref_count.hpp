/**
 *  @file   intrusive_ptr_ref_count.hpp
 *  @brief  intrusive_ptr�Ŏw���N���X�̎Q�ƃJ�E���^�����p�N���X(�p�����Ďg��)
 *  @note
 *  -   delete (T*)this; �������Ǝ��s�ł���悤�Ɍp������K�v������.
 *      �� intrusive_ptr_ref_count<T,C>���p����1�Ԗڂɂ���.
 *  -   delete�ŊJ������̂ŁAnew�Ŋm�ۂ����K�v����
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

/// �ʏ��new �ɑ΂��� delete
template<class T>
struct intrusive_ptr_ref_count_default_deleter {
    void operator()(T* p) {
        delete p;
    }
};

/// �ʏ��new[] �ɑ΂��� delete
template<class T>
struct intrusive_ptr_ref_count_default_array_deleter {
    void operator()(T* p) {
        delete[] p;
    }
};

/// intrusive_ptr�Ŏw���N���X�̎Q�ƃJ�E���^�����p�N���X(�p�����Ďg��)
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
