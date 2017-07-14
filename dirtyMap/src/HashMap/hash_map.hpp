
#ifndef FYP_MAPS_HASH_MAP_HPP
#define FYP_MAPS_HASH_MAP_HPP

namespace drt {

    /**
     * Hash map implementation that conserves memory and uses our custom memory
     * pool.
     *
     * @tparam Key  Type of key objects.
     * @tparam Val  Type of mapped objects.
     * @tparam Hash Type of hash function used for value lookups.
     */
    template<class Key, class Val, class Hash = std::hash<Key>>
    class Hashmap {

        using hash_table = drtx::hashtable<Key, std::pair<const Key, Val>, Hash, drtx::select_first>;
        hash_table ht;

    public:
        using key_type        =  Key;
        using mapped_type     =  Val;
        using value_type      =  typename hash_table::value_type;
        using iterator        =  typename hash_table::iterator;

        // constructors & destructor

        Hashmap() : ht() { }

        Hashmap(size_t n, const Hash &hf = Hash()) : ht(n, hf) { }

        ~Hashmap() = default;
        Hashmap(const Hashmap&) = default;
        Hashmap& operator=(const Hashmap&) = default;
        Hashmap(Hashmap&&) = default;
        Hashmap& operator=(Hashmap&&) = default;

        // size & capacity

        /// Returns the number of elements in the map.
        size_t size() const noexcept {
            return ht.size();
        }

        /// Returns maximum number of elements that can be stored.
        size_t max_size() const noexcept {
            return ht.max_size();
        }

        /// Returns the number of buckets in the map.
        size_t bucket_count() const noexcept {
            return ht.bucket_count();
        }

        /// Returns true if there are no elements in the map.
        bool empty() const noexcept {
            return ht.empty();
        }

        // modifiers

        /**
         * Removes all elements from the map. Does not change the number
         * of buckets.
         */
        void clear() {
            ht.clear();
        }

        /**
         * Removes and destroys the element corresponding to key k.
         *
         * @param k Key of the element to be removed.
         * @return  The number of elements that were removed (0 or 1).
         */
        size_t erase(const Key &k) {
            return ht.erase(k);
        }

        // insert/emplace methods?

        // lookup

        /**
         * @brief @c [] access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k.
         */
        mapped_type& operator[](const Key &k) {
            return ht[k];
        }

        /**
         * @brief @c [] access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k.
         */
        mapped_type& operator[](Key &&k) {
            return ht[std::move(k)];
        }

        /**
         * @brief Access to map elements.
         * @param k The key for which a mapped value should be returned.
         * @return A reference to the value associated with k, if it exists.
         *
         * Return the value mapped to the provided key. If it doesn't exist in
         * the map, throw an out_of_range error.
         */
        mapped_type& at(const Key &k) {
            return ht.at(k);
        }

        /**
         *
         * @param k The key for which to count elements.
         * @return The number of elements with the provided key (1 or 0).
         *
         * As there can be no duplicate key, this will only return 1 or 0.
         */
        size_t count(const Key &k) const {
            return ht.count(k);
        }

        // rehashing

        /// Returns maximum ratio of elements to buckets.
        float max_load_factor() const noexcept {
            return ht.max_load_factor();
        }

        /// Setter for the maximum load factor.
        void max_load_factor(float f) {
            ht.load_factor(f);
        }

        /// Returns the current ratio of elements to buckets.
        float load_factor() const noexcept {
            return ht.load_factor();
        }

        /**
         * Increases the number of buckets used to store elements and reassigns
         * all currently mapped elements to the proper bucket based on the new
         * size.
         *
         * @param new_size The new number of buckets to use.
         */
        void rehash(size_t new_size) {
            ht.rehash(new_size);
        }

        // iterators

        iterator begin() {
            return ht.begin();
        }

        iterator end() {
            return ht.end();
        }
    };

} // namespace drt

#endif //FYP_MAPS_HASH_MAP_HPP
