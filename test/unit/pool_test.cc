#include "gtest/gtest.h"
#include "test_utils.hpp"
#include "dirtyMap/Allocator.hpp"

using namespace drt;

class StackedPoolTest : public ::testing::Test {

protected:

    virtual void SetUp() {
        for (int i = 0; i < 3; ++i) {
            new(pool.allocate()) int(i);
            new(full_pool.allocate()) int(i);
            new(foo_pool.allocate()) TestFoo<int>();
        }

        for (int i = 3; i < 5; ++i) {
            new(full_pool.allocate()) int(i);
        }
    }

    StackedPool<int, five_count> empty_pool;
    StackedPool<int, five_count> pool;
    StackedPool<int, five_count> full_pool;
    StackedPool<TestFoo<int>, five_count> foo_pool;
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

TEST_F(StackedPoolTest, deallocate_test) {
    pool.deallocate(&(*pool.begin()));
    // we removed the first element - last one should have
    // been moved into its slot
    ASSERT_EQ(2, *pool.begin());
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

