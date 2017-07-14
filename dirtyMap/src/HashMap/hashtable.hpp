
#ifndef FYP_MAPS_HASHTABLE_HPP
#define FYP_MAPS_HASHTABLE_HPP

namespace drt {
namespace drtx {

    /// SelectKey for Hashtable
    struct select_first {
        template<typename Pair>
        const auto operator()(const Pair &p) const
        -> typename std::tuple_element<0, Pair>::type& {
            return p.first;
        }
    };

    /// SelectKey for Hashset
    struct select_whole {
        template<typename T>
        const T operator()(const T &t) const {
            return t;
        }
    };

    template<typename Key, typename Val, typename Hash, typename SelectKey>
    class hashtable;

    /* Similar to how GCC splits up map and set abilities */
    template<typename Key, typename Val, typename Hash, typename SelectKey>
    struct _map_base { };

    /**
     * This template specialization means that certain class methods will be present
     * in Hashmap but not Hashset.
     */
    template<typename Key, typename Val, typename Hash>
    struct _map_base<Key, Val, Hash, select_first> {

    private:
        using hash_table   =  hashtable<Key, Val, Hash, select_first>;
        using value_type   =  Val;
        using bucket_type  =  drtx::Bucket<Key, Val, select_first>;
        using bucket_node  =  drtx::_bNode<Val>;

    public:
        using key_type     =  Key;
        using mapped_type  =  typename std::tuple_element<1, Val>::type;

        /**
         * @brief @c [] access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k.
         */
        mapped_type& operator[](const key_type &k) {
            hash_table *h = static_cast<hash_table*>(this);
            size_t index = h->hash_index(k);
            value_type *element = h->buckets[index].search(k);

            if (!element) {
                // perform rehash first, if needed.
                bool r = h->maybe_rehash();
                if (r) index = h->hash_index(k);
                bucket_type &b = h->buckets[index];

                if (b.isEmpty()) {
                    // piecewise construct instantiation inspired by GNU source
                    element = static_cast<value_type*>(h->elem_alloc.allocate());
                    new(element) value_type(std::piecewise_construct,
                                            std::tuple<const Key&>(k),
                                            std::tuple<>());
                    b.insert_node(element);
                } else {
                    bucket_node *ptr = static_cast<bucket_node*>(h->node_alloc.allocate());
                    new(ptr) bucket_node(value_type(std::piecewise_construct,
                                                    std::tuple<const Key&>(k),
                                                    std::tuple<>()));
                    b.insert_node(ptr);
                    element = &ptr->element;
                }

                ++h->_element_count;
            }

            return element->second;
        }

        /**
         * @brief @c [] access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k.
         */
        mapped_type& operator[](key_type &&k) {
            hash_table *h = static_cast<hash_table*>(this);
            size_t index = h->hash_index(k);
            value_type *element = h->buckets[index].search(k);

            if (!element) {
                // perform rehash first, if needed.
                bool r = h->maybe_rehash();
                if (r) index = h->hash_index(k);
                bucket_type &b = h->buckets[index];

                if (b.isEmpty()) {
                    // piecewise construct instantiation inspired by GNU source
                    element = static_cast<value_type*>(h->elem_alloc.allocate());
                    new(element) value_type(std::piecewise_construct,
                                            std::forward_as_tuple(std::move(k)),
                                            std::tuple<>());
                    b.insert_node(element);
                } else {
                    bucket_node *ptr = static_cast<bucket_node*>(h->node_alloc.allocate());
                    new(ptr) bucket_node(value_type(std::piecewise_construct,
                                                    std::forward_as_tuple(std::move(k)),
                                                    std::tuple<>()));
                    b.insert_node(ptr);
                    element = &ptr->element;
                }

                ++h->_element_count;
            }

            return element->second;
        }

        /**
         * @brief Access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k, if it exists.
         *
         * Return the value mapped to the provided key. If it doesn't exist in
         * the map, throw an out_of_range error.
         */
        mapped_type& at(const key_type &k) {
            hash_table *h = static_cast<hash_table*>(this);
            size_t index = h->hash_index(k);
            value_type *element = h->buckets[index].search(k);

            if (!element) {
                throw std::out_of_range("hashtable::at");
            }

            return element->second;
        }
    };

    /**
     * This is the structure which underpins both Hashmap and Hashset.
     *
     * @tparam Key Type of key which is used to access stored elements.
     * @tparam Val The whole data structure which is stored.
     * @tparam Hash Type of hash function used for element storage and lookup.
     * @tparam SelectKey Function object used to extract the key from an element.
     */
    template<typename Key, typename Val, typename Hash, typename SelectKey>
    class hashtable : public _map_base<Key, Val, Hash, SelectKey> {

