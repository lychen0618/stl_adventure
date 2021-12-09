#ifndef SAD_HASHTABLE_H
#define SAD_HASHTABLE_H
#include <iterator>
#include <algorithm>
//#include <vector>
#include <utility>
#include "alloc.h"
#include "sad_vector.h"

namespace sad
{
    template <typename V>
    struct __hashtable_node
    {
        __hashtable_node *next;
        V val;
    };

    // forward declaration;
    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey,
              typename Alloc = pool_allocator<__hashtable_node<V>>>
    class hashtable;

    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    struct __hashtable_iterator
    {
        typedef hashtable<V, K, HashFcn, ExtractKey, EqualKey, Alloc> _hashtable;
        typedef __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc> iterator;
        // typedef __hashtable_const_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc> const_iterator;
        typedef __hashtable_node<V> node;
        typedef std::forward_iterator_tag iterator_category;
        typedef V value_type;
        typedef V *pointer;
        typedef V &reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        node *cur;
        _hashtable *ht;

        __hashtable_iterator() = default;
        __hashtable_iterator(node *_node, _hashtable *_ht) : cur(_node), ht(_ht) {}

        reference operator*() { return cur->val; }

        pointer operator->() { return &(operator*()); }

        // only declare here
        iterator &operator++();
        iterator operator++(int);

        bool operator==(const iterator &it) const { return cur == it.cur; }

        bool operator!=(const iterator &it) const { return cur != it.cur; }
    };

    // 前置
    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    typename __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::iterator &
    __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::operator++()
    {
        const node *old = cur;
        cur = cur->next;
        if (!cur)
        {
            size_type bucket = ht->bkt_num(old->val);
            while (!cur && ++bucket < ht->buckets.size())
                cur = ht->buckets[bucket];
        }
        return *this;
    }

    // 后置
    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    typename __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::iterator
    __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::operator++(int)
    {
        iterator temp = this;
        // 注意这里的写法
        ++*this;
        return temp;
    }

    static const int __stl_num_primes = 28;
    static const unsigned long __stl_prime_list[__stl_num_primes] = {
        53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
        49157, 98317, 196613, 393241, 786433, 1572869, 3145739,
        6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
        805306457, 1610612741, 3221225473ul, 4294967291ul};

    inline unsigned long __stl_next_prime(unsigned long n)
    {
        const unsigned long *first = __stl_prime_list;
        const unsigned long *last = __stl_prime_list + __stl_num_primes;
        const unsigned long *pos = std::lower_bound(first, last, n);
        return pos == last ? *(last - 1) : *pos;
    }

    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    class hashtable
    {
    public:
        typedef HashFcn hash_fcn;
        typedef EqualKey equal_key;
        typedef size_t size_type;
        typedef __hashtable_iterator<V, K, HashFcn, ExtractKey, EqualKey, Alloc> iterator;

    private:
        typedef __hashtable_node<V> node;
        typedef Alloc data_allocator;
        typedef V value_type;
        typedef K key_type;

        hash_fcn _hash_fcn;
        equal_key _equal_key;
        ExtractKey _extract_key;

        size_type num_elements;

        size_type max_bucket_count() const { return __stl_prime_list[__stl_num_primes - 1]; }

        node *new_node(const value_type &value)
        {
            node *_node = (node *)data_allocator::allocate(1);
            _node->next = 0;
            try
            {
                construct(&_node->val, value);
                return _node;
            }
            catch (...)
            {
            }
            data_allocator::deallocate(_node, 1);
        }

        void delete_node(node *_node)
        {
            destroy(_node);
            data_allocator::deallocate(_node, 1);
        }

        size_type next_size(size_type n) { return __stl_next_prime(n); }

        void initialize_buckets(size_type n)
        {
            const size_type n_buckets = next_size(n);
            buckets.resize(n_buckets);
            buckets.insert(buckets.end(), n_buckets, (node *)0);
            num_elements = 0;
        }

        void resize(size_type num_elements_hint);
        std::pair<iterator, bool> insert_unique_noresize(const value_type &obj);

    public:
        vector<node *, Alloc> buckets;

        size_type bkt_num(const value_type &obj, size_t n) const { return bkt_num_key(_extract_key(obj), n); }

        size_type bkt_num(const value_type &obj) const { return bkt_num_key(_extract_key(obj)); }

        size_type bkt_num_key(const key_type &key) const { return bkt_num_key(key, buckets.size()); }

        size_type bkt_num_key(const key_type &key, size_t n) const
        {
            return _hash_fcn(key) % n;
        }

        size_type bucket_count() const { return buckets.size(); }

        size_type size() const { return num_elements; }

        iterator begin() { return iterator(buckets[0], this); }

        hashtable(size_type n, const HashFcn &hf, const EqualKey &eq) : _hash_fcn(hf), _equal_key(eq), _extract_key(ExtractKey()), num_elements(0)
        {
            initialize_buckets(n);
        }

        std::pair<iterator, bool> insert_unique(const value_type &obj)
        {
            resize(num_elements + 1);
            return insert_unique_noresize(obj);
        }

        void clear()
        {
            for (size_type i = 0; i < buckets.size(); ++i)
            {
                node *cur = buckets[i];
                while (cur)
                {
                    node *next = cur->next;
                    delete_node(cur);
                    cur = next;
                }
                buckets[i] = 0;
            }
            num_elements = 0;
        }
    };

    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    void hashtable<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::resize(size_type num_elements_hint)
    {
        const size_type old_n = buckets.size();
        if (num_elements_hint > old_n)
        {
            const size_type n = next_size(num_elements_hint);
            if (n > old_n)
            {
                vector<node *, Alloc> temp(n, (node *)0);
                try
                {
                    for (size_type bucket = 0; bucket < old_n; ++bucket)
                    {
                        node *first = buckets[bucket];
                        while (first)
                        {
                            size_type new_bucket = bkt_num(first->val, n);
                            buckets[bucket] = first->next;
                            first->next = temp[new_bucket];
                            temp[new_bucket] = first;
                            first = buckets[bucket];
                        }
                    }
                    std::swap(buckets, temp);
                }
                catch (...)
                {
                }
            }
        }
    }

    template <typename V, typename K, typename HashFcn, typename ExtractKey, typename EqualKey, typename Alloc>
    std::pair<typename hashtable<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::iterator, bool>
    hashtable<V, K, HashFcn, ExtractKey, EqualKey, Alloc>::insert_unique_noresize(const value_type &obj)
    {
        const size_type n = bkt_num(obj);
        node *first = buckets[n];

        for (node *cur = first; cur; cur = cur->next)
        {
            if (_equal_key(_extract_key(cur->val), _extract_key(obj)))
                return std::pair<iterator, bool>(iterator(cur, this), false);
        }
        node *temp = new_node(obj);
        temp->next = first;
        buckets[n] = temp;
        ++num_elements;
        return std::pair<iterator, bool>(iterator(temp, this), true);
    }

}

#endif // SAD_HASHTABLE_H