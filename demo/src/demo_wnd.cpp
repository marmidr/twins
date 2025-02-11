/******************************************************************************
 * @brief   TWins - demo window definition
 * @author  Mariusz Midor
 *****************************************************************************/

#include "twins_transform_window.hpp"
#include "demo_wnd.hpp"

// -----------------------------------------------------------------------------

// Theme support enabled - user must provide theme colors
namespace twins
{

// RGB color values: https://en.wikipedia.org/wiki/Web_colors

const char* encodeClTheme(ColorFG cl)
{
    switch (cl)
    {
    // double-state colors
    case ColorFG::Checkbox:         return ESC_FG_YELLOW;
    case ColorFG::CheckboxIntense:  return ESC_FG_YELLOW_INTENSE;
    case ColorFG::Radio:            return ESC_FG_GREEN;
    case ColorFG::RadioIntense:     return ESC_FG_GREEN_INTENSE;
    // one-state colors
    case ColorFG::Window:           return ESC_FG_COLOR(158);
    case ColorFG::Label:            return ESC_FG_WHITE;
    case ColorFG::Listbox:          return ESC_FG_GREEN;
    case ColorFG::Button:           return ESC_FG_BLACK;
    case ColorFG::ButtonGreen:      return ESC_FG_WHITE;
    case ColorFG::ButtonRed:        return ESC_FG_WHITE;
    case ColorFG::ButtonOrange:     return ESC_FG_DarkRed;
    case ColorFG::PanelChbox:       return ESC_FG_MediumBlue;
    default:                        return ESC_FG_DEFAULT;
    }
}

const char* encodeClTheme(ColorBG cl)
{
    switch (cl)
    {
    case ColorBG::Window:           return ESC_BG_MidnightBlue;
    case ColorBG::Listbox:          return ESC_BG_WHITE;
    case ColorBG::Button:           return ESC_BG_BLACK_INTENSE;
    case ColorBG::ButtonGreen:      return ESC_BG_OliveDrab;
    case ColorBG::ButtonRed:        return ESC_BG_RED;
    case ColorBG::ButtonOrange:     return ESC_BG_Orange;
    case ColorBG::PanelChbox:       return ESC_BG_Gainsboro;
    case ColorBG::PanelVer:         return ESC_BG_COLOR(106);
    case ColorBG::PanelKeyCodes:    return ESC_BG_COLOR(169);
    case ColorBG::PanelLeds:        return ESC_BG_LightBlue;
    case ColorBG::LabelBlue:        return ESC_BG_DarkBlue;
    case ColorBG::Edit1:            return ESC_BG_CYAN;
    case ColorBG::Edit1Intense:     return ESC_BG_CYAN_INTENSE;
    case ColorBG::Edit2:            return ESC_BG_GREEN;
    case ColorBG::Edit2Intense:     return ESC_BG_GREEN_INTENSE;
    case ColorBG::LabelFtr:         return ESC_BG_Navy;
    default:                        return ESC_BG_DEFAULT;
    }
}

ColorFG intensifyClTheme(ColorFG cl)
{
    switch (cl)
    {
    case ColorFG::Checkbox:         return ColorFG::CheckboxIntense;
    case ColorFG::Radio:            return ColorFG::RadioIntense;
    default:                        return cl;
    }
}

ColorBG intensifyClTheme(ColorBG cl)
{
    switch (cl)
    {
    case ColorBG::Edit1:            return ColorBG::Edit1Intense;
    case ColorBG::Edit2:            return ColorBG::Edit2Intense;
    default:                        return cl;
    }
}

}

// -----------------------------------------------------------------------------

