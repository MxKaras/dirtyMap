
#ifndef FYP_MAPS_ALLOCATORS_HPP
#define FYP_MAPS_ALLOCATORS_HPP

namespace fyp {

namespace fypx {
    template<class Allocator>
    struct CascadeIterator;
}

    /**
     * Allocator that defers to one of two other allocator objects, allowing
     * objects of different sizes to be allocated through one object.
     *
     * The value of "threshold" should be sizeof(T) for type T that is held by
     * the "small" allocator SAllocator.
     *
     * @tparam threshold  Size of object held by smaller allocator.
     * @tparam SAllocator "Small" allocator class for first type.
     * @tparam BAllocator "Big" allocator class for second type.
     */
    template<size_t threshold, typename SAllocator, typename BAllocator>
    class Segregator {

        using big_iter   = typename BAllocator::iterator;
        using small_iter = typename SAllocator::iterator;

        BAllocator b_alloc;
        SAllocator s_alloc;

    public:
        Segregator()
                : s_alloc(), b_alloc() {};

        /**
         * Based on the amount of memory being requested, return the result of
         * allocating to the big or small allocator.
         *
         * @param n Number of bytes being requested.
         * @return A pointer to an empty memory block.
         */
        void *allocate(size_t n) {
            if (n <= threshold) {
                return s_alloc.allocate();
            }

            return b_alloc.allocate();
        }

        /**
         * Finds the allocator that owns ptr, then deallocates it.
         *
         * Note that this method searches the small allocator before the
         * big one. If the size of the object pointed to by ptr is known,
         * use deallocate(void*, size_t) instead.
         */
        void deallocate(void *ptr) {
            if (s_alloc.owns(ptr)) {
                s_alloc.deallocate(ptr);
            } else {
                b_alloc.deallocate(ptr);
            }
        }

        /**
         * Deallocates ptr from big or small allocator, depending on n.
         */
        void deallocate(void *ptr, size_t n) {
            if (n <= threshold) {
                s_alloc.deallocate(ptr);
            } else {
                b_alloc.deallocate(ptr);
            }
        }

        void* destroy(void* ptr) {
            if (s_alloc.owns(ptr)) {
                return s_alloc.destroy(ptr);
            } else {
                return b_alloc.destroy(ptr);
            }
        }

        void* destroy(void* ptr, size_t n) {
            if (n <= threshold) {
                return s_alloc.destroy(ptr);
            } else {
                return b_alloc.destroy(ptr);
            }
        }

        void destroyAll() {
            s_alloc.destroyAll();
            b_alloc.destroyAll();
        }

        big_iter big_begin() { return b_alloc.begin(); }
        big_iter big_end() { return b_alloc.end(); }

        small_iter small_begin() { return s_alloc.begin(); }
        small_iter small_end() { return s_alloc.end(); }
    };

    /**
     * Allocator that takes an allocator/pool class with fixed size, and spawns
     * new ones when more space is needed. It builds up a linked list of
     * Allocator objects.
     *
     * @tparam Allocator Allocator class used to store objects.
     */
    template<typename Allocator>
    class CascadingAllocator {

        friend struct fypx::CascadeIterator<Allocator>;

        struct ANode {
            ANode *next = nullptr;
            Allocator allocator;

            ANode() : allocator() {};

            ~ANode() { delete next; }

            bool operator==(const ANode &other) { return next == other.next; }
            bool operator!=(const ANode &other) { return !(*this == other); }
        };

        ANode *head;
        ANode *tail;

    public:
        using value_type = typename Allocator::value_type;
        using iterator   = fypx::CascadeIterator<Allocator>;

        CascadingAllocator() : head(), tail() { }

        ~CascadingAllocator() { delete head; }

        CascadingAllocator(const CascadingAllocator&) = default;
        CascadingAllocator& operator=(const CascadingAllocator&) = default;

        CascadingAllocator(CascadingAllocator &&other) noexcept
            : head(other.head), tail(other.tail) {
            other.head = nullptr;
            other.tail = nullptr;
        }

