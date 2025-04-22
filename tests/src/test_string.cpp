/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins.hpp"
#include "twins_pal_defimpl.hpp"

// -----------------------------------------------------------------------------

struct STRING : public testing::Test
{
    void SetUp() override
    {
        pal.stats = {};
    }

    void TearDown() override
    {
        EXPECT_EQ(0, pal.stats.memChunks);
    }

    twins::DefaultPAL& pal = (twins::DefaultPAL&)*twins::pPAL;
};

// -----------------------------------------------------------------------------

TEST_F(STRING, clear)
{
    {
        twins::String s;

        EXPECT_STREQ("", s.cstr());
        EXPECT_EQ(0, s.size());
        EXPECT_EQ(0, s.u8len());
        EXPECT_EQ(0, pal.stats.memChunks);

        s.clear();
        EXPECT_STREQ("", s.cstr());
        EXPECT_EQ(0, s.size());
        EXPECT_EQ(0, s.u8len());
        EXPECT_EQ(0, pal.stats.memChunks);
    }

    EXPECT_EQ(0, pal.stats.memChunksMax);

    {
        twins::String s;
        s.reserve(10);
        const char *buff1 = s.cstr();
        s.clear(10);
        EXPECT_EQ(buff1, s.cstr());
        s.reserve(10);
        EXPECT_EQ(buff1, s.cstr());
    }
}

TEST_F(STRING, append_no_resize)
{
    {
        twins::String s;
        EXPECT_STREQ("", s.cstr());

        s.append(nullptr);
        s.append("Cześć", 0); // append 0 copies
        s.append("Cześć");
        s.append(s.cstr()); // append ourselve shall fail

        EXPECT_STREQ("Cześć", s.cstr());
        EXPECT_EQ(7, s.size());
        EXPECT_EQ(5, s.u8len());
        EXPECT_EQ(1, pal.stats.memChunks);

        s.clear();
        EXPECT_EQ(1, pal.stats.memChunks);
        EXPECT_EQ(0, s.size());
        EXPECT_EQ(0, s.u8len());
    }

    EXPECT_EQ(1, pal.stats.memChunksMax);
}

TEST_F(STRING, append_resize_buffer)
{
    {
        twins::String s;
        s.append("12345");
        s.append("ABCDE", 6); // force buffer growth

        EXPECT_EQ(35, s.size());
        EXPECT_EQ(35, s.u8len());
        EXPECT_EQ(1, pal.stats.memChunks);

        s.clear();
        s.append('X', -5);
        s.append('X');
        EXPECT_EQ(1, pal.stats.memChunks);
        EXPECT_EQ(1, s.size());
        EXPECT_EQ(1, s.u8len());
    }

    EXPECT_EQ(2, pal.stats.memChunksMax);
}

TEST_F(STRING, append_very_long)
{
    twins::String s;
    s.append("12345ABCDE", 101);

    EXPECT_EQ(1010, s.size());
    EXPECT_EQ(1010, s.u8len());

    s.clear();
    s.append('X');
    EXPECT_EQ(1, s.size());
}

TEST_F(STRING, append_esc)
{
    twins::String s;
    s.append(ESC_BLINK "x" ESC_BLINK_OFF);

    EXPECT_EQ(10, s.size());
    EXPECT_EQ(10, s.u8len(false));
    EXPECT_EQ(1, s.u8len(true));
}

TEST_F(STRING, append_len)
{
    twins::String s;
    s.appendLen(nullptr, 3);
    EXPECT_EQ(0, s.size());
    s.appendLen("ABCDE", -15);
    EXPECT_EQ(0, s.size());

    s.appendLen("ABCDE", 3);
    s.appendLen("123456789", 5);

    EXPECT_EQ(8, s.size());
    EXPECT_EQ(8, s.u8len());
    EXPECT_STREQ("ABC12345", s.cstr());
}

TEST_F(STRING, append_fmt__fits_in_buffer)
{
    {
        twins::String s;
        s.append("12345");
        s.appendFmt(nullptr, "Fun()", __LINE__);
        EXPECT_EQ(5, s.size());

        s.appendFmt("%s:%4u", "Fun()", 2048);
        EXPECT_EQ(15, s.size());
        EXPECT_TRUE(strstr(s.cstr(), ":2048"));
        EXPECT_EQ(1, pal.stats.memChunks);
    }

    EXPECT_EQ(1, pal.stats.memChunksMax);
}

TEST_F(STRING, append_fmt__buffer_to_small)
{
    {
        twins::String s;
        s.append("12345", 6);
        EXPECT_EQ(30, s.size());

        s.appendFmt("%s:%4u", "Fun()", 2048); // buffer must be expanded
        EXPECT_TRUE(strstr(s.cstr(), ":2048"));
        EXPECT_EQ(40, s.size());
        EXPECT_EQ(1, pal.stats.memChunks);
    }

    EXPECT_EQ(2, pal.stats.memChunksMax);
}