static constexpr twins::Widget pnlStateChildren[] =
{
    {
        type    : twins::Widget::Led,
        id      : ID_LED_BATTERY,
        coord   : { 2, 1 },
        size    : {},
        { led : {
            text        : "(BATT)",
            fgColor     : twins::ColorFG::Black,
            bgColorOff  : twins::ColorBG::White,
            bgColorOn   : twins::ColorBG::Magenta,
        }}
    },
    {
        type    : twins::Widget::Led,
        id      : ID_LED_LOCK,
        coord   : { 9, 1 },
        size    : {},
        { led : {
            text        : "(LOCK)",
            fgColor     : twins::ColorFG::Black,
            bgColorOff  : twins::ColorBG::White,
            bgColorOn   : twins::ColorBG::Green,
        }}
    },
    {
        type    : twins::Widget::Led,
        id      : ID_LED_PUMP,
        coord   : { 16, 1 },
        size    : {},
        { led : {
            text        : "(PUMP)",
            fgColor     : twins::ColorFG::Black,
            bgColorOff  : twins::ColorBG::White,
            bgColorOn   : twins::ColorBG::Yellow,
        }}
    },
    // at the end of child's array must lay terminating element;
    // it is needed by window transformation routine and ignored in final definition
    { /* NUL */ }
};

