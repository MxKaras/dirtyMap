
#ifndef FYP_MAPS_ITERATORS_HPP
#define FYP_MAPS_ITERATORS_HPP

namespace drt {
namespace drtx {

    /**
     * Iterator class that traverses up the list of elements stored in a bucket.
     */
    template<typename Val>
    class BucketIterator {
    private:
        using value_type = Val;
        using node       = _bNode<value_type>;

    public:
        void *current;

        BucketIterator() {
            current = nullptr;
        }

        BucketIterator(void *bucket_head) : current(bucket_head) { }

        /// Return a reference to the current element.
        value_type& operator*() {
            return *current_element();
        }

        /// Return a pointer to the current element.
        value_type* operator->() {
            return current_element();
        }

        /**
         * Increment the pointer, setting it to null if we are at the end of
         * the list.
         */
        BucketIterator& operator++() {
            auto p = reinterpret_cast<uintptr_t>(current);
            if ((p & 3) == 1) {
                current = nullptr;
            } else {
                node *n = reinterpret_cast<node*>(p & ~3);
                current = n->next;
            }
            return *this;
        }

        value_type* current_element() const noexcept {
            return reinterpret_cast<value_type*>(
                    reinterpret_cast<uintptr_t>(current) & ~3);
        }

        bool operator==(const BucketIterator &other) const {
            return current == other.current;
        }

        bool operator!=(const BucketIterator &other) const {
            return !(*this == other);
        }
    };

    /**
     * Iterator class that traverses through stored elements in the map.
     *
     * As the map is a combination of a vector and linked lists, we must
     * travel up the vector until hitting upon an element, then travel up
     * the list at that location until reaching the end.
     */
    template<typename Val, typename B, typename Vit>
    class HashMapIterator {
    private:
        using bucket      = B;
        using value_type  = Val;
        using v_iterator  = Vit;
        using b_iterator  = BucketIterator<value_type>;

        v_iterator index;
        v_iterator end;
        b_iterator bit;

    public:
        HashMapIterator(v_iterator &b, v_iterator &e)
                : index(b), end(e), bit() {
            // Move pointer to first location in the vector that
            // contains an element.
            shiftIndex();
        }

        value_type& operator*() {
            return bit.operator*();
        }

        value_type* operator->() {
            return bit.operator->();
        }

        /**
         * Increases the bucket iterator. If it is at the end of its list, find
         * the next non-empty bucket.
         */
        HashMapIterator& operator++() {
            ++bit;

            if (!bit.current) {
                ++index;
                shiftIndex();
            }
            return *this;
        }

        /**
         * Increases the bucket iterator. If it is at the end of its list, find
         * the next non-empty bucket.
         */
        HashMapIterator& operator++(int) {
            HashMapIterator temp(index, end);
            ++bit;

            if (!bit.current) {
                ++index;
                shiftIndex();
            }
            return temp;
        }

        void* current() const {
            return bit.current;
        }

        bool operator==(const HashMapIterator<Val, B, Vit> &other) const {
            return (index == other.index) && (bit == other.bit);
        }

        bool operator!=(const HashMapIterator<Val, B, Vit> &other) const {
            return !(*this == other);
        }

    private:
        void shiftIndex() {
            while (index != end && index->isEmpty()) {
                ++index;
            }

            // Prevent a BucketIterator from being created that points
            // at an invalid memory address (valgrind finds this error).
            if (index != end) {
                bit = b_iterator(index->head);
            }
        }
    };

} // namespace drtx
} // namespace drt
#endif //FYP_MAPS_ITERATORS_HPP
