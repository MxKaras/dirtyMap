
#ifndef FYP_MAPS_BUCKET_HPP
#define FYP_MAPS_BUCKET_HPP

namespace drt {

    template<typename Key, typename Val, class Hash, class Alloc>
    class Hashmap;

namespace drtx {

    template<typename Key, typename Val, class Hash, class Alloc>
    class BucketIterator;

    template<typename T>
    struct _bNode {
        /* Main holder of data in bucket list. Every node except the
               final one in the list will be a BNode. */

        // DO NOT REORDER THESE!
        T element;
        void *next = nullptr;

        _bNode() = default;
        _bNode(T &&e) noexcept : element(std::move(e)) {}
        ~_bNode() = default;
    };

    template<typename Key, typename Val, class Hash, class Alloc>
    struct _bucketBase {

        using value_type  =  typename Hashmap<Key, Val, Hash, Alloc>::value_type;
        using bNode         =  _bNode<value_type>;
        using iterator      =  BucketIterator<Key, Val, Hash, Alloc>;
        // misc return type alias for brevity
        using bool_ptr      =  std::pair<bool, bNode*>;

        void *head = nullptr;

        _bucketBase() = default;
        ~_bucketBase() = default;

        /**
         * Searches for an element with key = k.
         *
         * @param  k The key to search for.
         * @return A value_type, if the key is matched; otherwise a nullptr.
         */
        value_type* search(const Key &k) const {
            iterator it = begin();

            while (it.current) {
                if (it->first == k) {
                    return it.current_element();
                }
                ++it;
            }

            return nullptr;
        }

        /**
         * Inserts an element as the head of the list. Should only be used on
         * empty buckets.
         *
         * @param element Pointer to an element.
         */
        void insert_node(value_type *element) {
            // Assign head and LSB to signify single element
            uintptr_t p = reinterpret_cast<uintptr_t>(element);
            head = reinterpret_cast<void*>(p ^ 1);
        }

        /**
         * Inserts a node at the beginning of the list. Should not be used on
         * an empty bucket.
         *
         * @param node Pointer to a node.
         */
        void insert_node(bNode *node) {
            uintptr_t p = reinterpret_cast<uintptr_t>(head);
            if (isChained()) p = p & ~3;

            node->next = reinterpret_cast<void*>(p);
            head = node;
            p = reinterpret_cast<uintptr_t>(head);
            head = reinterpret_cast<void*>(p ^ 3);
        }

        /**
         * Removes a node from the list. Note that this doesn't destroy the
         * object; that is taken care of in Hashmap. The contents of the
         * return type inform Hashmap on what to do next.
         */
        bool_ptr remove_node(void *to_remove) {
            if (isSingle()) {
                head = nullptr;
                return bool_ptr(true, nullptr);
            }

            if (isHead(to_remove)) {
                remove_first(reinterpret_cast<bNode*>(to_remove));
                return bool_ptr(false, nullptr);
            }

            // element to remove is somewhere past the first node
            bNode *b = node_before(to_remove);
            // to_remove might be the element at the end of the list
            if (isTail(b->next)) {
                remove_tail(b);
                // b needs to be moved from node -> element
                return bool_ptr(true, b);
            }

            bNode *node_to_remove = reinterpret_cast<bNode*>(b->next);
            b->next = node_to_remove->next;
            node_to_remove->next = nullptr;

            return bool_ptr(false, nullptr);
        }

        /**
         * Changes an invalid "next" pointer to the correct one.
         */
        void update_element(void *old_addr, void *new_addr) {
            uintptr_t new_addr_p = reinterpret_cast<uintptr_t>(new_addr) ^ 1;

            if (isSingle()) {
                head = reinterpret_cast<void*>(new_addr_p);
            } else {
                bNode *b = node_before(old_addr);
                b->next = reinterpret_cast<void*>(new_addr_p);
            }
        }

        /**
         * Changes an invalid "head"/"next" pointer to the correct one.
         */
        void update_node(void *old_addr, void *new_addr) {
            if (isHead(old_addr)) {
                head = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(new_addr) ^ 3);
            } else {
                bNode *b = node_before(old_addr);
                b->next = new_addr;
            }
        }

        iterator begin() const {
            return iterator(head);
        }

        /// @return true if there are no nodes in this bucket.
        bool isEmpty() const noexcept {
            return head == nullptr;
        }

        /// @return true if there is exactly one node in this bucket.
        bool isSingle() const noexcept {
            return (reinterpret_cast<uintptr_t>(head) & 3) == 1;
        }

        /// @return true if there are at least two nodes in this bucket.
        bool isChained() const noexcept {
            return (reinterpret_cast<uintptr_t>(head) & 3) == 3;
        }

