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

TEST(SECURE_PSW, valid)
{
    twins::SecurePassw psw{"my$ecreT"};

    EXPECT_TRUE(psw.hasPassword());
    EXPECT_FALSE(psw == "secret");
    EXPECT_FALSE(psw == " my$ecreT");
    EXPECT_FALSE(psw == "my$ecreT ");
    EXPECT_FALSE(psw == "my$ecre");
    EXPECT_TRUE(psw == "my$ecreT");
}

TEST(SECURE_PSW, too_long)
{
    twins::SecurePassw psw{"Pineapple-Does-Not-Belong-On-Pizza-Change-My-Mind"};
    EXPECT_FALSE(psw.hasPassword());
    EXPECT_FALSE(psw == "Pineapple-Does-Not-Belong-On-Pizza-Change-My-Mind");
}
