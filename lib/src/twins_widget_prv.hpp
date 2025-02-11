/******************************************************************************
 * @brief   TWins - main header file
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once
#include "twins.hpp"
#include "twins_string.hpp"
#include "twins_utf8str.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

struct WidgetSearchStruct
{
    WID   searchedID = {};      // given
    Coord parentCoord = {};     // expected
    bool  isVisible = true;     // expected
    const Widget *pWidget = {}; // expected
};

/** Widget drawing state object */
struct WidgetState
{
    String strbuff;                     // common string buff for widget drawers
    const Widget *pFocusedWgt = {};     //
    const Widget *pMouseDownWgt = {};   //
    const Widget *pCbxDropDown = {};
    KeyCode       mouseDownKeyCode = {};
    struct                              // state of Edit being modified
    {
        const Widget *pWgt = nullptr;
        int16_t cursorPos = 0;
        String  txt;
    } textEditState;
};

struct CallCtx
{
    CallCtx(const Widget* pWindowWidgets)
    {
        assert(pWindowWidgets);
        assert(pWindowWidgets->type == Widget::Window);
        pWidgets = pWindowWidgets;
        pState = pWindowWidgets->window.getState();
    }
    const Widget *  pWidgets = {};
    IWindowState *  pState = {};
    Coord           parentCoord; // current widget's parent left-top position
};

extern WidgetState& g_ws;

// -----------------------------------------------------------------------------

const Widget* getWidgetByWID(CallCtx &ctx, const WID widgetId);
const Widget* getWidgetAt(CallCtx &ctx, uint8_t col, uint8_t row, Rect &wgtRect);
bool isVisible(CallCtx &ctx, const Widget *pWgt);
bool isEnabled(CallCtx &ctx, const Widget *pWgt);

const Widget* getParent(const Widget *pWgt);

bool getWidgetWSS(CallCtx &ctx, WidgetSearchStruct &wss);
void setCursorAt(CallCtx &ctx, const Widget *pWgt);

// -----------------------------------------------------------------------------

} // twins