    public:
        using key_type      =  Key;
        using value_type    =  Val;

    private:
        using bucket_type   =  drtx::Bucket<key_type, value_type, SelectKey>;
        using bucket_node   =  drtx::_bNode<value_type>;
        using vector_type   =  std::vector<bucket_type>;
        using v_iterator    =  typename vector_type::iterator;
        using elem_alloc_t  =  DtPoolAllocator<value_type>;
        using node_alloc_t  =  DtPoolAllocator<bucket_node>;

        vector_type buckets;
        node_alloc_t node_alloc;
        elem_alloc_t elem_alloc;

        size_t _element_count = 0;
        float _max_load_factor = 1.0;

        // ebo one day?
        Hash hasher;

        // needs access to buckets
        friend class drtx::HashMapIterator<Val, bucket_type, v_iterator>;
        friend struct _map_base<Key, Val, Hash, SelectKey>;

    public:
        using iterator  =  drtx::HashMapIterator<Val, bucket_type, v_iterator>;

        // constructors & destructor

        hashtable() : buckets(1), hasher(), node_alloc(), elem_alloc() { }

        hashtable(size_t n, const Hash &hf = Hash()) : buckets(n), hasher(hf), node_alloc(), elem_alloc() { }

        ~hashtable() = default;
        hashtable(const hashtable&) = default;
        hashtable& operator=(const hashtable&) = default;
        hashtable(hashtable&&) = default;
        hashtable& operator=(hashtable&&) = default;

        // size & capacity

        /// Returns the number of elements in the map.
        size_t size() const noexcept {
            return _element_count;
        }

        /// Returns maximum number of elements that can be stored.
        size_t max_size() const noexcept {
            return buckets.max_size();
        }

        /// Returns the number of buckets in the map.
        size_t bucket_count() const noexcept {
            return buckets.capacity();
        }

        /// Returns true if there are no elements in the map.
        bool empty() const noexcept {
            return _element_count == 0;
        }

        // modifiers

        /**
         * Removes all elements from the map. Does not change the number
         * of buckets.
         */
        void clear() {
            node_alloc.destroyAll();
            elem_alloc.destroyAll();

            for (bucket_type &buk : buckets) {
                buk.head = nullptr;
            }

            _element_count = 0;
        }

        /**
         * Removes and destroys the element corresponding to key k.
         *
         * @param k Key of the element to be removed.
         * @return  The number of elements that were removed (0 or 1).
         */
        size_t erase(const Key &k) {
            size_t index = hasher(k) % bucket_count();
            bucket_type &b = buckets[index];
            value_type *element = b.search(k);

            if (!element) return 0;
            // pair<bool, bucket_node*>
            auto removed = b.remove_node(element);

            if (removed.first) {
                destroy_bucket_element(reinterpret_cast<void*>(element), element->first);

                if (removed.second) {
                    /* We removed an element, leaving a node at the tail of
                    a bucket. This node needs to be converted to an element */
                    // first make element from node to be replaced
                    value_type *replacement = static_cast<value_type*>(elem_alloc.allocate());
                    new(replacement) value_type(std::move(removed.second->element));
                    // update bucket tail with new element
                    b.update_element(reinterpret_cast<void*>(removed.second), replacement);
                    // destroy node and potentially update other moved node
                    destroy_bucket_node(reinterpret_cast<void*>(removed.second), removed.second->element.first);
                }
            } else {
                destroy_bucket_node(reinterpret_cast<void*>(element), element->first);
            }

            _element_count -= 1;
            return 1;
        }

        // insert/emplace methods?

        // lookup

        /**
         *
         * @param k The key for which to count elements.
         * @return The number of elements with the provided key (1 or 0).
         *
         * As there can be no duplicate key, this will only return 1 or 0.
         */
        size_t count(const Key &k) const {
            size_t index = hasher(k) % bucket_count();
            value_type *element = buckets[index].search(k);

            if (element) {
                return 1;
            }
            return 0;
        }

        // rehashing

        /// Returns maximum ratio of elements to buckets.
        float max_load_factor() const noexcept {
            return _max_load_factor;
        }

        /// Setter for the maximum load factor.
        void max_load_factor(float f) {
            _max_load_factor = f;
        }

        /// Returns the current ratio of elements to buckets.
        float load_factor() const noexcept {
            return static_cast<float>(size()) / static_cast<float>(bucket_count());
        }

