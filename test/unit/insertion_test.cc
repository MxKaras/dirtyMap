#include "gtest/gtest.h"
#include "test_utils.hpp"
#include "dirtyMap/HashMap.hpp"

using namespace drt;

/*
 * Test that basic element insertion routines and lookup work.
 * Use ZeroHF to force all inserts to go to the same bucket.
 */

class BasicTest : public ::testing::Test {

protected:
    using hmap = Hashmap<int, int, ZeroHF<int>>;

    virtual void SetUp() {
//        h = hmap(10);
    }

    hmap h;
};

TEST_F(BasicTest, TestEmpty) {
    EXPECT_TRUE(h.empty());
    EXPECT_EQ(0, h.size());
    EXPECT_EQ(1, h.bucket_count());
}

TEST_F(BasicTest, TestOp) {
    int x = h[1];
    ASSERT_EQ(0, x);
}

TEST_F(BasicTest, TestInsert1) {
    h[1] = 2;
    ASSERT_EQ(2, h[1]);
    ASSERT_EQ(1, h.size());
}

TEST_F(BasicTest, TestInsert2) {
    h[1] = 2;
    h[2] = 3;
    ASSERT_EQ(2, h[1]);
    ASSERT_EQ(3, h[2]);
    ASSERT_EQ(2, h.size());
}

TEST_F(BasicTest, TestInsert3) {
    h[1] = 2;
    h[2] = 3;
    h[3] = 4;
    ASSERT_EQ(2, h[1]);
    ASSERT_EQ(3, h[2]);
    ASSERT_EQ(4, h[3]);
    ASSERT_EQ(3, h.size());
}

TEST_F(BasicTest, atTest) {
    h[1] = 2;
    ASSERT_EQ(2, h.at(1));
}

TEST_F(BasicTest, atFailTest) {
    h[3] = 4;
    ASSERT_THROW(h.at(1), std::out_of_range);
}
