
#ifndef FYP_MAPS_UTIL_HPP
#define FYP_MAPS_UTIL_HPP

namespace drt {

    /**
     * Hash functor for tests.
     */
    template<typename T>
    class ZeroHF {

    public:
        size_t operator()(const T k) const {
            return 0;
        }
    };

} // namespace drt

#endif //FYP_MAPS_UTIL_HPP