/******************************************************************************
 * @brief   TWins - core
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins.hpp"
#include "twins_stack.hpp"
#include "twins_utils.hpp"

#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

// -----------------------------------------------------------------------------

#ifndef TWINS_PRECISE_TIMESTAMP
 #define TWINS_PRECISE_TIMESTAMP    0
#endif

namespace twins
{
struct StubPAL : twins::IPal
{
    int writeChar(char, int16_t) override { return 0; }
    int writeStr(const char *, int16_t) override { return 0; }
    int writeStrLen(const char *, uint16_t) override { return 0; }
    int writeStrVFmt(const char *, va_list) override { return 0; }
    void flushBuff() override {}
    void setLogging(bool) override {}
    void promptPrinted() override {}
    void* memAlloc(uint32_t) override { assert(!"PAL not set"); return nullptr; }
    void memFree(void *) override {}
    void sleep(uint16_t) override {}
    uint16_t getLogsRow() override { return 0; }
    uint32_t getTimeStamp() override { return 0; }
    uint32_t getTimeDiff(uint32_t) override { return 0; }
    bool lock(bool) override { return true; }
    void unlock() override {}
    void wgtDrawBegin(const void *) override {}
    void wgtDrawEnd(const void *) override {}
};

static StubPAL stubPal;

/** @brief Pointer to PAL used internally by TWins */
IPal *pPAL = &stubPal;

/** Local state */
struct TwinsState
{
    /** @brief Current font colors and attributes */
    ColorFG currentClFg = ColorFG::Default;
    ColorBG currentClBg = ColorBG::Default;
    int8_t  attrFaint = 0;

    /** @brief Font colors and attribute stacks */
    Stack<ColorFG> stackClFg;
    Stack<ColorBG> stackClBg;
    Stack<FontAttrib> stackAttr;

    /** logRaw() state */
    FontMementoManual logRawFontMemento;
};

// trick to avoid automatic variable creation/destruction causing calls to uninitialized PAL
static char ts_buff alignas(TwinsState) [sizeof(TwinsState)];
TwinsState& g_ts = (TwinsState&)ts_buff;

// -----------------------------------------------------------------------------

/** */
void FontMementoManual::store()
{
    szFg = g_ts.stackClFg.size();
    szBg = g_ts.stackClBg.size();
    szAttr = g_ts.stackAttr.size();

    // stackClFg.push(currentClFg);
    // stackClBg.push(currentClBg);
}

void FontMementoManual::restore()
{
    popClFg(g_ts.stackClFg.size() - szFg);
    popClBg(g_ts.stackClBg.size() - szBg);
    popAttr(g_ts.stackAttr.size() - szAttr);
}

// -----------------------------------------------------------------------------

extern void widgetInit(void);
extern void widgetDeInit(void);

namespace cli
{
extern void init(void);
extern void deInit(void);
} // cli

void init(IPal *pal)
{
    if (!pal) return;

    pPAL = pal;
    new (&g_ts) TwinsState{};
    widgetInit();
    cli::init();
}

void deinit(void)
{
    if (pPAL == &stubPal) return;
    g_ts.~TwinsState();
    widgetDeInit();
    cli::deInit();
    pPAL = &stubPal;
}

bool lock(bool wait)
{
    return pPAL->lock(wait);
}

void unlock(void)
{
    pPAL->unlock();
}

static inline void setLogging(bool on)
{
    pPAL->setLogging(on);
}

void writeCurrentTime(const uint64_t *pTimestamp)
{
    struct timeval tv;

    if (pTimestamp)
    {
        tv.tv_sec = *pTimestamp >> 16;
        tv.tv_usec = (*pTimestamp) & 0x3FF;
        tv.tv_usec *= 1000;
    }
    else
    {
        gettimeofday(&tv, nullptr);
    }

    struct tm *p_stm = localtime(&tv.tv_sec);

#if TWINS_PRECISE_TIMESTAMP
    tv.tv_usec /= 1000;
    writeStrFmt("[%2d:%02d:%02d.%03d]",
        p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec, tv.tv_usec);
#else
    writeStrFmt("[%2d:%02d:%02d]",
        p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec);
#endif
}

