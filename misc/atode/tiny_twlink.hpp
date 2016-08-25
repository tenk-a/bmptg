/**
 *  @file   tiny_twlink.hpp
 *  @brief  class�p���������o�[�ϐ��Ƃ��Ďg���A�ȈՂ� �o�����z���X�g.
 *  @author tenka@6809.net
 *  @date   2004-2011
 *  @note
 *  -   1�Ԗڂ�public�p��,���邢�͐擪�����o�[�Ƃ��āA�v�f���m�������N����.
 *      �擪�ȊO�̏ꍇ�� OFS ���w�肵��(�����o�[)�ϐ��ʒu���w�肷��K�v������.
 *      �� OFS�͒萔�Ȃ̂ł���ōςޔ͈͂̏ꍇ�̂�.
 *  -   twlink�N���X��virtual�f�X�g���N�^�������Ȃ��̂ŁA
 *      �p���������̂�twlink*�^��delete�����A�K�؂Ȍ^��delete���邱��.
 *  -   (1�Ԗڂ̌p���Ƃ���) T* == this(+OFS) �ɂȂ邱�Ƃ�O��Ɋȗ���.
 *  -   �X���b�h���l��. (�g�����ł��̑O��)
 *  - public �����o�[
 *      own()           �������g��T*��Ԃ�.
 *      after()         �����̌�Ƀ����N����Ă�����̂�Ԃ�.(T*)
 *      befor()         �����̑O�Ƀ����N����Ă�����̂�Ԃ�.(T*)
 *      insert_after(a) �����̌�� a ��}��.
 *      insert_befor(b) �����̑O�� b ��}��.
 *      next(t)         t�̎���Ԃ�.
 *      prev(t)         t�̑O��Ԃ�.
 *      unlink()        �������g�������N����O��.(delete����킯�ł͖���)
 *      link_size()     �������������郊���N�̃����N���𐔂��ĕԂ�.
 *      search(s)       s�������N�ɏ������Ă���|�C���^���𒲂ׂ�.
 *      for_each(func)  �����N����Ă���S�Ăɑ΂�func(t) �����s.
 *  - protected �����o�[
 *      to_twlink(t)     T* �^�� t ���� tiny_twlink<T,OFS>* �ɂ��ĕԂ�.
 *      after_twlink()   tiny_twlink<T,OFS>* �Ŏ����̌�̃����N��Ԃ�.
 *      before_twlink()  tiny_twlink<T,OFS>* �Ŏ����̑O�̃����N��Ԃ�.
 *      search_twlink(s) tiny_twlink<T,OFS>* �^��s��T��.
 */
#ifndef TINY_TWLINK_HPP
#define TINY_TWLINK_HPP

#include <cstddef>
#ifndef assert
#include <assert.h>
#endif

#if defined _MSC_VER
 #pragma warning(push)
 #pragma warning(disable: 4127) // ���������萔�ł�
 #pragma warning(disable: 4244) // �����^��bit���̏��Ȃ��^�ւ̕ϊ���"�f�[�^��������\��������"
 #pragma warning(disable: 4706) //
 #pragma warning(disable: 4996) // _vsnprintf��unsafe������_s�n�g��(_CRT_SECURE_NO_WARNINGS)
 #pragma warning(disable: 4355) // 'this'�������o�[�������q���X�g�Ŏg��ꂽ.
#endif

/// class�p���������o�[�ϐ��Ƃ��Ďg���A�ȈՂ� �o�����z���X�g. �����o�[�Ƃ��Ďg���ꍇ�͈ʒuOFS���w��.
template< class T, std::size_t OFS=0, std::size_t LMAX=0x7fffffff >
class tiny_twlink {
public:
    typedef tiny_twlink     this_type;

    tiny_twlink(T* t=0) {
        typedef char static_assert[(OFS <= sizeof(T) - sizeof(this_type)) ? 1: -1 ];
        if (t == 0) {
            //�f�t�H���g�l�ݒ�A�ƁA�����ɕ��@�Ōp���`�F�b�N.
            //T��tiny_twlink<T>���p�����Ă�n�Y�Ȃ̂�static_cast�ł��Ȃ��Ƃ����Ȃ�.
            t = static_cast<T*>(this);
            //t = (T*)(this);
        }
        assert(t > (this_type*)OFS);
        assert(((const char*)this - (const char*)t) == OFS
            && "template�����ł̃I�t�Z�b�g�Ǝ��ۂ̃I�t�Z�b�g���Ⴄ");
        //assert(static_cast<T*>(0) == static_cast<this_type*>(0) && "T��tiny_twlink<T>���p�����Ă��Ȃ�");
        after_  = before_ = to_twlink(t);
    }

    ~tiny_twlink() {
        unlink();
    }

