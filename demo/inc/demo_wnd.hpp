/******************************************************************************
 * @brief   TWins - demo window definition
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#include "twins.hpp"

// -----------------------------------------------------------------------------

enum WndMainIDs
{
    ID_INVALID,
    ID_WND,
        ID_PANEL_VERSIONS,
            ID_LABEL_FW_VERSION,
            ID_LABEL_DATE,
            ID_LABEL_TIME,
        ID_PANEL_CONFIG,
        ID_PANEL_KEYCODE,
            ID_LABEL_KEYCODE
};

// -----------------------------------------------------------------------------

extern const twins::WindowCallbacks wndMainClbcks;

const twins::Widget wndMain =
{
    // NOTE: all members must be initialized, in order they are declared,
    // otherwise GCC may fail to compile: 'sorry, unimplemented: non-trivial designated initializers not supported'

    type    : twins::Widget::Window,
    id      : ID_WND,
    coord   : { 10, 4 },
    size    : { 60, 12 },
    { window : {
        frameStyle  : twins::FrameStyle::Double,
        bgColor     : twins::ColorBG::BLUE,
        fgColor     : twins::ColorFG::WHITE,
        caption     : "Service Menu (Ctrl+D quit)",
        pCallbacks  : &wndMainClbcks,
        pChildrens  : (const twins::Widget[])
        {
            {
                type    : twins::Widget::Panel,
                id      : ID_PANEL_VERSIONS,
                coord   : { 2, 2 },
                size    : { 21, 5 },
                { panel : {
                    frameStyle  : twins::FrameStyle::Single,
                    bgColor     : twins::ColorBG::GREEN,
                    fgColor     : twins::ColorFG::WHITE,
                    caption     : "VER",
                    pChildrens  : (const twins::Widget[])
                    {
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LABEL_FW_VERSION,
                            coord   : { 2, 1 },
                            size    : { 14, 1 },
                            { label : {
                                bgColor : twins::ColorBG::BLACK_INTENSE,
                                fgColor : twins::ColorFG::WHITE,
                                text    : "FwVer: 1"
                            }}
                        },
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LABEL_DATE,
                            coord   : { 2, 2 },
                            size    : { 16, 1 },
                            { label : {
                                bgColor : twins::ColorBG::WHITE,
                                fgColor : twins::ColorFG::BLACK,
                                text    : "Date•" __DATE__
                            }}
                        },
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LABEL_TIME,
                            coord   : { 2, 3 },
                            size    : { 16, 1 },
                            { label : {
                                bgColor : twins::ColorBG::WHITE,
                                fgColor : twins::ColorFG::BLACK,
                                text    : "Time≡" __TIME__
                            }}
                        },
                    },
                    childCount : 3
                }} // panel
            },
            {
                type    : twins::Widget::Panel,
                id      : ID_PANEL_CONFIG,
                coord   : { 30, 2 },
                size    : { 25, 8 },
                { panel : {
                    frameStyle  : twins::FrameStyle::Single,
                    bgColor     : twins::ColorBG::GREEN,
                    fgColor     : twins::ColorFG::WHITE,
                    caption     : "CONFIG",
                    pChildrens  : {},
                    childCount  : {}
                }} // panel
            },
            {
                type    : twins::Widget::Panel,
                id      : ID_PANEL_KEYCODE,
                coord   : { 2, 8 },
                size    : { 21, 3 },
                { panel : {
                    frameStyle  : twins::FrameStyle::Single,
                    bgColor     : twins::ColorBG::CYAN,
                    fgColor     : twins::ColorFG::WHITE,
                    caption     : "KEY-CODES",
                    pChildrens  : (const twins::Widget[])
                    {
                        {
                            type    : twins::Widget::Label,
                            id      : ID_LABEL_KEYCODE,
                            coord   : { 2, 1 },
                            size    : { 17, 1 },
                            { label : {
                                bgColor : twins::ColorBG::WHITE,
                                fgColor : twins::ColorFG::RED,
                                text    : nullptr // use callback to get text
                            }}
                        },
                    },
                    childCount : 1
                }} // panel
            }
        },
        childCount : 3,
    }} // window
};

