/******************************************************************************
 * @brief   TWins - core
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#include "twins.hpp"
#include "twins_stack.hpp"

#include <string.h>
#include <stdio.h>
#include <time.h>

// -----------------------------------------------------------------------------

namespace twins
{

/** @brief Pointer to PAL used internally by TWins */
IPal *pPAL;

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
static char ts_buff[sizeof(TwinsState)] alignas(TwinsState);
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

extern void widgetInit();
extern void widgetDeInit();

void init(IPal *pal)
{
    if (!pal) return;

    pPAL = pal;
    new (&g_ts) TwinsState{};
    widgetInit();
}

void deinit(void)
{
    g_ts.~TwinsState();
    widgetDeInit();
    pPAL = nullptr;
}

IPal& pal(void)
{
    assert(pPAL);
    return *pPAL;
}

bool lock(bool wait)
{
    if (pPAL)
        return pPAL->lock(wait);
    return true;
}

void unlock(void)
{
    if (pPAL)
        pPAL->unlock();
}

void log(const char *file, const char *func, unsigned line, const char *fmt, ...)
{
    twins::Locker lck;

    // display only file name, trim the path
    if (const char *delim = strrchr(file, '/'))
        file = delim + 1;

    if (!pPAL)
    {
        if (fmt)
        {
            printf(ESC_COLORS_DEFAULT "%s:%u: " ESC_BOLD, file, line);
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
    // pushClFg(ColorFG::White);
    writeStr(ESC_FG_COLOR(245));

    uint16_t row = pPAL->getLogsRow();
    moveTo(1, row);
    insertLines(1);

    time_t t = time(NULL);
    struct tm *p_stm = localtime(&t);
    // writeStrFmt("[%2d:%02d:%02d] %s() %s:%u: ",
    //     p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec,
    //     func, file, line);
    writeStrFmt("[%2d:%02d:%02d] %s:%u: ",
        p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec,
        file, line);

    // pushClFg(ColorFG::WhiteIntense);
    writeStr(ESC_FG_COLOR(253));

    if (fmt)
    {
        va_list ap;
        va_start(ap, fmt);
        writeStrVFmt(fmt, ap);
        va_end(ap);
    }

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
    // pushClFg(ColorFG::White);
    writeStr(ESC_FG_COLOR(245));

    if (timeStamp)
    {
        time_t t = time(NULL);
        struct tm *p_stm = localtime(&t);
        writeStrFmt("[%2d:%02d:%02d] ",
            p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec);
    }

    // pushClFg(ColorFG::WhiteIntense);
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
    cursorRestorePos();
    g_ts.logRawFontMemento.restore();
    flushBuffer();
}

void sleepMs(uint16_t ms)
{
    if (pPAL)
        pPAL->sleep(ms);
}

int writeChar(char c, int16_t repeat)
{
    return pPAL ? pPAL->writeChar(c, repeat) : 0;
}

int writeStr(const char *s, int16_t repeat)
{
    return pPAL ? pPAL->writeStr(s, repeat) : 0;
}

int writeStrLen(const char *s, uint16_t sLen)
{
    return pPAL ? pPAL->writeStrLen(s, sLen) : 0;
}

int writeStrFmt(const char *fmt, ...)
{
    if (!(fmt && pPAL))
        return 0;

    va_list ap;
    va_start(ap, fmt);
    int n = pPAL->writeStrVFmt(fmt, ap);
    va_end(ap);
    return n;
}

int writeStrVFmt(const char *fmt, va_list ap)
{
    return pPAL ? pPAL->writeStrVFmt(fmt, ap) : 0;
}

void flushBuffer()
{
    if (pPAL)
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
    while (g_ts.stackClFg.size() && n-- > 0)
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
    while (g_ts.stackClBg.size() && n-- > 0)
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
    while (g_ts.stackAttr.size() && n-- > 0)
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

}
