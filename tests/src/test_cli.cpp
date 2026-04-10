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

#define CMD_ACCESS_ALL          0x00
#define CMD_ACCESS_RESTRICTED   0x02

TEST_F(CLI, commands)
{
    static bool ver_called;
    static bool default_called;
    static char move_dir;
    static bool do_restart;
    static const twins::cli::Cmd *p_commands = nullptr; // to solve lack of lambda captures

    ver_called = false;
    default_called = false;
    move_dir = 0;
    do_restart = false;

    const twins::cli::Cmd commands[] =
    {
        {
            "",
            "default cmd handler",
            // uses a default TWINS_CLI_ACCESS_UNRESTRICTED
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
            CMD_ACCESS_ALL,
            TWINS_CLI_HANDLER
            {
                ver_called = true;
            }
        },
        #endif
        {
            "call_ver",
            "    call 'V'",
            CMD_ACCESS_ALL,
            TWINS_CLI_HANDLER
            {
                assert(p_commands);
                twins::cli::execLine(p_commands, "    V  ");
            }
        },
        {
            "move",
            "<up/dn/home>" "\r\n"
            "    Perform a move",
            CMD_ACCESS_ALL,
            TWINS_CLI_HANDLER
            {
                if (argv.size() >= 2)
                    move_dir = *argv[1];
            }
        },
        {
            "restart",
            "",
            CMD_ACCESS_RESTRICTED,
            TWINS_CLI_HANDLER
            {
                do_restart = true;
            }
        },
        { /* terminator */ }
    };

    twins::cli::processInput(commands, "ver" "\r\n");
    twins::cli::checkAndExec(commands);
    twins::cli::processInput(commands, "move up" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(ver_called);
    EXPECT_EQ('u', move_dir);

    // known command, but extra chars
    move_dir = 0;
    default_called = false;
    twins::cli::processInput(commands, "moveEEE up" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(default_called);
    EXPECT_EQ(0, move_dir);

    // test for alias
    ver_called = false;
    twins::cli::processInput(commands, "V" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(ver_called);

    // test for call
    p_commands = commands;
    ver_called = false;
    twins::cli::processInput(commands, "call_ver" "\r\n");
    twins::cli::checkAndExec(commands);
    EXPECT_TRUE(ver_called);

    // print history
    twins::cli::processInput(commands, "hist" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // print help
    twins::cli::processInput(commands, "help" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // print help of given command
    twins::cli::processInput(commands, "help ver" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // unknown cmd
    default_called = false;
    twins::cli::processInput(commands, "say-ello\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(default_called);

    // restricted command
    {
        twins::cli::processInput(commands, "restart" "\r\n");
        EXPECT_FALSE(twins::cli::checkAndExec(commands));
        EXPECT_FALSE(do_restart);
        EXPECT_TRUE(twins::cli::passwordModeIsEnabled());

        // disable password mode and set the access flag manually
        twins::cli::passwordModeEnable(false);
        twins::cli::accessFlags = CMD_ACCESS_RESTRICTED;
        twins::cli::processInput(commands, "restart" "\r\n");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(do_restart);
    }

    // get a history
    EXPECT_GE(twins::cli::getHistory().items.size(), 2);
    twins::cli::reset();
    EXPECT_EQ(twins::cli::getHistory().items.size(), 0);
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
    twins::cli::processInput(commands, "\r\n");
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // put something to history
    twins::cli::processInput(commands, "HELLO\r\n");
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // put wrong command and modify it as in terminal
    {
        twins::cli::reset();
        twins::cli::processInput(commands, "HERO");
        // left
        twins::cli::processInput(commands, "\e[D");
        twins::cli::processInput(commands, "\e[D");
        // del R
        twins::cli::processInput(commands, "\e[3~");
        // insert LL
        twins::cli::processInput(commands, "LL");
        // right
        twins::cli::processInput(commands, "\e[C");
        // append !
        twins::cli::processInput(commands, "!");
        // home
        twins::cli::processInput(commands, "\e[H");
        twins::cli::processInput(commands, "*");
        // end
        twins::cli::processInput(commands, "\e[F");
        twins::cli::processInput(commands, "##");
        // backspace
        twins::cli::processInput(commands, "\x7F"); // Ansi::DEL == Backspace
        // and run it
        twins::cli::processInput(commands, "\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
    }

    // no command ready
    EXPECT_FALSE(twins::cli::checkAndExec(commands));

    // up - recall from history
    twins::cli::processInput(commands, "\e[A");
    twins::cli::processInput(commands, "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));

    // down - recall from history
    twins::cli::processInput(commands, "\e[B");
    twins::cli::processInput(commands, "\r");
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
    twins::cli::processInput(commands, "name" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_TRUE(args_value.empty());

    // arg ok
    args_value.clear();
    twins::cli::processInput(commands, "name   Tiamat \"Heaven Of High\" -s TFG" "\r\n");
    EXPECT_TRUE(twins::cli::checkAndExec(commands));
    EXPECT_STREQ(args_value.c_str(), "Tiamat - Heaven Of High");

    // missing closing quote
    args_value.clear();
    twins::cli::processInput(commands, " name Therion \"Clavicula 🔱 Nox" "\r\n");
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
    twins::cli::processInput(commands, "nae");
    // left
    twins::cli::processInput(commands, "\e"); // shall not be interpreted as ESC
    twins::cli::processInput(commands, "[D"); // now give "Left"
    // insert missing 'm' and "Enter"
    twins::cli::processInput(commands, "m\r");
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
    twins::cli::processInput(commands, "push 1\r" "size\r" "pop\r ");
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
        twins::cli::processInput(commands, "push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_FALSE(push_called);
        EXPECT_TRUE(temporary_called);

        push_called = false;
        temporary_called = false;
        twins::cli::processInput(commands, "any weird command\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(temporary_called);

        // unregister override manually
        push_called = false;
        temporary_called = false;
        twins::cli::setOverrideHandler({});
        twins::cli::processInput(commands, "push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(push_called);
    }

    // single call override
    {
        push_called = false;
        temporary_called = false;

        // register override
        twins::cli::setOverrideHandler(temporary_handler);
        twins::cli::processInput(commands, "unregister\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_FALSE(push_called);
        EXPECT_TRUE(temporary_called);

        // second call - handler from commands expected to be called
        push_called = false;
        temporary_called = false;
        twins::cli::processInput(commands, "push 1\r");
        EXPECT_TRUE(twins::cli::checkAndExec(commands));
        EXPECT_TRUE(push_called);
        EXPECT_FALSE(temporary_called);
    }
}

TEST_F(CLI, password_mode)
{
    // initializer_list<SecurePassw>
    {
        EXPECT_FALSE(twins::cli::passwordValue().hasPassword());

        twins::cli::passwordModeEnable(true);
        EXPECT_TRUE(twins::cli::passwordModeIsEnabled());

        twins::cli::passwordSet({});
        EXPECT_FALSE(twins::cli::passwordModeIsEnabled());
    }

    // initializer_list<SecurePassw>
    {
        const twins::cli::Cmd commands[] = { { /* terminator */ } };

        {
            twins::SecurePassw pswQwert{"qwerty"};
            twins::SecurePassw pswPo{"po"};
            twins::SecurePassw pswLelum{"lelum"};

            twins::cli::passwordSet({pswQwert, pswPo, pswLelum});
            twins::cli::passwordModeEnable(true);
            EXPECT_TRUE(twins::cli::passwordModeIsEnabled());

            // wrong password
            twins::cli::processInput(commands, "X\r");
            EXPECT_TRUE(twins::cli::checkAndExec(commands));
            EXPECT_FALSE(twins::cli::passwordModeIsEnabled());
            EXPECT_FALSE(twins::cli::passwordValue().hasPassword());

            // correct password
            twins::cli::processInput(commands, "qwerty\r");
            twins::cli::passwordModeEnable(true);
            EXPECT_TRUE(twins::cli::checkAndExec(commands));
            EXPECT_FALSE(twins::cli::passwordModeIsEnabled());
            EXPECT_TRUE(twins::cli::passwordValue() == pswQwert);

            // destroy the original password variable
            pswQwert = {};
        }

        // if the provided password is stored as value (not as a reference),
        // this test shall pass
        EXPECT_TRUE(twins::cli::passwordValue() == "qwerty");
    }
}

#pragma GCC diagnostic pop // ignored "-Wunused-parameters"
