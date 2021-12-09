#ifndef ALLOC_H
#define ALLOC_H

#include "construct.h"
#include <cstdlib>
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

    template <typename _Tp>
    size_t pool_allocator<_Tp>::heap_size = 0;

    template <typename _Tp>
    char *pool_allocator<_Tp>::start_free = 0;

    template <typename _Tp>
    char *pool_allocator<_Tp>::end_free = 0;
    template <typename _Tp>
    class pool_allocator<_Tp>::node *
        pool_allocator<_Tp>::free_lists[__SIZE_OF_FREELISTS] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    template <typename _Tp>
    void *pool_allocator<_Tp>::allocate(size_t n)
    {
        if (n > static_cast<size_t>(__MAX_BYTES))
        {
            return malloc_allocator<_Tp>::allocate(n);
        }
        node **p_free_list = free_lists + free_list_index(n);
        node *res = *p_free_list;
        // res为空指针，初始时free_lists存的都是空指针
        if (res == nullptr)
        {
            void *r = refill(round_up(n));
            return r;
        }
        *p_free_list = res->next;
        return res;
    }

    template <typename _Tp>
    void pool_allocator<_Tp>::deallocate(void *p, size_t n)
    {
        if (n > static_cast<size_t>(__MAX_BYTES))
        {
            malloc_allocator<_Tp>::deallocate(p, n);
        }
        node *q = static_cast<node *>(p);
        node **p_free_list = free_lists + free_list_index(n);
        q->next = *p_free_list;
        *p_free_list = q;
    }

    template <typename _Tp>
    void *pool_allocator<_Tp>::realloc(void *p, size_t old_sz, size_t new_sz)
    {
        return 0;
    }

    template <typename _Tp>
    void *pool_allocator<_Tp>::refill(size_t n)
    {
        int num_of_nodes = 20;
        char *chunk = chunk_alloc(n, num_of_nodes);
        if (num_of_nodes == 1)
            return chunk;
        node **p_free_list = free_lists + free_list_index(n);
        node *result = (node*)chunk;
        *p_free_list = (node*)(chunk + n);
        node *cur = *p_free_list;
        for (int i = 1;; ++i)
        {
            if (i == num_of_nodes - 1)
            {
                cur->next = 0;
                break;
            }
            cur->next = (node*)((char*)(cur) + n);
            cur = cur->next;
        }
        return result;
    }

    template <typename _Tp>
    char *pool_allocator<_Tp>::chunk_alloc(size_t size, int &num_of_nodes)
    {
        char *result;
        size_t total_bytes = size * num_of_nodes;
        size_t left_bytes = end_free - start_free;
        if (left_bytes >= total_bytes)
        {
            result = start_free;
            start_free += total_bytes;
        }
        else if (left_bytes >= size)
        {
            num_of_nodes = left_bytes / size;
            total_bytes = num_of_nodes * size;
            result = start_free;
            start_free += total_bytes;
        }
        else
        {
            // 剩余内存一定是8的倍数
            if (left_bytes > 0)
            {
                node **p_free_list = free_lists + free_list_index(left_bytes);
                ((node*)start_free)->next = *p_free_list;
                *p_free_list = (node*)(start_free);
            }

            size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
            start_free = (char*)(malloc(bytes_to_get));
            if (start_free == 0)
            {
                for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
                {
                    node **p_free_list = free_lists + free_list_index(i);
                    node *cur = *p_free_list;
                    if (cur != 0)
                    {
                        *p_free_list = cur->next;
                        start_free = (char*)(cur);
                        end_free = start_free + i;
                        return chunk_alloc(size, num_of_nodes);
                    }
                }
                end_free = 0;
                start_free = (char*)(malloc_allocator<_Tp>::allocate(bytes_to_get));
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            result = chunk_alloc(size, num_of_nodes);
        }
        return result;
    }

}

#endif // ALLOC_H