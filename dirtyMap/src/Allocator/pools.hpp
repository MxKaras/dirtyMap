
#ifndef FYP_MAPS_POOLS_HPP
#define FYP_MAPS_POOLS_HPP

#define MPAGE_SIZE 4096
/*
 * Buddy order MUST be a power of 2, and NOT greater than 1024.
 * The resulting pool size = page size * buddy order
 * i.e. 128 -> 0.5MB, 512 -> 2MB, etc (for 4K pages).
 */
#define MBUDDY_ORDER 256

namespace drt {

namespace drtx {

    template<typename T, typename C>
    struct PoolIterator;

    template<typename T>
    struct buddy_mb_count {
        enum { value = (MPAGE_SIZE * MBUDDY_ORDER) / sizeof(T) };
    };

    template<size_t N>
    struct alignas(64) pool_storage {
        unsigned char chunk[N];

        pool_storage() = default;
        ~pool_storage() = default;
    };

    template <typename T, typename obj_count>
    class _stackPoolBase {

        using uchar  = unsigned char;
        using pool_t = pool_storage<obj_count::value * sizeof(T)>;

        friend class PoolIterator<T, obj_count>;

        uintptr_t sp;
        pool_t *storage;

    public:
        using value_type = T;
        using iterator   = PoolIterator<T, obj_count>;

        _stackPoolBase() {
            storage = new pool_t();
            sp = reinterpret_cast<uintptr_t>(storage);
        }

        ~_stackPoolBase() { delete storage; }

        // need noexcept so that pools are move-constructed when the pool vector resizes
        _stackPoolBase(_stackPoolBase&& other) noexcept : sp(other.sp), storage(other.storage) {
            other.sp = 0;
            other.storage = nullptr;
        }

        _stackPoolBase& operator=(_stackPoolBase&& other) noexcept {
            if (this != &other) {
                delete storage;
                storage = other.storage;
                sp = other.sp;
                other.storage = nullptr;
                other.sp = 0;
            }

            return *this;
        }

        /// @return the maximum number of objects that can be stored.
        constexpr size_t capacity() const {
            return obj_count::value;
        }

        /// @return the number of bytes held by the pool.
        constexpr size_t capacity_bytes() const {
            return capacity() * sizeof(T);
        }

        /// @return the number of objects stored in the pool.
        size_t size() const {
            return (sp - reinterpret_cast<uintptr_t>(storage)) / sizeof(T);
        }

        /// @return true if ptr points to an object within the pool.
        bool owns(const void *ptr) const {
            return ptr >= storage && ptr < reinterpret_cast<void*>(sp);
        }

        /// @return true if all blocks have been allocated.
        bool full() const {
            return size() == capacity();
        }

        /// @return true if all blocks are unallocated.
        bool empty() const {
            return sp == reinterpret_cast<uintptr_t>(storage);
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
         */
        void deallocate(void *deallocated) {
            // replace deallocated block with last object in the array
            uintptr_t top = sp - sizeof(T);

            if (deallocated < reinterpret_cast<void*>(top)) {
                T* replacement = reinterpret_cast<T*>(top);
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

        /// @return iterator pointing to index 0 of array.
        iterator begin() {
            return iterator(reinterpret_cast<T*>(storage), 0);
        }

        /// @return iterator pointing to one index past last allocated object.
        iterator end() {
            return iterator(reinterpret_cast<T*>(storage), size());
        }

        iterator find(void *ptr) {
            return iterator(reinterpret_cast<T*>(storage), index_of(ptr));
        }

    protected:
        void set_sp(uintptr_t p) {
            sp = p;
        }

        pool_t* get_storage() const {
            return storage;
        }

        uintptr_t get_storage_u() const {
            return reinterpret_cast<uintptr_t>(storage);
        }

    private:
        size_t index_of(void *ptr) const {
            return (reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(storage)) / sizeof(T);
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
    template<typename T, typename obj_count = drtx::buddy_mb_count<T>,
            bool no_destruct = std::is_trivially_destructible<T>::value >
    class StackedPool;

    /// Partial speciality for storing objects that require no destruction.
    template<typename T, typename obj_count>
    class StackedPool<T, obj_count, true> : public drtx::_stackPoolBase<T, obj_count> {
        using base_type = drtx::_stackPoolBase<T, obj_count>;

    public:
        using iterator   = typename base_type::iterator;
        using value_type = typename base_type::value_type;

        StackedPool() = default;
        ~StackedPool() = default;
        StackedPool(const StackedPool&) = default;
        StackedPool& operator=(const StackedPool&) = default;
        StackedPool(StackedPool&&) = default;
        StackedPool& operator=(StackedPool&&) = default;

        void destroyAll() {
            this->set_sp(this->get_storage_u());
        }
    };

    /// Partial speciality for storing objects that require destruction.
    template<typename T, typename obj_count>
    class StackedPool<T, obj_count, false> : public drtx::_stackPoolBase<T, obj_count> {
        using base_type = drtx::_stackPoolBase<T, obj_count>;

    public:
        using iterator   = typename base_type::iterator;
        using value_type = typename base_type::value_type;

        StackedPool() = default;
        ~StackedPool() { destroyAll(); }
        StackedPool(const StackedPool&) = default;
        StackedPool& operator=(const StackedPool&) = default;
        StackedPool(StackedPool&&) = default;
        StackedPool& operator=(StackedPool&&) = default;

        void destroyAll() {
            T *obj = reinterpret_cast<T*>(this->get_storage_u());

            for (int i = 0; i < this->size(); ++i, ++obj) {
                obj->~T();
            }

            this->set_sp(this->get_storage_u());
        }
    };

    template<typename T, typename C>
    inline bool operator==(const StackedPool<T, C> &lhs, const StackedPool<T, C> &rhs) {
        return lhs.storage == rhs.storage;
    }

    template<typename T, typename C>
    inline bool operator!=(const StackedPool<T, C> &lhs, const StackedPool<T, C> &rhs) {
        return !(lhs == rhs);
    }

namespace drtx {

    /**
     * Iterator class for FixedPool. Simply iterates through the pool's array,
     * cast as type T.
     */
    template<typename T, typename C>
    struct PoolIterator {

        using pool_type = _stackPoolBase<T, C>;

        T *pool;
        size_t loc;

        PoolIterator() : pool(nullptr), loc(0) {}
        PoolIterator(T *p, size_t i) : pool(p), loc(i) {}

        /// Return reference to pool element at current index.
        T& operator*() {
            return pool[loc];
        }

        /// Prefix increment
        PoolIterator& operator++() {
            ++loc;
            return *this;
        }

        /// Postfix increment
        PoolIterator operator++(int) {
            PoolIterator temp(*this);
            ++loc;
            return temp;
        }

        /// Prefix decrement
        PoolIterator& operator--() {
            --loc;
            return *this;
        }

        /// Postfix decrement
        PoolIterator operator--(int) {
            PoolIterator temp(*this);
            --loc;
            return temp;
        }

        bool operator==(const PoolIterator<T, C> &other) {
            // comparing pool triggers valgrind on empty allocator
            return pool == other.pool && loc == other.loc;
        }

        bool operator!=(const PoolIterator<T, C> &other) {
            return !(*this == other);
        }
    };

} // namespace drtx
} // namespace drt

#endif //FYP_MAPS_POOLS_HPP
