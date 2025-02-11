/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_cli.hpp"
#include <string>

// -----------------------------------------------------------------------------

class CLI : public testing::Test
{
protected:
    void SetUp() override
    {
        twins::cli::reset();
    }

    void TearDown() override
    {
    }
};

// -----------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"


TEST_F(CLI, commands)
{
    static bool ver_called;
    static bool default_called;
    static char move_dir;
    static const twins::cli::Cmd *p_commands = nullptr; // to solve lack of lambda captures

    ver_called = false;
    default_called = false;
    move_dir = 0;

    const twins::cli::Cmd commands[] =
    {
        {
            "",
            "default cmd handler",
            TWINS_CLI_HANDLER
            {
                default_called = true;
            }
        },
        #if TWINS_LAMBDA_CMD
        {
            "ver|V",
            "    Show SW version; alias 'V'",
            [&vcalled = ver_called](twins::cli::Argv &argv)
            {
                // with full lambda, captures are possible but costs more
                vcalled = true;
            }
        },
        #else
        {
            "ver|V",
            "    Show SW version; alias 'V'",
            TWINS_CLI_HANDLER
            {
                ver_called = true;
            }
        },
        #endif
        {
            "call_ver",
            "    call 'V'",
            TWINS_CLI_HANDLER
            {
                assert(p_commands);
                twins::cli::execLine("    V  ", p_commands);
            }
        },
        {
            "move",
            "<up/dn/home>" "\r\n"
            "    Perform a move",
            TWINS_CLI_HANDLER
            {
                if (argv.size() >= 2)
                    move_dir = *argv[1];
            }
        },
        { /* terminator */ }
    };

    twins::cli::processInput("ver" "\r\n");
    twins::cli::checkAndExec(commands);
    twins::cli::processInput("move up" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(ver_called);
    EXPECT_EQ('u', move_dir);

    // known command, but extra chars
    move_dir = 0;
    default_called = false;
    twins::cli::processInput("moveEEE up" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(default_called);
    EXPECT_EQ(0, move_dir);

    // test for alias
    ver_called = false;
    twins::cli::processInput("V" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(ver_called);

    // test for call
    p_commands = commands;
    ver_called = false;
    twins::cli::processInput("call_ver" "\r\n");
    twins::cli::checkAndExec(commands);
    EXPECT_TRUE(ver_called);

    // print history
    twins::cli::processInput("hist" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // print help
    twins::cli::processInput("help" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // print help of given command
    twins::cli::processInput("help ver" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // unknown cmd
    default_called = false;
    twins::cli::processInput("say-ello\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(default_called);

    // get history
    EXPECT_GE(twins::cli::getHistory().size(), 2);
    twins::cli::reset();
    EXPECT_EQ(twins::cli::getHistory().size(), 0);
}

TEST_F(CLI, control_codes)
{
    const twins::cli::Cmd commands[] =
    {
        {
            "*HELLO!#",
            "",
            TWINS_CLI_HANDLER {}
        },
        { /* terminator */ }
    };

    // empty command
    twins::cli::processInput("\r\n");
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // put something to history
    twins::cli::processInput("HELLO\r\n");
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // put wrong command and modify it as in terminal
    {
        twins::cli::reset();
        twins::cli::processInput("HERO");
        // left
        twins::cli::processInput("\e[D");
        twins::cli::processInput("\e[D");
        // del R
        twins::cli::processInput("\e[3~");
        // insert LL
        twins::cli::processInput("LL");
        // right
        twins::cli::processInput("\e[C");
        // append !
        twins::cli::processInput("!");
        // home
        twins::cli::processInput("\e[H");
        twins::cli::processInput("*");
        // end
        twins::cli::processInput("\e[F");
        twins::cli::processInput("##");
        // backspace
        twins::cli::processInput("\x7F"); // Ansi::DEL == Backspace
        // and run it
        twins::cli::processInput("\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
    }

    // no command ready
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // up - recall from history
    twins::cli::processInput("\e[A");
    twins::cli::processInput("\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // down - recall from history
    twins::cli::processInput("\e[B");
    twins::cli::processInput("\r");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
}

static std::string args_value;

TEST_F(CLI, quoted_args)
{
    const twins::cli::Cmd commands[] =
    {
        {
            "name",
            "  <\"first name\">",
            TWINS_CLI_HANDLER
            {
                if (argv.size() >= 3)
                {
                    args_value =  argv[1];
                    args_value += " - ";
                    args_value += argv[2];
                }
            }
        },
        { /* terminator */ }
    };

    // no arg
    args_value.clear();
    twins::cli::processInput("name" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(args_value.empty());

    // arg ok
    args_value.clear();
    twins::cli::processInput("name   Tiamat \"Heaven Of High\" -s TFG" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_STREQ(args_value.c_str(), "Tiamat - Heaven Of High");

    // missing closing quote
    args_value.clear();
    twins::cli::processInput(" name Therion \"Clavicula 🔱 Nox" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_STREQ(args_value.c_str(), "Therion - Clavicula 🔱 Nox");

    // double
    // args_value.clear();
    // twins::cli::process("nasib \"\"The Mawarannahr\"\"" "\r\n");
    // EXPECT_TRUE(twins::cli::checkAndExec(commands));
    // EXPECT_STREQ(args_value.c_str(), "nasib \"The Mawarannahr\"");
}

TEST_F(CLI, sliced_esc)
{
    const twins::cli::Cmd commands[] =
    {
        {
            "name",
            "",
            TWINS_CLI_HANDLER { }
        },
        { /* terminator */ }
    };

    // incomplete command
    twins::cli::processInput("nae");
    // left
    twins::cli::processInput("\e"); // shall not be interpreted as ESC
    twins::cli::processInput("[D"); // now give "Left"
    // insert missing 'm' and "Enter"
    twins::cli::processInput("m\r");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
}

TEST_F(CLI, many_cmds_in_buffer)
{
    static bool push_called, pop_called, size_called;

    const twins::cli::Cmd commands[] =
    {
        {
            "push", "", TWINS_CLI_HANDLER { push_called = true; }
        },
        {
            "pop", "", TWINS_CLI_HANDLER { pop_called = true; }
        },
        {
            "size", "", TWINS_CLI_HANDLER { size_called = true; }
        },
        { /* terminator */ }
    };

    push_called = pop_called = size_called = false;

    // push a few commands before checking
    twins::cli::processInput("push 1\r" "size\r" "pop\r ");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(push_called);
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(size_called);
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(pop_called);
    EXPECT_FALSE(twins::cli::checkAndExec(commands));
}

TEST_F(CLI, override_handler)
{
    static bool push_called;

    const twins::cli::Cmd commands[] =
    {
        {
            "push", "", TWINS_CLI_HANDLER { push_called = true; }
        },
        { /* terminator */ }
    };

    bool temporary_called = false;

    // warning: holds reference to a local variable - must be unregistered before function ends;
    // use shared_ptr<> to avoid problems
    auto temporary_handler = [&temporary_called](twins::cli::Argv &argv)
    {
        temporary_called = true;

        // handler unregisters itself
        if (argv.size() && (twins::String(argv[0]) == "unregister"))
            twins::cli::setOverrideHandler({});
    };

    // lifetime override
    {
        push_called = false;
        temporary_called = false;

        // register override
        twins::cli::setOverrideHandler(temporary_handler);
        twins::cli::processInput("push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_FALSE(push_called);
        EXPECT_TRUE(temporary_called);

        push_called = false;
        temporary_called = false;
        twins::cli::processInput("any weird command\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(temporary_called);

        // unregister override manually
        push_called = false;
        temporary_called = false;
        twins::cli::setOverrideHandler({});
        twins::cli::processInput("push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(push_called);
    }

    // single call override
    {
        push_called = false;
        temporary_called = false;

        // register override
        twins::cli::setOverrideHandler(temporary_handler);
        twins::cli::processInput("unregister\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_FALSE(push_called);
        EXPECT_TRUE(temporary_called);

        // second call - handler from commands expected to be called
        push_called = false;
        temporary_called = false;
        twins::cli::processInput("push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(push_called);
        EXPECT_FALSE(temporary_called);
    }
}

#pragma GCC diagnostic pop // ignored "-Wunused-parameters"
