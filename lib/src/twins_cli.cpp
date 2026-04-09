/******************************************************************************
 * @brief   TWins - command line interface
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins.hpp"
#include "twins_cli.hpp"
#include "twins_ringbuffer.hpp"
#include "twins_queue.hpp"

#include <string.h>

// -----------------------------------------------------------------------------

#ifndef TWINS_CLI_MAXCMDLEN
# define TWINS_CLI_MAXCMDLEN    120
#endif

#ifndef TWINS_CLI_MAXHIST
# define TWINS_CLI_MAXHIST      30
#endif

#define CRLF                    "\r\n"

static_assert(TWINS_CLI_MAXCMDLEN > 5);
static_assert(TWINS_CLI_MAXHIST > 0);

namespace twins::cli
{

struct CliState
{
    using Passwords = twins::Vector<twins::SecurePassw>;

    CliState() {}
    ~CliState() {}
    void resetPasswValue() { pPasswordValue = &mEmptyPassword; }

    String          lineBuff;
    History         history;
    int16_t         cursorPos = 0;
    RingBuff<char>  seqRingBuff;
    Queue<String>   cmdQue;
    CmdHandler      overrideHandler;
    //
    Passwords           passwords;
    String              passwordMatchPrompt;
    bool                passwordEntryMode = {};
    const SecurePassw*  pPasswordValue = &mEmptyPassword;

private:
    const SecurePassw mEmptyPassword;
};

// trick to avoid automatic variable creation/destruction causing calls to uninitialized PAL
static char cs_buff alignas(CliState) [sizeof(CliState)];
CliState& g_cs = (CliState&)cs_buff;

// global variables
bool verbose = true;
bool echoNlAfterCr = false;
uint8_t accessFlags = 0;

// private code

/// @brief get the command name (before '|' if present)
static CStrView getCmdName(const char *name);

// -----------------------------------------------------------------------------

void init(void)
{
    new (&g_cs) CliState{};
}

void deInit(void)
{
    g_cs.~CliState();
}

static inline bool streq(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

// -----------------------------------------------------------------------------

void reset(void)
{
    g_cs.seqRingBuff.clear();
    g_cs.lineBuff.clear();
    g_cs.cursorPos = 0;
    g_cs.history.reset();
}

void passwordSet(const std::initializer_list<twins::SecurePassw> &passwords, const char *onPasswordMatchPrompt)
{
    g_cs.passwords = passwords;
    g_cs.passwordMatchPrompt = onPasswordMatchPrompt;
    g_cs.resetPasswValue();

    if (g_cs.passwords.size() == 0)
        g_cs.passwordEntryMode = false;
}

void passwordModeEnable(bool en)
{
    g_cs.passwordEntryMode = en;
}


bool passwordModeIsEnabled()
{
    return g_cs.passwordEntryMode;
}

const twins::SecurePassw& passwordValue()
{
    return *g_cs.pPasswordValue;
}

void processInput(const Cmd *pCommands, const char* data, uint8_t dataLen)
{
    if (!data)
        data = "";

    if (!dataLen)
        dataLen = strlen(data);

    if (g_cs.seqRingBuff.capacity() == 0)
        g_cs.seqRingBuff.init(ESC_SEQ_MAX_LENGTH+2);

    while (dataLen)
    {
        uint8_t to_write = dataLen > ESC_SEQ_MAX_LENGTH ? ESC_SEQ_MAX_LENGTH : dataLen;

        g_cs.seqRingBuff.write(data, to_write);
        processInput(pCommands, g_cs.seqRingBuff);

        data += to_write;
        dataLen -= to_write;
    }
}

void processInput(const Cmd *pCommands, twins::RingBuff<char> &rb)
{
    char seq[ESC_SEQ_MAX_LENGTH];

    while (true)
    {
        // copy bytes from ringbufer to linear local array capable of storing the longest ANSI sequence
        rb.copy(seq, sizeof(seq));

        KeyCode kc = {};
        uint8_t seq_sz = decodeInputSeq(rb, kc);

        if (kc.key == Key::None)
            break;

        if (seq_sz >= ESC_SEQ_MAX_LENGTH)
            seq_sz = ESC_SEQ_MAX_LENGTH-1;

        seq[seq_sz] = '\0';
        const char *p_seq = seq; // echo decoded sequence

        if (kc.m_spec || kc.m_ctrl)
        {
            // fprintf(stderr, ESC_FG_RED "^" ESC_FG_DEFAULT); fflush(stderr);

            switch (kc.key)
            {
            case Key::Up:
            case Key::Down:
                // history inactive in password mode
                if (g_cs.history.items.size() && !g_cs.passwordEntryMode)
                {
                    g_cs.history.idx += kc.key == Key::Up ? -1 : 1;

                    if (g_cs.history.idx < 0)
                        g_cs.history.idx = 0;
                    else if (g_cs.history.idx >= (int)g_cs.history.items.size())
                        g_cs.history.idx = g_cs.history.items.size()-1;

                    moveBy(-(int16_t)g_cs.lineBuff.u8len(), 0);
                    writeStr(ESC_LINE_ERASE_RIGHT);
                    g_cs.lineBuff = g_cs.history.items[g_cs.history.idx];
                    g_cs.cursorPos = g_cs.lineBuff.u8len();
                    writeStrLen(g_cs.lineBuff.cstr(), g_cs.lineBuff.size());
                }
                p_seq = nullptr; // suppress echo
                break;
            case Key::Left:
                if (g_cs.cursorPos > 0)
                    g_cs.cursorPos--;
                else
                    p_seq = nullptr; // suppress echo
                break;
            case Key::Right:
                if (g_cs.cursorPos < (signed)g_cs.lineBuff.u8len())
                    g_cs.cursorPos++;
                else
                    p_seq = nullptr; // suppress echo
                break;
            case Key::Home:
                moveBy(-g_cs.cursorPos, 0);
                g_cs.cursorPos = 0;
                p_seq = nullptr; // suppress echo
                break;
            case Key::End:
                moveBy(-g_cs.cursorPos, 0);
                g_cs.cursorPos = g_cs.lineBuff.u8len();
                moveBy(g_cs.cursorPos, 0);
                p_seq = nullptr; // suppress echo
                break;
            case Key::Delete:
                if (g_cs.cursorPos >= 0)
                {
                    if (kc.m_ctrl)
                    {
                        g_cs.lineBuff.trim(g_cs.cursorPos);
                        writeStr(ESC_LINE_ERASE_RIGHT);
                    }
                    else
                    {
                        g_cs.lineBuff.erase(g_cs.cursorPos);
                        writeStr(ESC_CHAR_DELETE(1));
                    }
                }
                p_seq = nullptr; // suppress echo
                break;
            case Key::Backspace:
                if (g_cs.cursorPos > 0)
                {
                    if (kc.m_ctrl)
                    {
                        g_cs.lineBuff.erase(0, g_cs.cursorPos);
                        moveBy(-g_cs.cursorPos, 0);
                        writeStrFmt(ESC_CHAR_DELETE_FMT, g_cs.cursorPos);
                        g_cs.cursorPos = 0;
                    }
                    else
                    {
                        g_cs.lineBuff.erase(g_cs.cursorPos-1);
                        g_cs.cursorPos--;

                        moveBy(-1, 0);
                        writeStr(ESC_CHAR_DELETE(1));
                    }
                }
                p_seq = nullptr; // suppress echo
                break;
            case Key::Tab:
                // auto complete - only if at least one character was typed
                if (g_cs.lineBuff.size() > 0 && pCommands)
                {
                    autocompleteCommand(pCommands, g_cs.lineBuff.cstr());
                }
                p_seq = nullptr; // suppress echo
                break;
            case Key::Enter:
                if (g_cs.lineBuff.size())
                {
                    // ensure cursor moves to new line when Enter was hit, but single \r was received
                    if (echoNlAfterCr) twins::writeChar('\n');

                    // append to history, limit history size;
                    //   prevent storing a password in the history
                    if (!g_cs.passwordEntryMode)
                    {
                        int idx = 0;
                        if (auto *str = g_cs.history.items.find(g_cs.lineBuff, &idx))
                        {
                            // move to top
                            String tmp = std::move(*str);
                            g_cs.history.items.remove(idx, true);
                            g_cs.history.items.append(std::move(tmp));
                        }
                        else
                        {
                            g_cs.history.items.append(g_cs.lineBuff);
                            if (g_cs.history.items.size() > TWINS_CLI_MAXHIST)
                                g_cs.history.items.remove(0, true);
                        }
                    }
                    g_cs.history.idx = g_cs.history.items.size();
                    g_cs.cmdQue.write(std::move(g_cs.lineBuff));
                    g_cs.lineBuff.clear();
                }
                else
                {
                    if (g_cs.passwordEntryMode)
                    {
                        g_cs.passwordEntryMode = false;
                        writeStr(CRLF ESC_FG_RED_INTENSE);
                        writeStr("Password cannot be empty.");
                        writeStr(ESC_FG_DEFAULT);
                    }
                    prompt(true);
                }
                p_seq = nullptr; // suppress echo
                g_cs.cursorPos = 0;
                break;
            default:
                break;
            }

            // echo
            if (p_seq)
                writeStrLen(p_seq, seq_sz);
        }
        else
        {
            // process non-control inputs (letters etc)
            if (g_cs.lineBuff.size() < TWINS_CLI_MAXCMDLEN)
            {
                g_cs.lineBuff.insert(g_cs.cursorPos, kc.utf8);
                g_cs.cursorPos += 1;

                // echo received character; in password mode, replace it with '*'
                if (g_cs.passwordEntryMode)
                {
                    p_seq = "*";
                    seq_sz = 1;
                }
                writeStr(ESC_CHAR_INSERT(1));
                writeStrLen(p_seq, seq_sz);
            }
            else
            {
                // line length limit exceeded
                writeStr(ESC_BELL);
            }
        }
    }

    flushBuffer();
}

History& getHistory(void)
{
    return g_cs.history;
}

bool autocompleteCommand(const Cmd *pCommands, const char *prefix)
{
    assert(pCommands);

    if (!prefix || !*prefix)
        return false;

    const uint16_t prefix_len = strlen(prefix);
    const Cmd* p_first_match = nullptr;
    uint16_t match_count = 0;

    // First pass: count matches and find the first one
    auto check_commands = [&](const Cmd* pCmd)
    {
        while (pCmd->name)
        {
            if (pCmd->name[0] != '\0')
            {
                CStrView cmd_name = getCmdName(pCmd->name);

                // check if command starts with prefix
                if ((cmd_name.size >= prefix_len) && strncmp(cmd_name.data, prefix, prefix_len) == 0)
                {
                    match_count++;
                    if (!p_first_match)
                        p_first_match = pCmd;
                }
            }
            pCmd++;
        }
    };

    constexpr Cmd internal_cmds[] = { Cmd{"help"}, Cmd{"hist"}, Cmd{} };
    check_commands(pCommands);
    check_commands(internal_cmds);

    // If exactly one match -> autocomplete the command name
    if (match_count == 1 && p_first_match)
    {
        CStrView cmd_name = getCmdName(p_first_match->name);
        uint16_t remaining_len = cmd_name.size - prefix_len;

        if (remaining_len > 0)
        {
            // append remaining part of the matched command
            g_cs.lineBuff.appendLen(cmd_name.data + prefix_len, remaining_len);
            g_cs.cursorPos += remaining_len;
            writeStrLen(cmd_name.data + prefix_len, remaining_len);
        }

        // append space to confirm that the cmd was found
        g_cs.lineBuff.append(" ");
        g_cs.cursorPos++;
        writeStr(" ");
        flushBuffer();
        return true;
    }
    else if (match_count > 1)
    {
        // If multiple matches, display them
        auto print_cmds = [&](const Cmd *pCmd)
        {
            while (pCmd->name)
            {
                if (pCmd->name[0] != '\0')
                {
                    CStrView cmd_name = getCmdName(pCmd->name);

                    if (cmd_name.size >= prefix_len && strncmp(cmd_name.data, prefix, prefix_len) == 0)
                    {
                        writeStr(" ");
                        writeStrLen(cmd_name.data, cmd_name.size);
                    }
                }
                pCmd++;
            }
        };

        writeStr("... ");
        print_cmds(pCommands);
        print_cmds(internal_cmds);
        prompt();
        flushBuffer();
        g_cs.lineBuff.clear();
        g_cs.cursorPos = 0;
        return true;
    }

    // nothing found
    writeStr(ESC_BELL);
    flushBuffer();
    return false;
}

void printHelp(Argv &argv, const Cmd* pCommands)
{
    const char *pSubCmdHelp = argv.size() > 1 ? argv[1] : nullptr;
    bool subCmdFound = false;

    if (!pSubCmdHelp)
    {
        writeStr(
            ESC_BOLD "help" ESC_NORMAL
            " <cmd>" CRLF
            "    this help" CRLF
        );
        writeStr(
            ESC_BOLD "hist" ESC_NORMAL CRLF
            "           commands history" CRLF
            "    --clr  clear the history" CRLF
        );
    }

    while (pCommands->name)
    {
        // default cmd has empty name
        if (pCommands->name[0] != '\0')
        {
            if (pSubCmdHelp)
            {
                // get the command name end in case of 'cmd|alias'
                CStrView cmd_name = getCmdName(pCommands->name);

                // help for this single command
                if (strncmp(pSubCmdHelp, cmd_name.data, cmd_name.size) != 0)
                {
                    pCommands++;
                    continue;
                }
            }

            writeStr(ESC_BOLD);
            writeStr(pCommands->name);
            pCommands->access > 0 ? writeStr(ESC_NORMAL " " TWINS_CLI_LOCK_SYMBOL " ") : writeStr(ESC_NORMAL " ");
            writeStr(pCommands->help);
            writeStr(CRLF);
            flushBuffer();

            if (pSubCmdHelp)
            {
                subCmdFound = true;
                // asked for this single command - we can quit the loop
                break;
            }
        }

        pCommands++;
    }

    if (pSubCmdHelp && !subCmdFound)
    {
        writeStrFmt("command '%s' not found" CRLF, pSubCmdHelp);
    }
}

void printHistory()
{
    int i = 1;
    for (const auto &s : g_cs.history.items)
        writeStrFmt("%2d. %s" CRLF, i++, s.cstr());
    flushBuffer();
}

void tokenize(StringBuff &cmd, Argv &argv)
{
    char *p = cmd.data();
    // fputs(">>", stderr);
    // fputs(p, stderr);
    // fputs("<<\n", stderr);

    // skip leading spaces
    while (*p == ' ')
        p++;

    while (*p)
    {
        // support for quoted arguments, eg: `cmd -n "Bob Walker"`
        if (*p == '"')
        {
            p++;
            argv.append(p);

            while (*p && *p != '"')
                p++;

            if (!*p) // end of command line ?
                break;

            // erase closing quote
            *p = '\0';
            p++;
        }
        else
        {
            argv.append(p);
        }

        // search for separator
        while (*p && *p != ' ')
            p++;

        if (!*p) // end of command line ?
            break;

        *p = '\0'; // mark end of argument
        p++;

        // skip extra spaces
        while (*p == ' ')
            p++;
    }

    if (verbose)
    {
        // debug:
        writeStr(ESC_ITALICS_ON ESC_FG_BLACK_INTENSE "Command: ");
        for (const char *a : argv)
            writeStrFmt("\'%s\' ", a);
        writeStr(ESC_ITALICS_OFF ESC_FG_DEFAULT CRLF);
        flushBuffer();
    }
}

const Cmd* findCmdHandler(const Cmd* pCommands, Argv &argv)
{
    const Cmd* p_dflt = nullptr;

    if (argv.size())
    {
        const char *entered_cmd_name = argv[0];

        for (; pCommands->name; pCommands++)
        {
            // name
            // name|alias
            const char *name = pCommands->name;

            if (*name == '\0')
            {
                p_dflt = pCommands;
                continue;
            }

            // get the command name end in case of 'cmd|alias'
            CStrView cmd_name = getCmdName(name);
            const char *ename = name + cmd_name.size;

            const size_t entered_cmd_len = strlen(entered_cmd_name);
            // avoid matching 'moveee' for 'move' command
            if ((size_t)cmd_name.size == entered_cmd_len)
            {
                if (strncmp(name, entered_cmd_name, entered_cmd_len) == 0)
                    return pCommands;
            }

            if (*ename == '|')
            {
                name = ename+1;
                if (strcmp(name, entered_cmd_name) == 0)
                    return pCommands;
            }
        }
    }

    return p_dflt ? p_dflt : nullptr;
}

void prompt(bool newLn)
{
    if (newLn) writeStr(CRLF);
    writeStr(ESC_FG_GREEN_INTENSE "> " ESC_FG_WHITE_INTENSE);
    pPAL->promptPrinted();
}

bool checkAccessGranted(uint8_t cmdAccessFlags, bool promptAccDenied, bool enterPasswMode)
{
    if (cmdAccessFlags == 0)
        return true;

    if ((cmdAccessFlags & accessFlags) != 0)
        return true;

    if (promptAccDenied)
    {
        writeStr(ESC_FG_RED_INTENSE);
        writeStr("Access denied - enter the password: ");
        writeStr(ESC_FG_DEFAULT);
        flushBuffer();
        g_cs.passwordEntryMode |= enterPasswMode;
    }

    return false;
}

bool checkAndExec(const Cmd* pCommands, bool lastCommandSet)
{
    assert(pCommands);
    if (g_cs.cmdQue.size() == 0)
        return false;

    // drop empty command string
    if (g_cs.cmdQue.front()->size() == 0)
    {
        g_cs.cmdQue.read();
        return false;
    }

    if (g_cs.overrideHandler)
        lastCommandSet = true;

    StringBuff cmd;
    if (lastCommandSet)
        cmd = g_cs.cmdQue.read();
    else
        cmd = *g_cs.cmdQue.front();

    writeStr(ESC_FG_DEFAULT);

    if (!g_cs.overrideHandler)
    {
        if (g_cs.passwordEntryMode)
        {
            bool passw_ok = false;
            for (const auto &psw : g_cs.passwords)
            {
                if (psw == cmd.cstr())
                {
                    passw_ok = true;
                    g_cs.pPasswordValue = &psw;
                    break;
                }
            }

            g_cs.passwordEntryMode = false;

            if (passw_ok)
            {
                writeStr(ESC_FG_GREEN_INTENSE);
                writeStr("Access granted." CRLF);
                writeStr(ESC_FG_DEFAULT);
                writeStr(g_cs.passwordMatchPrompt.cstr());
                prompt(true);
                flushBuffer();

                g_cs.cmdQue.read();
                return true;
            }
            else
            {
                g_cs.resetPasswValue();

                // writeStr(CRLF);
                writeStr(ESC_FG_RED_INTENSE);
                writeStr("Incorrect password - access denied.");
                writeStr(ESC_FG_DEFAULT);
                prompt(true);
                flushBuffer();

                g_cs.cmdQue.read();
                return true;
            }
        }
    }

    Argv argv;
    tokenize(cmd, argv);
    bool cmd_executed = false;

    if (g_cs.overrideHandler)
    {
        g_cs.overrideHandler(argv);
        cmd_executed = true;
    }
    else if (argv.size() > 0 && streq(argv[0], "hist"))
    {
        if (argv.size() > 1)
        {
            if (streq(argv[1], "--clr"))
            {
                g_cs.history.reset();
            }
            else
            {
                writeStr("unknown argument" CRLF);
            }
        }
        else
        {
            printHistory();
            prompt(false);
            flushBuffer();
            g_cs.cmdQue.read();
            return true;
        }
    }
    else if (argv.size() > 0 && streq(argv[0], "help"))
    {
        printHelp(argv, pCommands);
        prompt(false);
        flushBuffer();
        g_cs.cmdQue.read();
        return true;
    }
    else if (const auto *p_cmd = findCmdHandler(pCommands, argv))
    {
        if (checkAccessGranted(p_cmd->access, true, true))
        {
            p_cmd->handler(argv);
            cmd_executed = true;
            if (!lastCommandSet)
                g_cs.cmdQue.read();
        }
    }
    else
    {
        if (lastCommandSet)
            writeStr("unknown command - type 'help' for available commands" CRLF);
    }

    if (lastCommandSet)
        prompt(false);

    flushBuffer();
    return cmd_executed;
}

bool execLine(const Cmd* pCommands, const char *cmdline)
{
    assert(cmdline);
    assert(pCommands);

    Argv argv;
    StringBuff cmd(cmdline);
    tokenize(cmd, argv);
    bool found = false;

    if (const auto *p_cmd = findCmdHandler(pCommands, argv))
    {
        found = true;
        p_cmd->handler(argv);
    }
    else
    {
        writeStr("unknown command" CRLF);
    }

    prompt(false);
    flushBuffer();
    return found;
}

void setOverrideHandler(CmdHandler handler)
{
    g_cs.overrideHandler = std::move(handler);
}

static CStrView getCmdName(const char *name)
{
    if (!name)
        return {};

    const char *ename = strchr(name, '|');
    if (!ename)
        ename = name + strlen(name);

    return CStrView{name, static_cast<unsigned>(ename - name)};
}

// -----------------------------------------------------------------------------

} // twins::cli
