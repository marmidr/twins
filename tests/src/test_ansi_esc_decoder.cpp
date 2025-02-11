/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins.hpp"

// -----------------------------------------------------------------------------

namespace twins
{
void decodeInputSeqReset();
}

static char rbBuffer[15];


// -----------------------------------------------------------------------------

TEST(ANSI_INPUTDECODER, empty)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    ASSERT_EQ(0, input.size());
    ASSERT_EQ(sizeof(rbBuffer), input.capacity());
    twins::decodeInputSeq(input, kc);

    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_EQ(twins::Key::None, kc.key);
    EXPECT_STREQ("<?>", kc.name);
}

TEST(ANSI_INPUTDECODER, unknown)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\033[1234");
    decodeInputSeq(input, kc);

    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_EQ(twins::Key::None, kc.key);
    EXPECT_STREQ("<?>", kc.name);
}

TEST(ANSI_INPUTDECODER, u8_character)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("Ź");
    decodeInputSeq(input, kc);

    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_STREQ("Ź", kc.utf8);
    EXPECT_STRNE("", kc.name);
    EXPECT_STRNE("<?>", kc.name);
}

TEST(ANSI_INPUTDECODER, EscFollowedByEsc)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    // write and decode single ESC - first attempt shall be ignored, waiting for sequence data
    input.write((char)twins::Ansi::ESC);
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::None, kc.key);

    // write second ESC
    input.write((char)twins::Ansi::ESC);
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Esc, kc.key);
    EXPECT_EQ(KEY_MOD_SPECIAL, kc.mod_all);
}

TEST(ANSI_INPUTDECODER, EscFollowedByNothing)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    // write and decode single ESC - first attempt shall be ignored, waiting for sequence data
    input.write((char)twins::Ansi::ESC);
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::None, kc.key);

    // second attempt to decode the same buffer - shall output ESC code
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Esc, kc.key);
}

TEST(ANSI_INPUTDECODER, Ctrl_S)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write((char)0x13);
    decodeInputSeq(input, kc);

    EXPECT_EQ(KEY_MOD_CTRL, kc.mod_all);
    EXPECT_STREQ("S", kc.utf8);
    EXPECT_STRNE("", kc.name);
    EXPECT_STRNE("<?>", kc.name);
}

TEST(ANSI_INPUTDECODER, Ctrl_F1)
{
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\033[1;5H");
    decodeInputSeq(input, kc);

    EXPECT_EQ(KEY_MOD_SPECIAL | KEY_MOD_CTRL, kc.mod_all);
    EXPECT_EQ(twins::Key::Home, kc.key);
    EXPECT_STRNE("", kc.name);
    EXPECT_STRNE("<?>", kc.name);
}

TEST(ANSI_INPUTDECODER, UnknownSeq__Ctrl_Home)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\033*42~");
    input.write("\033[1;5H@");

    decodeInputSeq(input, kc);
    EXPECT_EQ(KEY_MOD_SPECIAL | KEY_MOD_CTRL, kc.mod_all);
    EXPECT_EQ(twins::Key::Home, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_STREQ("@", kc.utf8);
}

TEST(ANSI_INPUTDECODER, LoongUnknownSeq__Ctrl_Home)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    // next ESC is more that 7 bytes further,
    // so entire buffer will be cleared
    input.write("\033*123456789~");
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::None, kc.key);

    input.write("\033[1;5H");
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::None, kc.key);

    input.write("+");
    decodeInputSeq(input, kc); // 3rd try - abandon
    EXPECT_EQ(twins::Key::None, kc.key);
    EXPECT_EQ(0, input.size());
}

TEST(ANSI_INPUTDECODER, NUL_InInput)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write('\0');
    input.write("\t");
    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::None, kc.key);
}

TEST(ANSI_INPUTDECODER, DISABLED_Ctrl_F1__incomplete)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    EXPECT_TRUE(input.write("\033["));
    EXPECT_EQ(2, input.size());
    decodeInputSeq(input, kc);

    EXPECT_EQ(2, input.size());
    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_EQ(twins::Key::None, kc.key);

    // write rest of previous sequence and additional one key
    EXPECT_TRUE(input.write("1;5H\033"));
    EXPECT_EQ(7, input.size());
    decodeInputSeq(input, kc);
    EXPECT_EQ(1, input.size());
    EXPECT_EQ(KEY_MOD_SPECIAL | KEY_MOD_CTRL, kc.mod_all);
    EXPECT_EQ(twins::Key::Home, kc.key);

    // decode rest of the input
    decodeInputSeq(input, kc);
    EXPECT_EQ(0, input.size());
    EXPECT_EQ(KEY_MOD_SPECIAL, kc.mod_all);
    EXPECT_EQ(twins::Key::Esc, kc.key);
}

TEST(ANSI_INPUTDECODER, L__S_C_UP__O)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    // write rest of previous sequence and additional one key
    EXPECT_TRUE(input.write("Ł\033[1;6AÓ*"));

    decodeInputSeq(input, kc);
    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_STREQ("Ł", kc.utf8);

    decodeInputSeq(input, kc);
    EXPECT_EQ(KEY_MOD_SPECIAL | KEY_MOD_SHIFT | KEY_MOD_CTRL, kc.mod_all);
    EXPECT_EQ(twins::Key::Up, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(KEY_MOD_NONE, kc.mod_all);
    EXPECT_STREQ("Ó", kc.utf8);

    // remains '*'
    EXPECT_EQ(1, input.size());
}

TEST(ANSI_INPUTDECODER, CR)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\r\r\t");

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Tab, kc.key);
}

TEST(ANSI_INPUTDECODER, LF)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\n\n\t");

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Tab, kc.key);
}

TEST(ANSI_INPUTDECODER, CR_LF_CR)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\n\r\n\t\n\r\t");

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Tab, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Enter, kc.key);

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::Tab, kc.key);
}

TEST(ANSI_INPUTDECODER, Mouse_click_at_11)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\e[M !!");

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::MouseEvent, kc.key);
    EXPECT_EQ(twins::MouseBtn::ButtonLeft, kc.mouse.btn);
    EXPECT_EQ(1, kc.mouse.col);
    EXPECT_EQ(1, kc.mouse.row);
    EXPECT_EQ(0, kc.mod_all);
}

TEST(ANSI_INPUTDECODER, Mouse_wheel_down)
{
    twins::decodeInputSeqReset();
    twins::RingBuff<char> input(rbBuffer);
    twins::KeyCode kc;

    input.write("\e[Ma$\"");

    decodeInputSeq(input, kc);
    EXPECT_EQ(twins::Key::MouseEvent, kc.key);
    EXPECT_EQ(twins::MouseBtn::WheelDown, kc.mouse.btn);
    EXPECT_EQ(4, kc.mouse.col);
    EXPECT_EQ(2, kc.mouse.row);
    EXPECT_EQ(0, kc.mod_all);
}
