/******************************************************************************
 * @brief   TWins - colors
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
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

// -----------------------------------------------------------------------------

const char* encodeCl(ColorFG cl)
{
    if ((int)cl < arrSize(mapClFg))
        return mapClFg[(int)cl];

    #ifdef TWINS_THEME
    if (INRANGE(cl, ColorFG::ThemeBegin, ColorFG::ThemeEnd))
        return encodeClTheme(cl);
    #endif

    return "";
}

const char* encodeCl(ColorBG cl)
{
    if ((int)cl < arrSize(mapClBg))
        return mapClBg[(int)cl];

    #ifdef TWINS_THEME
    if (INRANGE(cl, ColorBG::ThemeBegin, ColorBG::ThemeEnd))
        return encodeClTheme(cl);
    #endif

    return "";
}

ColorFG intenseCl(ColorFG cl)
{
    // normal -> intense
    if (cl > ColorFG::Default && cl < ColorFG::WhiteIntense)
        return ColorFG((int)cl + 1);

    if (cl == ColorFG::Default) // may not be correct
        return ColorFG::WhiteIntense;

    #ifdef TWINS_THEME
    if (INRANGE(cl, ColorFG::ThemeBegin, ColorFG::ThemeEnd))
        return intenseClTheme(cl);
    #endif

    return cl;
}

ColorBG intenseCl(ColorBG cl)
{
    // normal -> intense
    if (cl > ColorBG::Default && cl < ColorBG::WhiteIntense)
        return ColorBG((int)cl + 1);

    if (cl == ColorBG::Default) // may not be correct
        return ColorBG::BlackIntense;

    #ifdef TWINS_THEME
    if (INRANGE(cl, ColorBG::ThemeBegin, ColorBG::ThemeEnd))
        return intenseClTheme(cl);
    #endif

    return cl;
}

}