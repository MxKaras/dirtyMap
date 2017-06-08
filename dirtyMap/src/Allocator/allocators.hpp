
#ifndef FYP_MAPS_ALLOCATORS_HPP
#define FYP_MAPS_ALLOCATORS_HPP

#include <vector>

namespace drt {

namespace drtx {

    template<typename T, typename obj_count>
    struct DtIterator;
}

    template<typename T, typename obj_count = drtx::buddy_mb_count<T>>
    class DtPoolAllocator {

        using pool_type  = StackedPool<T, obj_count>;
        using iterator   = drtx::DtIterator<T, obj_count>;
        using v_iterator = typename std::vector<pool_type>::iterator;

        friend class drtx::DtIterator<T, obj_count>;

        std::vector<pool_type> pools;

    public:
        using value_type = T;

        DtPoolAllocator() : pools() {
            pools.reserve(16);
        }

        ~DtPoolAllocator() = default;

        void* allocate() {
            // get block if available
            void* ptr = try_to_allocate();
            if (ptr) return ptr;

            // need to create new pool
            pools.emplace_back();
            switch_first();
            return (pools.front()).allocate();
        }

        void deallocate(void* ptr) {
            for (pool_type &s : pools) {
                if (s.owns(ptr)) {
                    s.deallocate(ptr);
                    return;
                }
            }
        }

        void* destroy(void* ptr) {
            for (pool_type &s : pools) {
                if (s.owns(ptr)) {
                    return s.destroy(ptr);
                }
            }
            return nullptr;
        }

        void destroyAll() {
            pools.clear();
            pools.shrink_to_fit();
            pools.reserve(16);
        }

        iterator begin() {
            auto it = pools.begin();
            return iterator(it, it->begin());
        }

        iterator end() {
            auto it = pools.end();
            return iterator(it, it->begin());
        }

        // needed?
        iterator find(void *ptr) {
            int i = 0;

            for (pool_type &s : pools) {
                if (s.owns(ptr)) {
                    auto it = pools.begin() + i;
                    return iterator(it, it->find(ptr));
                }
                ++i;
            }
        }

    private:
        void* try_to_allocate() {
            void* ptr = nullptr;

            for (pool_type &s : pools) {
                ptr = s.allocate();
                if (ptr) return ptr;
            }

            return ptr;
        }

        void switch_first() {
            auto tmp = std::move(pools.front());
            pools.front() = std::move(pools.back());
            pools.back() = std::move(tmp);
        }
    };

namespace drtx {

    template<typename T, typename C>
    struct DtIterator {

        using value_type = T;
        using alloc_type = DtPoolAllocator<T, C>;
        using v_iterator = typename alloc_type::v_iterator;
        using p_iterator = typename alloc_type::pool_type::iterator;

//        DtPoolAllocator<T, C> *alloc;
        v_iterator vit;
        p_iterator it;

        DtIterator(v_iterator v, p_iterator i)
                : vit(v), it(i) { }

        ~DtIterator() = default;

        /**
         * @brief Shortcut for deallocating an element when iterating through pools.
         *
         * If we want to deallocate an element obtained through an iterator,
         * doing so here is much faster than iterating through the whole list.
         */
        void deallocate(void *ptr) {
            vit->deallocate(ptr);
        }

        /// @return reference to current object in current pool.
        value_type& operator*() {
            return *it;
        }

        /**
         * Increments position in current pool, moving to the beginning of the
         * next pool if necessary.
         */
        DtIterator& operator++() {
            ++it;

            if (it == vit->end()) {
                ++vit;
                it = vit->begin();
            }

            return *this;
        }

        /**
         * Decrements position in current pool, but does not move backwards
         * through nodes. For this reason, calls to operator-- should always be
         * followed up with a call to operator++ before dereferencing again.
         */
        DtIterator& operator--() {
            /* NOTE: this doesn't move backwards through nodes!
            This functionality is here so that we can revisit
            a pool location after moving objects -- NOT for
            reverse traversal! */
            --it;
            return *this;
        }

        // comparing iterator will compare the allocator as well
        bool operator==(const DtIterator<T, C> &other) {
            return it == other.it;
        }

        bool operator!=(const DtIterator<T, C> &other) {
            return !(*this == other);
        }
    };

} // namespace drtx

} // namespace drt

#endif // FYP_MAPS_ALLOCATORS_HPP