static constexpr twins::Widget pageVersionChildren[] =
{
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_VERSIONS,
        coord   : { 1, 1 },
        size    : { 21, 5 },
        { panel : {
            title       : "VER 🍁",
            fgColor     : twins::ColorFG::White,
            bgColor     : twins::ColorBG::PanelVer,
        }},
        link    : { (const twins::Widget[]) // set first field in union - pChildren
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_FW_VERSION,
                coord   : { 2, 1 },
                size    : {}, //{ 12, 1 },
                { label : {
                    text    : "FwVer: 1.1",
                    fgColor : twins::ColorFG::YellowIntense,
                }}
            },
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_DATE,
                coord   : { 2, 2 },
                size    : { 15, 1 },
                { label : {
                    text    : "Date•" __DATE__,
                    fgColor : twins::ColorFG::Black,
                }}
            },
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_ABOUT,
                coord   : { 2, 3 },
                size    : { 0, 1 },
                { label : {
                    text    : {},
                    fgColor : twins::ColorFG::Blue,
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_STATE,
        coord   : { 30, 1 },
        size    : { 25, 3 },
        { panel : {
            title       : "STATE: Leds",
            fgColor     : twins::ColorFG::Blue,
            bgColor     : twins::ColorBG::PanelLeds,
        }},
        link    : { pnlStateChildren }
    },
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_KEY,
        coord   : { 1, 7 },
        size    : { 26, 4 },
        { panel : {
            title       : "KEY-CODES",
            fgColor     : twins::ColorFG::White,
            bgColor     : twins::ColorBG::PanelKeyCodes,
        }},
        link    : { (const twins::Widget[])
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_KEYSEQ,
                coord   : { 2, 1 },
                size    : { 22, 1 },
                { label : {
                    text    : nullptr, // use callback to get text
                    fgColor : twins::ColorFG::White,
                }}
            },
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_KEYNAME,
                coord   : { 2, 2 },
                size    : { 17, 1 },
                { label : {
                    text    : nullptr, // use callback to get text
                    fgColor : twins::ColorFG::White,
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::CheckBox,
        id      : ID_CHBX_ENBL,
        coord   : { 30, 5 },
        size    : {},
        { checkbox : {
            text    : "Enable",
            fgColor : twins::ColorFG::Green
        }}
    },
    {
        type    : twins::Widget::CheckBox,
        id      : ID_CHBX_LOCK,
        coord   : { 45, 5 },
        size    : {},
        { checkbox : {
            text    : "Lock"
        }}
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_YES,
        coord   : { 30, 7 },
        size    : {},
        { button : {
            text    : "YES",
            fgColor : twins::ColorFG::ButtonGreen,
            bgColor : twins::ColorBG::ButtonGreen,
            style   : twins::ButtonStyle::Solid
        }}
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_NO,
        coord   : { 38, 7 },
        size    : {},
        { button : {
            text    : "NO",
            fgColor : twins::ColorFG::ButtonOrange,
            bgColor : twins::ColorBG::ButtonOrange,
            style   : twins::ButtonStyle::Solid
        }}
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_POPUP,
        coord   : { 45, 7 },
        size    : {},
        { button : {
            text    : "POPUP",
            fgColor : twins::ColorFG::White,
            bgColor : {},
            style   : twins::ButtonStyle::Simple
        }}
    },
    {
        type    : twins::Widget::ProgressBar,
        id      : ID_PRGBAR_1,
        coord   : { 30, 9 },
        size    : { 25, 1 },
        { progressbar : {
            fgColor : twins::ColorFG::Yellow,
            style   : twins::PgBarStyle::Hash
        }}
    },
    {
        type    : twins::Widget::ProgressBar,
        id      : ID_PRGBAR_2,
        coord   : { 30, 10 },
        size    : { 12, 1 },
        { progressbar : {
            fgColor : twins::ColorFG::White,
            style   : twins::PgBarStyle::Shade
        }}
    },
    {
        type    : twins::Widget::ProgressBar,
        id      : ID_PRGBAR_3,
        coord   : { 43, 10 },
        size    : { 12, 1 },
        { progressbar : {
            fgColor : twins::ColorFG::White,
            style   : twins::PgBarStyle::Rectangle
        }}
    },
    { /* NUL */ }
};

static constexpr twins::Widget pageServiceChildren[] =
{
    {
        type    : twins::Widget::Layer,
        id      : ID_LAYER_1,
        coord   : {},
        size    : {},
        { layer : {} },
        link    : { (const twins::Widget[])
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LABEL_MULTI_FMT,
                coord   : { 24, 2 },
                size    : { 35, 4 },
                { label : {
                    text    : "  ▫▫▫▫▫ " ESC_INVERSE_ON "ListBox" ESC_INVERSE_OFF " ▫▫▫▫▫" "\n"
                            "• " ESC_UNDERLINE_ON "Up/Down" ESC_UNDERLINE_OFF " -> change item" "\n"
                            "• " ESC_UNDERLINE_ON "PgUp/PgDown" ESC_UNDERLINE_OFF " -> scroll page" "\n"
                            "• " ESC_UNDERLINE_ON "Enter" ESC_UNDERLINE_OFF " -> select the item",
                    fgColor : twins::ColorFG::YellowIntense,
                    bgColor : twins::ColorBG::LabelBlue,
                }}
            },
            {
                type    : twins::Widget::ListBox,
                id      : ID_LISTBOX,
                coord   : { 2, 2 },
                size    : { 20, 8 },
                { listbox : {
                    fgColor : twins::ColorFG::Green,
                    bgColor : twins::ColorBG::White,
                    noFrame : false
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::Layer,
        id      : ID_LAYER_2,
        coord   : {},
        size    : {},
        { layer : {} },
        link    : { (const twins::Widget[])
        {
            {
                type    : twins::Widget::Radio,
                id      : ID_RADIO_1,
                coord   : { 25, 7 },
                size    : {},
                { radio : {
                    text    : "YES",
                    fgColor : twins::ColorFG::Radio,
                    groupId : 1,
                    radioId : 0,
                }}
            },
            {
                type    : twins::Widget::Radio,
                id      : ID_RADIO_2,
                coord   : { 35, 7 },
                size    : {},
                { radio : {
                    text    : "NO",
                    fgColor : twins::ColorFG::Yellow,
                    groupId : 1,
                    radioId : 1,
                }}
            },
            {
                type    : twins::Widget::Radio,
                id      : ID_RADIO_3,
                coord   : { 44, 7 },
                size    : {},
                { radio : {
                    text    : "Don't know",
                    fgColor : {},
                    groupId : 1,
                    radioId : 2,
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::CheckBox,
        id      : ID_CHBX_L1,
        coord   : { 25, 9 },
        size    : {},
        { checkbox : {
            text    : "Layer 1",
            fgColor : {}
        }}
    },
    {
        type    : twins::Widget::CheckBox,
        id      : ID_CHBX_L2,
        coord   : { 40, 9 },
        size    : {},
        { checkbox : {
            text    : "Layer 2",
            fgColor : {}
        }}
    },
    { /* NUL */ }
};

static constexpr twins::Widget pageDiagnosticsChildren[] =
{
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_EDT,
        coord   : { 2, 1 },
        size    : { 32, 5 },
        { panel : {
            title       : {},
            fgColor     : twins::ColorFG::White,
            bgColor     : twins::ColorBG::White,
            noFrame     : true
        }},
        link    : { (const twins::Widget[]) // set first field in union - pChildren
        {
            {
                type    : twins::Widget::TextEdit,
                id      : ID_EDT_1,
                coord   : { 1, 1 },
                size    : { 30, 1 },
                { textedit : {
                    fgColor     : twins::ColorFG::Black,
                    bgColor     : twins::ColorBG::Edit1,
                }}
            },
            {
                type    : twins::Widget::TextEdit,
                id      : ID_EDT_2,
                coord   : { 1, 3 },
                size    : { 30, 1 },
                { textedit : {
                    fgColor     : twins::ColorFG::Black,
                    bgColor     : twins::ColorBG::Edit2,
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::CustomWgt,
        id      : ID_CUSTOMWGT1,
        coord   : { 2, 6 },
        size    : { 32, 4 },
    },
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_CHBX,
        coord   : { 36, 1 },
        size    : { 22, 10 },
        { panel : {
            title       : {},
            fgColor     : twins::ColorFG::PanelChbox,
            bgColor     : twins::ColorBG::PanelChbox,
        }},
        link    : { (const twins::Widget[]) // set first field in union - pChildren
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LBL_CHBXTITLE,
                coord   : { 2, 2 },
                size    : { 14, 1 },
                { label : {
                    text    : ESC_BOLD "Check list:" ESC_NORMAL,
                    fgColor : twins::ColorFG::Blue,
                }}
            },
            {
                type    : twins::Widget::CheckBox,
                id      : ID_CHBX_A,
                coord   : { 2, 4 },
                size    : {},
                { checkbox : {
                    text    : "Check A ",
                    fgColor : twins::ColorFG::Green
                }}
            },
            {
                type    : twins::Widget::CheckBox,
                id      : ID_CHBX_B,
                coord   : { 2, 5 },
                size    : {},
                { checkbox : {
                    text    : "Check B "
                }}
            },
            {
                type    : twins::Widget::CheckBox,
                id      : ID_CHBX_C,
                coord   : { 2, 6 },
                size    : {},
                { checkbox : {
                    text    : "Check C "
                }}
            },
            {
                type    : twins::Widget::CheckBox,
                id      : ID_CHBX_D,
                coord   : { 2, 7 },
                size    : {},
                { checkbox : {
                    text    : "Check D "
                }}
            },

            { /* NUL */ }
        }}
    },
    { /* NUL */ }
};

static constexpr twins::Widget pageInactiveChildren[] =
{
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_EMPTY_1,
        coord   : { 5, 1 },
        size    : { 20, 10 },
        { panel : {
            title   : "Word-wrap",
            fgColor : {},
            bgColor : {},
        }},
        link    : { (const twins::Widget[])
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LBL_WORDWRAP,
                coord   : { 2, 1 },
                size    : { 16, 6 },
                { label : {
                    text    : {},
                    fgColor : twins::ColorFG::White,
                    bgColor : twins::ColorBG::Blue
                }}
            },
            {
                type    : twins::Widget::Button,
                id      : ID_BTN_NOACTION,
                coord   : { 5, 8 },
                size    : {},
                { button : {
                    text    : "...",
                    fgColor : twins::ColorFG::White,
                    bgColor : {},
                    style   : twins::ButtonStyle::Simple
                }}
            },
            { /* NUL */ }
        }}
    },
    {
        type    : twins::Widget::Panel,
        id      : ID_PANEL_EMPTY_2,
        coord   : { 40, 1 },
        size    : { 12, 10 },
        { panel : {
            title   : "...",
            fgColor : {},
            bgColor : {},
        }},
        link    : { (const twins::Widget[])
        {
            {
                type    : twins::Widget::Label,
                id      : ID_LBL_EMPTY_2,
                coord   : { 2, 2 },
                size    : { 5, 1 },
                { label : {
                    text    : "---",
                }}
            },
            { /* NUL */ }
        }}
    },
    { /* NUL */ }
};

static constexpr twins::Widget pageTextboxChildren[] =
{
    {
        type    : twins::Widget::TextBox,
        id      : ID_TBX_LOREMIPSUM,
        coord   : { 3, 1 },
        size    : { 40, 10 },
        { textbox : {
            fgColor : twins::ColorFG::White,
            bgColor : {},
        }},
    },
    {
        type    : twins::Widget::TextBox,
        id      : ID_TBX_1LINE,
        coord   : { 46, 1 },
        size    : { 12, 10 },
        { textbox : {
            fgColor : twins::ColorFG::White,
            bgColor : twins::ColorBG::Blue,
        }},
    },
    { /* NUL */ }
};

static constexpr twins::Widget pageComboboxChildren[] =
{
    {
        type    : twins::Widget::ComboBox,
        id      : ID_CBX_OPTIONS,
        coord   : { 10, 2 },
        size    : { 20, 1 },
        { combobox : {
            fgColor     : twins::ColorFG::Blue,
            bgColor     : twins::ColorBG::White,
            dropDownSize: 4
        }},
    },
    {
        type    : twins::Widget::ComboBox,
        id      : ID_CBX_COLORS,
        coord   : { 8, 4 },
        size    : { 24, 1 },
        { combobox : {
            fgColor     : twins::ColorFG::GreenIntense,
            bgColor     : twins::ColorBG::Black,
            dropDownSize: 4
        }},
    },
    {
        type    : twins::Widget::ListBox,
        id      : ID_LBX_UNDEROPTIONS,
        coord   : { 5, 6 },
        size    : { 30, 5 },
        { listbox : {
        }},
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_SAYYES,
        coord   : { 38, 2 },
        size    : {},
        { button : {
            text    : "Say YES",
            fgColor : twins::ColorFG::White,
            bgColor : twins::ColorBG::ButtonGreen,
            style   : twins::ButtonStyle::Simple
        }}
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_SAYNO,
        coord   : { 38, 4 },
        size    : {},
        { button : {
            text    : "Say NO",
            fgColor : twins::ColorFG::White,
            bgColor : twins::ColorBG::ButtonRed,
            style   : twins::ButtonStyle::Simple
        }}
    },
    {
        type    : twins::Widget::Button,
        id      : ID_BTN_1P5,
        coord   : { 38, 7 },
        size    : { 0, 0 },
        { button : {
            text    : {},
            fgColor : twins::ColorFG::White,
            bgColor : twins::ColorBG::ButtonGreen,
            style   : twins::ButtonStyle::Solid1p5
        }}
    },
    { /* NUL */ }
};

static constexpr twins::Widget wndMain =
{
    // NOTE: all members must be initialized, in order they are declared,
    // otherwise GCC may fail to compile: 'sorry, unimplemented: non-trivial designated initializers not supported'

    type    : twins::Widget::Window,
    id      : ID_WND,
    coord   : { 15, 1 },
    size    : { 80, 15 },
    { window : {
        title       : ESC_FG_WHITE_INTENSE "Service Menu " ESC_UNDERLINE_ON "(Ctrl+D quit)" ESC_UNDERLINE_OFF,
        fgColor     : twins::ColorFG::Window,
        bgColor     : twins::ColorBG::Window,
        isPopup     : {},
        getState    : getWndMain,
    }},
    link    : { (const twins::Widget[])
    {
        {
            type    : twins::Widget::Button,
            id      : ID_BTN_TOASTER,
            coord   : { 1, 1 },
            size    : { 14, 1 },
            { button : {
                text    : {},
                fgColor : twins::ColorFG::Yellow,
                bgColor : {},
                style   : twins::ButtonStyle::Simple
            }}
        },
        {
            type    : twins::Widget::PageCtrl,
            id      : ID_PGCONTROL,
            coord   : { 1,  1 },
            size    : { 75, 12 },
            { pagectrl : {
                tabWidth    : 14,
                vertOffs    : 2,
            }},
            link    : { (const twins::Widget[])
            {
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_VER,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Version",
                        fgColor     : twins::ColorFG::Yellow,
                    }},
                    link    : { pageVersionChildren }
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_SERV,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Service ∑",
                        fgColor     : twins::ColorFG::White,
                    }},
                    link    : { pageServiceChildren }
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_DIAG,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Diagnostics",
                        fgColor     : twins::ColorFG::Yellow,
                    }},
                    link    : { pageDiagnosticsChildren }
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_INACTIV,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Inactiv 🍀",
                        fgColor     : twins::ColorFG::White,
                    }},
                    link    : { pageInactiveChildren }
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_TEXTBOX,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Text Box",
                        fgColor     : twins::ColorFG::White,
                    }},
                    link    : { pageTextboxChildren }
                },
                {
                    type    : twins::Widget::Page,
                    id      : ID_PAGE_COMBOBOX,
                    coord   : {},
                    size    : {},
                    { page : {
                        title       : "Combo Box",
                        fgColor     : twins::ColorFG::White,
                    }},
                    link    : { pageComboboxChildren }
                },
                { /* NUL */ }
            }}
        },
        {
            type    : twins::Widget::Label,
            id      : ID_LABEL_FTR,
            coord   : { 1, 13 },
            size    : {}, //{ 78, 1 },
            { label : {
                text    :  {},
                fgColor : twins::ColorFG::White,
                bgColor : twins::ColorBG::LabelFtr,
            }}
        },
        { /* NUL */ }
    }}
};



