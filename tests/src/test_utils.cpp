/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_utils.hpp"

// -----------------------------------------------------------------------------

#define MKSTDSTR(range) std::string(range.data, range.size).c_str()

TEST(UTILS, splitWords)
{
    {
        auto words = twins::util::splitWords(nullptr);
        EXPECT_EQ(0, words.size());
    }

    {
        auto words = twins::util::splitWords("");
        EXPECT_EQ(0, words.size());
    }

    {
        auto words = twins::util::splitWords("  \t \n ");
        EXPECT_EQ(0, words.size());
    }

    {
        auto words = twins::util::splitWords("  \t \n ", "\t\n ", true);
        EXPECT_EQ(1, words.size());
    }

    {
        auto words = twins::util::splitWords("abc");
        EXPECT_EQ(1, words.size());
        EXPECT_STREQ("abc", MKSTDSTR(words[0]));
    }

    {
        auto words = twins::util::splitWords(" abc def ");
        ASSERT_EQ(2, words.size());

        EXPECT_STREQ("abc", MKSTDSTR(words[0]));
        EXPECT_STREQ("def", MKSTDSTR(words[1]));
    }

    {
        auto words = twins::util::splitWords(" \t abc def ", "\t ", true);
        ASSERT_EQ(5, words.size());

        EXPECT_STREQ(" \t ", MKSTDSTR(words[0]));
        EXPECT_STREQ("abc",  MKSTDSTR(words[1]));
        EXPECT_STREQ(" ",    MKSTDSTR(words[2]));
        EXPECT_STREQ("def",  MKSTDSTR(words[3]));
        EXPECT_STREQ(" ",    MKSTDSTR(words[4]));
    }

    {
        auto words = twins::util::splitWords(" abc\n def   g ");
        ASSERT_EQ(3, words.size());
        EXPECT_STREQ("abc", MKSTDSTR(words[0]));
        EXPECT_STREQ("def", MKSTDSTR(words[1]));
        EXPECT_STREQ("g",   MKSTDSTR(words[2]));
    }

    {
        auto words = twins::util::splitWords(" abc def ", ",");
        ASSERT_EQ(1, words.size());
        EXPECT_STREQ(" abc def ", MKSTDSTR(words[0]));
    }

    {
        auto words = twins::util::splitWords("abc\e[cDE f", " ");
        ASSERT_EQ(4, words.size());
        EXPECT_STREQ("abc",  MKSTDSTR(words[0]));
        EXPECT_STREQ("\e[c", MKSTDSTR(words[1]));
        EXPECT_STREQ("DE",   MKSTDSTR(words[2]));
        EXPECT_STREQ("f",    MKSTDSTR(words[3]));
    }

    {
        auto words = twins::util::splitWords("\e[c D", " ");
        ASSERT_EQ(2, words.size());
        EXPECT_STREQ("\e[c", MKSTDSTR(words[0]));
        EXPECT_STREQ("D",    MKSTDSTR(words[1]));
    }

    {
        auto words = twins::util::splitWords(" \e[c   D", " ");
        ASSERT_EQ(2, words.size());
        EXPECT_STREQ("\e[c", MKSTDSTR(words[0]));
        EXPECT_STREQ("D",    MKSTDSTR(words[1]));
    }
}

