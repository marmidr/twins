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
# define TWINS_CLI_MAXHIST      20
#endif

static_assert(TWINS_CLI_MAXCMDLEN > 5);
static_assert(TWINS_CLI_MAXHIST > 0);

namespace twins::cli
{

struct CliState
{
    String          lineBuff;
    History         history;
    int16_t         cursorPos = 0;
    int16_t         historyIdx = 0;
    RingBuff<char>  seqRingBuff;
    Queue<String>   cmdQue;
    CmdHandler      overrideHandler;
    String          password = {};
    bool            passwordMode  = {};
};

// trick to avoid automatic variable creation/destruction causing calls to uninitialized PAL
static char cs_buff alignas(CliState) [sizeof(CliState)];
CliState& g_cs = (CliState&)cs_buff;

// global variables
bool verbose = true;
bool echoNlAfterCr = false;

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
    g_cs.history.clear();
    g_cs.historyIdx = 0;
}

void setPassword(String pw)
{
    g_cs.password = std::move(pw);
    g_cs.passwordMode = g_cs.password.size() > 0;
}

bool passwordModeActive()
{
    return g_cs.passwordMode;
}

void processInput(const char* data, uint8_t dataLen)
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
        processInput(g_cs.seqRingBuff);

        data += to_write;
        dataLen -= to_write;
    }
}

void processInput(twins::RingBuff<char> &rb)
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
                if (g_cs.history.size() && !g_cs.passwordMode)
                {
                    g_cs.historyIdx += kc.key == Key::Up ? -1 : 1;

                    if (g_cs.historyIdx < 0)
                        g_cs.historyIdx = 0;
                    else if (g_cs.historyIdx >= (int)g_cs.history.size())
                        g_cs.historyIdx = g_cs.history.size()-1;

                    moveBy(-(int16_t)g_cs.lineBuff.u8len(), 0);
                    writeStr(ESC_LINE_ERASE_RIGHT);
                    g_cs.lineBuff = g_cs.history[g_cs.historyIdx];
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
                // auto complete
                p_seq = nullptr; // suppress echo
                break;
            case Key::Enter:
                if (g_cs.lineBuff.size())
                {
                    // ensure cursor moves to new line when Enter was hit, but single \r was received
                    if (echoNlAfterCr) twins::writeChar('\n');

                    // append to history, limit history size;
                    // prevents password to be stored in history
                    if (!g_cs.passwordMode)
                    {
                        int idx = 0;
                        if (auto *str = g_cs.history.find(g_cs.lineBuff, &idx))
                        {
                            // move to top
                            String tmp = std::move(*str);
                            g_cs.history.remove(idx, true);
                            g_cs.history.append(std::move(tmp));
                        }
                        else
                        {
                            g_cs.history.append(g_cs.lineBuff);
                            if (g_cs.history.size() > TWINS_CLI_MAXHIST)
                                g_cs.history.remove(0, true);
                        }
                    }
                    g_cs.historyIdx = g_cs.history.size();
                    g_cs.cmdQue.write(std::move(g_cs.lineBuff));
                    g_cs.lineBuff.clear();
                }
                else
                {
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
                if (g_cs.passwordMode)
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

void printHelp(Argv &argv, const Cmd* pCommands)
{
    const char *pSubCmdHelp = argv.size() > 1 ? argv[1] : nullptr;
    bool subCmdFound = false;

    if (!pSubCmdHelp)
    {
        writeStr(ESC_BOLD "help" ESC_NORMAL " <cmd>" "\r\n" "    this help" "\r\n");
        writeStr(ESC_BOLD "hist" ESC_NORMAL "\r\n" "    commands history" "\r\n");
    }

    while (pCommands->name)
    {
        // default cmd has empty name
        if (pCommands->name[0] != '\0')
        {
            if (pSubCmdHelp)
            {
                // help for this single command
                if (!streq(pSubCmdHelp, pCommands->name))
                {
                    pCommands++;
                    continue;
                }
            }

            writeStr(ESC_BOLD);
            writeStr(pCommands->name);
            writeStr(ESC_NORMAL " ");
            writeStr(pCommands->help);
            writeStr("\r\n");
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
        writeStrFmt("command '%s' not found" "\r\n", pSubCmdHelp);
    }
}

void printHistory()
{
    int i = 1;
    for (const auto &s : g_cs.history)
        writeStrFmt("%2d. %s\r\n", i++, s.cstr());
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
        writeStr(ESC_ITALICS_OFF ESC_FG_DEFAULT "\r\n");
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
            const char *ename = strchr(name, '|');
            if (!ename)
                ename = name + strlen(name);

            const size_t entered_cmd_len = strlen(entered_cmd_name);
            // avoid matching 'moveee' for 'move' command
            if ((size_t)(ename - name) == entered_cmd_len)
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
    if (newLn) writeStr("\r\n");
    writeStr(ESC_FG_GREEN_INTENSE "> " ESC_FG_WHITE_INTENSE);
    pPAL->promptPrinted();
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
        if (g_cs.passwordMode)
        {
            if (cmd == g_cs.password)
            {
                g_cs.passwordMode  = false;
                // writeStr("\r\n");
                writeStr(ESC_FG_GREEN_INTENSE);
                writeStr("Access granted." "\r\n");
                writeStr(ESC_FG_DEFAULT);
                writeStr("CLI interface ready; type 'help' for available commands.");
                prompt(true);
                flushBuffer();
                g_cs.cmdQue.read();
                return true;
            }
            else
            {
                // writeStr("\r\n");
                writeStr(ESC_FG_RED_INTENSE);
                writeStr("Incorrect password, access denied.");
                writeStr(ESC_FG_DEFAULT);
                prompt(true);
                flushBuffer();
                g_cs.cmdQue.read();
                return true;
            }
        }

        if (cmd == "hist")
        {
            printHistory();
            prompt(false);
            flushBuffer();
            g_cs.cmdQue.read();
            return true;
        }
    }

    Argv argv;
    tokenize(cmd, argv);
    bool found = false;

    if (g_cs.overrideHandler)
    {
        g_cs.overrideHandler(argv);
        found = true;
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
        p_cmd->handler(argv);
        found = true;
        if (!lastCommandSet)
            g_cs.cmdQue.read();
    }
    else
    {
        if (lastCommandSet)
            writeStr("unknown command - type 'help' for available commands" "\r\n");
    }

    if (lastCommandSet)
        prompt(false);

    flushBuffer();
    return found;
}

bool execLine(const char *cmdline, const Cmd* pCommands)
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
        writeStr("unknown command" "\r\n");
    }

    prompt(false);
    flushBuffer();
    return found;
}

void setOverrideHandler(CmdHandler handler)
{
    g_cs.overrideHandler = std::move(handler);
}

// -----------------------------------------------------------------------------

} // twins::cli