        /**
         * Increases the number of buckets used to store elements and reassigns
         * all currently mapped elements to the proper bucket based on the new
         * size.
         *
         * @param new_size The new number of buckets to use.
         */
        void rehash(size_t new_size) {
            // no point rehashing to smaller size.
            if (new_size <= bucket_count()) return;

            vector_type temp(new_size);

            // put elements from old vector into new bucket locations
            // in new vector. Ordering of these methods is important!
            reassign_elements(temp, new_size);
            reassign_nodes(temp, new_size);

            buckets.swap(temp);
        }

        // iterators

        iterator begin() {
            v_iterator b = buckets.begin();
            v_iterator e = buckets.end();
            return iterator(b, e);
        }

        iterator end() {
            v_iterator e = buckets.end();
            return iterator(e, e);
        }

    private:
        bool maybe_rehash() {
            // check if rehash needed, and if so, new array size.
            std::pair<bool, size_t> need_rehash = check_rehash_needed();

            if (need_rehash.first) {
                rehash(need_rehash.second);
                return true;
            }
            return false;
        }

        /**
         * If a rehash is needed, pick the new size for the array.
         *
         * @return std::pair(true, new_size) if rehash is needed.
         */
        std::pair<bool, size_t> check_rehash_needed() {
            if (load_factor() < max_load_factor()) {
                return std::pair<bool, size_t>(false, 0);
            }

            // quick and dirty for now.
            size_t new_size = (bucket_count() * 2) + 1;

            return std::pair<bool, size_t>(true, new_size);
        };

        /**
         * Takes elements stored by the allocator and assigns them to new
         * buckets in a fresh vector. If an element needs to be stored in a
         * node, it is moved to the node pool but NOT inserted into a bucket.
         *
         * @param vec  A temporary vector used during rehashing.
         * @param size The size of the vector.
         */
        void reassign_elements(vector_type &vec, size_t size) {
            auto it = elem_alloc.begin();
            auto end_ = elem_alloc.end();

            for (; it != end_; ++it) {
                value_type &element = *it;
                size_t index = hasher(element.first) % size;
                bucket_type &b = vec[index];

                if (b.isEmpty()) {
                    // inserting into empty bucket -> easy
                    b.insert_node(&element);
                } else {
                    // Need to move element from element pool into BNode in node pool.
                    // Don't insert it into a bucket yet though, as it'll get swept up
                    // in the node pool sweep.

                    // First make new node and put it in node pool
                    bucket_node *node_ptr = static_cast<bucket_node*>(node_alloc.allocate());
                    new(node_ptr) bucket_node(std::move(element));
                    // Remove original element from element pool
                    it.deallocate(&element);
                    // The deallocated block gets refilled, so need to look at this block again
                    --it;
                    end_ = elem_alloc.end();
                }
            }
        }

        /**
         * Takes nodes stored by the allocator and assigns them to new buckets
         * in a fresh vector. If a node needs to become an element, it is moved
         * to the element pool as well as inserted into a bucket.
         *
         * @param vec  A temporary vector used during rehashing.
         * @param size The size of the vector.
         */
        void reassign_nodes(vector_type &vec, size_t size) {
            auto it = node_alloc.begin();
            auto end_ = node_alloc.end();

            for (; it != end_; ++it) {
                bucket_node &node = *it;
                size_t index = hasher(node.element.first) % size;
                bucket_type &b = vec[index];

                if (b.isEmpty()) {
                    // inserting into empty bucket -> need to transfer pools
                    value_type *ele_ptr = static_cast<value_type*>(elem_alloc.allocate());
                    new(ele_ptr) value_type(std::move(node.element));
                    // remove node from node pool
                    it.deallocate(&node);
                    --it;
                    end_ = node_alloc.end();
                    b.insert_node(ele_ptr);
                } else {
                    b.insert_node(&node);
                }
            }
        }

        /**
         * Destroys node at `ptr` and updates invalidated bucket pointer if
         * necessary.
         */
        void destroy_bucket_node(void *ptr, const Key &k) {
            // When object at ptr is destroyed, a new object may be moved
            // to its address, which changes the value of k.
            void *prev = node_alloc.destroy(ptr);

            if (prev) {
                size_t to_update = hasher(k) % bucket_count();
                buckets[to_update].update_node(prev, ptr);
            }
        }

        /**
         * Destroys element at `ptr` and updates invalidated bucket pointer if
         * necessary.
         */
        void destroy_bucket_element(void *ptr, const Key &k) {
            // When object at ptr is destroyed, a new object may be moved
            // to its address, which changes the value of k.
            void *prev = elem_alloc.destroy(ptr);

            if (prev) {
                size_t to_update = hasher(k) % bucket_count();
                buckets[to_update].update_element(prev, ptr);
            }
        }

        size_t hash_index(const key_type &k) const {
            return hasher(k) % bucket_count();
        }
    };

}
}

#endif //FYP_MAPS_HASHTABLE_HPP
