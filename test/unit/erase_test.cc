#include <functional>
#include "gtest/gtest.h"
#include "hash_function.hpp"
#include "dirtyMap/HashMap.hpp"

using namespace drt;

class EraseTest : public ::testing::Test {

protected:
    using hmap = Hashmap<int, int, ZeroHF<int>, 5>;

    virtual void SetUp() {
        h = hmap(10);
    }

    hmap h;
};

TEST_F(EraseTest, noErase) {
    auto x = h.erase(3);
    EXPECT_EQ(0, h.size());
    EXPECT_EQ(0, x);
}

TEST_F(EraseTest, removeFirst) {
    h[1] = 1;
    h.erase(1);
    EXPECT_EQ(0, h.size());
    EXPECT_EQ(0, h[1]);
}

TEST_F(EraseTest, removeFirst2) {
    h[1] = 1;
    h[2] = 2;
    h.erase(2);
    EXPECT_EQ(1, h[1]);
    EXPECT_EQ(0, h[2]);
}

TEST_F(EraseTest, removeFirst3) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h.erase(3);
    EXPECT_EQ(2, h[2]);
    EXPECT_EQ(1, h[1]);
    EXPECT_EQ(0, h[3]);
}

TEST_F(EraseTest, removeLast) {
    h[1] = 1;
    h[2] = 2;
    h.erase(1);
    EXPECT_EQ(1, h.size());
    EXPECT_EQ(2, h[2]);
    EXPECT_EQ(0, h[1]);
}

TEST_F(EraseTest, removeLast2) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h.erase(1);
    EXPECT_EQ(2, h[2]);
    EXPECT_EQ(3, h[3]);
    EXPECT_EQ(0, h[1]);
}

TEST_F(EraseTest, removeMid) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h.erase(2);
    EXPECT_EQ(1, h[1]);
    EXPECT_EQ(3, h[3]);
    EXPECT_EQ(0, h[2]);
}

TEST_F(EraseTest, removeMid2) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h[4] = 4;
    h.erase(3);
    EXPECT_EQ(1, h[1]);
    EXPECT_EQ(2, h[2]);
    EXPECT_EQ(4, h[4]);
    EXPECT_EQ(0, h[3]);
}

TEST_F(EraseTest, removeMid3) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h[4] = 4;
    h.erase(2);
    EXPECT_EQ(1, h[1]);
}

TEST_F(EraseTest, removeMid4) {
    h[1] = 1;
    h[2] = 2;
    h[3] = 3;
    h[4] = 4;
    h[5] = 5;
    h.erase(3);
    EXPECT_EQ(1, h[1]);
}

TEST_F(EraseTest, clear1) {
    for (int i = 0; i < 5; ++i) {
        h[i] = i;
    }
    EXPECT_EQ(5, h.size());
    h.clear();
    EXPECT_EQ(0, h.size());
    EXPECT_EQ(0, h[3]);
}

TEST_F(EraseTest, clear2) {
    Hashmap<int, int, std::hash<int>, 5> m(6);
    for (int i = 1; i < 6; ++i) {
        m[i] = i;
    }
    EXPECT_EQ(5, m.size());
    m.clear();
    EXPECT_EQ(0, m.size());
    EXPECT_EQ(0, m[3]);
}
