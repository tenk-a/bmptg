/**
 *  @file   tiny_twlink.hpp
 *  @brief  class継承かメンバー変数として使う、簡易な 双方向循環リスト.
 *  @author tenka@6809.net
 *  @date   2004-2011
 *  @note
 *  -   1番目のpublic継承,あるいは先頭メンバーとして、要素同士をリンクする.
 *      先頭以外の場合は OFS を指定して(メンバー)変数位置を指定する必要がある.
 *      ※ OFSは定数なのでそれで済む範囲の場合のみ.
 *  -   twlinkクラスはvirtualデストラクタを持たないので、
 *      継承したものをtwlink*型でdeleteせず、適切な型でdeleteすること.
 *  -   (1番目の継承として) T* == this(+OFS) になることを前提に簡略化.
 *  -   スレッド未考慮. (使う側でやるの前提)
 *  - public メンバー
 *      own()           自分自身のT*を返す.
 *      after()         自分の後にリンクされているものを返す.(T*)
 *      befor()         自分の前にリンクされているものを返す.(T*)
 *      insert_after(a) 自分の後に a を挿入.
 *      insert_befor(b) 自分の前に b を挿入.
 *      next(t)         tの次を返す.
 *      prev(t)         tの前を返す.
 *      unlink()        自分自身をリンクから外す.(deleteするわけでは無い)
 *      link_size()     自分が所属するリンクのリンク数を数えて返す.
 *      search(s)       sがリンクに所属しているポインタかを調べる.
 *      for_each(func)  リンクされている全てに対しfunc(t) を実行.
 *  - protected メンバー
 *      to_twlink(t)     T* 型の t から tiny_twlink<T,OFS>* にして返す.
 *      after_twlink()   tiny_twlink<T,OFS>* で自分の後のリンクを返す.
 *      before_twlink()  tiny_twlink<T,OFS>* で自分の前のリンクを返す.
 *      search_twlink(s) tiny_twlink<T,OFS>* 型のsを探す.
 */
#ifndef TINY_TWLINK_HPP
#define TINY_TWLINK_HPP

#include <cstddef>
#ifndef assert
#include <assert.h>
#endif

#if defined _MSC_VER
 #pragma warning(push)
 #pragma warning(disable: 4127) // 条件式が定数です
 #pragma warning(disable: 4244) // 整数型でbit数の少ない型への変換で"データが失われる可能性がある"
 #pragma warning(disable: 4706) //
 #pragma warning(disable: 4996) // _vsnprintf等unsafeだから_s系使え(_CRT_SECURE_NO_WARNINGS)
 #pragma warning(disable: 4355) // 'this'がメンバー初期化子リストで使われた.
#endif

/// class継承かメンバー変数として使う、簡易な 双方向循環リスト. メンバーとして使う場合は位置OFSを指定.
template< class T, std::size_t OFS=0, std::size_t LMAX=0x7fffffff >
class tiny_twlink {
public:
    typedef tiny_twlink     this_type;

    tiny_twlink(T* t=0) {
        typedef char static_assert[(OFS <= sizeof(T) - sizeof(this_type)) ? 1: -1 ];
        if (t == 0) {
            //デフォルト値設定、と、同時に文法で継承チェック.
            //Tはtiny_twlink<T>を継承してるハズなのでstatic_castできないといけない.
            t = static_cast<T*>(this);
            //t = (T*)(this);
        }
        assert(t > (this_type*)OFS);
        assert(((const char*)this - (const char*)t) == OFS
            && "template引数でのオフセットと実際のオフセットが違う");
        //assert(static_cast<T*>(0) == static_cast<this_type*>(0) && "Tがtiny_twlink<T>を継承していない");
        after_  = before_ = to_twlink(t);
    }

    ~tiny_twlink() {
        unlink();
    }

    ///< 自分自身のT*を返す.
    const T*    own()    const  { return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) - OFS); }
    T*          own()           { return reinterpret_cast<      T*>(reinterpret_cast<      char*>(this) - OFS); }
    const T*    after()  const  { return after_->own(); }                           ///< 自分の前の要素を返す.
    T*          after()         { return after_->own(); }
    const T*    before() const  { return before_->own();}                           ///< 自分の後の要素を返す.
    T*          before()        { return before_->own();}
    void        insert_after(T* a)  { insert_after_twlink(to_twlink(a)); }          ///< 自身の次に a をつなげる.
    void        insert_before(T* b) { insert_before_twlink(to_twlink(b)); }         ///< 自身の前に b をつなげる.
    std::size_t search( const T* s ) const { return search_twlink(to_twlink(s)); }  ///< sがリストにあれば1以上の値をなければ0を返す.
    bool        unique() const { return this && this == after_; }
    void        swap(tiny_twlink& r) {std::swap(after_, r.after_); std::swap(before_,r.before_); }
    void        invariant() const { assert( this && after_ && before_ ); }

    static const T* next(const T* t) { assert(t); return to_twlink(t)->after();}
    static T*       next(T* t)       { assert(t); return to_twlink(t)->after();}
    static const T* prev(const T* t) { assert(t); return to_twlink(t)->before();}
    static T*       prev(T* t)       { assert(t); return to_twlink(t)->before();}

    /** リンクされているモノの数を数える.
     */
    std::size_t link_size() const {
        unsigned size = 1;
        for (this_type* a = after_; a != this && size <= LMAX; a = a->after_) {
            ++size;
            assert( size < LMAX );
        }
        return size;
    }

    /** 自身をリストから取り外す.
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

    /** 再帰無しで 子供に対し FUNC を実行する.
     *  func(p) が trueを返した時点で終了.
     *  実行した数を返す.
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

    /** srch がリンク中にあれば 真 を返す
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

    /** 自身の次に a をつなげる
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

    /** 自身の前に b をつなげる
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
