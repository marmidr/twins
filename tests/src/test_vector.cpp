/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_vector.hpp"

#include <array>

// -----------------------------------------------------------------------------

TEST(VECTOR, reserve)
{
    twins::Vector<int> vi;

    EXPECT_EQ(0, vi.size());
    EXPECT_EQ(0, vi.capacity());
    EXPECT_FALSE(vi.getAt(0));

    vi.reserve(100);

    EXPECT_EQ(0, vi.size());
    EXPECT_EQ(100, vi.capacity());
    EXPECT_FALSE(vi.getAt(0));
}

TEST(VECTOR, copy_ctr)
{
    twins::Vector<int> v1(5);
    EXPECT_EQ(5, v1.size());
    EXPECT_EQ(5, v1.capacity());
    v1[1] = 42;
    EXPECT_TRUE(v1.getAt(0));

    twins::Vector<int> v2(v1);
    EXPECT_EQ(5, v2.size());
    EXPECT_GE(v2.capacity(), 5);

    EXPECT_EQ(5, v1.size());
    EXPECT_EQ(5, v1.capacity());
    EXPECT_TRUE(v2 == v1);
}

TEST(VECTOR, mv_ctr)
{
    twins::Vector<int> v1(5);
    EXPECT_EQ(5, v1.size());
    EXPECT_EQ(5, v1.capacity());
    v1[1] = 42;

    twins::Vector<int> v2(std::move(v1));
    EXPECT_EQ(5, v2.size());
    EXPECT_EQ(5, v2.capacity());

    EXPECT_EQ(0, v1.size());
    EXPECT_EQ(0, v1.capacity());
    EXPECT_FALSE(v2 == v1);
}

TEST(VECTOR, cpy_assign)
{
    twins::Vector<int> v1(5);
    EXPECT_EQ(5, v1.size());

    twins::Vector<int> v2;
    v2 = v1;
    EXPECT_EQ(5, v2.size());

    v1 = v1;
    EXPECT_EQ(5, v1.size());
}

TEST(VECTOR, mv_assign)
{
    twins::Vector<int> v1(5);
    EXPECT_EQ(5, v1.size());
    EXPECT_EQ(5, v1.capacity());
    v1[1] = 42;

    twins::Vector<int> v2;
    v2 = std::move(v1);
    EXPECT_EQ(5, v2.size());
    EXPECT_EQ(5, v2.capacity());
    EXPECT_EQ(0, v1.size());
    EXPECT_EQ(0, v1.capacity());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    v2 = std::move(v2);
#pragma GCC diagnostic pop

    EXPECT_EQ(5, v2.size());
}

TEST(VECTOR, find)
{
    twins::Vector<int> v1(5);
    v1[1] = 42;
    v1[4] = 777;

    EXPECT_FALSE(v1.contains(33));

    int idx = -1;
    EXPECT_TRUE(v1.find(777, &idx));
    EXPECT_EQ(4, idx);
}


TEST(VECTOR, range_for)
{
    twins::Vector<int> v;

    v.append() = 1;
    v.append() = 2;
    v.append() = 3;
    v.append() = 5;

    EXPECT_TRUE(v.contains(1));
    EXPECT_TRUE(v.contains(5));
    EXPECT_EQ(4, v.size());

    int sum = 0;
    for (auto i : v)
        sum += i;

    EXPECT_EQ(11, sum);
}

TEST(VECTOR, insert)
{
    twins::Vector<int> v;
    EXPECT_EQ(0, v.size());

    v.insert(0, 77);
    EXPECT_TRUE(v == (twins::Vector<int>{77}));

    v.insert(0, 111);
    EXPECT_TRUE(v == (twins::Vector<int>{111, 77}));

    v.insert(100, -3);
    EXPECT_TRUE(v == (twins::Vector<int>{111, 77, -3}));
}

TEST(VECTOR, remove)
{
    twins::Vector<int> v(std::initializer_list<int>{1, 2, 3, 5, 11, 16});
    EXPECT_EQ(6, v.size());

    v.remove(1, true);
    EXPECT_TRUE(v == (twins::Vector<int>{1, 3, 5, 11, 16}));

    v.remove(1, false);
    EXPECT_TRUE(v == (twins::Vector<int>{1, 16, 5, 11}));

    v.clear();
    EXPECT_EQ(0, v.size());
    v.remove(0);
    v.remove(1);

    v.append(1);
    EXPECT_EQ(1, v.size());
    v.remove(0);
    EXPECT_EQ(0, v.size());
}

TEST(VECTOR, resize_down)
{
    twins::Vector<int> v(std::initializer_list<int>{1, 2, 3, 5, 11, 16});
    EXPECT_EQ(6, v.size());

    v.resize(4);
    EXPECT_EQ(4, v.size());
}

TEST(VECTOR, shrink)
{
    using bigarray_t = std::array<int, 100>;
    twins::Vector<bigarray_t> v;

    v.reserve(5);
    v.append(bigarray_t{});
    v.append(bigarray_t{});
    v.append(bigarray_t{});
    EXPECT_EQ(3, v.size());
    EXPECT_GE(v.capacity(), 5);

    v.resize(v.size());
    EXPECT_EQ(3, v.size());

    v.shrink();
    EXPECT_EQ(3, v.size());
    EXPECT_EQ(3, v.capacity());

    EXPECT_EQ(100, v.begin()->size());
}

TEST(VECTOR, std_string)
{
    twins::Vector<std::string> v;
    // test for proper content destruction to catch memory leak
    std::string s;
    s.resize(100);
    v.append(s);
    v.insert(0, "X");
}
