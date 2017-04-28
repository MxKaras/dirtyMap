
#ifndef FYP_MAPS_BENCHMARK_UTILS_HPP
#define FYP_MAPS_BENCHMARK_UTILS_HPP

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace drt_testing {

    using std::string;

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *            HELPER FUNCTIONS
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     */

    // adapted from code provided by Dr A Coles
    uint64_t current_process_vm() {
        std::ifstream f("/proc/self/status");
        string next;

        while (f.good()) {
            f >> next;

            if (next == "VmSize:") {
                f >> next;
                uint64_t vm_size_kb;
                std::istringstream s(next);
                s >> vm_size_kb;

//            f >> next;
//            std::cout << next << std::endl;
                return vm_size_kb * 1024;
            }
        }

        return 0;
    }

    float to_mb(uint64_t kb) {
        return (float) ((double) kb / (1024 * 1024));
    }

    template<class T>
    void fill_vector(std::vector<T> &v) {
        std::seed_seq seq{314159, 271828, 14142135, 16180339};

        if (sizeof(T) == 8) {
            std::mt19937_64 rng(seq);

            for (size_t i = 0; i < v.capacity(); ++i) {
                v.push_back(rng());
            }
        } else {
            std::mt19937 rng(seq);

            for (size_t i = 0; i < v.capacity(); ++i) {
                v.push_back(rng());
            }
        }

    }

    template<class T>
    void shuffle_vector(std::vector<T> &v) {
        std::random_shuffle(v.begin(), v.end());
    }

    template<class T, class HMap>
    void fill_map(std::vector<T> &v, HMap &h) {
        size_t size = v.size();

        for (size_t i = 0; i < size; ++i) {
            h[v[i]] = 42;
        }
    };

    template<class T, class HMap>
    void search_map(std::vector<T> &v, HMap &h) {
        size_t size = v.size();

        for (size_t i = 0; i < size; ++i) {
            h.count(v[i]);
        }
    };

    template<class T, class HMap>
    void erase_map(std::vector<T> &v, HMap &h) {
        size_t size = v.size();

        for (size_t i = 0; i < size; ++i) {
            h.erase(v[i]);
        }
    };

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *              TEST CASES
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     */

    struct tbase {
        size_t num;
        string tname;
        string mname;

        tbase(size_t _n, string _t, string _m) : num(_n), tname(_t), mname(_m) { }

        virtual void run() {}
    };

    template<class T, class HMap>
    struct RandomInsertTest : tbase {
        std::vector<T> v;
        HMap &h;

        RandomInsertTest(HMap &_h, size_t _n, string _m)
                : tbase(_n, "RandomInsertTest", _m), h(_h) {

            v.reserve(num);
            fill_vector<T>(v);
        }

        void run() {
            fill_map<T, HMap>(v, h);
        }
    };

    template<class T, class HMap>
    struct SequentialInsertTest : tbase {
        HMap &h;

        SequentialInsertTest(HMap &_h, size_t _n, string _m)
                : tbase(_n, "SequentialInsertTest", _m), h(_h) {}

        void run() {
            T lim = (T) num;
            for (T i = 0; i < lim; ++i) {
                h[i] = 0;
            }
        }
    };

    template<class T, class HMap>
    struct RandomSearchTest : tbase {
        std::vector<T> v;
        HMap &h;

        RandomSearchTest(HMap &_h, size_t _n, string _m)
                : tbase(_n, "RandomSearchTest", _m), h(_h) {

            v.reserve(num);
            fill_vector<T>(v);
            fill_map<T, HMap>(v, h);
            shuffle_vector<T>(v);
        }

        void run() {
            search_map(v, h);
        }
    };

    template<class T, class HMap>
    struct SequentialSearchTest : tbase {
        std::vector<T> v;
        HMap &h;

        SequentialSearchTest(HMap &_h, size_t _n, string _m)
                : tbase(_n, "SequentialSearchTest", _m), h(_h) {

            v.reserve(num);
            fill_vector(v);
            fill_map(v, h);
        }

        void run() {
            search_map(v, h);
        }
    };

    template<class T, class HMap>
    struct RandomEraseTest : tbase {
        std::vector<T> v;
        HMap &h;

        RandomEraseTest(HMap &_h, size_t _n, string _m)
                : tbase(_n, "EraseTest", _m), h(_h) {

            v.reserve(num);
            fill_vector(v);
            fill_map(v, h);
            shuffle_vector(v);
        }

        void run() {
            erase_map(v, h);
        }
    };

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *             TEST RUNNERS
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     */

    void _print_results(float _b, float _a, string _n, string _m, size_t _s) {
        int i = printf("| %s [%lu] with %s |\n", _n.c_str(), _s, _m.c_str());
        printf("%*c%-9s %7.2f MB %*c\n", -(i / 4), '|', "Before:", _b, i - (i / 4) - 22, '|');
        printf("%*c%-9s %7.2f MB %*c\n", -(i / 4), '|', "After:", _a, i - (i / 4) - 22, '|');
        printf("%*c%-9s %7.2f MB %*c\n", -(i / 4), '|', "Gain:", _a - _b, i - (i / 4) - 22, '|');
        printf("|");
        for (int j = 0; j < i - 3; ++j) printf("#");
        printf("|\n");
    }

    void _print_results(double _dur, string _n, string _m, size_t _s) {
        int i = printf("| %s [%lu] with %s |\n", _n.c_str(), _s, _m.c_str());
        printf("%*c%9s %.6lf s %*c\n", -(i/3), '|', "Duration:", _dur/1000.0, i-(i/3)-22, '|');
        printf("|");
        for (int j = 0; j < i - 3; ++j) printf("#");
        printf("|\n");
    }

    void run_memory_test(tbase &_test) {
        float mem_before = to_mb(current_process_vm());
        _test.run();
        float mem_after = to_mb(current_process_vm());
        _print_results(mem_before, mem_after, _test.tname, _test.mname, _test.num);
    }

    void run_time_test(tbase &_test) {
        auto _start = std::chrono::steady_clock::now();
        _test.run();
        auto _diff = std::chrono::steady_clock::now() - _start;
        auto _duration = std::chrono::duration<double, std::milli>(_diff).count();
        _print_results(_duration, _test.tname, _test.mname, _test.num);
    }

} // namespace fyp_testing

#endif //FYP_MAPS_BENCHMARK_UTILS_HPP
