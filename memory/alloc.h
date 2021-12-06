#ifndef ALLOC_H
#define ALLOC_H

#include "construct.h"
#include <functional>

namespace sad
{
    template <typename _Tp>
    class malloc_allocator
    {
    public:
        typedef void (*oom_handler_functor)();

        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef _Tp *pointer;
        typedef const _Tp *const_pointer;
        typedef _Tp &reference;
        typedef const _Tp &const_reference;
        typedef _Tp value_type;

        malloc_allocator() = default;

        void *allocate(size_t n)
        {
            auto result = ::operator new(n * sizeof(value_type));
            return result;
        }

        void deallocate(void *p, size_t)
        {
            ::operator delete(p);
        }

        void *reallocate(void *p, size_t, size_t new_sz)
        {
            return nullptr;
        }

        template <typename _T1, typename... _Args>
        void construct(_T1 *__p, _Args &&...__args)
        {
            sad::_Construct(__p, __args...);
        }

        void destroy(_Tp *p)
        {
            sad::_Destroy(p);
        }

        // void *default_oom_malloc(size_t n)
        // {
        //     void *result;
        //     while (true)
        //     {
        //         if (oom_handler == 0)
        //         {
        //             exit(1);
        //         }
        //         oom_handler();
        //         result = ::operator new(n);
        //         if (result != 0)
        //             return result;
        //     }
        // }

        // void *default_oom_realloc(void *, size_t n)
        // {
        //     return nullptr;
        // }

        void set_oom_handler(oom_handler_functor oom_handler)
        {
            std::set_new_handler(oom_handler);
        }
    };

}

#endif // ALLOC_H