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

static const char *getLineBuff()
{
    if (auto *pal = dynamic_cast<twins::DefaultPAL*>(twins::pPAL))
    {
        return pal->lineBuff.cstr();
    }

    return "";
}

static void clrLineBuff()
{
    if (auto *pal = dynamic_cast<twins::DefaultPAL*>(twins::pPAL))
        pal->lineBuff.clear();
}

// -----------------------------------------------------------------------------

class TWINS : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        twins::flushBuffer();
    }
};

// -----------------------------------------------------------------------------

TEST_F(TWINS, encodeCl)
{
    {
        const char *cl = twins::encodeCl(twins::ColorFG::White);
        EXPECT_STRNE(cl, "");

        cl = twins::encodeCl(twins::ColorFG::Inherit);
        EXPECT_STREQ(cl, "");

        cl = twins::encodeCl(twins::ColorFG::ThemeBegin);
        EXPECT_STREQ(cl, "");
    }

    {
        const char *cl = twins::encodeCl(twins::ColorBG::White);
        EXPECT_STRNE(cl, "");

        cl = twins::encodeCl(twins::ColorBG::Inherit);
        EXPECT_STREQ(cl, "");

        cl = twins::encodeCl(twins::ColorBG::ThemeBegin);
        EXPECT_STREQ(cl, "");
    }
}

TEST_F(TWINS, intensifyCl)
{
    {
        auto cl = twins::intensifyCl(twins::ColorFG::White);
        EXPECT_EQ(twins::ColorFG::WhiteIntense, cl);

        cl = twins::intensifyCl(twins::ColorFG::Inherit);
        EXPECT_EQ(twins::ColorFG::Inherit, cl);

        cl = twins::intensifyCl(twins::ColorFG::Default);
        EXPECT_EQ(twins::ColorFG::WhiteIntense, cl);
    }

    {
        auto cl = twins::intensifyCl(twins::ColorBG::White);
        EXPECT_EQ(twins::ColorBG::WhiteIntense, cl);

        cl = twins::intensifyCl(twins::ColorBG::Inherit);
        EXPECT_EQ(twins::ColorBG::Inherit, cl);

        cl = twins::intensifyCl(twins::ColorBG::Default);
        EXPECT_EQ(twins::ColorBG::BlackIntense, cl);
    }

    {
 	    auto cl = twins::ColorBG::White;
        twins::intensifyClIf(false, cl);
        EXPECT_EQ(twins::ColorBG::White, cl);

        twins::intensifyClIf(true, cl);
        EXPECT_EQ(twins::ColorBG::WhiteIntense, cl);
    }
}

TEST_F(TWINS, transcodeClBg2Fg)
{
    {
        const char*cl_fg = twins::transcodeClBg2Fg(nullptr);
        EXPECT_STREQ("", cl_fg);
    }

    {
        const char*cl_fg = twins::transcodeClBg2Fg("Forward");
        EXPECT_STREQ("Forward", cl_fg);
    }

    {
        const char*cl_fg = twins::transcodeClBg2Fg(ESC_BG_Salmon);
        EXPECT_STREQ(ESC_FG_Salmon, cl_fg);
    }

    {
        const char*cl_fg = twins::transcodeClBg2Fg(ESC_BG_GREEN);
        EXPECT_STREQ(ESC_FG_GREEN, cl_fg);
    }

    {
        const char*cl_fg = twins::transcodeClBg2Fg(ESC_BG_GREEN_INTENSE);
        EXPECT_STREQ(ESC_FG_GREEN_INTENSE, cl_fg);
    }

    {
        const char*cl_fg = twins::transcodeClBg2Fg(ESC_BG_COLOR(15));
        EXPECT_STREQ(ESC_FG_COLOR(15), cl_fg);
    }
}

TEST_F(TWINS, log)
{
    twins::Locker lck;
    EXPECT_TRUE(lck.isLocked());

    // PAL present
    TWINS_LOG_I("123");
    twins::flushBuffer();
    twins::log(nullptr, __FILE__, __LINE__, nullptr, nullptr);

    // own timestamp
    uint64_t timestamp;
    twins::log(&timestamp, __FILE__, __LINE__, "-I-", "Message");
}

TEST_F(TWINS, attr)
{
    {
        twins::pushAttr(twins::FontAttrib::None);
        twins::pushAttr(twins::FontAttrib::Bold);
        twins::pushAttr(twins::FontAttrib::Faint);
        twins::pushAttr(twins::FontAttrib::Italics);
        twins::pushAttr(twins::FontAttrib::Underline);
        twins::pushAttr(twins::FontAttrib::Blink);
        twins::pushAttr(twins::FontAttrib::Inverse);
        twins::pushAttr(twins::FontAttrib::Invisible);
        twins::pushAttr(twins::FontAttrib::StrikeThrough);
        twins::pushAttr(twins::FontAttrib::StrikeThrough);
        twins::popAttr(6);

        twins::resetAttr();
        twins::resetClFg();
        twins::resetClBg();
    }

    {
        twins::pushAttr(twins::FontAttrib::Faint);
        twins::popAttr();
    }

    {
        twins::pushAttr(twins::FontAttrib::Italics);
        twins::popAttr();
    }
}

TEST_F(TWINS, logRaw)
{
    twins::logRawBegin("START", true);
    twins::logRawWrite("raw string");
    twins::logRawEnd("END");
}

TEST_F(TWINS, sleep)
{
    twins::sleepMs(10);
}

TEST_F(TWINS, preserveFaint)
{
    const char *s = "";

    // normal
    {
        s = "A" ESC_BOLD "B" ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ(s, getLineBuff());
        clrLineBuff();

        s = "A" ESC_BOLD ESC_NORMAL "B";
        twins::writeStr(s);
        EXPECT_STREQ(s, getLineBuff());
        clrLineBuff();

        s = ESC_BOLD ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ(s, getLineBuff());
        clrLineBuff();

        s = ESC_BOLD "A" ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ(s, getLineBuff());
        clrLineBuff();

        s = "AB";
        twins::writeStr(s);
        EXPECT_STREQ(s, getLineBuff());
        clrLineBuff();
    }

    // fainted
    twins::FontMemento m;
    twins::pushAttr(twins::FontAttrib::Faint);
    clrLineBuff();

    {
        s = "A" ESC_BOLD "B" ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ("AB", getLineBuff());
        clrLineBuff();

        s = "A" ESC_BOLD ESC_NORMAL "B";
        twins::writeStr(s);
        EXPECT_STREQ("AB", getLineBuff());
        clrLineBuff();

        s = ESC_BOLD ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ("", getLineBuff());
        clrLineBuff();

        s = ESC_BOLD "A" ESC_NORMAL;
        twins::writeStr(s);
        EXPECT_STREQ("A", getLineBuff());
        clrLineBuff();

        s = "AB";
        twins::writeStr(s);
        EXPECT_STREQ("AB", getLineBuff());
        clrLineBuff();
    }
}