        CascadingAllocator& operator=(CascadingAllocator &&other) noexcept {
            if (this != &other) {
                delete head;
                head = other.head;
                tail = other.tail;
                other.head = nullptr;
                other.tail = nullptr;
            }
            return *this;
        }

        /**
         * Finds a pointer to an available block of memory. If needed, a new
         * allocator node will be created.
         */
        void* allocate() {
            // get block if available
            void* ptr = try_to_allocate();
            if (ptr) return ptr;

            // need to create a new node
            ANode *node = new ANode();
            node->next = head;
            if (!head) tail = node;
            head = node;

            return node->allocator.allocate();
        }

        /**
         * Finds the allocator node that owns ptr, then deallocates it.
         */
        void deallocate(void *ptr) {
            auto node = head;

            while (node) {
                if (node->allocator.owns(ptr)) {
                    node->allocator.deallocate(ptr);
                    return;
                }
                node = node->next;
            }
        }

        void* destroy(void *ptr) {
            auto node = head;

            while (node) {
                if (node->allocator.owns(ptr)){
                    return node->allocator.destroy(ptr);
                }
                node = node->next;
            }
            return nullptr;
        }

        /// Resets allocator to "new" state.
        void destroyAll() {
            delete head;
            head = nullptr;
            tail = nullptr;
        }

        iterator begin() {
            return iterator(head, head->allocator.begin());
        }

        iterator end() {
            // avoid a segfault from tail->allocator.end() when tail is null.
            if (tail) return iterator(tail, tail->allocator.end());

            // this works even is head is null.
            return begin();
        }

    private:
        /**
         * Traverses through allocator node list, trying to get a memory block
         * from an allocator with available space.
         *
         * @return nullptr if no space is found.
         */
        void* try_to_allocate() {
            auto node = head;
            void *ptr = nullptr;

            while (node) {
                ptr = node->allocator.allocate();
                if (ptr) return ptr;
                node = node->next;
            }

            return ptr;
        }
    };

namespace fypx {

    template<class Allocator>
    struct CascadeIterator {

        using value_type = typename CascadingAllocator<Allocator>::value_type;
        using node_type  = typename CascadingAllocator<Allocator>::ANode;
        using iterator   = typename Allocator::iterator;

        // current node, holding allocator
        node_type *node;
        // current spot in array held by current node
        iterator it;

        CascadeIterator(node_type *n, iterator i) : node(n), it(i) {};

        /**
         * @brief Shortcut for deallocating an element when iterating through pools.
         *
         * If we want to deallocate an element obtained through an iterator,
         * doing so here is much faster than iterating through the whole list.
         */
        void deallocate(void *ptr) {
            node->allocator.deallocate(ptr);
        }

        /// @return reference to current object in current pool.
        value_type& operator*() {
            return *it;
        }

        /**
         * Increments position in current pool, moving to the beginning of the
         * next pool if necessary.
         */
        CascadeIterator& operator++() {
            ++it;

            if (it == node->allocator.end()) {
                node = node->next;
                if (node) it = node->allocator.begin();
            }

            return *this;
        }

        /**
         * Decrements position in current pool, but does not move backwards
         * through nodes. For this reason, calls to operator-- should always be
         * followed up with a call to operator++ before dereferencing again.
         */
        CascadeIterator& operator--() {
            /* NOTE: this doesn't move backwards through nodes!
            This functionality is here so that we can revisit
            a pool location after moving objects -- NOT for
            reverse traversal! */
            --it;
            return *this;
        }

        // comparing iterator will compare the allocator as well
        bool operator==(const CascadeIterator<Allocator> &other) {
            return it == other.it;
        }

        bool operator!=(const CascadeIterator<Allocator> &other) {
            return !(*this == other);
        }
    };
} // namespace fypx

} // namespace fyp

#endif // FYP_MAPS_ALLOCATORS_HPP
