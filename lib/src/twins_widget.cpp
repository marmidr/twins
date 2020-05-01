/******************************************************************************
 * @brief   TWins - widget drawing
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#include "twins_widget_prv.hpp"

#include <string.h>
#include <assert.h>
#include <time.h>
#include <utility> //std::swap

// -----------------------------------------------------------------------------

namespace twins
{

Glob g;

// forward decl
static bool isPointWithin(uint8_t col, uint8_t row, const Rect& e);
static bool isRectWithin(const Rect& i, const Rect& e);
static bool isVisible(const Widget *pWgt);

// -----------------------------------------------------------------------------
// ---- TWINS INTERNAL FUNCTIONS -----------------------------------------------
// -----------------------------------------------------------------------------

bool findWidget(WidgetSearchStruct &wss)
{
    if (wss.searchedID == WIDGET_ID_NONE)
        return false;

    const Widget *p_wgt = g.pWndArray;

    for (;; p_wgt++)
    {
        if (p_wgt->id == wss.searchedID)
            break;

        // pWndArray is terminated by empty entry
        if (p_wgt->id == WIDGET_ID_NONE)
            return false;
    }

    wss.pWidget = p_wgt;
    wss.isVisible = g.pWndState->isVisible(p_wgt);

    // go up the widgets hierarchy
    int parent_idx = p_wgt->link.parentIdx;

    for (;;)
    {
        const auto *p_parent = g.pWndArray + parent_idx;
        wss.isVisible &= g.pWndState->isVisible(p_parent);
        wss.parentCoord += p_parent->coord;

        if (p_parent->type == Widget::Type::PageCtrl)
            wss.parentCoord.col += p_parent->pagectrl.tabWidth;

        if (parent_idx == 0)
            break;

        parent_idx = p_parent->link.parentIdx;
    }

    return true;
}

const Widget* findWidget(const WID widgetId)
{
    WidgetSearchStruct wss { searchedID : widgetId };

    if (findWidget(wss))
        return wss.pWidget;

    return nullptr;
}

const Widget* getParent(const Widget *pWgt)
{
    assert(pWgt->link.parentIdx <= pWgt->link.ownIdx);

    const Widget *p_parent = pWgt;
    p_parent -= pWgt->link.ownIdx - pWgt->link.parentIdx;
    return p_parent;
}

const Widget* getWidgetAt(uint8_t col, uint8_t row, Rect &wgtRect)
{
    const Widget *p_wgt_at = nullptr;
    Rect best_rect;
    best_rect.setMax();

    for (unsigned i = 0; g.pWndArray[i].type != Widget::None; i++)
    {
        Rect r;
        const auto *p_wgt = g.pWndArray + i;
        r.coord = getScreenCoord(p_wgt);
        r.size = p_wgt->size;

        // correct the widget size
        switch (p_wgt->type)
        {
        case Widget::Edit:
            break;
        case Widget::CheckBox:
            r.size.height = 1;
            r.size.width = 4 + utf8len(p_wgt->checkbox.text);
            break;
        case Widget::Radio:
            r.size.height = 1;
            r.size.width = 4 + utf8len(p_wgt->radio.text);
            break;
        case Widget::Button:
            r.size.height = 1;
            r.size.width = 4 + utf8len(p_wgt->button.text);
            break;
        case Widget::PageCtrl:
            r.size.width = p_wgt->pagectrl.tabWidth;
            break;
        case Widget::ListBox:
            break;
        case Widget::DropDownList:
            break;
        default:
            break;
        }

        if (isPointWithin(col, row, r))
        {
            bool is_visible = isVisible(p_wgt); // controls on tabs? solved

            if (is_visible && isRectWithin(r, best_rect))
            {
                p_wgt_at = p_wgt;
                best_rect = r;
                wgtRect = r;
            }
        }
    }

    return p_wgt_at;
}

void setCursorAt(const Widget *pWgt)
{
    if (!pWgt)
     return;

    Coord coord = getScreenCoord(pWgt);

    switch (pWgt->type)
    {
    case Widget::Edit:
        if (g.editState.pWgt == pWgt)
            coord.col += g.editState.cursorCol;
        else
            coord.col += pWgt->size.width-2;
        break;
    case Widget::CheckBox:
        coord.col += 1;
        break;
    case Widget::Radio:
        coord.col += 1;
        break;
    case Widget::Button:
        coord.col += (utf8len(pWgt->button.text) + 4) / 2;
        break;
    case Widget::ListBox:
        {
            coord.col += 1;
            coord.row += 1;
        }
        break;
    case Widget::DropDownList:
        break;
    default:
        break;
    }

    moveTo(coord.col, coord.row);
}

// -----------------------------------------------------------------------------
// ---- TWINS PRIVATE FUNCTIONS ------------------------------------------------
// -----------------------------------------------------------------------------

static bool isPointWithin(uint8_t col, uint8_t row, const Rect& e)
{
    return col >= e.coord.col &&
           col <  e.coord.col + e.size.width &&
           row >= e.coord.row &&
           row <  e.coord.row + e.size.height;
}

static bool isRectWithin(const Rect& i, const Rect& e)
{
    return i.coord.col                 >= e.coord.col &&
           i.coord.col + i.size.width  <  e.coord.col + e.size.width &&
           i.coord.row                 >= e.coord.row &&
           i.coord.row + i.size.height <  e.coord.row + e.size.height;
}

static void invalidateRadioGroup(const Widget *pRadio)
{
    const Widget *p_parent = g.pWndArray + pRadio->link.parentIdx;
    const auto group_id = pRadio->radio.groupId;

    for (unsigned i = 0; i < p_parent->link.childsCnt; i++)
    {
        const auto *p_wgt = g.pWndArray + p_parent->link.childsIdx + i;
        if (p_wgt->type == Widget::Type::Radio && p_wgt->radio.groupId == group_id)
            g.pWndState->invalidate(p_wgt->id);
    }
}

static bool isVisible(const Widget *pWgt)
{
    bool vis = g.pWndState->isVisible(pWgt);
    int parent_idx = pWgt->link.parentIdx;

    for (; vis;)
    {
        const auto *p_parent = g.pWndArray + parent_idx;
        vis &= g.pWndState->isVisible(p_parent);

        if (parent_idx == 0)
            break;

        parent_idx = p_parent->link.parentIdx;
    }

    return vis;
}

static bool isParent(const Widget *pWgt)
{
    if (!pWgt)
        return false;

    switch (pWgt->type)
    {
    case Widget::Window:
    case Widget::Panel:
    case Widget::PageCtrl:
    case Widget::Page:
        return true;
    default:
        return false;
    }
}

static bool isFocusable(const Widget *pWgt)
{
    if (!pWgt)
        return false;

    switch (pWgt->type)
    {
    case Widget::Edit:
    case Widget::CheckBox:
    case Widget::Radio:
    case Widget::Button:
    //case Widget::PageCtrl:
    case Widget::ListBox:
    case Widget::DropDownList:
        return true;
    default:
        return false;
    }
}

static const Widget* getNextFocusable(const Widget *pParent, const WID focusedID, bool forward)
{
    if (!pParent)
        return nullptr;

    const Widget *p_childs = {};
    uint16_t child_cnt = 0;

    switch (pParent->type)
    {
    case Widget::Window:
    case Widget::Panel:
    case Widget::Page:
        p_childs  = &g.pWndArray[pParent->link.childsIdx];
        child_cnt = pParent->link.childsCnt;
        break;
    case Widget::PageCtrl:
        {
            // get selected page childrens
            int idx = g.pWndState->getPageCtrlPageIndex(pParent);
            if (idx >= 0 && idx < pParent->link.childsCnt)
            {
                pParent = &g.pWndArray[pParent->link.childsIdx + idx];
                p_childs  = &g.pWndArray[pParent->link.childsIdx];
                child_cnt = pParent->link.childsCnt;
            }
            else
            {
                return nullptr;
            }
        }
        break;
    default:
        TWINS_LOG("-E- no-parent widget");
        return nullptr;
    }

    assert(p_childs);

    if (focusedID == WIDGET_ID_NONE)
    {
        //TWINS_LOG("Search * in %s items[%d]", toString(pParent->type), child_cnt);

        // give me first focusable
        for (uint16_t i = 0; i < child_cnt; i++)
        {
            const auto *p_wgt = &p_childs[i];
            //TWINS_LOG("  %s(%d)", toString(p_wgt->type), p_wgt->id);

            if (isFocusable(p_wgt))
                return p_wgt;

            if (isParent(p_wgt))
            {
                //TWINS_LOG("Search parent %s(%d)", toString(p_wgt->type), p_wgt->id);
                if ((p_wgt = getNextFocusable(p_wgt, focusedID, forward)))
                    return p_wgt;
            }
        }
    }
    else
    {
        const Widget *p_wgt = {};

        // find widget next (prev) to current
        for (uint16_t i = 0; i < child_cnt && !p_wgt; i++)
        {
            if (p_childs[i].id == focusedID)
            {
                if (forward)
                {
                    if (i + 1 == child_cnt)
                        p_wgt = &p_childs[0];
                    else
                        p_wgt = &p_childs[i+1];
                }
                else
                {
                    if (i > 0)
                        p_wgt = &p_childs[i-1];
                    else
                        p_wgt = &p_childs[child_cnt-1];
                }

                break;
            }
        }

        if (p_wgt)
        {
            // TWINS_LOG("Search in %s childs[%d]", toString(pParent->type), child_cnt);

            // iterate until focusable found
            for (uint16_t i = 0; i < child_cnt; i++)
            {
                // TWINS_LOG("  %s(%d)", toString(p_wgt->type), p_wgt->id);

                if (isFocusable(p_wgt))
                    return p_wgt;

                if (isParent(p_wgt))
                {
                    //TWINS_LOG("Search parent %s(%d)", toString(p_wgt->type), p_wgt->id);
                    if (const auto *p = getNextFocusable(p_wgt, focusedID, forward))
                        return p;
                }

                if (forward)
                {
                    p_wgt++;
                    if (p_wgt == p_childs + child_cnt)
                        p_wgt = p_childs;
                }
                else
                {
                    p_wgt--;
                    if (p_wgt < p_childs)
                        p_wgt = p_childs + child_cnt - 1;
                }
            }
        }
    }

    return nullptr;
}

static WID getNextToFocus(const WID focusedID, bool forward)
{
    WidgetSearchStruct wss { searchedID : focusedID };

    if (!findWidget(wss))
    {
        // here, find may fail only if invalid focusedID was given
        wss.pWidget = g.pWndArray;
    }

    // use the parent to get next widget
    if (auto *p_next = getNextFocusable(g.pWndArray + wss.pWidget->link.parentIdx, focusedID, forward))
    {
        return p_next->id;
    }

    return WIDGET_ID_NONE;
}

static WID getParentToFocus(WID focusedID)
{
    if (focusedID == WIDGET_ID_NONE)
        return g.pWndArray[0].id;

    WidgetSearchStruct wss { searchedID : focusedID };

    if (findWidget(wss))
    {
        const auto *p_wgt = &g.pWndArray[wss.pWidget->link.parentIdx];
        g.parentCoord -= wss.pWidget->coord;
        return p_wgt->id;
    }

    return WIDGET_ID_NONE;
}

static bool changeFocusTo(WID newID)
{
    auto &curr_id = g.pWndState->getFocusedID();

    if (newID != curr_id)
    {
        auto prev_id = curr_id;
        curr_id = newID;

        WidgetSearchStruct wss { searchedID : newID };
        if (findWidget(wss))
        {
            if (wss.pWidget->type == Widget::ListBox)
            {
                int idx, cnt;
                g.pWndState->getListBoxState(wss.pWidget, idx, cnt);
                g.listboxHighlightIdx = idx;
            }
        }

        g.pWndState->invalidate(prev_id);
        g.pWndState->invalidate(newID);
        setCursorAt(wss.pWidget);
        g.pFocusedWgt = wss.pWidget;
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------
// ---- TWINS PRIVATE FUNCTIONS ------------------------------------------------
// -----------------------------------------------------------------------------

static bool processKey_Edit(const Widget *pWgt, const KeyCode &kc)
{
    bool key_handled = false;

    // TODO: solve cursor beyound control for long strings
    if (g.editState.pWgt)
    {
        if (kc.m_spec)
        {
            switch (kc.key)
            {
            case Key::Esc:
                // cancel editing
                g.editState.pWgt = nullptr;
                g.pWndState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::Enter:
                // finish editing
                g.pWndState->onEditChange(pWgt, std::move(g.editState.str));
                g.editState.pWgt = nullptr;
                g.pWndState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::Backspace:
                if (g.editState.cursorCol > 0)
                {
                    if (kc.m_ctrl)
                    {
                        g.editState.str.erase(0, g.editState.cursorCol);
                        g.editState.cursorCol = 0;
                    }
                    else
                    {
                        g.editState.str.erase(g.editState.cursorCol-1);
                        g.editState.cursorCol--;
                    }
                    g.pWndState->invalidate(pWgt->id);
                }
                break;
            case Key::Delete:
                if (kc.m_ctrl)
                    g.editState.str.trim(g.editState.cursorCol);
                else
                    g.editState.str.erase(g.editState.cursorCol);

                g.pWndState->invalidate(pWgt->id);
                break;
            case Key::Up:
            case Key::Down:
                break;
            case Key::Left:
                if (g.editState.cursorCol > 0)
                {
                    g.editState.cursorCol --;
                    g.pWndState->invalidate(pWgt->id);
                }
                break;
            case Key::Right:
                if (g.editState.cursorCol < g.editState.str.u8len())
                {
                    g.editState.cursorCol++;
                    g.pWndState->invalidate(pWgt->id);
                }
                break;
            case Key::Home:
                g.editState.cursorCol = 0;
                g.pWndState->invalidate(pWgt->id);
                break;
            case Key::End:
                g.editState.cursorCol = g.editState.str.u8len();
                g.pWndState->invalidate(pWgt->id);
                break;
            default:
                break;
            }
        }
        else
        {
            g.editState.str.insert(g.editState.cursorCol, kc.utf8);
            g.editState.cursorCol++;
            g.pWndState->invalidate(pWgt->id);
        }
    }
    else if (kc.key == Key::Enter)
    {
        // enter editit mode
        g.editState.pWgt = pWgt;
        g.pWndState->getEditText(pWgt, g.editState.str);
        g.editState.cursorCol = g.editState.str.u8len();
        g.pWndState->invalidate(pWgt->id);
        key_handled = true;
    }

    return key_handled;
}

static bool processKey_CheckBox(const Widget *pWgt, const KeyCode &kc)
{
    if (kc.mod_all == KEY_MOD_NONE && kc.utf8[0] == ' ')
    {
        g.pWndState->onCheckboxToggle(pWgt);
        g.pWndState->invalidate(pWgt->id);
        return true;
    }

    if (kc.key == Key::Enter)
    {
        g.pWndState->onCheckboxToggle(pWgt);
        g.pWndState->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_Radio(const Widget *pWgt, const KeyCode &kc)
{
    if (kc.mod_all == KEY_MOD_NONE && kc.utf8[0] == ' ')
    {
        g.pWndState->onRadioSelect(pWgt);
        invalidateRadioGroup(pWgt);
        return true;
    }

    if (kc.key == Key::Enter)
    {
        g.pWndState->onRadioSelect(pWgt);
        invalidateRadioGroup(pWgt);
        return true;
    }

    return false;
}

static bool processKey_Button(const Widget *pWgt, const KeyCode &kc)
{
    if (kc.key == Key::Enter)
    {
        g.pWndState->onButtonClick(pWgt);
        g.pWndState->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_PageCtrl(const Widget *pWgt, const KeyCode &kc)
{
    if (kc.key == Key::PgDown || kc.key == Key::PgUp)
    {
        int idx = g.pWndState->getPageCtrlPageIndex(pWgt);
        idx += kc.key == Key::PgDown ? 1 : -1;
        if (idx < 0)                     idx = pWgt->link.childsCnt -1;
        if (idx >= pWgt->link.childsCnt) idx = 0;

        //TWINS_LOG("PG.UP/DWN: newPage%d", idx);
        changeFocusTo(pWgt->id);
        g.pWndState->onPageControlPageChange(pWgt, idx);
        g.pWndState->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_ListBox(const Widget *pWgt, const KeyCode &kc)
{
    if (kc.key == Key::Enter)
    {
        g.pWndState->onListBoxSelect(pWgt, g.listboxHighlightIdx);
        g.pWndState->invalidate(pWgt->id);
        return true;
    }
    else if (kc.key == Key::Up || kc.key == Key::Down)
    {
        int idx, cnt;
        g.pWndState->getListBoxState(pWgt, idx, cnt);
        int delta = kc.key == Key::Up ? -1 : 1;
        if (kc.m_ctrl) delta *= pWgt->size.height-2;
        g.listboxHighlightIdx += delta;

        if (g.listboxHighlightIdx < 0)
            g.listboxHighlightIdx = cnt - 1;

        if (g.listboxHighlightIdx >= cnt)
            g.listboxHighlightIdx = 0;

        g.pWndState->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_DropDownList(const Widget *pWgt, const KeyCode &kc)
{
    return false;
}

static bool processKey(const KeyCode &kc)
{
    auto focused_id = g.pWndState->getFocusedID();
    const Widget* p_wgt = findWidget(focused_id);
    bool key_handled = false;

    if (!p_wgt)
        return false;

    switch (p_wgt->type)
    {
        case Widget::Edit:
            key_handled = processKey_Edit(p_wgt, kc);
            break;
        case Widget::CheckBox:
            key_handled = processKey_CheckBox(p_wgt, kc);
            break;
        case Widget::Radio:
            key_handled = processKey_Radio(p_wgt, kc);
            break;
        case Widget::Button:
            key_handled = processKey_Button(p_wgt, kc);
            break;
        case Widget::PageCtrl:
            key_handled = processKey_PageCtrl(p_wgt, kc);
            break;
        case Widget::ListBox:
            key_handled = processKey_ListBox(p_wgt, kc);
            break;
        case Widget::DropDownList:
            key_handled = processKey_DropDownList(p_wgt, kc);
            break;
        default:
            break;
    }

    return key_handled;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static void processMouse_Edit(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
    }
}

static void processMouse_CheckBox(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
        g.pWndState->onCheckboxToggle(pWgt);
        g.pWndState->invalidate(pWgt->id);
    }
}

static void processMouse_Radio(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
        g.pWndState->onRadioSelect(pWgt);
        invalidateRadioGroup(pWgt);
    }
}

static void processMouse_Button(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
    }
    else if (kc.mouse.btn == MouseBtn::ButtonReleased)
    {
        g.pWndState->onButtonClick(pWgt);
        g.pWndState->invalidate(pWgt->id);
    }
}

static void processMouse_PageCtrl(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
        int idx = kc.mouse.row - wgtRect.coord.row - 1;

        if (idx >= 0 && idx < pWgt->link.childsCnt)
        {
            g.pWndState->onPageControlPageChange(pWgt, idx);
            g.pWndState->invalidate(pWgt->id);
        }
    }
    else if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        int idx = g.pWndState->getPageCtrlPageIndex(pWgt);
        idx += kc.mouse.btn == MouseBtn::WheelDown ? 1 : -1;
        if (idx < 0)                     idx = pWgt->link.childsCnt -1;
        if (idx >= pWgt->link.childsCnt) idx = 0;

        changeFocusTo(pWgt->id);
        g.pWndState->onPageControlPageChange(pWgt, idx);
        g.pWndState->invalidate(pWgt->id);
    }
}

static void processMouse_ListBox(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
    }
    else if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        int idx, cnt;
        g.pWndState->getListBoxState(pWgt, idx, cnt);
        int delta = kc.mouse.btn == MouseBtn::WheelUp ? -1 : 1;
        if (kc.m_ctrl) delta *= pWgt->size.height-2;
        g.listboxHighlightIdx += delta;

        if (g.listboxHighlightIdx < 0)
            g.listboxHighlightIdx = cnt - 1;

        if (g.listboxHighlightIdx >= cnt)
            g.listboxHighlightIdx = 0;

        changeFocusTo(pWgt->id);
        g.pWndState->invalidate(pWgt->id);
    }
}

static void processMouse_DropDownList(const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(pWgt->id);
    }
}

static bool processMouse(const KeyCode &kc)
{
    Rect rct;
    const Widget *p_wgt = getWidgetAt(kc.mouse.col, kc.mouse.row, rct);

    if (!p_wgt)
        return false;

    //TWINS_LOG("WidgetAt(%2d:%2d)=%s ID:%u", kc.mouse.col, kc.mouse.row, toString(p_wgt->type), p_wgt->id);

    switch (p_wgt->type)
    {
    case Widget::Edit:
        processMouse_Edit(p_wgt, rct, kc);
        break;
    case Widget::CheckBox:
        processMouse_CheckBox(p_wgt, rct, kc);
        break;
    case Widget::Radio:
        processMouse_Radio(p_wgt, rct, kc);
        break;
    case Widget::Button:
        processMouse_Button(p_wgt, rct, kc);
        break;
    case Widget::PageCtrl:
        processMouse_PageCtrl(p_wgt, rct, kc);
        break;
    case Widget::ListBox:
        processMouse_ListBox(p_wgt, rct, kc);
        break;
    case Widget::DropDownList:
        processMouse_DropDownList(p_wgt, rct, kc);
        break;
    default:
        moveToHome();
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// ---- TWINS  P U B L I C  FUNCTIONS ------------------------------------------
// -----------------------------------------------------------------------------

void log(const char *file, const char *func, unsigned line, const char *fmt, ...)
{
    FontMemento _m;
    cursorSavePos();

    pushClBg(ColorBG::Default);
    pushClFg(ColorFG::White);

    uint16_t row = pIOs->getLogsRow();
    moveTo(1, row);
    insertLines(1);

    // display only file name, trim the path
    if (const char *delim = strrchr(file, '/'))
        file = delim + 1;

    time_t t = time(NULL);
    struct tm *p_stm = localtime(&t);
    writeStrFmt("[%2d:%02d:%02d] %s() %s:%u: ",
        p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec,
        func, file, line);

    pushClFg(ColorFG::WhiteIntense);

    va_list ap;
    va_start(ap, fmt);
    pIOs->writeStrFmt(fmt, ap);
    pIOs->flushBuff();
    va_end(ap);

    cursorRestorePos();
}

const char * toString(Widget::Type type)
{
    #define CASE_WGT_STR(t)     case Widget::t: return #t;

    switch (type)
    {
    CASE_WGT_STR(None)
    CASE_WGT_STR(Window)
    CASE_WGT_STR(Panel)
    CASE_WGT_STR(Label)
    CASE_WGT_STR(Edit)
    CASE_WGT_STR(CheckBox)
    CASE_WGT_STR(Radio)
    CASE_WGT_STR(Button)
    CASE_WGT_STR(Led)
    CASE_WGT_STR(PageCtrl)
    CASE_WGT_STR(Page)
    CASE_WGT_STR(ProgressBar)
    CASE_WGT_STR(ListBox)
    CASE_WGT_STR(DropDownList)
    CASE_WGT_STR(Canvas)
    default: return "?";
    }
}

Coord getScreenCoord(const Widget *pWgt)
{
    Coord coord = pWgt->coord;

    // go up the widgets hierarchy
    const auto *p_parent = getParent(pWgt);// g.pWndArray + parent_idx;

    for (;;)
    {
        coord += p_parent->coord;

        if (p_parent->type == Widget::Type::PageCtrl)
            coord.col += p_parent->pagectrl.tabWidth;

        if (p_parent->link.ownIdx == 0)
            break;

        p_parent = getParent(p_parent);
    }

    return coord;
}

bool processKey(const Widget *pWindowArray, const KeyCode &kc)
{
    assert(pWindowArray);
    assert(pWindowArray->type == Widget::Window);
    g.pWndArray = pWindowArray;
    g.pWndState = pWindowArray->window.getState();
    assert(g.pWndState);
    bool key_processed = false;

    if (kc.key == Key::None)
        return true;

    //TWINS_LOG("---");

    if (kc.key == Key::MouseEvent)
    {
        key_processed = processMouse(kc);
    }
    else
    {
        key_processed = processKey(kc);

        if (!key_processed && kc.m_spec)
        {
            switch (kc.key)
            {
            case Key::Esc:
            {
                auto curr_id = g.pWndState->getFocusedID();
                auto new_id = getParentToFocus(curr_id);
                key_processed = changeFocusTo(new_id);
                break;
            }
            case Key::Tab:
            {
                auto curr_id = g.pWndState->getFocusedID();
                auto new_id = getNextToFocus(curr_id, !kc.m_shift);
                key_processed = changeFocusTo(new_id);
                break;
            }
            case Key::PgUp:
            case Key::PgDown:
            {
                // Ctrl+PgUp/PgDown will be directed to window's first PageControl widget
                if (kc.m_ctrl)
                {
                    for (unsigned i = 0; i < g.pWndArray[0].link.childsCnt; i++)
                    {
                        const auto *p_wgt = &g.pWndArray[g.pWndArray[0].link.childsIdx + i];

                        if (p_wgt->type == Widget::PageCtrl)
                        {
                            processKey_PageCtrl(p_wgt, kc);
                            key_processed = true;
                            break;
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }

    return key_processed;
}

// -----------------------------------------------------------------------------

}
