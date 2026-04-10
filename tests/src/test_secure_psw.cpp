/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_secure_psw.hpp"

// -----------------------------------------------------------------------------

TEST(SECURE_PSW, empty)
{
    twins::SecurePassw psw{""};

    EXPECT_FALSE(psw.hasPassword());
    EXPECT_FALSE(psw == "");
}

TEST(SECURE_PSW, short_psw)
{
    const twins::SecurePassw psw{"my$ecreT"};

    EXPECT_TRUE(psw.hasPassword());
    EXPECT_FALSE(psw == "secret");
    EXPECT_FALSE(psw == " my$ecreT");
    EXPECT_FALSE(psw == "my$ecreT ");
    EXPECT_FALSE(psw == "my$ecre");
    EXPECT_TRUE(psw == "my$ecreT");
}

TEST(SECURE_PSW, long_psw)
{
    constexpr twins::SecurePassw psw("Pineapple-Does-Not-Belong-On-Pizza-Change-My-Mind");
    EXPECT_TRUE(psw.hasPassword());
    EXPECT_TRUE(psw == "Pineapple-Does-Not-Belong-On-Pizza-Change-My-Mind");
}

TEST(SECURE_PSW, compare)
{
    constexpr twins::SecurePassw psw0a{""};
    constexpr twins::SecurePassw psw0b{""};
    twins::SecurePassw psw1{"mySecret"};
    twins::SecurePassw psw2{"my_ecret"};
    twins::SecurePassw psw3{"mySecret"};

    EXPECT_FALSE(psw0a == psw0b);
    EXPECT_FALSE(psw0a == psw1);
    EXPECT_FALSE(psw1 == psw0a);
    EXPECT_FALSE(psw1 == psw2);
    // EXPECT_TRUE(psw1 == psw3);
}