TEST_F(STRING, stream_append)
{
    twins::String s;
    s.append("x");
    EXPECT_STREQ("x", s.cstr());

    s << "► " << "Service Menu" << ':';
    EXPECT_STREQ("x► Service Menu:", s.cstr());
}

TEST_F(STRING, trim_no_ellipsis)
{
    twins::String s;
    s.append("► Service Menu");

    // beyound text
    auto sz = s.size();
    s.trim(s.size());
    EXPECT_EQ(sz, s.size());

    // inside text
    s.trim(10);
    EXPECT_EQ(10, s.u8len());
    EXPECT_STREQ("► Service ", s.cstr());
}

TEST_F(STRING, trim_ellipsis_1)
{
    twins::String s;
    s = "► Service Menu";
    s.trim(10, true);
    EXPECT_EQ(10, s.u8len()); // trimmed at apace - no ellipsis added
    EXPECT_STREQ("► Service ", s.cstr());
}

TEST_F(STRING, trim_ellipsis_2)
{
    twins::String s;
    s = "► Service Menu";
    s.trim(12, true); // trim at non-space character
    EXPECT_EQ(12, s.u8len());
    EXPECT_STREQ("► Service M…", s.cstr());
}

TEST_F(STRING, trim_ignore_esc)
{
    twins::String s;
    s.append("►" ESC_BOLD " Service" ESC_NORMAL " Menu");

    s.trim(10, false, true);
    EXPECT_STREQ("►" ESC_BOLD " Service" ESC_NORMAL " ", s.cstr());
}

TEST_F(STRING, strip_empty)
{
    twins::String s;
    s.strip();
    EXPECT_STREQ("", s.cstr());
}

TEST_F(STRING, strip_allwhite)
{
    twins::String s(" \t\r\n ");
    s.strip();
    EXPECT_STREQ("", s.cstr());
}

TEST_F(STRING, strip_left_right)
{
    twins::String s("\t  \t123 \t\t ");

    s.strip(false, false);
    EXPECT_STREQ("\t  \t123 \t\t ", s.cstr());

    s.strip(true, false);
    EXPECT_STREQ("123 \t\t ", s.cstr());

    s.strip(false, true);
    EXPECT_STREQ("123", s.cstr());
}

TEST_F(STRING, set_width)
{
    {
        twins::String s;
        s = "X";
        s.setWidth(-1);
        EXPECT_STREQ("X", s.cstr());
    }

    {
        twins::String s;
        s = "1.";
        s.setWidth(10);
        EXPECT_STREQ("1.        ", s.cstr());
        s.setWidth(3);
        EXPECT_STREQ("1. ", s.cstr());
    }

    {
        twins::String s;
        s = "12345";

        s.setWidth(6);
        EXPECT_STREQ("12345 ", s.cstr());

        s.setWidth(5, false);
        EXPECT_STREQ("12345", s.cstr());

        s.setWidth(5, true);
        EXPECT_STREQ("12345", s.cstr());

        s.setWidth(2, true);
        EXPECT_STREQ("1…", s.cstr());

        s.setWidth(3, true);
        EXPECT_STREQ("1… ", s.cstr());
    }

    {
        twins::String s;
        s.append("►" ESC_BOLD " Service" ESC_NORMAL " Menu");

        s.setWidth(20);
        EXPECT_STREQ("►" ESC_BOLD " Service" ESC_NORMAL " Menu" "      ", s.cstr());

        s.setWidth(10);
        EXPECT_STREQ("►" ESC_BOLD " Service" ESC_NORMAL " ", s.cstr());
    }
}

TEST_F(STRING, copy_assign)
{
    twins::String s;
    s = "Menu";

    // such try shall fail
    s = s;
    EXPECT_STREQ("Menu", s.cstr());

    s = s.cstr();
    EXPECT_STREQ("Menu", s.cstr());

    s = s.cstr() + 3;
    EXPECT_STREQ("Menu", s.cstr());
}

TEST_F(STRING, move_assign)
{
    twins::String s1;
    s1 = "Menu";

    // such try shall fail
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
    s1 = std::move(s1);
#pragma GCC diagnostic pop

    EXPECT_STREQ("Menu", s1.cstr());

    twins::String s2;
    s2 = std::move(s1);

    EXPECT_EQ(0, s1.size());
    EXPECT_EQ(4, s2.size());

    twins::String s3 = std::move(s2);
    EXPECT_EQ(0, s2.size());
    EXPECT_EQ(4, s3.size());
}

