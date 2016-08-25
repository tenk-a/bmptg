
//--------------------------------------------------
// copy_backward auxiliary functions

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _Distance>
inline _BidirectionalIter2 __copy_backward(_BidirectionalIter1 __first,
                                           _BidirectionalIter1 __last,
                                           _BidirectionalIter2 __result,
                                           const bidirectional_iterator_tag &,
                                           _Distance*) {
  while (__first != __last)
    *--__result = *--__last;
  return __result;
}

template <class _RandomAccessIter, class _BidirectionalIter, class _Distance>
inline _BidirectionalIter __copy_backward(_RandomAccessIter __first,
                                          _RandomAccessIter __last,
                                          _BidirectionalIter __result,
                                          const random_access_iterator_tag &,
                                          _Distance*) {
  for (_Distance __n = __last - __first; __n > 0; --__n)
    *--__result = *--__last;
  return __result;
}

inline void*
__copy_trivial_backward(const void* __first, const void* __last, void* __result) {
  const ptrdiff_t _Num = (const char*)__last - (const char*)__first;
  return (_Num > 0) ? memmove((char*)__result - _Num, __first, _Num) : __result ;
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_ptrs(_InputIter __first, _InputIter __last, _OutputIter __result,
                               const __false_type& /*IsOKToMemCpy*/) {
  return _STLP_PRIV __copy(__first, __last, __result, random_access_iterator_tag(), (ptrdiff_t*)0);
}
template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_ptrs(_InputIter __first, _InputIter __last, _OutputIter __result,
                               const __true_type& /*IsOKToMemCpy*/) {
  // we know they all pointers, so this cast is OK
  //  return (_OutputIter)__copy_trivial(&(*__first), &(*__last), &(*__result));
  return (_OutputIter)_STLP_PRIV __copy_trivial(__first, __last, __result);
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_aux(_InputIter __first, _InputIter __last, _OutputIter __result,
                              const __true_type& /*BothPtrType*/) {
  return _STLP_PRIV __copy_ptrs(__first, __last, __result,
                                _UseTrivialCopy(_STLP_VALUE_TYPE(__first, _InputIter),
                                                _STLP_VALUE_TYPE(__result, _OutputIter))._Answer());
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_aux(_InputIter __first, _InputIter __last, _OutputIter __result,
                              const __false_type& /*BothPtrType*/) {
  return _STLP_PRIV __copy(__first, __last, __result,
                           _STLP_ITERATOR_CATEGORY(__first, _InputIter),
                           _STLP_DISTANCE_TYPE(__first, _InputIter));
}

_STLP_MOVE_TO_STD_NAMESPACE

template <class _InputIter, class _OutputIter>
inline _OutputIter copy(_InputIter __first, _InputIter __last, _OutputIter __result) {
  _STLP_DEBUG_CHECK(_STLP_PRIV __check_range(__first, __last))
  return _STLP_PRIV __copy_aux(__first, __last, __result, _BothPtrType< _InputIter, _OutputIter>::_Answer());
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_backward_ptrs(_InputIter __first, _InputIter __last,
                                        _OutputIter __result, const __false_type& /*TrivialAssignment*/) {
  return _STLP_PRIV __copy_backward(__first, __last, __result,
                                    _STLP_ITERATOR_CATEGORY(__first, _InputIter),
                                    _STLP_DISTANCE_TYPE(__first, _InputIter));
}
template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_backward_ptrs(_InputIter __first, _InputIter __last,
                                        _OutputIter __result, const __true_type& /*TrivialAssignment*/) {
  return (_OutputIter)_STLP_PRIV __copy_trivial_backward(__first, __last, __result);
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_backward_aux(_InputIter __first, _InputIter __last, _OutputIter __result, const __false_type&) {
  return _STLP_PRIV __copy_backward(__first, __last, __result,
                                    _STLP_ITERATOR_CATEGORY(__first,_InputIter),
                                    _STLP_DISTANCE_TYPE(__first, _InputIter));
}

template <class _InputIter, class _OutputIter>
inline _OutputIter __copy_backward_aux(_InputIter __first, _InputIter __last, _OutputIter __result, const __true_type&) {
  return __copy_backward_ptrs(__first, __last, __result, _UseTrivialCopy(_STLP_VALUE_TYPE(__first, _InputIter), _STLP_VALUE_TYPE(__result, _OutputIter))._Answer());
}

template <class _InputIter, class _OutputIter>
inline _OutputIter copy_backward(_InputIter __first, _InputIter __last, _OutputIter __result) {
  return _STLP_PRIV __copy_backward_aux(__first, __last, __result, _BothPtrType< _InputIter, _OutputIter>::_Answer() );
}

template <class _RandomAccessIter, class _Tp, class _Compare>
void __unguarded_linear_insert(_RandomAccessIter __last, _Tp __val, _Compare __comp) {
  _RandomAccessIter __next = __last;
  --__next;
  while (__comp(__val, *__next)) {
    *__last = *__next;
    __last = __next;
    --__next;
  }
  *__last = __val;
}

template <class _RandomAccessIter, class _Tp, class _Compare> inline
void __linear_insert(_RandomAccessIter __first, _RandomAccessIter __last, _Tp __val, _Compare __comp) {
  if (__comp(__val, *__first)) {
    copy_backward(__first, __last, __last + 1);
    *__first = __val;
  }
  else
    __unguarded_linear_insert(__last, __val, __comp);
}

template <class _RandomAccessIter, class _Tp, class _Compare>
void __insertion_sort(_RandomAccessIter __first, _RandomAccessIter __last, _Tp *, _Compare __comp) {
  if (__first == __last)
    return;
  for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
    __linear_insert<_RandomAccessIter, _Tp, _Compare>(__first, __i, *__i, __comp);
}

template <class _Tp, class _Compare> inline
const _Tp& __median(const _Tp& __a, const _Tp& __b, const _Tp& __c, _Compare __comp) {
  if (__comp(__a, __b)) {
    if (__comp(__b, __c))
      return __b;
    else if (__comp(__a, __c))
      return __c;
    else
      return __a;
  }
  if (__comp(__a, __c))
    return __a;
  else if (__comp(__b, __c))
    return __c;
  else
    return __b;
}

template <class _RandomAccessIter, class _Tp, class _Compare>
void __nth_element(_RandomAccessIter __first, _RandomAccessIter __nth, _RandomAccessIter __last, _Tp*, _Compare __comp) {
  while (__last - __first > 3) {
    _RandomAccessIter __cut = __unguarded_partition(__first, __last , _Tp(__median(*__first, *(__first + (__last - __first)/2), *(__last - 1), __comp)), __comp);
    if (__cut <= __nth)
      __first = __cut;
    else
      __last = __cut;
  }
  __insertion_sort(__first, __last, _STLP_VALUE_TYPE(__first,_RandomAccessIter), __comp);
}

template <class _RandomAccessIter, class _Compare>
void nth_element(_RandomAccessIter __first, _RandomAccessIter __nth, _RandomAccessIter __last, _Compare __comp)
{
  __nth_element(__first, __nth, __last, (iterator_traits<_RandomAccessIter>::value_type*)0, __comp);
}

template <class _RandomAccessIter>
void nth_element(_RandomAccessIter __first, _RandomAccessIter __nth, _RandomAccessIter __last) {
  __nth_element(__first, __nth, __last, (iterator_traits<_RandomAccessIter>::value_type*)0, std::less<_RandomAccessIter>());
}
