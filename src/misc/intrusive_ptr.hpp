/** @file intrusive_ptr.hpp
 *  @brief  boost::intrusive_ptr を単体で使用できるようにしたヘッダ.
 *  @note
 *  Copyright (c) 2001, 2002 Peter Dimov
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
 *
 */
#ifndef ARA_INTRUSIVE_PTR_HPP
#define ARA_INTRUSIVE_PTR_HPP

#ifdef _WIN32
#pragma once
#endif

#ifdef _MSC_VER  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator->
#endif

#include <functional>           // for std::less
#ifndef assert
#include <cassert>
#endif

/**  intrusive_ptr

  A smart pointer that uses intrusive reference counting.

  Relies on unqualified calls to

      void intrusive_ptr_add_ref(T * p);
      void intrusive_ptr_release(T * p);

          (p != 0)

  The object is responsible for destroying itself.
 */

template<class T>
class intrusive_ptr {
    typedef intrusive_ptr   this_type;
public:
    typedef T               element_type;

    intrusive_ptr(): p_(0) { }

    intrusive_ptr(T * p, bool add_ref = true): p_(p)
    {
        if (p_ != 0 && add_ref)
            intrusive_ptr_add_ref(p_);
    }

    template<class U>
    intrusive_ptr(intrusive_ptr<U> const & rhs): p_(rhs.get())
    {
        if (p_ != 0)
            intrusive_ptr_add_ref(p_);
    }

    intrusive_ptr(intrusive_ptr const & rhs): p_(rhs.p_)
    {
        if (p_ != 0)
            intrusive_ptr_add_ref(p_);
    }

    ~intrusive_ptr()
    {
        if (p_ != 0)
            intrusive_ptr_release(p_);
    }

    template<class U>
    intrusive_ptr & operator=(intrusive_ptr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(intrusive_ptr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void reset( T * rhs )
    {
        this_type( rhs ).swap( *this );
    }

    T * get() const
    {
        return p_;
    }

    T & operator*() const
    {
        assert( p_ != 0 );
        return *p_;
    }

    T * operator->() const
    {
        assert( p_ != 0 );
        return p_;
    }


    bool operator! () const
    {
        return p_ == 0;
    }


    // p == 0 や p != 0 を表現可能にするためのキャスト関数
  #ifdef NDEBUG // 誤ってdelete出来てしまうが、ヘボいコンパイラ対策でNDEBUG時はコレ.
    typedef T const* const* const* unspecified_bool_type;
    operator unspecified_bool_type() const { return (unspecified_bool_type)p_; }
  #elif defined(__MWERKS__)
    typedef T * (this_type::*unspecified_bool_type)() const;
    operator unspecified_bool_type() const {
        return p_ ? &this_type::get : 0;
    }
  #else // _MSC_VER
    typedef T * this_type::*unspecified_bool_type;
    operator unspecified_bool_type () const {
        return p_ ? &this_type::p_ : 0;
    }
  #endif

    void swap(intrusive_ptr & rhs) {
        T * tmp = p_;
        p_      = rhs.p_;
        rhs.p_  = tmp;
    }


private:
    T * p_;
};



template<class T, class U> inline
bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() == b.get();
}


template<class T, class U> inline
bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() != b.get();
}


template<class T, class U> inline
bool operator==(intrusive_ptr<T> const & a, U * b)
{
    return a.get() == b;
}


template<class T, class U> inline
bool operator!=(intrusive_ptr<T> const & a, U * b)
{
    return a.get() != b;
}


template<class T, class U> inline
bool operator==(T * a, intrusive_ptr<U> const & b)
{
    return a == b.get();
}


template<class T, class U> inline
bool operator!=(T * a, intrusive_ptr<U> const & b)
{
    return a != b.get();
}


template<class T> inline
bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return std::less<T *>()(a.get(), b.get());
}


template<class T>
void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
    lhs.swap(rhs);
}


template<class T>
T * get_pointer(intrusive_ptr<T> const & p)
{
    return p.get();
}


template<class T, class U>
intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}


template<class T, class U>
intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const & p)
{
    return const_cast<T *>(p.get());
}


template<class T, class U>
intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}


#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif  // ARA_INTRUSIVE_PTR_HPP
