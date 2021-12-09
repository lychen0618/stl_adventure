#ifndef SAD_VECTOR_H
#define SAD_VECTOR_H
#include "alloc.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace sad
{
    template <typename T, typename Alloc = pool_allocator<T>>
    class vector
    {
    public:
        typedef T value_type;
        typedef value_type *pointer;
        typedef value_type *iterator;
        typedef value_type &reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

    protected:
        typedef Alloc data_allocator;
        iterator start;
        iterator finish;
        iterator end_of_storage;

        void insert_aux(iterator position, const T &x);

        void deallocate()
        {
            if (start)
            {
                data_allocator::deallocate(start, end_of_storage - start);
            }
        }

        void fill_initialize(size_type n, const T &value)
        {
            start = allocate_and_fill(n, value);
            finish = start + n;
            end_of_storage = finish;
        }

        iterator allocate_and_fill(size_type n, const T &value)
        {
            iterator result = (iterator)data_allocator::allocate(n);
            std::uninitialized_fill_n(result, n, value);
            return result;
        }

    public:
        vector() : start(0), finish(0), end_of_storage(0) {}

        vector(size_type n, const T &value) { fill_initialize(n, value); }

        explicit vector(size_type n) { fill_initialize(n, T()); }

        ~vector()
        {
            destroy(start, finish);
            deallocate();
        }

        iterator begin() { return start; }

        iterator end() { return finish; }

        size_type size() { return static_cast<size_type>(end() - begin()); }

        size_type capacity() const { return static_cast<size_type>(end_of_storage() - begin()); }

        bool empty() const { return end() == begin(); }

        reference operator[](size_type n) { return *(begin() + n); }

        reference front() { return *begin(); }

        reference back() { return *(end() - 1); }

        void push_back(const T &value)
        {
            if (finish != end_of_storage)
            {
                construct(finish, value);
                ++finish;
            }
            else
                insert_aux(end(), value);
        }

        void pop_back()
        {
            --finish;
            destroy(finish);
        }

        iterator erase(iterator position)
        {
            if (position + 1 != end())
            {
                copy(position + 1, finish, position);
            }
            --finish;
            destroy(finish);
            return position;
        }

        iterator erase(iterator st_position, iterator ed_position)
        {
            if (ed_position <= finish)
            {
                iterator i = copy(ed_position, finish, st_position);
                destroy(i, finish);
                finish = i;
            }
            else
                erase(st_position, finish);
            return st_position;
        }

        void resize(size_type new_size, const T &x)
        {
            if (new_size < size())
                erase(start + new_size, finish);
            else
                insert(finish, new_size - size(), x);
        }

        void resize(size_type new_size) { resize(new_size, T()); }

        void clear() { erase(begin(), end()); }

        void insert(iterator position, size_type n, const T &x);
    };

    template <typename T, typename Alloc>
    void vector<T, Alloc>::insert_aux(iterator position, const T &x)
    {
        if (finish != end_of_storage)
        {
            construct(finish, *(finish - 1));
            ++finish;
            T x_copy = x;
            std::copy_backward(position, finish - 2, finish - 1);
            (*position) = x_copy;
        }
        else
        {
            const size_type old_size = size();
            const size_type len = (old_size != 0 ? 2 * old_size : 1);
            iterator new_start = (iterator)data_allocator::allocate(len);
            iterator new_finish = new_start;
            try
            {
                new_finish = std::uninitialized_copy(start, position, new_start);
                construct(new_finish, x);
                ++new_finish;
                new_finish = std::uninitialized_copy(position, finish, new_finish);
            }
            catch (...)
            {
                // commit or rollback
                destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }

            destroy(start, finish);
            deallocate();
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + len;
        }
    }

    template <typename T, typename Alloc>
    void vector<T, Alloc>::insert(iterator position, size_type n, const T &x)
    {
        if (n != 0)
        {
            if (static_cast<size_type>(end_of_storage - finish) >= n)
            {
                T x_copy = x;
                const size_type elements_after = finish - position;
                iterator old_finish = finish;
                if (elements_after > n)
                {
                    std::uninitialized_copy(finish - n, finish, finish);
                    finish += n;
                    std::copy_backward(position, old_finish - n, old_finish);
                    std::fill(position, position + n, x_copy);
                }
                else
                {
                    std::uninitialized_fill_n(finish, n - elements_after, x_copy);
                    finish += n;
                    std::uninitialized_copy(position, old_finish, old_finish + n - elements_after);
                    std::fill(position, old_finish, x_copy);
                }
            }
            else
            {
                const size_type old_size = size();
                const size_type len = old_size + std::max(old_size, n);
                iterator new_start = (iterator)data_allocator::allocate(len);
                iterator new_finish = new_start;
                try
                {
                    new_finish = std::uninitialized_copy(start, position, new_start);
                    new_finish = std::uninitialized_fill_n(new_finish, n, x);
                    new_finish = std::uninitialized_copy(position, finish, new_finish);
                }
                catch (...)
                {
                    // commit or rollback
                    destroy(new_start, new_finish);
                    data_allocator::deallocate(new_start, len);
                    throw;
                }
                destroy(start, finish);
                deallocate();
                start = new_start;
                finish = new_finish;
                end_of_storage = start + len;
            }
        }
    }

}

#endif // SAD_VECTOR_H