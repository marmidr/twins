/******************************************************************************
 * @brief   TWins - colors
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

const char * const mapClFg[] =
{
    "",
    ESC_FG_DEFAULT,
    ESC_FG_BLACK,
    ESC_FG_BLACK_INTENSE,
    ESC_FG_RED,
    ESC_FG_RED_INTENSE,
    ESC_FG_GREEN,
    ESC_FG_GREEN_INTENSE,
    ESC_FG_YELLOW,
    ESC_FG_YELLOW_INTENSE,
    ESC_FG_BLUE,
    ESC_FG_BLUE_INTENSE,
    ESC_FG_MAGENTA,
    ESC_FG_MAGENTA_INTENSE,
    ESC_FG_CYAN,
    ESC_FG_CYAN_INTENSE,
    ESC_FG_WHITE,
    ESC_FG_WHITE_INTENSE
};

const char * const mapClBg[] =
{
    "",
    ESC_BG_DEFAULT,
    ESC_BG_BLACK,
    ESC_BG_BLACK_INTENSE,
    ESC_BG_RED,
    ESC_BG_RED_INTENSE,
    ESC_BG_GREEN,
    ESC_BG_GREEN_INTENSE,
    ESC_BG_YELLOW,
    ESC_BG_YELLOW_INTENSE,
    ESC_BG_BLUE,
    ESC_BG_BLUE_INTENSE,
    ESC_BG_MAGENTA,
    ESC_BG_MAGENTA_INTENSE,
    ESC_BG_CYAN,
    ESC_BG_CYAN_INTENSE,
    ESC_BG_WHITE,
    ESC_BG_WHITE_INTENSE
};

static char clCodeBuffer[20];

// -----------------------------------------------------------------------------

const char* encodeCl(ColorFG cl)
{
    if ((int)cl < arrSize(mapClFg))
        return mapClFg[(int)cl];

    #ifdef TWINS_THEMES
    if (INRANGE(cl, ColorFG::ThemeBegin, ColorFG::ThemeEnd))
        return encodeClTheme(cl);
    #endif

    return "";
}

const char* encodeCl(ColorBG cl)
{
    if ((int)cl < arrSize(mapClBg))
        return mapClBg[(int)cl];

    #ifdef TWINS_THEMES
    if (INRANGE(cl, ColorBG::ThemeBegin, ColorBG::ThemeEnd))
        return encodeClTheme(cl);
    #endif

    return "";
}

const char* transcodeClBg2Fg(const char *bgColorCode)
{
    if (!bgColorCode)
        return "";
    if (*bgColorCode != '\e')
        return bgColorCode;

    strncpy(clCodeBuffer, bgColorCode, sizeof(clCodeBuffer));
    clCodeBuffer[sizeof(clCodeBuffer)-1] = '\0';

    // \e[4?m               --> \e[3?m
    // \e[48;2;000;111;222m --> \e[38;2;000;111;222m
    // \e[48;5;253m         --> \e[38;5;253m
    // \e[10?m              --> \e[9?m
    char c2 = clCodeBuffer[2];
    char c3 = clCodeBuffer[3];

    // lazy, lazy check...
    if (c2 == '4')
    {
        clCodeBuffer[2] = '3';
    }
    else if (c2 == '1' && c3 == '0')
    {
        memmove(clCodeBuffer+3, clCodeBuffer+4, sizeof(clCodeBuffer)-4);
        clCodeBuffer[2] = '9';
    }

    return clCodeBuffer;
}

ColorFG intensifyCl(ColorFG cl)
{
    // normal -> intense
    if (cl > ColorFG::Default && cl < ColorFG::WhiteIntense)
        return ColorFG((int)cl + 1);

    if (cl == ColorFG::Default) // may not be correct
        return ColorFG::WhiteIntense;

    #ifdef TWINS_THEMES
    if (INRANGE(cl, ColorFG::ThemeBegin, ColorFG::ThemeEnd))
        return intensifyClTheme(cl);
    #endif

    return cl;
}

ColorBG intensifyCl(ColorBG cl)
{
    // normal -> intense
    if (cl > ColorBG::Default && cl < ColorBG::WhiteIntense)
        return ColorBG((int)cl + 1);

    if (cl == ColorBG::Default) // may not be correct
        return ColorBG::BlackIntense;

    #ifdef TWINS_THEMES
    if (INRANGE(cl, ColorBG::ThemeBegin, ColorBG::ThemeEnd))
        return intensifyClTheme(cl);
    #endif

    return cl;
}

// -----------------------------------------------------------------------------

} // twins