    protected:
        /// Return true if p points to an element.
        bool isTail(uintptr_t p) const noexcept {
            return ((p & 3) == 1);
        }

        /// Return true if ptr points to an element.
        bool isTail(void *ptr) const noexcept {
            return isTail(reinterpret_cast<uintptr_t>(ptr));
        }

        bool isHead(uintptr_t p) const noexcept {
            return (reinterpret_cast<uintptr_t>(head) & ~3) == p;
        }

        bool isHead(void *ptr) const noexcept {
            return isHead(reinterpret_cast<uintptr_t>(ptr));
        }

    private:
        /// Finds the bNode whose (cleaned) `next` is `ptr`.
        bNode* node_before(void *ptr) const {
            bNode *b = reinterpret_cast<bNode*>(reinterpret_cast<uintptr_t>(head) & ~3);

            while ((reinterpret_cast<uintptr_t>(b->next) & ~3) != reinterpret_cast<uintptr_t>(ptr)) {
                b = reinterpret_cast<bNode*>(reinterpret_cast<uintptr_t>(b->next) & ~3);
            }
            return b;
        }

        /// Removes the node pointed to by `head` (and updates head).
        void remove_first(bNode *to_remove) {
            if (isTail(to_remove->next)) {
                head = to_remove->next;
            } else {
                uintptr_t head_p = reinterpret_cast<uintptr_t>(to_remove->next);
                head = reinterpret_cast<void*>(head_p ^ 3);
            }
            to_remove->next = nullptr;
        }

        void remove_tail(bNode *penultimate) {
            penultimate->next = nullptr;
            uintptr_t p = reinterpret_cast<uintptr_t>(penultimate);
            if (isHead(p)) {
                head = reinterpret_cast<void*>(p ^ 1);
            }
        }
    };

    /**
     * Container for nodes in the Hashmap.
     *
     * @tparam Key Type of key object.
     * @tparam Val Type of mapped objects.
     */
    template<typename Key, typename Val, class Hash, class Alloc, bool no_destroy = true>
    struct Bucket;

    /// Bucket for pooled hashmap.
    template<typename Key, typename Val, class Hash, class Alloc>
    struct Bucket<Key, Val, Hash, Alloc, true>
            : public _bucketBase<Key, Val, Hash, Alloc> {

        using value_type  = typename _bucketBase<Key, Val, Hash, Alloc>::value_type;
        using iterator      = typename _bucketBase<Key, Val, Hash, Alloc>::iterator;
        using bool_ptr      = typename _bucketBase<Key, Val, Hash, Alloc>::bool_ptr;

        struct BNode : public _bNode<value_type> {
            using parent = _bNode<value_type>;

            BNode(value_type &&e) noexcept : parent(std::move(e)) { }
            BNode() : parent() { }
            ~BNode() = default;
            BNode(const BNode &) = default;
            BNode &operator=(const BNode &) = default;
            BNode(BNode &&) = default;
            BNode &operator=(BNode &&) = default;
        };

        Bucket() = default;
        ~Bucket() = default;
    };

    /// Bucket for non-pooled (i.e. default) hashmap
    template<typename Key, typename Val, class Hash, class Alloc>
    struct Bucket<Key, Val, Hash, Alloc, false>
            : public _bucketBase<Key, Val, Hash, Alloc> {

        using value_type  = typename _bucketBase<Key, Val, Hash, Alloc>::value_type;
        using iterator      = typename _bucketBase<Key, Val, Hash, Alloc>::iterator;
        using bool_ptr      = typename _bucketBase<Key, Val, Hash, Alloc>::bool_ptr;

        struct BNode : public _bNode<value_type> {
            using parent = _bNode<value_type>;

            BNode(value_type &&e) noexcept : parent(std::move(e)) { }
            BNode() : parent() { }

            ~BNode() {
                if (this->next) {
                    uintptr_t p = reinterpret_cast<uintptr_t>(this->next);

                    if ((p & 3) == 1) {
                        delete reinterpret_cast<value_type*>(p & ~1);
                    } else {
                        delete reinterpret_cast<BNode*>(p);
                    }
                }
            }

            BNode(const BNode &) = default;
            BNode &operator=(const BNode &) = default;
            BNode(BNode &&) = default;
            BNode &operator=(BNode &&) = default;
        };

        Bucket() = default;

        ~Bucket() {
            if (this->head) {
                uintptr_t p = reinterpret_cast<uintptr_t>(this->head);
                this->head = nullptr;

                if (this->isTail(p)) {
                    delete reinterpret_cast<value_type*>(p & ~1);
                } else {
                    delete reinterpret_cast<BNode*>(p & ~3);
                }
            }
        }
    };

} // namespace drtx
} // namespace drt

#endif //FYP_MAPS_BUCKET_HPP