static constexpr twins::Widget wndPopup =
{
    type    : twins::Widget::Window,
    id      : IDPP_WND,
    coord   : { },
    size    : { 34, 10 },
    { window : {
        title       : {},
        fgColor     : twins::ColorFG::Blue,
        bgColor     : twins::ColorBG::White,
        isPopup     : true,
        getState    : getWndPopup,
    }},
    link    : { (const twins::Widget[])
    {
        {
            type    : twins::Widget::Label,
            id      : IDPP_LBL_MSG,
            coord   : { 2, 2 },
            size    : { 30, 4 },
            { label : {
                text    : {},
                fgColor : {},
                bgColor : {},
            }}
        },
        {
            type    : twins::Widget::Button,
            id      : IDPP_BTN_YES,
            coord   : { 5, 7 },
            size    : {},
            { button : {
                text    : "YES",
                fgColor : twins::ColorFG::ButtonGreen,
                bgColor : twins::ColorBG::ButtonGreen,
                style   : twins::ButtonStyle::Solid
            }}
        },
        {
            type    : twins::Widget::Button,
            id      : IDPP_BTN_NO,
            coord   : { 13, 7 },
            size    : {},
            { button : {
                text    : "NO",
                fgColor : twins::ColorFG::ButtonRed,
                bgColor : twins::ColorBG::ButtonRed,
                style   : twins::ButtonStyle::Solid
            }}
        },
        {
            type    : twins::Widget::Button,
            id      : IDPP_BTN_CANCEL,
            coord   : { 20, 7 },
            size    : {},
            { button : {
                text    : "CANCEL",
                fgColor : twins::ColorFG::White,
                bgColor : twins::ColorBG::BlackIntense,
                style   : twins::ButtonStyle::Solid
            }}
        },
        {
            type    : twins::Widget::Button,
            id      : IDPP_BTN_OK,
            coord   : { 13, 7 },
            size    : {},
            { button : {
                text    : "OK",
                fgColor : twins::ColorFG::ButtonGreen,
                bgColor : twins::ColorBG::ButtonGreen,
                style   : twins::ButtonStyle::Solid
            }}
        },
        { /* NUL */ }
    }}
};

// -----------------------------------------------------------------------------

constexpr auto wndMainWidgets = twins::transforWindowDefinition<&wndMain>();
const twins::Widget * pWndMainWidgets = wndMainWidgets.begin();
const uint16_t wndMainNumPages = twins::getPagesCount(&wndMain);

constexpr auto wndPopupWidgets = twins::transforWindowDefinition<&wndPopup>();
const twins::Widget * pWndPopupWidgets = wndPopupWidgets.begin();