TEST(UTILS, wordWrap)
{
    {
        auto s = twins::util::wordWrap(nullptr, 1);
        EXPECT_EQ(0, s.size());
    }

    {
        auto s = twins::util::wordWrap("a", 0);
        EXPECT_EQ(0, s.size());
    }

    {
        auto s = twins::util::wordWrap("a", 10, "");
        EXPECT_EQ(0, s.size());
    }

    {
        auto s = twins::util::wordWrap("ab cdę fghi z", 3, " ", ".");
        EXPECT_STREQ("ab .cdę .fg….z", s.cstr());
    }

    {
        auto s = twins::util::wordWrap("ab cdę   fghi", 20, " ", ".");
        EXPECT_STREQ("ab cdę   fghi", s.cstr());
    }

    {
        auto s = twins::util::wordWrap("abc de", 2, " ", "|");
        EXPECT_STREQ("a…|de", s.cstr());
    }

    {
        // test for proper \n handling
        auto s = twins::util::wordWrap("ab\n   ćde \t fg  ", 3, " \t\n", ".");
        EXPECT_STREQ("ab\n   .ćde \t .fg  ", s.cstr());
    }

    {
        auto s = twins::util::wordWrap("ab cd ęfgh i  jk", 5, " ", ".");
        EXPECT_STREQ("ab cd .ęfgh .i  jk", s.cstr());
    }

    {
        // test for ESC sequence handling
        auto s = twins::util::wordWrap("AB\e[1mCD\e[0mE  ęfgh", 5, " ", ".");
        EXPECT_STREQ("AB\e[1mCD\e[0mE  .ęfgh", s.cstr());
    }
}

TEST(UTILS, splitLines)
{
    {
        auto vec = twins::util::splitLines(nullptr);
        EXPECT_EQ(0, vec.size());
    }

    {
        auto vec = twins::util::splitLines("");
        EXPECT_EQ(0, vec.size());
    }

    {
        auto vec = twins::util::splitLines(" abc ");
        ASSERT_EQ(1, vec.size());
        twins::String s; s.appendLen(vec[0].data, vec[0].size);
        EXPECT_STREQ(" abc ", s.cstr());
    }

    {
        auto vec = twins::util::splitLines(" abc \n");
        ASSERT_EQ(1, vec.size());
        twins::String s; s.appendLen(vec[0].data, vec[0].size);
        EXPECT_STREQ(" abc ", s.cstr());
    }

    {
        auto vec = twins::util::splitLines("\n\n\n");
        EXPECT_EQ(3, vec.size());
        EXPECT_EQ(0, vec[0].size);
        EXPECT_EQ(0, vec[1].size);
        EXPECT_EQ(0, vec[2].size);
    }

    {
        auto vec = twins::util::splitLines("\n..\n\n...");
        EXPECT_EQ(4, vec.size());
        EXPECT_EQ(0, vec[0].size);
        EXPECT_EQ(2, vec[1].size);
        EXPECT_EQ(0, vec[2].size);
        EXPECT_EQ(3, vec[3].size);
    }
}

TEST(UTILS, centerText)
{
    {
        auto s = twins::util::centerText("aBc", 2);
        EXPECT_STREQ("aBc", s.cstr());
    }

    {
        auto s = twins::util::centerText("aBc", 4);
        EXPECT_STREQ("aBc ", s.cstr());
    }

    {
        auto s = twins::util::centerText("aBc", 5);
        EXPECT_STREQ(" aBc ", s.cstr());
    }

    {
        auto s = twins::util::centerText("🔷aBc", 7);
        EXPECT_STREQ(" 🔷aBc ", s.cstr());
    }

    {
        auto s = twins::util::centerText("aBc", 10);
        EXPECT_STREQ("   aBc    ", s.cstr());
    }
}

TEST(UTILS, WrappedString_empty)
{
    twins::util::WrappedString ws;
    EXPECT_FALSE(ws.isDirty());
    EXPECT_EQ(0, ws.getLines().size());
}

TEST(UTILS, WrappedString_getlines)
{
    twins::util::WrappedString ws;
    EXPECT_FALSE(ws.getSourceStr().size());

    ws = "\e[1m" "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam arcu magna, placerat sit amet libero at, aliquam fermentum augue.\n" "\e[0m";
    EXPECT_TRUE(ws.getSourceStr().size());
    EXPECT_FALSE(ws.getWrappedStr().size());
    EXPECT_TRUE(ws.isDirty());

    ws.updateLines();
    EXPECT_TRUE(ws.getWrappedStr().size());
    EXPECT_FALSE(ws.isDirty());

    ws.config(10);
    EXPECT_TRUE(ws.isDirty());
    EXPECT_EQ(16, ws.getLines().size());
}

