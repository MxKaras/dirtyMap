
#ifndef FYP_MAPS_ALLOCATOR_HPP
#define FYP_MAPS_ALLOCATOR_HPP

#include "src/Allocator/pools.hpp"
#include "src/Allocator/allocators.hpp"

#ifdef FYP_MAPS_HASHMAP_HPP

namespace drt {
    /**
     * Memory allocator for use with Hashmap that is provided for convenience.
     * It sets up two expanding memory pools: one for elements and one for
     * list nodes. By 'expanding' we mean that the pool is a linked list of
     * pools, each storing `count` objects.
     *
     * @tparam K     Type of key objects.
     * @tparam V     Type of mapped objects.
     * @tparam count Number of objects to store in pool nodes.
     */
    template<class K, class V, size_t count = 4088 / sizeof(std::pair<K, V>)>
    class MyPoolAllocator
            : public Segregator<sizeof(std::pair<K, V>),
                CascadingAllocator<StackedPool<std::pair<const K, V>, count>>,
                CascadingAllocator<StackedPool<drtx::_bNode<std::pair<const K, V>>, count>>> {
    public:
        MyPoolAllocator() = default;
    };
}

#endif

#endif //FYP_MAPS_ALLOCATOR_HPP