void log(const uint64_t *pTimestamp, const char *file, unsigned line, const char *prefix, const char *fmt, ...)
{
    twins::Locker lck;

    if (!prefix) prefix = "";

    // display only file name, trim the path
    if (const char *delim = strrchr(file, '/'))
        file = delim + 1;

    if (pPAL == &stubPal)
    {
        if (fmt)
        {
            printf(ESC_FG_COLOR(245));
            printf(ESC_COLORS_DEFAULT "%s:%u%s" ESC_BOLD, file, line, prefix);
            printf(ESC_FG_COLOR(253));
            va_list ap;
            va_start(ap, fmt);
            vprintf(fmt, ap);
            va_end(ap);
            printf("" ESC_NORMAL "\n");
            fflush(stdout);
        }
        return;
    }

    FontMemento _m;
    cursorSavePos();
    pushClBg(ColorBG::Default);
    moveTo(1, pPAL->getLogsRow());
    insertLines(1);

    setLogging(true);
    writeStr(ESC_FG_COLOR(245));
    writeCurrentTime(pTimestamp);
    writeStrFmt(" %s:%u%s", file, line, prefix);

    if (strstr(prefix, "-D-"))
        writeStr(ESC_FG_COLOR(248));
    else
        writeStr(ESC_FG_COLOR(253));

    if (fmt)
    {
        va_list ap;
        va_start(ap, fmt);
        writeStrVFmt(fmt, ap);
        va_end(ap);
    }

    setLogging(false);
    cursorRestorePos();
    flushBuffer();
}

void logRawBegin(const char *prologue, bool timeStamp)
{
    g_ts.logRawFontMemento.store();
    cursorSavePos();
    moveTo(1, pPAL->getLogsRow());
    insertLines(1);
    pushClBg(ColorBG::Default);

    setLogging(true);
    writeStr(ESC_FG_COLOR(245));

    if (timeStamp)
        writeCurrentTime();

    writeStr(ESC_FG_COLOR(253));
    writeStr(prologue);
}

void logRawWrite(const char *msg)
{
    writeStr(msg);
}

void logRawEnd(const char *epilogue)
{
    writeStr(epilogue);
    setLogging(false);

    cursorRestorePos();
    g_ts.logRawFontMemento.restore();
    flushBuffer();
}

void sleepMs(uint16_t ms)
{
    pPAL->sleep(ms);
}

int writeChar(char c, int16_t repeat)
{
    return pPAL->writeChar(c, repeat);
}

int writeStr(const char *s, int16_t repeat)
{
    if (!s) return 0;

    if (g_ts.attrFaint)
    {
        int written = 0;
        unsigned sl = strlen(s);

        while (repeat--)
            written += writeStrLen(s, sl);

        return written;
    }
    else
    {
        return pPAL->writeStr(s, repeat);
    }
}

inline uint16_t beginsWith(const char *str, const char *preffix, uint16_t preffixLen)
{
    return (strncmp(str, preffix, preffixLen) == 0) ? preffixLen : 0;
}

int writeStrLen(const char *s, uint16_t sLen)
{
    if (!s) return 0;

    if (g_ts.attrFaint)
    {
        int written = 0;
        const char *ps = s;
        const char *const es = s + sLen;
        const char *esc = util::strechr(ps, es, '\e');

        // \e[1m***\e[0m
        // \e[1m\e[30m***\e[0m
        // ###\e[1m***\e[0m
        // ###\e[0m\e[1m

        while (esc)
        {
            // write text before ESC
            int n = esc - ps;
            pPAL->writeStrLen(ps, n);
            ps += n;
            written += n;

            // weird, but working contruct
            (n = beginsWith(ps, ESC_BOLD, 4)) || (n = beginsWith(ps, ESC_NORMAL, 5));

            if (n)
            {
                // skip bold/normal sequence to preserve faint attribute
                ps += n;
                // find next ESC
                esc = util::strechr(ps, es, '\e');
            }
            else
            {
                // find next ESC
                esc = util::strechr(ps + 1, es, '\e');
            }

            n = esc ? esc - ps : es - ps;
            pPAL->writeStrLen(ps, n);
            ps += n;
            written += n;
        }

        if (ps < es)
        {
            pPAL->writeStrLen(ps, es - ps);
            written += es - ps;
        }

        return written;
    }
    else
    {
        return pPAL->writeStrLen(s, sLen);
    }
}

int writeStrFmt(const char *fmt, ...)
{
    if (!fmt) return 0;

    va_list ap;
    va_start(ap, fmt);
    int n = writeStrVFmt(fmt, ap);
    va_end(ap);
    return n;
}

int writeStrVFmt(const char *fmt, va_list ap)
{
    if (!fmt) return 0;
    return pPAL->writeStrVFmt(fmt, ap);
}

void flushBuffer()
{
    pPAL->flushBuff();
}

// -----------------------------------------------------------------------------

void moveTo(uint16_t col, uint16_t row)
{
    writeStrFmt(ESC_CURSOR_GOTO_FMT, row, col);
}