TEST(UTILS, WrappedString_assign)
{
    twins::util::WrappedString ws;
    twins::String s; s = "abc";
    EXPECT_FALSE(ws.getSourceStr().size());
    ws = s;
    EXPECT_EQ(3, ws.getSourceStr().size());

    ws.getSourceStr().clear();
    EXPECT_EQ(0, ws.getSourceStr().size());
    ws = std::move(s);
    EXPECT_EQ(3, ws.getSourceStr().size());
}

TEST(UTILS, numEdit)
{
    twins::KeyCode kc;
    twins::String str;
    int16_t cursorPos = 0;

    // invalid key - handled as 'rejected'
    bool succ = twins::util::numEditInputEvt(kc, str, cursorPos);
    EXPECT_TRUE(succ);

    // too long number
    str.append('9', 20);
    succ = twins::util::numEditInputEvt(kc, str, cursorPos);
    EXPECT_TRUE(succ);
    str.clear();

    // out of range
    {
        // empty or under limit
        str.clear();
        kc.key = twins::Key::Enter;
        kc.mod_all = KEY_MOD_SPECIAL;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos, 33, 911);
        EXPECT_FALSE(succ);
        EXPECT_STREQ("33", str.cstr());
        str.clear();

        // over limit
        str = "912";
        kc.key = twins::Key::Enter;
        kc.mod_all = KEY_MOD_SPECIAL;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos, 33, 911);
        EXPECT_FALSE(succ);
        EXPECT_STREQ("911", str.cstr());
        str.clear();
    }

    // edit accept - to be handled by caller
    kc.key = twins::Key::Enter;
    kc.mod_all = KEY_MOD_SPECIAL;
    succ = twins::util::numEditInputEvt(kc, str, cursorPos);
    EXPECT_FALSE(succ);

    // edit abort - to be handled by caller
    kc.key = twins::Key::Esc;
    kc.mod_all = KEY_MOD_SPECIAL;
    succ = twins::util::numEditInputEvt(kc, str, cursorPos);
    EXPECT_FALSE(succ);

    // UP arrow
    {
        str = "";
        kc.key = twins::Key::Up;
        kc.mod_all = KEY_MOD_SPECIAL;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("1", str.cstr());

        // Ctrl+UP
        kc.key = twins::Key::Up;
        kc.mod_all = KEY_MOD_NONE;
        kc.m_spec = true;
        kc.m_ctrl = true;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("11", str.cstr());

        // Shift+UP
        kc.key = twins::Key::Up;
        kc.mod_all = KEY_MOD_NONE;
        kc.m_spec = true;
        kc.m_shift = true;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("111", str.cstr());
    }

    // DOWN arrow
    {
        kc.key = twins::Key::Down;
        kc.mod_all = KEY_MOD_NONE;
        kc.m_spec = true;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("110", str.cstr());

        // Ctrl+Down
        kc.key = twins::Key::Down;
        kc.mod_all = KEY_MOD_NONE;
        kc.m_spec = true;
        kc.m_ctrl = true;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("100", str.cstr());

        // Shift+Down
        kc.key = twins::Key::Down;
        kc.mod_all = KEY_MOD_NONE;
        kc.m_spec = true;
        kc.m_shift = true;
        succ = twins::util::numEditInputEvt(kc, str, cursorPos);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("0", str.cstr());
    }
}

TEST(UTILS, numEditLimited)
{
    twins::KeyCode kc = {};
    kc.m_spec = true;
    twins::String str;
    int16_t cursorPos = 0;
    bool succ;

    // up, no wrap
    kc.key = twins::Key::Up;

    {
        str = "0";

        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, false);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("1", str.cstr());

        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, false);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("1", str.cstr());
    }

    // up, wrap
    {
        str = "1";

        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, true);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("-1", str.cstr());
    }

    // down, no wrap
    kc.key = twins::Key::Down;

    {
        str = "0";

        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, false);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("-1", str.cstr());

        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, false);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("-1", str.cstr());
    }

    // down, wrap
    {
        str = "-1";
        succ = twins::util::numEditInputEvt(kc, str, cursorPos, -1, 1, true);
        EXPECT_TRUE(succ);
        EXPECT_STREQ("1", str.cstr());
    }
}
