#include "gtest/gtest.h"
#include "test_utils.hpp"
#include "dirtyMap/Allocator.hpp"

using namespace drt;

class AllocTest : public ::testing::Test {

protected:

    virtual void SetUp() {
        // fill one pool
        for (int i = 0; i < 5; ++i) {
            void* ptr = alloc.allocate();
            v.push_back(static_cast<int*>(ptr));
            new(ptr) int(i);
        }
    }

    DtPoolAllocator<int, five_count> alloc;
    std::vector<int*> v;
};

void addElements(int s, int e, DtPoolAllocator<int, five_count> &a, std::vector<int*> &v) {
    for (int i = s; i < e; ++i) {
        void* ptr = a.allocate();
        v.push_back(static_cast<int*>(ptr));
        new(ptr) int(i);
    }
}

// first three tests are really just more tests of the poool
TEST_F(AllocTest, dealloc1) {
    ASSERT_EQ(1, *(v[1]));
    alloc.deallocate(v[1]);
    ASSERT_EQ(4, *(v[1]));
}

TEST_F(AllocTest, destroy1) {
    auto p = alloc.destroy(v[2]);
    ASSERT_EQ(v[4], static_cast<int*>(p));
    ASSERT_EQ(4, *(v[2]));
}

TEST_F(AllocTest, destroy2) {
    // remove all but one
    for (int i = 0; i < 4; ++i) {
        alloc.destroy(v[i]);
    }

    auto p = alloc.destroy(v[4]);
    ASSERT_EQ(nullptr, p);
}

TEST_F(AllocTest, iterators1) {
    addElements(11, 14, alloc, v);

    auto it = alloc.begin();
    ASSERT_EQ(11, *it);
}

TEST_F(AllocTest, iterators2) {
    addElements(11, 14, alloc, v);

    auto it = alloc.begin();
    ++it;
    ++it;
    ASSERT_EQ(13, *it);
    ++it;
    ASSERT_EQ(0, *it);
    it.deallocate(v[0]);
    ASSERT_EQ(4, *it);
}

TEST_F(AllocTest, findTest) {
    addElements(11, 14, alloc, v);
    auto it = alloc.find(v[3]);
    ASSERT_EQ(3, *it);
    ++it;
    ++it;
    auto e = alloc.end();
    ASSERT_TRUE(it == alloc.end());
}

