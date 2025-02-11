/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_queue.hpp"

// -----------------------------------------------------------------------------

TEST(QUEUE, write_read_2int)
{
    twins::Queue<int> qi;

    EXPECT_EQ(0, qi.size());
    EXPECT_EQ(0, qi.read());
    EXPECT_EQ(nullptr, qi.front());

    qi.write(1);
    qi.write(2);

    EXPECT_EQ(2, qi.size());
    EXPECT_EQ(1, *qi.front());
    EXPECT_EQ(1, qi.read());
    EXPECT_EQ(1, qi.size());

    EXPECT_EQ(2, qi.read());
    EXPECT_EQ(0, qi.size());
    EXPECT_EQ(nullptr, qi.front());
}

TEST(QUEUE, write_read_100int)
{
    twins::Queue<int> qi;

    EXPECT_EQ(0, qi.size());
    for (int i = 0; i < 100; i++)
        qi.write(i);

    EXPECT_EQ(100, qi.size());
    ASSERT_EQ(0, qi.read());

    for (int i = 1; i < 100; i++)
        EXPECT_EQ(i, qi.read());

    EXPECT_EQ(0, qi.size());
    qi.clear();
    EXPECT_EQ(0, qi.read());
    EXPECT_EQ(nullptr, qi.front());
    EXPECT_EQ(0, qi.size());
}

TEST(QUEUE, write_read_string)
{
    twins::Queue<std::string> qstr;
    // test for proper content destruction to catch memory leak
    std::string s;
    s.resize(100);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(0, qstr.size());

    // write copy
    qstr.write(s);
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(1, qstr.size());

    // write-by-move
    qstr.write(std::move(s));
    EXPECT_EQ(2, qstr.size());
    EXPECT_TRUE(s.empty());
}
