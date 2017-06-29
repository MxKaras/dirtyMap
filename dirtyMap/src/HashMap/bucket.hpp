
#ifndef FYP_MAPS_BUCKET_HPP
#define FYP_MAPS_BUCKET_HPP

namespace drt {

    template<typename Key, typename Val, class Hash = std::hash<Key>>
    class Hashmap;

namespace drtx {

    template<typename Val>
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

    /**
     * Container for nodes in the Hashmap.
     *
     * @tparam Key Type of key object.
     * @tparam Val Type of mapped objects.
     */
    template<typename Key, typename Val, class Hash>
    struct Bucket {

        using value_type  =  typename Hashmap<Key, Val, Hash>::value_type;
        using bNode       =  _bNode<value_type>;
        using iterator    =  BucketIterator<value_type>;
        // misc return type alias for brevity
        using bool_ptr    =  std::pair<bool, bNode*>;

        void *head = nullptr;

        Bucket() = default;
        ~Bucket() = default;

        /**
         * Searches for an element with key = k.
         *
         * @param  k The key to search for.
         * @return A value_type, if the key is matched; otherwise a nullptr.
         */
        value_type* search(const Key &k) const {
            iterator it = begin();

            while (it.current) {
                value_type *element = it.current_element();

                if (element->first == k) {
                    return element;
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
            head = flag(element, 1);
        }

        /**
         * Inserts a node at the beginning of the list. Should not be used on
         * an empty bucket.
         *
         * @param node Pointer to a node.
         */
        void insert_node(bNode *node) {
            node->next = head;
            head = flag(node, 3);
        }

        /**
         * Removes a node from the list. Note that this doesn't destroy the
         * object; that is taken care of in Hashmap. The contents of the
         * return type inform Hashmap on what to do next.
         */
        bool_ptr remove_node(void *to_remove) {
            if (flag(head) == 1) {
                head = nullptr;
                return bool_ptr(true, nullptr);
            } else if (clean(head) == reinterpret_cast<uintptr_t>(to_remove)) {
                bNode *r = reinterpret_cast<bNode*>(to_remove);
                head = r->next;
                r->next = nullptr;
                return bool_ptr(false, nullptr);
            }

            // element to remove is somewhere past the first node
            bNode *b = node_before(to_remove);
            // to_remove might be the element at the end of the list
            if (flag(b->next) == 1) {
                b->next = nullptr;

                if (clean(head) == reinterpret_cast<uintptr_t>(b)) {
                    head = reinterpret_cast<void*>(clean(head) ^ 1);
                }

                // b needs to be moved from node -> element
                return bool_ptr(true, b);
            }

            // to_remove is a node
            bNode *node_to_remove = reinterpret_cast<bNode*>(to_remove);
            b->next = node_to_remove->next;
            node_to_remove->next = nullptr;

            return bool_ptr(false, nullptr);
        }

        /**
         * Changes an invalid "next" pointer to the correct one.
         */
        void update_element(void *old_addr, void *new_addr) {
            if (isSingle()) {
                head = flag(new_addr, 1);
            } else {
                bNode *b = node_before(old_addr);
                b->next = flag(new_addr, 1);
            }
        }

        /**
         * Changes an invalid "head"/"next" pointer to the correct one.
         */
        void update_node(void *old_addr, void *new_addr) {
            if (isHead(old_addr)) {
                head = flag(new_addr, 3);
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
        /// Strips dirty bits from address
        uintptr_t clean(void *ptr) const {
            return reinterpret_cast<uintptr_t>(ptr) & ~3;
        }

        /// Stores dirty bits in an address
        void* flag(void *ptr, unsigned int i) const {
            return reinterpret_cast<void*>(
                    reinterpret_cast<uintptr_t>(ptr) ^ i);
        }

        /// @return the value of some dirty bits
        uintptr_t flag(void *ptr) const {
            return reinterpret_cast<uintptr_t>(ptr) & 3;
        }

        /// Finds the bNode whose (cleaned) `next` is `ptr`.
        bNode* node_before(void *ptr) const {
            bNode *b = reinterpret_cast<bNode*>(clean(head));

            while (clean(b->next) != reinterpret_cast<uintptr_t>(ptr)) {
                b = reinterpret_cast<bNode*>(clean(b->next));
            }
            return b;
        }
    };

} // namespace drtx
} // namespace drt

#endif //FYP_MAPS_BUCKET_HPP