TEST_F(STRING, erase)
{
    {
        twins::String s;

        s = "A";
        s.erase(-2, 1);
        EXPECT_STREQ("A", s.cstr());
        s.erase(0, -1);
        EXPECT_STREQ("A", s.cstr());
        s.erase(1, 1);
        EXPECT_STREQ("A", s.cstr());
    }

    {
        twins::String s;

        s = "*ĄBĆDĘ#";
        s.erase(1, 1);
        EXPECT_STREQ("*BĆDĘ#", s.cstr());
        s.erase(3, 2);
        EXPECT_STREQ("*BĆ#", s.cstr());
        s.erase(1, 15);
        EXPECT_STREQ("*", s.cstr());
    }
}

TEST_F(STRING, insert)
{
    {
        twins::String s;

        s = "A";
        s.insert(-2, "*");
        EXPECT_STREQ("A", s.cstr());
        s.insert(0, nullptr);
        EXPECT_STREQ("A", s.cstr());
        s.insert(0, "");
        EXPECT_STREQ("A", s.cstr());
        s.insert(5, ".");
        EXPECT_STREQ("A.", s.cstr());
    }

    {
        twins::String s;

        s = "*ĄBĆDĘ#";
        s.insert(1, ".");
        EXPECT_STREQ("*.ĄBĆDĘ#", s.cstr());
        s.insert(5, "••");
        EXPECT_STREQ("*.ĄBĆ••DĘ#", s.cstr());
        s.insert(11, "X");
        EXPECT_STREQ("*.ĄBĆ••DĘ#X", s.cstr());
    }

    {
        twins::String s;

        s.insert(0, "••");
        EXPECT_STREQ("••", s.cstr());
    }

    {
        twins::String s("123");

        s.insert(1, "•", 0);
        EXPECT_STREQ("123", s.cstr());

        s.insert(0, "•", 3);
        EXPECT_STREQ("•••123", s.cstr());
        s.insert(3, "ABC", 2);
        EXPECT_STREQ("•••ABCABC123", s.cstr());
    }
}

TEST_F(STRING, replaceChar)
{
    // empty
    {
        twins::String s;
        s.replaceChar('x', ' ');
        EXPECT_STREQ("", s.cstr());
    }

    // not-empty
    {
        twins::String s("x");
        s.replaceChar('x', '-');
        EXPECT_STREQ("-", s.cstr());
    }

    // not-empty
    {
        twins::String s(" 123 456 xyz! ");
        s.replaceChar(' ', '-');
        EXPECT_STREQ("-123-456-xyz!-", s.cstr());
    }
}

TEST_F(STRING, escLen)
{
    EXPECT_EQ(0, twins::String::escLen(nullptr));
    EXPECT_EQ(0, twins::String::escLen(""));
    // Up
    EXPECT_EQ(0, twins::String::escLen("x\e[A"));
    EXPECT_EQ(3, twins::String::escLen("\e[A"));
    // Home
    EXPECT_EQ(4, twins::String::escLen("\e[1~"));
    // F1
    EXPECT_EQ(5, twins::String::escLen("\e[23^"));
    // F1
    EXPECT_EQ(3, twins::String::escLen("\eOP"));
    // C-S-F1
    EXPECT_EQ(5, twins::String::escLen("\e[23@"));
    // Mouse l-click
    EXPECT_EQ(6, twins::String::escLen("\e[M !!"));
    // Mouse wheel down
    EXPECT_EQ(6, twins::String::escLen("\e[Ma$\""));

    // Home - incomplete
    EXPECT_EQ(0, twins::String::escLen("\e[1"));
    //  Mouse wheel down - incomplete
    EXPECT_EQ(0, twins::String::escLen("\e[Ma"));
    //  Mouse wheel down - incomplete
    EXPECT_EQ(0, twins::String::escLen("\e[Ma$\"", "\e[Ma$\""+5));
}

TEST_F(STRING, u8len_IgnoreEsc)
{
    EXPECT_EQ(0, twins::String::u8len(nullptr));
    EXPECT_EQ(0, twins::String::u8len(""));

    EXPECT_EQ(3, twins::String::u8len("ABC", nullptr, true));
    EXPECT_EQ(3, twins::String::u8len("ĄBĆ", nullptr, true));

    EXPECT_EQ(3, twins::String::u8len("ĄBĆ\e[A", nullptr, true));
    EXPECT_EQ(3, twins::String::u8len("\e[AĄBĆ", nullptr, true));
    EXPECT_EQ(4, twins::String::u8len("Ą\e[ABĆ\e[48;2;255;255;255mĘ", nullptr, true));
}

