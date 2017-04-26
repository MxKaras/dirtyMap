
#ifndef FYP_MAPS_POOLS_HPP
#define FYP_MAPS_POOLS_HPP

namespace drt {

namespace drtx {

    template<typename T, size_t size>
    struct PoolIterator;

    template<typename T, size_t size>
    class _stackPoolBase {

        friend class PoolIterator<T, size>;

        using uchar = unsigned char;

        // address of next free block
        uintptr_t sp;
        alignas(T) uchar chunk[size * sizeof(T)];

    public:
        using iterator   = PoolIterator<T, size>;
        using value_type = T;

        _stackPoolBase() {
            sp = reinterpret_cast<uintptr_t>(&chunk);
        }

        /**
         * Finds the next free block of memory and returns a pointer to it, if
         * there is an unallocated block remaining.
         */
        void* allocate() {
            if (full()) return nullptr;

            void *a = reinterpret_cast<void*>(sp);
            sp += sizeof(T);
            return a;
        }

        /**
         * Removes an object from the pool. If this leaves a hole in the stack
         * then it is filled with the object at the top of the stack.
         *
         * Note: as this class was designed to be used by another, which does
         * perform bounds checking before deallocating, no check is made to
         * ensure that 'deallocated' belongs to this pool (or corresponds to
         * the start of a block).
         */
        void deallocate(void *deallocated) {
            // replace deallocated block with last object in the array
            uintptr_t top = sp - sizeof(T);
            T* replacement = reinterpret_cast<T*>(top);

            if (deallocated < static_cast<void*>(replacement)) {
                new(deallocated) T(std::move(*replacement));
            }

            sp = top;
        }

        /**
         * Destroys and deallocates element at ptr. As this may cause another
         * element to be moved within the pool, we return the address that the
         * moved element formerly occupied, as any pointers to it will have
         * been invalidated.
         *
         * @return Address of element that was previously at top of pool.
         */
        void* destroy(void *ptr) {
            T *obj = reinterpret_cast<T*>(ptr);
            obj->~T();
            deallocate(ptr);

            // means that destroyed element was on "top" of pool; nothing moved
            if (ptr == reinterpret_cast<void*>(sp)) return nullptr;
            return reinterpret_cast<void*>(sp);
        }

        /**
         * Calls the destructor on all objects stored in this pool.
         */
        void destroyAll() {
            T *obj = reinterpret_cast<T*>(&chunk);

            for (int i = 0; i < next_index(); ++i, ++obj) {
                obj->~T();
            }
        }

        /**
         * @param ptr A pointer.
         * @return true if ptr points to allocated memory within the pool.
         */
        bool owns(const void *ptr) const {
            return ptr >= chunk && ptr < reinterpret_cast<void*>(sp);
        }

        /// @return true if all blocks have been allocated.
        bool full() const {
            return next_index() == size;
        }

        /// @return true if all blocks are unallocated.
        bool empty() const {
            return sp == reinterpret_cast<uintptr_t>(chunk);
        }

        /// @return iterator pointing to index 0 of array.
        iterator begin() {
            return iterator(reinterpret_cast<T*>(chunk), 0);
        }

        /// @return iterator pointing to one index past last allocated object.
        iterator end() {
            return iterator(reinterpret_cast<T*>(chunk), next_index());
        }

    private:
        /// Converts address at 'sp' into the index it would be in a T[] array.
        size_t next_index() const {
            return (sp - reinterpret_cast<uintptr_t>(chunk)) / sizeof(T);
        }

        /// Converts address at p into the index it would be in a T[] array.
        size_t index_of(const uintptr_t p) const {
            return (p - reinterpret_cast<uintptr_t>(chunk)) / sizeof(T);
        }
    };

} // namespace drtx

    /**
     * Memory pool with a fixed capacity. The size parameter is the number of
     * objects that you want to store.
     *
     * We refer to this as a "stack" pool because we enforce contiguous objects
     * and maintain a stack pointer which points to the next free block. When
     * objects are deallocated or destroyed the object underneath the stack
     * pointer is moved to fill the gap.
     *
     * @tparam T The type of object to store.
     * @tparam size The number of T objects to reserve space for.
     */
    template<typename T, size_t size,
            bool no_destruct = std::is_trivially_destructible<T>::value >
    class StackedPool;

    /// Partial speciality for storing objects that require no destruction.
    template<typename T, size_t size>
    class StackedPool<T, size, true> : public drtx::_stackPoolBase<T, size> {
        using base_type = drtx::_stackPoolBase<T, size>;

    public:
        using iterator   = typename base_type::iterator;
        using value_type = typename base_type::value_type;

        StackedPool() : base_type() { }
    };

    /// Partial speciality for storing objects that require destruction.
    template<typename T, size_t size>
    class StackedPool<T, size, false> : public drtx::_stackPoolBase<T, size> {
        using base_type = drtx::_stackPoolBase<T, size>;

    public:
        using iterator   = typename base_type::iterator;
        using value_type = typename base_type::value_type;

        StackedPool() : base_type() { }

        ~StackedPool() {
            // call destructors on all stored objects.
            base_type::destroyAll();
        }
    };

    template<typename T, size_t size>
    inline bool operator==(const StackedPool<T, size> &lhs, const StackedPool<T, size> &rhs) {
        return lhs.chunk == rhs.chunk;
    }

    template<typename T, size_t size>
    inline bool operator!=(const StackedPool<T, size> &lhs, const StackedPool<T, size> &rhs) {
        return !(lhs == rhs);
    }

namespace drtx {

    /**
     * Iterator class for FixedPool. Simply iterates through the pool's array,
     * cast as type T.
     */
    template<typename T, size_t size>
    struct PoolIterator {

        using pool_type = _stackPoolBase<T, size>;
        using iterator  = typename pool_type::iterator;

        T *pool;
        size_t loc;

        PoolIterator(T *p, size_t i) : pool(p), loc(i) {}

        /// Return reference to pool element at current index.
        T& operator*() {
            return pool[loc];
        }

        /// Prefix increment
        iterator& operator++() {
            ++loc;
            return *this;
        }

        /// Postfix increment
        iterator operator++(int) {
            iterator temp(*this);
            ++loc;
            return temp;
        }

        /// Prefix decrement
        iterator& operator--() {
            --loc;
            return *this;
        }

        /// Postfix decrement
        iterator operator--(int) {
            iterator temp(*this);
            --loc;
            return temp;
        }

        bool operator==(const iterator &other) {
            return pool == other.pool && loc == other.loc;
        }

        bool operator!=(const iterator &other) {
            return !(*this == other);
        }
    };

} // namespace drtx
} // namespace drt

#endif //FYP_MAPS_POOLS_HPP
