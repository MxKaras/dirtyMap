
#ifndef FYP_MAPS_ITERATORS_HPP
#define FYP_MAPS_ITERATORS_HPP

#include <utility>

namespace fyp {
namespace fypx {

    /**
     * Iterator class that traverses up the list of elements stored in a bucket.
     */
    template<typename Key, typename Val, class Hash, class Alloc>
    class BucketIterator {
    private:
        using bucket       = Bucket<Key, Val, Hash, Alloc>;
        using node         = typename bucket::BNode;
        using value_type   = typename bucket::value_type;

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
    template<typename Key, typename Val, class Hash, class Alloc>
    class HashMapIterator {
    private:
        using map_type     = fyp::Hashmap<Key, Val, Hash, Alloc>;
        using bucket       = typename map_type::bucket_type;
        using value_type = typename map_type::value_type;
        using v_iterator   = typename map_type::v_iterator;
        using b_iterator   = typename bucket::iterator;

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
            typename map_type::iterator temp(index, end);
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

        bool operator==(const HashMapIterator<Key, Val, Hash, Alloc> &other) const {
            return (index == other.index) && (bit == other.bit);
        }

        bool operator!=(const HashMapIterator<Key, Val, Hash, Alloc> &other) const {
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

    /**
     * Identical to HashMapIterator except for the fact that buckets are
     * destroyed after we move on from them. This is used in the default
     * version of Hashmap when rehashing.
     */
    template<typename Key, typename Val, class Hash, class Alloc>
    class DestructiveIterator {
    private:
        using map_type     = fyp::Hashmap<Key, Val, Hash, Alloc>;
        using bucket       = typename map_type::bucket_type;
        using value_type = typename map_type::value_type;
        using v_iterator   = typename map_type::v_iterator;
        using b_iterator   = typename bucket::iterator;

        v_iterator index;
        v_iterator end;
        b_iterator bit;

    public:
        DestructiveIterator(v_iterator &b, v_iterator &e)
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
         * Destroys the current bucket before moving to the next one.
         */
        DestructiveIterator& operator++() {
            ++bit;

            if (!bit.current) {
                auto &b = *index;
                b.~bucket();
                shiftIndex();
            }
            return *this;
        }

        void* current() const {
            return bit.current;
        }

        bool operator==(const DestructiveIterator<Key, Val, Hash, Alloc> &other) const {
            return (index == other.index) && (bit == other.bit);
        }

        bool operator!=(const DestructiveIterator<Key, Val, Hash, Alloc> &other) const {
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

} // namespace fypx
} // namespace fyp
#endif //FYP_MAPS_ITERATORS_HPP
