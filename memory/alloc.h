#ifndef ALLOC_H
#define ALLOC_H

#include "construct.h"
//#include <functional>

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

        static void *allocate(size_t n)
        {
            auto result = ::operator new(n * sizeof(value_type));
            return result;
        }

        static void deallocate(void *p, size_t)
        {
            ::operator delete(p);
        }

        static void *reallocate(void *p, size_t, size_t new_sz)
        {
            return nullptr;
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

        static void set_oom_handler(oom_handler_functor oom_handler)
        {
            std::set_new_handler(oom_handler);
        }
    };

    namespace
    {
        enum
        {
            __ALIGN = 8
        };
        enum
        {
            __MAX_BYTES = 128
        };
        enum
        {
            __SIZE_OF_FREELISTS = __MAX_BYTES / __ALIGN
        };
    }

    template <typename _Tp>
    class pool_allocator
    {
    private:
        union node
        {
            union node *next;
            char client_data[1];
        };

        static node *free_lists[__SIZE_OF_FREELISTS];
        static char *start_free;
        static char *end_free;
        static size_t heap_size;

        static size_t free_list_index(size_t bytes)
        {
            return (bytes + __ALIGN - 1) / __ALIGN - 1;
        }

        static size_t round_up(size_t bytes)
        {
            return (bytes + __ALIGN - 1) & ~(__ALIGN - 1);
        }

        static void *refill(size_t n);

        static char *chunk_alloc(size_t size, int &num_of_nodes);

    public:
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef _Tp *pointer;
        typedef const _Tp *const_pointer;
        typedef _Tp &reference;
        typedef const _Tp &const_reference;
        typedef _Tp value_type;

        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *realloc(void *p, size_t old_sz, size_t new_sz);
    };
}

#endif // ALLOC_H