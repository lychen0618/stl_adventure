#include "alloc.h"

using namespace sad;

template <typename _Tp>
void *pool_allocator<_Tp>::allocate(size_t n)
{
    if (n > static_cast<size_t>(__MAX_BYTES))
    {
        return malloc_allocator::allocate(n);
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
        malloc_allocator::deallocate(p, n);
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
        return static_cast<node *>(chunk);
    node **p_free_list = free_lists + free_list_index(n);
    node *result = static_cast<node *>(chunk);
    *p_free_list = static_cast<node *>(chunk + n);
    for (int i = 1;; ++i)
    {
        if (i == num_of_nodes - 1)
        {
            cur->next = 0;
            break;
        }
        cur->next = static_cast<node *>(static_cast<char *>(cur) + n);
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
            node **p_free_list = free_lists + free_list_index[left_bytes];
            static_cast<node *>(start_free)->next = *p_free_list;
            *p_free_list = static_cast<node *>(start_free);
        }

        size_t bytes_to_get = 2 * total_bytes + round_up(heap >> 4);
        start_free = static_cast<char *>(malloc(bytes_to_get));
        if (start_free == 0)
        {
            for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
            {
                node **p_free_list = free_lists + free_list_index(i);
                node *cur = *p_free_list;
                if (cur != 0)
                {
                    *p_free_list = cur->next;
                    start_free = static_cast<char *>(cur);
                    end_free = start_free + i;
                    return chunk_alloc(size, num_of_nodes);
                }
            }
            end_free = 0;
            start_free = static_cast<char *>(malloc_allocator::allocate(bytes_to_get));
        }
        head += bytes_to_get;
        end_free = start_free + bytes_to_get;
        result = chunk_alloc(size, num_of_nodes);
    }
    return result;
}