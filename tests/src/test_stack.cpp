/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_stack.hpp"

// -----------------------------------------------------------------------------

TEST(STACK, push_pop_2int)
{
    twins::Stack<int> si;

    EXPECT_EQ(0, si.size());
    EXPECT_EQ(nullptr, si.pop());

    si.push(1);
    si.push(2);
    EXPECT_EQ(2, si.size());

    EXPECT_EQ(2, *si.pop());
    EXPECT_EQ(1, *si.pop());
    EXPECT_EQ(nullptr, si.pop());
}

TEST(STACK, push_pop_100int)
{
    twins::Stack<int> si;
    EXPECT_FALSE(si.top());

    for (int i = 0; i < 100; i++)
        si.push(i);

    EXPECT_EQ(99, *si.top());
    EXPECT_EQ(100, si.size());
    EXPECT_EQ(99, *si.pop());

    si.clear();
}

TEST(STACK, push_pop_string)
{
    twins::Stack<std::string> stck;
    // test for proper content destruction to catch memory leak
    std::string s;
    s.resize(100);

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(0, stck.size());

    stck.push(s);
    EXPECT_FALSE(s.empty());
    EXPECT_EQ(1, stck.size());

    stck.push(std::move(s));
    EXPECT_EQ(2, stck.size());
    EXPECT_TRUE(s.empty());
}

TEST(STACK, destroy_string)
{
    twins::Stack<std::string> stck;
    // test for proper content destruction to catch memory leak
    std::string s;
    s.resize(100);
    stck.push(s);
    stck.push(s);

    stck.clear(true);
}
