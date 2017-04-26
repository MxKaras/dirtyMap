
#include <cstdint>
#include <iostream>
#include <functional>

#include "benchmark_utils.hpp"

#define MAP_DEFINED 1
#if STD
#include <unordered_map>
#elif BOOST
#include <boost/unordered_map.hpp>
#elif GOOGLE
#include <google/sparse_hash_map>
#elif FYP
#include "FYPMaps/HashMap.hpp"
#elif FYP_POOL
#include "FYPMaps/HashMap.hpp"
#endif

int main(int argc, char* argv[]) {

    size_t millions = 0;

    if (argc > 1) {
        float factor = std::strtof(argv[1], nullptr);
        if (factor <= 0.0) {
            std::cout << "USE A POSITIVE NUMBER!!!\n";
            return 0;
        }
        millions = (size_t) (1000000 * factor);
    } else {
        std::cout << "No arguments\n";
        return 0;
    }

    using _t = uint64_t;

#if STD
    std::string map_name = "std::unordered_map";
    using map_type = std::unordered_map<_t, _t>;
    map_type h;
#elif BOOST
    std::string map_name = "boost::unordered::unordered_map";
    using map_type = boost::unordered::unordered_map<_t, _t>;
    map_type h;
#elif GOOGLE
    std::string map_name = "google::sparse_hash_map";
    using map_type = google::sparse_hash_map<_t, _t>;
    map_type h;
#elif FYP
    std::string map_name = "fyp::Hashmap";
    using map_type = fyp::Hashmap<_t, _t>;
    map_type h;
#elif FYP_POOL
    std::string map_name = "fyp::Hashmap (pooled - " + std::to_string(POOL_SIZE) + ")";
    using map_type = fyp::Hashmap<_t, _t, std::hash<_t>, fyp::MyPoolAllocator<_t, _t, POOL_SIZE>>;
    map_type h;
#else
#undef MAP_DEFINED
    std::cout << "No class defined (see file)\n";
    return 0;
#endif

#if MAP_DEFINED
    fyp_testing::RandomSearchTest<_t, map_type> _test(h, millions, map_name);
    fyp_testing::run_time_test(_test);

    return 0;
#endif
}