TEST_F(STRING, u8skipEsc)
{
    EXPECT_STREQ("", twins::String::u8skip(nullptr, 0));
    EXPECT_STREQ("", twins::String::u8skip("", 5));

    EXPECT_STREQ("ABC", twins::String::u8skip("ABC", 0));
    EXPECT_STREQ("C", twins::String::u8skip("ABC", 2));
    EXPECT_STREQ("", twins::String::u8skip("ABC", 5));
    EXPECT_STREQ("Ć", twins::String::u8skip("ĄBĆ", 2));

    EXPECT_STREQ("Ć\e[1;2AĘ", twins::String::u8skip("Ą\e[ABĆ\e[1;2AĘ", 2));
    EXPECT_STREQ("", twins::String::u8skip("Ą\e[ABĆ\e[1;2AĘ", 4));
}

TEST_F(STRING, emoticons)
{
    EXPECT_EQ(11, twins::String::u8len("😉\e[1m*\e[0m🍺", nullptr, false, false));
    EXPECT_EQ(13, twins::String::u8len("😉\e[1m*\e[0m🍺", nullptr, false, true));
    EXPECT_EQ( 3, twins::String::u8len("😉\e[1m*\e[0m🍺", nullptr, true, false));
    EXPECT_EQ( 5, twins::String::u8len("😉\e[1m*\e[0m🍺", nullptr, true, true));

    EXPECT_EQ( 5, twins::String::width("😉\e[1m*\e[0m🍺"));

    twins::String s("😉\e[1m*\e[0m🍺");
    EXPECT_EQ( 5, s.width());
}

TEST_F(STRING, startsWith)
{
    {
        twins::String s;
        EXPECT_FALSE(s.startsWith(nullptr));
        EXPECT_FALSE(s.startsWith(""));

        s = "*ĄBĆDĘ#";
        EXPECT_TRUE(s.startsWith("*Ą"));
        EXPECT_TRUE(s.startsWith(s.cstr()));
        EXPECT_FALSE(s.startsWith("0123456789.123456789"));
        EXPECT_FALSE(s.startsWith("?"));
    }

    {
        twins::String s;
        EXPECT_FALSE(s.endsWith(nullptr));
        EXPECT_FALSE(s.endsWith(""));

        s = "*ĄBĆDĘ#";
        EXPECT_TRUE(s.endsWith("Ę#"));
        EXPECT_TRUE(s.endsWith(s.cstr()));
        EXPECT_FALSE(s.endsWith("0123456789.123456789"));
        EXPECT_FALSE(s.endsWith("?"));
    }
}

TEST_F(STRING, find)
{
    twins::String s;
    EXPECT_EQ(-1, s.find(nullptr));
    EXPECT_EQ(-1, s.find(""));

    s = "*ĄBĆDĘ#";
    EXPECT_EQ(-1, s.find(""));
    EXPECT_EQ(0,  s.find("*"));
    EXPECT_EQ(4,  s.find("Ć"));
}

TEST_F(STRING, eq)
{
    twins::String s;
    EXPECT_FALSE(s == nullptr);
    EXPECT_TRUE(s == "");

    s = "*ĄBĆDĘ#";
    EXPECT_TRUE(s == "*ĄBĆDĘ#");
    EXPECT_TRUE(s == s);

    EXPECT_FALSE(s == "*ĄBĆDĘ?#");
}

// -----------------------------------------------------------------------------

TEST(STRINGBUFF, create_empty)
{
    twins::StringBuff sb;
    EXPECT_EQ(0, sb.size());
}

TEST(STRINGBUFF, create_from_cstr)
{
    {
        twins::StringBuff sb(nullptr);
        EXPECT_EQ(0, sb.size());
    }

    {
        twins::StringBuff sb("");
        EXPECT_EQ(0, sb.size());
    }

    {
        twins::StringBuff sb("Blume");
        EXPECT_EQ(5, sb.size());
    }
}

TEST(STRINGBUFF, create_from_string)
{
    {
        twins::String s(nullptr);
        twins::StringBuff sb(std::move(s));
        EXPECT_EQ(0, sb.size());
    }

    {
        twins::String s("ChilloutDeer");
        twins::StringBuff sb(std::move(s));
        EXPECT_EQ(0, s.size());
        EXPECT_EQ(12, sb.size());
    }
}

TEST(STRINGBUFF, copy_from_string)
{
    twins::String s("ChilloutDeer");
    twins::StringBuff sb;
    EXPECT_EQ(12, s.size());
    EXPECT_EQ(0, sb.size());

    sb = s;
    EXPECT_STREQ("ChilloutDeer", s.cstr());
    EXPECT_STREQ("ChilloutDeer", sb.cstr());
}

TEST(STRINGBUFF, move_from_string)
{
    twins::String s("ChilloutDeer");
    twins::StringBuff sb;
    EXPECT_EQ(12, s.size());
    EXPECT_EQ(0, sb.size());

    sb = std::move(s);
    EXPECT_STREQ("", s.cstr());
    EXPECT_STREQ("ChilloutDeer", sb.cstr());
}
