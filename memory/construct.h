#ifndef CONSTRUCT_H
#define CONSTRUCT_H

#include <new>
#include <bits/stl_iterator_base_types.h>
#include <bits/move.h>
#include <memory>
namespace sad
{
#if __cplusplus >= 201103L
    template <typename _T1, typename... _Args>
    inline void
    _Construct(_T1 *__p, _Args &&...__args)
    {
        ::new (static_cast<void *>(__p)) _T1(std::forward<_Args>(__args)...);
    }
#else
    template <typename _T1, typename _T2>
    inline void
    _Construct(_T1 *__p, const _T2 &__value)
    {
        ::new (static_cast<void *>(__p)) _T1(__value);
    }
#endif

    template <typename T>
    inline void _Destroy(T *pointer)
    {
        pointer->~T();
    }

    template <bool>
    struct _Destroy_aux
    {
        template <typename _ForwardIterator>
        static void
        __destroy(_ForwardIterator __first, _ForwardIterator __last)
        {
            for (; __first != __last; ++__first)
                sad::_Destroy(std::__addressof(*__first));
        }
    };

    template <>
    struct _Destroy_aux<true>
    {
        template <typename _ForwardIterator>
        static void
        __destroy(_ForwardIterator, _ForwardIterator) {}
    };

    template <typename _ForwardIterator>
    inline void
    _Destroy(_ForwardIterator __first, _ForwardIterator __last)
    {
        typedef typename std::iterator_traits<_ForwardIterator>::value_type _Value_type;
        //__has_trivial_destructor(_Value_type)应该是编译器内置的函数
        sad::_Destroy_aux<__has_trivial_destructor(_Value_type)>::
            __destroy(__first, __last);
    }
}

#endif // CONSTRUCT_H