
#ifndef FYP_TEST_UTIL_HPP
#define FYP_TEST_UTIL_HPP

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

    struct five_count {
        enum { value = 5 };
    };

    template<typename T>
    struct TestFoo {

        T* my_pointer;

        TestFoo() {
            my_pointer = new T();
        }

        ~TestFoo() { delete my_pointer; }

        TestFoo(TestFoo &&other) {
            my_pointer = other.my_pointer;
            other.my_pointer = nullptr;
        }

        TestFoo& operator=(TestFoo &&other) {
            if (this != &other) {
                delete my_pointer;
                my_pointer = other.my_pointer;
                other.my_pointer = nullptr;
            }
        }

        bool operator==(const TestFoo &other) {
            return my_pointer == other.my_pointer;
        }

        bool operator!=(const TestFoo &other) {
            return this != &other;
        }
    };

} // namespace drt

#endif //FYP_TEST_UTIL_HPP