void moveToCol(uint16_t col)
{
    writeStrFmt(ESC_CURSOR_COLUMN_FMT, col);
}

void moveBy(int16_t cols, int16_t rows)
{
    if (cols < 0)
        writeStrFmt(ESC_CURSOR_BACKWARD_FMT, -cols);
    else if (cols > 0)
        writeStrFmt(ESC_CURSOR_FORWARD_FMT, cols);


    if (rows < 0)
        writeStrFmt(ESC_CURSOR_UP_FMT, -rows);
    else if (rows > 0)
        writeStrFmt(ESC_CURSOR_DOWN_FMT, rows);
}

void mouseMode(MouseMode mode)
{
    switch (mode)
    {
    case MouseMode::Off:
        writeStr(ESC_MOUSE_REPORTING_M1_OFF ESC_MOUSE_REPORTING_M2_OFF);
        break;
    case MouseMode::M1:
        writeStr(ESC_MOUSE_REPORTING_M1_ON);
        break;
    case MouseMode::M2:
        writeStr(ESC_MOUSE_REPORTING_M2_ON);
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------------

void pushClFg(ColorFG cl)
{
    g_ts.stackClFg.push(g_ts.currentClFg);
    g_ts.currentClFg = cl;
    writeStr(encodeCl(g_ts.currentClFg));
}

void popClFg(int n)
{
    while (g_ts.stackClFg.size() && (n-- > 0))
        g_ts.currentClFg = *g_ts.stackClFg.pop();

    writeStr(encodeCl(g_ts.currentClFg));
}

void resetClFg()
{
    g_ts.stackClFg.clear();
    writeStr(ESC_FG_DEFAULT);
}

// -----------------------------------------------------------------------------

void pushClBg(ColorBG cl)
{
    g_ts.stackClBg.push(g_ts.currentClBg);
    g_ts.currentClBg = cl;
    writeStr(encodeCl(g_ts.currentClBg));
}

void popClBg(int n)
{
    while (g_ts.stackClBg.size() && (n-- > 0))
        g_ts.currentClBg = *g_ts.stackClBg.pop();

    writeStr(encodeCl(g_ts.currentClBg));
}

void resetClBg()
{
    g_ts.stackClBg.clear();
    writeStr(ESC_BG_DEFAULT);
}

// -----------------------------------------------------------------------------

void pushAttr(FontAttrib attr)
{
    g_ts.stackAttr.push(attr);

    switch (attr)
    {
    case FontAttrib::Bold:          if (!g_ts.attrFaint) writeStr(ESC_BOLD); break;
    case FontAttrib::Faint:         g_ts.attrFaint++;    writeStr(ESC_FAINT); break;
    case FontAttrib::Italics:       writeStr(ESC_ITALICS_ON); break;
    case FontAttrib::Underline:     writeStr(ESC_UNDERLINE_ON); break;
    case FontAttrib::Blink:         writeStr(ESC_BLINK); break;
    case FontAttrib::Inverse:       writeStr(ESC_INVERSE_ON); break;
    case FontAttrib::Invisible:     writeStr(ESC_INVISIBLE_ON); break;
    case FontAttrib::StrikeThrough: writeStr(ESC_STRIKETHROUGH_ON); break;
    default: break;
    }
}

void popAttr(int n)
{
    while (g_ts.stackAttr.size() && (n-- > 0))
    {
        auto *pAttr = g_ts.stackAttr.pop();

        switch (*pAttr)
        {
        case FontAttrib::Bold:          if (!g_ts.attrFaint)   writeStr(ESC_NORMAL); break;
        case FontAttrib::Faint:         if (!--g_ts.attrFaint) writeStr(ESC_NORMAL); break;
        case FontAttrib::Italics:       writeStr(ESC_ITALICS_OFF); break;
        case FontAttrib::Underline:     writeStr(ESC_UNDERLINE_OFF); break;
        case FontAttrib::Blink:         writeStr(ESC_BLINK_OFF); break;
        case FontAttrib::Inverse:       writeStr(ESC_INVERSE_OFF); break;
        case FontAttrib::Invisible:     writeStr(ESC_INVISIBLE_OFF); break;
        case FontAttrib::StrikeThrough: writeStr(ESC_STRIKETHROUGH_OFF); break;
        default: break;
        }
    }
}

void resetAttr()
{
    g_ts.attrFaint = 0;
    g_ts.stackAttr.clear();
    writeStr(ESC_ATTRIBUTES_DEFAULT);
}

// -----------------------------------------------------------------------------

} // twins