    ///< �������g��T*��Ԃ�.
    const T*    own()    const  { return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) - OFS); }
    T*          own()           { return reinterpret_cast<      T*>(reinterpret_cast<      char*>(this) - OFS); }
    const T*    after()  const  { return after_->own(); }                           ///< �����̑O�̗v�f��Ԃ�.
    T*          after()         { return after_->own(); }
    const T*    before() const  { return before_->own();}                           ///< �����̌�̗v�f��Ԃ�.
    T*          before()        { return before_->own();}
    void        insert_after(T* a)  { insert_after_twlink(to_twlink(a)); }          ///< ���g�̎��� a ���Ȃ���.
    void        insert_before(T* b) { insert_before_twlink(to_twlink(b)); }         ///< ���g�̑O�� b ���Ȃ���.
    std::size_t search( const T* s ) const { return search_twlink(to_twlink(s)); }  ///< s�����X�g�ɂ����1�ȏ�̒l���Ȃ����0��Ԃ�.
    bool        unique() const { return this && this == after_; }
    void        swap(tiny_twlink& r) {std::swap(after_, r.after_); std::swap(before_,r.before_); }
    void        invariant() const { assert( this && after_ && before_ ); }

    static const T* next(const T* t) { assert(t); return to_twlink(t)->after();}
    static T*       next(T* t)       { assert(t); return to_twlink(t)->after();}
    static const T* prev(const T* t) { assert(t); return to_twlink(t)->before();}
    static T*       prev(T* t)       { assert(t); return to_twlink(t)->before();}

    /** �����N����Ă��郂�m�̐��𐔂���.
     */
    std::size_t link_size() const {
        unsigned size = 1;
        for (this_type* a = after_; a != this && size <= LMAX; a = a->after_) {
            ++size;
            assert( size < LMAX );
        }
        return size;
    }

    /** ���g�����X�g������O��.
     */
    bool unlink() {
        this_type* a    = after_;
        this_type* b    = before_;
        if (this == a) {
            assert( this == b );
            return false;
        }
        b->after_   = a;
        a->before_  = b;
        after_      = this;
        before_     = this;
        invariant();
        return true;
    }

    /** �ċA������ �q���ɑ΂� FUNC �����s����.
     *  func(p) �� true��Ԃ������_�ŏI��.
     *  ���s��������Ԃ�.
     */
    template<class FUNC>
    std::size_t for_each(FUNC func) {
        assert(this != NULL);
        std::size_t size = 1;
        if (func(own()))
            return size;
        for (this_type* a = after_; a != this && ++size < LMAX; a = a->after_) {
            if (func(a->own()))
                return size;
        }
        assert( size < LMAX && "tiny_twlink::fro_each(): broken link");
        return size;
    }

protected:
    static this_type*       to_twlink(T* t) {
        return reinterpret_cast<this_type*>(reinterpret_cast<char*>(t) + OFS);
    }
    static const this_type* to_twlink(const T* t) {
        return reinterpret_cast<const this_type*>(reinterpret_cast<const char*>(t) + OFS);
    }
    const this_type*    after_twlink()  const   { return after_; }
    this_type*          after_twlink()          { return after_; }
    const this_type*    before_twlink() const   { return before_;}
    this_type*          before_twlink()         { return before_;}

    /** srch �������N���ɂ���� �^ ��Ԃ�
     */
    std::size_t search_twlink( const this_type* s ) const {
        assert(s > (this_type*)OFS);
        invariant();
        std::size_t n = 1;
        if (this != s) {
            for (this_type* a = after_; ++n, a != s; a = a->after_) {
                if (a == this)
                    return 0;
            }
        }
        return n;
    }

    /** ���g�̎��� a ���Ȃ���
     */
    void insert_after_twlink(tiny_twlink* a) {
        assert(a > reinterpret_cast<this_type*>(OFS));
        assert(a == a->after_ && a == a->before_ );
        a->after_       = after_;
        a->before_      = this;
        after_->before_ = a;
        after_          = a;
        invariant();
    }

    /** ���g�̑O�� b ���Ȃ���
     */
    void insert_before_twlink(tiny_twlink* b) {
        assert(b > reinterpret_cast<this_type*>(OFS));
        assert(b == b->after_ && b == b->before_ );
        b->after_       = this;
        b->before_      = before_;
        before_->after_ = b;
        before_         = b;
        invariant();
    }

private:
    this_type*      after_;
    this_type*      before_;
};


namespace std {
    template<class T, unsigned OFS, unsigned LMAX>
    void swap(tiny_twlink<T,OFS,LMAX>& l, tiny_twlink<T,OFS,LMAX>& r) {
        l.swap(r);
    }
}   // std


#if defined _MSC_VER
 #pragma warning(pop)
#endif
#endif  // ARA_TINY_TWLINK_HPP
