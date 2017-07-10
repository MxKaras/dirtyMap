#include "gtest/gtest.h"
#include "test_utils.hpp"
#include "dirtyMap/Allocator.hpp"

using namespace drt;

class StackedPoolTest : public ::testing::Test {

protected:

    virtual void SetUp() {
        v.reserve(5);

        for (int i = 0; i < 3; ++i) {
            new(pool.allocate()) int(i);
            auto p = full_pool.allocate();
            new(p) int(i);
            v.push_back(static_cast<int*>(p));
            new(foo_pool.allocate()) TestFoo<int>();
        }

        for (int i = 3; i < 5; ++i) {
            auto p = full_pool.allocate();
            new(p) int(i);
            v.push_back(static_cast<int*>(p));
        }
    }

    StackedPool<int, five_count> empty_pool;
    StackedPool<int, five_count> pool;
    StackedPool<int, five_count> full_pool;
    StackedPool<TestFoo<int>, five_count> foo_pool;
    std::vector<int*> v;
};

TEST_F(StackedPoolTest, empty_capacity) {
    ASSERT_EQ(5, empty_pool.capacity());
    ASSERT_EQ(5 * sizeof(int), empty_pool.capacity_bytes());
    ASSERT_EQ(0, empty_pool.size());
    ASSERT_FALSE(empty_pool.full());
    ASSERT_TRUE(empty_pool.empty());
}

TEST_F(StackedPoolTest, capacity) {
    ASSERT_EQ(3, pool.size());
    ASSERT_FALSE(pool.full());
    ASSERT_FALSE(pool.empty());
}

TEST_F(StackedPoolTest, full_capacity) {
    ASSERT_EQ(5, full_pool.size());
    ASSERT_TRUE(full_pool.full());
    ASSERT_FALSE(full_pool.empty());
}

TEST_F(StackedPoolTest, iterator_test) {
    auto _start = pool.begin();
    auto _end = pool.end();

    int i = 0;

    for (; _start != _end; ++_start, ++i) {
        ASSERT_EQ(i, *_start);
    }

    ASSERT_TRUE(_start == _end);
}

TEST_F(StackedPoolTest, iterator_test2) {
    auto start = full_pool.begin();
    ++start;
    ++start;
    ++start;
    ++start;
    ++start;
    ASSERT_TRUE(start == full_pool.end());
    --start;
    --start;
    --start;
    --start;
    --start;
    ASSERT_TRUE(start == full_pool.begin());
}

TEST_F(StackedPoolTest, iteratorTest3) {
    auto it = pool.begin();
    ASSERT_EQ(0, *it);
    auto x = it++;
    ASSERT_EQ(0, *x);
    ASSERT_EQ(1, *it);
}

TEST_F(StackedPoolTest, deallocate_test) {
    pool.deallocate(&(*pool.begin()));
    // we removed the first element - last one should have
    // been moved into its slot
    ASSERT_EQ(2, *pool.begin());
}

TEST_F(StackedPoolTest, findTest) {
    auto it = pool.find(v[2]);
    ASSERT_EQ(2, *it);
    ++it;
    ASSERT_EQ(3, *it);
}

// the following tests are really for Valgrind
TEST_F(StackedPoolTest, fooDestroyAll) {
    foo_pool.destroyAll();
    ASSERT_TRUE(foo_pool.empty());
}

TEST_F(StackedPoolTest, fooDestroy) {
    foo_pool.destroy(&(*foo_pool.begin()));
    ASSERT_EQ(2, foo_pool.size());
}

