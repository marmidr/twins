/******************************************************************************
 * @brief   TWins - widget drawing
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins_widget_prv.hpp"
#include "twins_utils.hpp"

#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <utility> //std::swap

// -----------------------------------------------------------------------------

namespace twins
{

// trick to avoid automatic variable creation/destruction causing calls to uninitialized PAL
static char wds_buff alignas(WidgetState) [sizeof(WidgetState)];
WidgetState& g_ws = (WidgetState&)wds_buff;

// -----------------------------------------------------------------------------
// ---- TWINS INTERNAL FUNCTIONS -----------------------------------------------
// -----------------------------------------------------------------------------

void widgetInit()
{
    new (&g_ws) WidgetState{};
}

void widgetDeInit()
{
    g_ws.~WidgetState();
}

bool getWidgetWSS(CallCtx &ctx, WidgetSearchStruct &wss)
{
    if (wss.searchedID == WIDGET_ID_NONE)
        return false;

    const Widget *p_wgt = ctx.pWidgets;

    for (;; p_wgt++)
    {
        if (p_wgt->id == wss.searchedID)
            break;

        // pWndArray is terminated by empty entry
        if (p_wgt->id == WIDGET_ID_NONE)
            return false;
    }

    wss.pWidget = p_wgt;
    wss.isVisible = ctx.pState->isVisible(p_wgt);

    // go up the widgets hierarchy
    int parent_idx = p_wgt->link.parentIdx;

    for (;;)
    {
        const auto *p_parent = ctx.pWidgets + parent_idx;
        wss.isVisible &= ctx.pState->isVisible(p_parent);

        Coord coord = p_parent->coord;
        if (p_parent->type == Widget::Type::Window)
            ctx.pState->getWindowCoord(p_parent, coord);
        wss.parentCoord += coord;

        if (p_parent->type == Widget::Type::PageCtrl)
            wss.parentCoord.col += p_parent->pagectrl.tabWidth;

        if (parent_idx == 0)
            break;

        parent_idx = p_parent->link.parentIdx;
    }

    return true;
}

const Widget* getWidgetByWID(CallCtx &ctx, const WID widgetId)
{
    for (unsigned i = 0; ctx.pWidgets[i].type != Widget::None; i++)
        if (ctx.pWidgets[i].id == widgetId)
            return &ctx.pWidgets[i];

    return nullptr;
}

const Widget* getParent(const Widget *pWgt)
{
    assert(pWgt->link.parentIdx <= pWgt->link.ownIdx);

    const Widget *p_parent = pWgt;
    p_parent -= pWgt->link.ownIdx - pWgt->link.parentIdx;
    return p_parent;
}

const Widget* getWidgetAt(CallCtx &ctx, uint8_t col, uint8_t row, Rect &wgtRect)
{
    const Widget *p_wgt_at = nullptr;
    Rect best_rect;
    best_rect.setMax();

    for (unsigned i = 0; ctx.pWidgets[i].type != Widget::None; i++)
    {
        bool stop_searching = true;
        Rect r;
        const auto *p_wgt = ctx.pWidgets + i;
        r.coord = getScreenCoord(p_wgt);
        r.size = p_wgt->size;

        // correct the widget size
        switch (p_wgt->type)
        {
        case Widget::TextEdit:
            break;
        case Widget::CheckBox:
            r.size.height = 1;
            r.size.width = 4 + String::width(p_wgt->checkbox.text);
            break;
        case Widget::Radio:
            r.size.height = 1;
            r.size.width = 4 + String::width(p_wgt->radio.text);
            break;
        case Widget::Button:
        {
            uint16_t txt_w = 0;

            if (p_wgt->button.text)
                txt_w = String::width(p_wgt->button.text);
            else if (p_wgt->size.width)
                txt_w = p_wgt->size.width;
            else
            {
                g_ws.strbuff.clear();
                ctx.pState->getButtonText(p_wgt, g_ws.strbuff);
                txt_w = g_ws.strbuff.width();
            }

            switch (p_wgt->button.style)
            {
            case ButtonStyle::Simple:
                r.size.height = 1;
                r.size.width = 4 + txt_w;
                break;
            case ButtonStyle::Solid:
                r.size.height = 1;
                r.size.width = 2 + txt_w;
                break;
            case ButtonStyle::Solid1p5:
                r.size.height = 3;
                r.size.width = 2 + txt_w;
                break;
            default:
                break;
            }
            break;
        }
        case Widget::PageCtrl:
            r.size.width = p_wgt->pagectrl.tabWidth;
            break;
        case Widget::ListBox:
            break;
        case Widget::ComboBox:
            break;
        default:
            stop_searching = false;
            break;
        }

        if (isPointWithin(col, row, r))
        {
            bool is_visible = isVisible(ctx, p_wgt); // controls on tabs? solved

            if (is_visible && isRectWithin(r, best_rect))
            {
                p_wgt_at = p_wgt;
                best_rect = r;
                wgtRect = r;

                // visible and clickable widget found?
                if (stop_searching)
                    break;
            }
        }
    }

    return p_wgt_at;
}

void setCursorAt(CallCtx &ctx, const Widget *pWgt)
{
    if (!pWgt)
        return;

    Coord coord = getScreenCoord(pWgt);

    switch (pWgt->type)
    {
    case Widget::TextEdit:
        if (g_ws.textEditState.pWgt == pWgt)
        {
            const int16_t max_w = pWgt->size.width-3;
            coord.col += g_ws.textEditState.cursorPos;
            auto cursor_pos = g_ws.textEditState.cursorPos;
            auto delta = (max_w/2);
            while (cursor_pos >= max_w-1)
            {
                coord.col -= delta;
                cursor_pos -= delta;
            }
        }
        else
        {
            coord.col += pWgt->size.width-2;
        }
        break;
    case Widget::CheckBox:
        coord.col += 1;
        break;
    case Widget::Radio:
        coord.col += 1;
        break;
    case Widget::Button:
        switch (pWgt->button.style)
        {
        case ButtonStyle::Simple:
            coord.col += 2;
            break;
        case ButtonStyle::Solid:
            coord.col += 1;
            break;
        case ButtonStyle::Solid1p5:
            coord.col += 1;
            coord.row += 1;
            break;
        default:
            break;
        }
        break;
    case Widget::PageCtrl:
        coord.row += 1 + pWgt->pagectrl.vertOffs;
        coord.row += ctx.pState->getPageCtrlPageIndex(pWgt);
        break;
    case Widget::ListBox:
    {
        int16_t idx = 0, selidx = 0, cnt = 0;
        const uint8_t frame_size = !pWgt->listbox.noFrame;
        ctx.pState->getListBoxState(pWgt, idx, selidx, cnt);

        int page_size = pWgt->size.height - (frame_size * 2);
        int row = selidx % page_size;

        coord.col += frame_size;
        coord.row += frame_size + row;
        break;
    }
    case Widget::ComboBox:
        break;
    case Widget::TextBox:
        break;
    default:
        break;
    }

    moveTo(coord.col, coord.row);
}

bool isVisible(CallCtx &ctx, const Widget *pWgt)
{
    bool vis = ctx.pState->isVisible(pWgt);
    int parent_idx = pWgt->link.parentIdx;

    for (; vis;)
    {
        const auto *p_parent = ctx.pWidgets + parent_idx;
        vis &= ctx.pState->isVisible(p_parent);

        if (parent_idx == 0)
            break;

        parent_idx = p_parent->link.parentIdx;
    }

    return vis;
}

bool isEnabled(CallCtx &ctx, const Widget *pWgt)
{
    bool en = ctx.pState->isEnabled(pWgt);
    int parent_idx = pWgt->link.parentIdx;

    for (; en;)
    {
        const auto *p_parent = ctx.pWidgets + parent_idx;
        en &= ctx.pState->isEnabled(p_parent);

        if (parent_idx == 0)
            break;

        parent_idx = p_parent->link.parentIdx;
    }

    return en;
}

bool isPointWithin(uint8_t col, uint8_t row, const Rect& r)
{
    return col >= r.coord.col &&
           col <  r.coord.col + r.size.width &&
           row >= r.coord.row &&
           row <  r.coord.row + r.size.height;
}

bool isRectWithin(const Rect& i, const Rect& e)
{
    // i'ntern <= e'xtern
    return i.coord.col                 >= e.coord.col &&
           i.coord.col + i.size.width  <= e.coord.col + e.size.width &&
           i.coord.row                 >= e.coord.row &&
           i.coord.row + i.size.height <= e.coord.row + e.size.height;
}

// -----------------------------------------------------------------------------
// ---- TWINS PRIVATE FUNCTIONS ------------------------------------------------
// -----------------------------------------------------------------------------

static void invalidateRadioGroup(CallCtx &ctx, const Widget *pRadio)
{
    const Widget *p_parent = ctx.pWidgets + pRadio->link.parentIdx;
    const auto group_id = pRadio->radio.groupId;

    for (unsigned i = 0; i < p_parent->link.childrenCnt; i++)
    {
        const auto *p_wgt = ctx.pWidgets + p_parent->link.childrenIdx + i;
        if (p_wgt->type == Widget::Type::Radio && p_wgt->radio.groupId == group_id)
            ctx.pState->invalidate(p_wgt->id);
    }
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
    case Widget::Layer:
        return true;
    default:
        return false;
    }
}

static bool isFocusable(CallCtx &ctx, const Widget *pWgt)
{
    if (!pWgt)
        return false;

    switch (pWgt->type)
    {
    case Widget::TextEdit:
    case Widget::CheckBox:
    case Widget::Radio:
    case Widget::Button:
    //case Widget::PageCtrl:
    case Widget::ListBox:
    case Widget::ComboBox:
    case Widget::TextBox:
        return isEnabled(ctx, pWgt);
    default:
        return false;
    }
}

static bool isFocusable(CallCtx &ctx, const WID widgetId)
{
    if (const auto *p_wgt = getWidgetByWID(ctx, widgetId))
        return isFocusable(ctx, p_wgt);
    return false;
}

static const Widget* getNextFocusable(CallCtx &ctx, const Widget *pParent, WID focusedID, bool forward, const Widget *pFirstParent = nullptr, bool *pBreak = nullptr)
{
    bool brk = false;

    if (!pParent)
        return nullptr;

    if (pParent == pFirstParent)
    {
        // TWINS_LOG_E("full loop detected (pFirstParent id=%d)", pFirstParent?pFirstParent->id:-1);
        if (pBreak) *pBreak = true;
        return nullptr;
    }

    if (pParent->id == focusedID)
        return nullptr;

    const Widget *p_children = {};
    uint16_t child_cnt = 0;

    // get childrens and their number
    switch (pParent->type)
    {
    case Widget::Window:
    case Widget::Panel:
    case Widget::Page:
    case Widget::Layer:
    {
        p_children  = ctx.pWidgets + pParent->link.childrenIdx;
        child_cnt = pParent->link.childrenCnt;
        break;
    }
    case Widget::PageCtrl:
    {
        // get selected page childrens
        int idx = ctx.pState->getPageCtrlPageIndex(pParent);
        if (idx >= 0 && idx < pParent->link.childrenCnt)
        {
            pParent   = ctx.pWidgets + pParent->link.childrenIdx + idx;
            p_children  = ctx.pWidgets + pParent->link.childrenIdx;
            child_cnt = pParent->link.childrenCnt;
        }
        else
        {
            return nullptr;
        }
        break;
    }
    default:
        TWINS_LOG_E("Not a parent type widget");
        return nullptr;
    }

    if (child_cnt == 0)
        return nullptr;

    switch (pParent->type)
    {
    case Widget::Page:
    case Widget::Panel:
    case Widget::Layer:
        if (!pFirstParent)
        {
            // it must be Panel/Page/Layer because while traversing we never step below Page level
            // TWINS_LOG_D("1st parent[%s id:%u]", toString(pParent->type), pParent->id);
            pFirstParent = pParent;
        }
        break;
    default:
        break;
    }

    assert(p_children);
    const Widget *p_wgt = nullptr;

    // TWINS_LOG_D("pParent[%s id:%u] focusedId=%d", toString(pParent->type), pParent->id, focusedID); twins::sleepMs(200);

    if (focusedID == WIDGET_ID_NONE)
    {
        // get first/last of the children ID
        p_wgt = forward ? &p_children[0] : &p_children[child_cnt-1];
        focusedID = p_wgt->id;

        if (isFocusable(ctx, p_wgt) && isVisible(ctx, p_wgt))
            return p_wgt;

        if (isParent(p_wgt))
        {
            if (const auto *p = getNextFocusable(ctx, p_wgt, WIDGET_ID_NONE, forward, pFirstParent, pBreak))
                return p;
        }
    }
    else
    {
        // get pointer to focusedID
        p_wgt = p_children;

        while (p_wgt->id != focusedID && p_wgt < p_children + child_cnt)
            p_wgt++;

        // expect that children have focusedID
        //assert(p_wgt < p_children + child_cnt); // occures rarely
        if (p_wgt >= p_children + child_cnt)
        {
            TWINS_LOG_W("Focused ID=%d not found on parent ID=%d", focusedID, pParent->id);
            return nullptr;
        }
    }


    // TWINS_LOG_D("search in [%s id:%d children:%d]", toString(pParent->type), pParent->id, child_cnt);
    // iterate until focusable found or children border reached
    assert(p_wgt);

    for (uint16_t i = 0; i < child_cnt; i++)
    {
        p_wgt += forward ? 1 : -1;

        if (p_wgt == p_children + child_cnt || p_wgt == p_children - 1)
        {
            // border reached: if we are on Panel or Layer, jump to next sibling
            if (pParent->type == Widget::Panel || pParent->type == Widget::Layer)
                return getNextFocusable(ctx, getParent(pParent), pParent->id, forward, pFirstParent);

            if (p_wgt > p_children) p_wgt = p_children;
            else                  p_wgt = p_children + child_cnt - 1;
        }

        if (isFocusable(ctx, p_wgt) && isVisible(ctx, p_wgt))
            return p_wgt;

        if (isParent(p_wgt))
        {
            if (!pBreak)
                pBreak = &brk;
            if (const auto *p = getNextFocusable(ctx, p_wgt, WIDGET_ID_NONE, forward, pFirstParent, pBreak))
                return p;
            if (*pBreak)
                break;
        }
    }

    return nullptr;
}

static WID getNextToFocus(CallCtx &ctx, const WID focusedID, bool forward)
{
    WidgetSearchStruct wss { searchedID : focusedID };

    if (!getWidgetWSS(ctx, wss))
    {
        // here, find may fail only if invalid focusedID was given
        wss.pWidget = ctx.pWidgets;
    }

    // use the parent to get next widget
    if (auto *p_next = getNextFocusable(ctx, ctx.pWidgets + wss.pWidget->link.parentIdx, focusedID, forward))
    {
        return p_next->id;
    }

    return WIDGET_ID_NONE;
}

static WID getParentToFocus(CallCtx &ctx, WID focusedID)
{
    if (focusedID == WIDGET_ID_NONE)
        return ctx.pWidgets[0].id;

    WidgetSearchStruct wss { searchedID : focusedID };

    if (getWidgetWSS(ctx, wss))
    {
        const auto *p_wgt = &ctx.pWidgets[wss.pWidget->link.parentIdx];
        // g_ds.parentCoord -= wss.pWidget->coord;
        return p_wgt->id;
    }

    return ctx.pWidgets[0].id;
}

static bool changeFocusTo(CallCtx &ctx, WID newID)
{
    auto &curr_id = ctx.pState->getFocusedID();
    // TWINS_LOG_D("curr_id=%d, newID=%d", curr_id, newID);

    if (newID != curr_id)
    {
        auto prev_id = curr_id;
        curr_id = newID;
        WidgetSearchStruct wss { searchedID : newID };

        if (getWidgetWSS(ctx, wss))
        {
            if (wss.pWidget->type == Widget::ListBox)
            {
                int16_t idx = 0, selidx = 0, cnt = 0;
                ctx.pState->getListBoxState(wss.pWidget, idx, selidx, cnt);

                if (idx < 0 && cnt > 0)
                {
                    ctx.pState->onListBoxSelect(wss.pWidget, selidx);
                }
            }
        }

        if (isFocusable(ctx, prev_id))
            ctx.pState->invalidate(prev_id);

        if (isFocusable(ctx, newID))
            ctx.pState->invalidate(newID);

        setCursorAt(ctx, wss.pWidget);
        g_ws.pFocusedWgt = wss.pWidget;
        return true;
    }

    return false;
}

static const Widget *findMainPgControl(CallCtx &ctx)
{
    const auto *p_wnd = &ctx.pWidgets[0];

    for (unsigned i = 0; i < p_wnd->link.childrenCnt; i++)
    {
        const auto *p_wgt = ctx.pWidgets + p_wnd->link.childrenIdx + i;

        if (p_wgt->type == Widget::PageCtrl)
            return p_wgt;
    }

    return nullptr;
}

static void pgControlChangePage(CallCtx &ctx, const Widget *pWgt, bool next)
{
    if (!pWgt) return;
    assert(pWgt->type == Widget::PageCtrl);

    int idx = ctx.pState->getPageCtrlPageIndex(pWgt);
    idx += next ? 1 : -1;
    if (idx < 0)                     idx = pWgt->link.childrenCnt -1;
    if (idx >= pWgt->link.childrenCnt) idx = 0;

    // changeFocusTo(ctx, pWgt->id); // DON'T or separate focus for each Tab will not work
    ctx.pState->onPageControlPageChange(pWgt, idx);
    ctx.pState->invalidate(pWgt->id);

    // cancel EDIT mode
    g_ws.textEditState.pWgt = nullptr;

    if (const auto *p_wgt = getWidgetByWID(ctx, ctx.pState->getFocusedID()))
    {
        // TWINS_LOG_D("focused id=%d (%s)", p_wgt->id, toString(p_wgt->type));
        g_ws.pFocusedWgt = p_wgt;
        setCursorAt(ctx, p_wgt);
    }
    else
    {
        g_ws.pFocusedWgt = p_wgt;
        moveToHome();
    }
}

static void comboBoxHideList(CallCtx &ctx, const Widget *pWgt)
{
    assert(pWgt);
    assert(pWgt->type == Widget::ComboBox);

    ctx.pState->onComboBoxDrop(pWgt, false);
    // redraw parent to hide list
    const auto *p_parent = getParent(pWgt);
    ctx.pState->invalidate(p_parent->id);
    g_ws.pCbxDropDown = nullptr;
}

// -----------------------------------------------------------------------------

static bool processKey_TextEdit(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    if (pWgt == g_ws.textEditState.pWgt)
    {
        // if in edit state, allow user to handle key
        if (ctx.pState->onTextEditInputEvt(pWgt, kc, g_ws.textEditState.txt, g_ws.textEditState.cursorPos))
        {
            ctx.pState->invalidate(pWgt->id);
            return true;
        }
        // user let us continue checking the key
    }

    bool key_handled = false;

    if (g_ws.textEditState.pWgt)
    {
        auto cursor_pos = g_ws.textEditState.cursorPos;

        if (kc.m_spec)
        {
            switch (kc.key)
            {
            case Key::Esc:
                // cancel editing
                g_ws.textEditState.pWgt = nullptr;
                ctx.pState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::Tab:
                // real TAB may have different widths and require extra processing
                g_ws.textEditState.txt.insert(cursor_pos, "    ");
                cursor_pos += 4;
                ctx.pState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::Enter:
                // finish editing
                ctx.pState->onTextEditChange(pWgt, std::move(g_ws.textEditState.txt));
                g_ws.textEditState.pWgt = nullptr;
                ctx.pState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::Backspace:
                if (cursor_pos > 0)
                {
                    if (kc.m_ctrl)
                    {
                        g_ws.textEditState.txt.erase(0, cursor_pos);
                        cursor_pos = 0;
                    }
                    else
                    {
                        g_ws.textEditState.txt.erase(cursor_pos-1);
                        cursor_pos--;
                    }
                    ctx.pState->invalidate(pWgt->id);
                }
                key_handled = true;
                break;
            case Key::Delete:
                if (kc.m_ctrl)
                    g_ws.textEditState.txt.trim(cursor_pos);
                else
                    g_ws.textEditState.txt.erase(cursor_pos);

                key_handled = true;
                ctx.pState->invalidate(pWgt->id);
                break;
            case Key::Up:
            case Key::Down:
                break;
            case Key::Left:
                if (cursor_pos > 0)
                {
                    cursor_pos --;
                    ctx.pState->invalidate(pWgt->id);
                }
                key_handled = true;
                break;
            case Key::Right:
                if (cursor_pos < (signed)g_ws.textEditState.txt.u8len())
                {
                    cursor_pos++;
                    ctx.pState->invalidate(pWgt->id);
                }
                key_handled = true;
                break;
            case Key::Home:
                cursor_pos = 0;
                ctx.pState->invalidate(pWgt->id);
                key_handled = true;
                break;
            case Key::End:
                cursor_pos = g_ws.textEditState.txt.u8len();
                ctx.pState->invalidate(pWgt->id);
                key_handled = true;
                break;
            default:
                break;
            }
        }
        else
        {
            g_ws.textEditState.txt.insert(cursor_pos, kc.utf8);
            cursor_pos++;
            ctx.pState->invalidate(pWgt->id);
            key_handled = true;
        }

        g_ws.textEditState.cursorPos = cursor_pos;
    }
    else if (kc.key == Key::Enter)
    {
        // enter edit mode
        g_ws.textEditState.pWgt = pWgt;
        g_ws.textEditState.txt.clear();
        ctx.pState->getTextEditText(pWgt, g_ws.textEditState.txt, true);
        g_ws.textEditState.cursorPos = g_ws.textEditState.txt.u8len();
        ctx.pState->invalidate(pWgt->id);
        key_handled = true;
    }

    return key_handled;
}

static bool processKey_CheckBox(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    if (kc.mod_all == KEY_MOD_NONE && kc.utf8[0] == ' ')
    {
        ctx.pState->onCheckboxToggle(pWgt);
        ctx.pState->invalidate(pWgt->id);
        return true;
    }

    if (kc.key == Key::Enter)
    {
        ctx.pState->onCheckboxToggle(pWgt);
        ctx.pState->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_Radio(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    if (kc.mod_all == KEY_MOD_NONE && kc.utf8[0] == ' ')
    {
        ctx.pState->onRadioSelect(pWgt);
        invalidateRadioGroup(ctx, pWgt);
        return true;
    }

    if (kc.key == Key::Enter)
    {
        ctx.pState->onRadioSelect(pWgt);
        invalidateRadioGroup(ctx, pWgt);
        return true;
    }

    return false;
}

static bool processKey_Button(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    auto *p_wstate = ctx.pState;

    if (p_wstate->onButtonKey(pWgt, kc))
    {
        // user handled the keyboard event
        return true;
    }

    if (kc.key == Key::Enter)
    {
        // pointer may change between onButtonUp and onButtonClick, so remember it
        g_ws.pMouseDownWgt = pWgt;
        p_wstate->onButtonDown(pWgt, kc);
        p_wstate->invalidate(pWgt->id, true);
        sleepMs(50);
        g_ws.pMouseDownWgt = nullptr;
        p_wstate->onButtonUp(pWgt, kc);
        p_wstate->onButtonClick(pWgt, kc);
        p_wstate->invalidate(pWgt->id);
        return true;
    }

    return false;
}

static bool processKey_PageCtrl(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    if (kc.key == Key::PgDown || kc.key == Key::PgUp ||
        kc.key == Key::F11 || kc.key == Key::F12)
    {
        pgControlChangePage(ctx, pWgt, kc.key == Key::PgDown || kc.key == Key::F12);
        return true;
    }

    return false;
}

static bool processKey_ListBox(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    int delta = 0;
    const uint16_t items_visible = pWgt->size.height-2;

    switch (kc.key)
    {
    case Key::Enter:
    {
        int16_t idx = 0, selidx = 0, cnt = 0;
        ctx.pState->getListBoxState(pWgt, idx, selidx, cnt);
        if (cnt > 0)
        {
            if (selidx >= 0 && selidx != idx)
                ctx.pState->onListBoxChange(pWgt, selidx);
            ctx.pState->invalidate(pWgt->id);
        }
        return true;
    }
    case Key::Up:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? -1 : 0;
        break;
    case Key::Down:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? 1 : 0;
        break;
    case Key::PgUp:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? -items_visible : 0;
        break;
    case Key::PgDown:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? items_visible : 0;
        break;
    default:
        break;
    }

    if (delta != 0)
    {
        int16_t idx = 0, selidx = 0, cnt = 0;
        ctx.pState->getListBoxState(pWgt, idx, selidx, cnt);

        if (cnt > 0)
        {
            selidx += delta;

            if (selidx < 0)
                selidx = cnt - 1;

            if (selidx >= cnt)
                selidx = 0;

            ctx.pState->onListBoxSelect(pWgt, selidx);
            ctx.pState->invalidate(pWgt->id);
        }
        return true;
    }

    return false;
}

static bool processKey_ComboBox(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    int16_t idx = 0, selidx = 0, cnt = 0; bool drop_down = false;
    ctx.pState->getComboBoxState(pWgt, idx, selidx, cnt, drop_down);

    if (kc.utf8[0] == ' ')
    {
        if (cnt > 0)
        {
            drop_down = !drop_down;

            if (drop_down)
            {
                ctx.pState->onComboBoxDrop(pWgt, true);
                g_ws.pCbxDropDown = pWgt;
            }
            else
            {
                comboBoxHideList(ctx, pWgt);
            }
        }
    }
    else if (kc.key == Key::Esc)
    {
        comboBoxHideList(ctx, pWgt);
    }
    else if (drop_down)
    {
        if (kc.key == Key::Up)
        {
            if (--selidx < 0) selidx = cnt-1;
            ctx.pState->onComboBoxSelect(pWgt, selidx);
        }
        else if (kc.key == Key::Down)
        {
            if (++selidx >= cnt) selidx = 0;
            ctx.pState->onComboBoxSelect(pWgt, selidx);
        }
        else if (kc.key == Key::PgUp && kc.mod_all == KEY_MOD_SPECIAL)
        {
            selidx -= pWgt->combobox.dropDownSize;
            if (selidx < 0) selidx = cnt-1;
            ctx.pState->onComboBoxSelect(pWgt, selidx);
        }
        else if (kc.key == Key::PgDown && kc.mod_all == KEY_MOD_SPECIAL)
        {
            selidx += pWgt->combobox.dropDownSize;
            if (selidx >= cnt) selidx = 0;
            ctx.pState->onComboBoxSelect(pWgt, selidx);
        }
        else if (kc.key == Key::Enter)
        {
            ctx.pState->onComboBoxChange(pWgt, selidx);
            comboBoxHideList(ctx, pWgt);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    ctx.pState->invalidate(pWgt->id);
    return true;
}

static bool processKey_TextBox(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    int delta = 0;
    const uint16_t lines_visible = pWgt->size.height - 2;

    switch (kc.key)
    {
    case Key::Up:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? -1 : 0;
        break;
    case Key::Down:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? 1 : 0;
        break;
    case Key::PgUp:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? -lines_visible : 0;
        break;
    case Key::PgDown:
        delta = kc.mod_all == KEY_MOD_SPECIAL ? lines_visible : 0;
        break;
    default:
        break;
    }

    if (delta != 0)
    {
        const twins::Vector<twins::CStrView> *p_lines = nullptr;
        int16_t top_line = 0;

        ctx.pState->getTextBoxState(pWgt, &p_lines, top_line);

        if (p_lines)
        {
            top_line += delta;

            if (top_line > (int)p_lines->size() - lines_visible)
                top_line = p_lines->size() - lines_visible;

            if (top_line < 0)
                top_line = 0;

            ctx.pState->onTextBoxScroll(pWgt, top_line);
            ctx.pState->invalidate(pWgt->id);
        }
        return true;
    }

    return false;
}

static bool processKey(CallCtx &ctx, const KeyCode &kc)
{
    auto focused_id = ctx.pState->getFocusedID();
    const Widget* p_wgt = getWidgetByWID(ctx, focused_id);
    bool key_handled = false;

    if (!p_wgt)
        return false;

    if (!isEnabled(ctx, p_wgt))
        return true;

    switch (p_wgt->type)
    {
    case Widget::TextEdit:
        key_handled = processKey_TextEdit(ctx, p_wgt, kc);
        break;
    case Widget::CheckBox:
        key_handled = processKey_CheckBox(ctx, p_wgt, kc);
        break;
    case Widget::Radio:
        key_handled = processKey_Radio(ctx, p_wgt, kc);
        break;
    case Widget::Button:
        key_handled = processKey_Button(ctx, p_wgt, kc);
        break;
    case Widget::PageCtrl:
        key_handled = processKey_PageCtrl(ctx, p_wgt, kc);
        break;
    case Widget::ListBox:
        key_handled = processKey_ListBox(ctx, p_wgt, kc);
        break;
    case Widget::ComboBox:
        key_handled = processKey_ComboBox(ctx, p_wgt, kc);
        break;
    case Widget::TextBox:
        key_handled = processKey_TextBox(ctx, p_wgt, kc);
        break;
    default:
        break;
    }

    return key_handled;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static void processMouse_TextEdit(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);
    }
}

static void processMouse_CheckBox(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);
        ctx.pState->onCheckboxToggle(pWgt);
        ctx.pState->invalidate(pWgt->id);
    }
}

static void processMouse_Radio(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);
        ctx.pState->onRadioSelect(pWgt);
        invalidateRadioGroup(ctx, pWgt);
    }
}

static void processMouse_Button(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    // pointer may change between onButtonUp and onButtonClick, so remember it
    auto *p_wstate = ctx.pState;

    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);
        p_wstate->onButtonDown(pWgt, kc);
        p_wstate->invalidate(pWgt->id);
    }
    else if (kc.mouse.btn == MouseBtn::ButtonReleased && g_ws.pMouseDownWgt == pWgt)
    {
        p_wstate->onButtonUp(pWgt, kc);
        p_wstate->onButtonClick(pWgt, g_ws.mouseDownKeyCode);
        g_ws.pMouseDownWgt = nullptr;
        p_wstate->invalidate(pWgt->id);
    }
    else
    {
        g_ws.pMouseDownWgt = nullptr;
    }
}

static void processMouse_Button_Release(CallCtx &ctx, const Widget *pWgt, const KeyCode &kc)
{
    auto *p_wstate = ctx.pState;

    p_wstate->onButtonUp(pWgt, kc);
    g_ws.pMouseDownWgt = nullptr;
    p_wstate->invalidate(pWgt->id);
}

static void processMouse_PageCtrl(CallCtx &ctx, const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);
        int idx = ctx.pState->getPageCtrlPageIndex(pWgt);
        int new_idx = kc.mouse.row - wgtRect.coord.row - 1 - pWgt->pagectrl.vertOffs;

        if (new_idx != idx && new_idx >= 0 && new_idx < pWgt->link.childrenCnt)
        {
            ctx.pState->onPageControlPageChange(pWgt, new_idx);
            ctx.pState->invalidate(pWgt->id);
        }
    }
    else if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        pgControlChangePage(ctx, pWgt, kc.mouse.btn == MouseBtn::WheelDown);
    }
}

static void processMouse_ListBox(CallCtx &ctx, const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    const uint16_t items_visible = pWgt->size.height-2;

    if (kc.mouse.btn == MouseBtn::ButtonLeft || kc.mouse.btn == MouseBtn::ButtonMid)
    {
        bool focus_changed = changeFocusTo(ctx, pWgt->id);

        int16_t idx = 0, selidx = 0, cnt = 0;
        ctx.pState->getListBoxState(pWgt, idx, selidx, cnt);

        if (cnt <= 0)
            return;

        int page = selidx / items_visible;
        unsigned new_selidx = page * items_visible;
        new_selidx += (int)kc.mouse.row - wgtRect.coord.row - 1;

        if (kc.mouse.btn == MouseBtn::ButtonLeft)
        {
            if (new_selidx < (unsigned)cnt && (((signed)new_selidx != selidx) || focus_changed))
            {
                selidx = new_selidx;
                ctx.pState->onListBoxSelect(pWgt, selidx);
            }
        }
        else
        {
            if (new_selidx < (unsigned)cnt && new_selidx != (unsigned)idx)
            {
                selidx = new_selidx;
                ctx.pState->onListBoxSelect(pWgt, selidx);
                ctx.pState->onListBoxChange(pWgt, selidx);
            }
        }

        ctx.pState->invalidate(pWgt->id);
    }
    else if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        changeFocusTo(ctx, pWgt->id);

        int16_t idx = 0, selidx = 0, cnt = 0;
        ctx.pState->getListBoxState(pWgt, idx, selidx, cnt);

        if (cnt <= 0)
            return;

        int delta = kc.mouse.btn == MouseBtn::WheelUp ? -1 : 1;
        if (kc.m_ctrl) delta *= items_visible;
        selidx += delta;

        if (selidx < 0)
            selidx = cnt - 1;

        if (selidx >= cnt)
            selidx = 0;

        ctx.pState->onListBoxSelect(pWgt, selidx);
        ctx.pState->invalidate(pWgt->id);
    }
}

static void processMouse_ComboBox(CallCtx &ctx, const Widget *pWgt, const Rect &wgtRect, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonLeft)
    {
        changeFocusTo(ctx, pWgt->id);

        auto col = kc.mouse.col - wgtRect.coord.col;
        auto row = kc.mouse.row - wgtRect.coord.row - 1;

        if (row >= 0 && row < pWgt->combobox.dropDownSize)
        {
            int16_t idx = 0, selidx = 0, cnt = 0; bool drop_down = false;
            ctx.pState->getComboBoxState(pWgt, idx, selidx, cnt, drop_down);
            selidx = (selidx / pWgt->combobox.dropDownSize) * pWgt->combobox.dropDownSize; // top item
            selidx += row;
            if (selidx < cnt)
            {
                ctx.pState->onComboBoxSelect(pWgt, selidx);
                ctx.pState->invalidate(pWgt->id);
            }
        }
        else if (col >= wgtRect.size.width - 3 && col <= wgtRect.size.width - 1)
        {
            // drop down arrow clicked
            int16_t _, cnt = 0; bool drop_down = false;
            ctx.pState->getComboBoxState(pWgt, _, _, cnt, drop_down);

            if (cnt <= 0)
                return;

            drop_down = !drop_down;

            if (drop_down)
            {
                ctx.pState->onComboBoxDrop(pWgt, true);
                ctx.pState->invalidate(pWgt->id);
                g_ws.pCbxDropDown = pWgt;
            }
            else
            {
                comboBoxHideList(ctx, pWgt);
            }
        }
    }
    else if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        changeFocusTo(ctx, pWgt->id);

        int16_t idx = 0, selidx = 0, cnt = 0; bool drop_down = false;
        ctx.pState->getComboBoxState(pWgt, idx, selidx, cnt, drop_down);

        if (!drop_down || cnt <= 0)
            return;

        int delta = kc.mouse.btn == MouseBtn::WheelUp ? -1 : 1;
        if (kc.m_ctrl) delta *= pWgt->combobox.dropDownSize;
        selidx += delta;

        if (selidx < 0)
            selidx = cnt - 1;

        if (selidx >= cnt)
            selidx = 0;

        ctx.pState->onComboBoxSelect(pWgt, selidx);
        ctx.pState->invalidate(pWgt->id);
    }
    else if (kc.mouse.btn == MouseBtn::ButtonMid)
    {
        twins::KeyCode key_left = kc;
        key_left.mouse.btn = MouseBtn::ButtonLeft;
        processMouse_ComboBox(ctx, pWgt, wgtRect, key_left);

        int16_t _, selidx = 0; bool drop_down = false;
        ctx.pState->getComboBoxState(pWgt, _, selidx, _, drop_down);

        if (!drop_down)
            return;

        ctx.pState->onComboBoxChange(pWgt, selidx);
        comboBoxHideList(ctx, pWgt);
    }
}

static void processMouse_CustomWgt(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    ctx.pState->onCustomWidgetInputEvt(pWgt, kc);
}

static void processMouse_TextBox(CallCtx &ctx, const Widget *pWgt, const Rect &/* wgtRect */, const KeyCode &kc)
{
    changeFocusTo(ctx, pWgt->id);

    if (kc.mouse.btn == MouseBtn::WheelUp || kc.mouse.btn == MouseBtn::WheelDown)
    {
        const twins::Vector<twins::CStrView> *p_lines = nullptr;
        int16_t top_line = 0;

        ctx.pState->getTextBoxState(pWgt, &p_lines, top_line);

        if (p_lines && p_lines->size())
        {
            int delta = kc.mouse.btn == MouseBtn::WheelUp ? -1 : 1;
            const uint16_t lines_visible = pWgt->size.height - 2;
            if (kc.m_ctrl) delta *= lines_visible;

            top_line += delta;

            if (top_line > (int)p_lines->size() - lines_visible)
                top_line = p_lines->size() - lines_visible;

            if (top_line < 0)
                top_line = 0;

            changeFocusTo(ctx, pWgt->id);
            ctx.pState->onTextBoxScroll(pWgt, top_line);
            ctx.pState->invalidate(pWgt->id);
        }
    }
}

static bool processMouse(CallCtx &ctx, const KeyCode &kc)
{
    if (kc.mouse.btn == MouseBtn::ButtonGoBack || kc.mouse.btn == MouseBtn::ButtonGoForward)
    {
        if (const auto *p_wgt = findMainPgControl(ctx))
        {
            if (isEnabled(ctx, p_wgt))
                pgControlChangePage(ctx, p_wgt, kc.mouse.btn == MouseBtn::ButtonGoForward);
            return true;
        }
    }

    Rect rct;
    const Widget *p_wgt = getWidgetAt(ctx, kc.mouse.col, kc.mouse.row, rct);

    if (g_ws.pMouseDownWgt)
    {
        // apply only for Button widget
        if (g_ws.pMouseDownWgt->type == Widget::Button)
        {
            // mouse button released over another widget - generate Up event for previously clicked button
            if (kc.mouse.btn == MouseBtn::ButtonReleased && g_ws.pMouseDownWgt != p_wgt)
            {
                processMouse_Button_Release(ctx, g_ws.pMouseDownWgt, kc);
                return true;
            }
        }
    }
    else if (p_wgt)
    {
        // remember clicked widget
        if (kc.mouse.btn >= MouseBtn::ButtonLeft && kc.mouse.btn < MouseBtn::ButtonReleased)
        {
            g_ws.pMouseDownWgt = p_wgt;
            g_ws.mouseDownKeyCode = kc;
        }
    }

    if (!p_wgt)
        return false;

    // TWINS_LOG_D("WidgetAt(%2d:%2d)=%s ID:%u", kc.mouse.col, kc.mouse.row, toString(p_wgt->type), p_wgt->id);

    if (g_ws.pCbxDropDown)
    {
        // check if drop-down list clicked
        Rect dropdownlist_rct;
        dropdownlist_rct.coord = getScreenCoord(g_ws.pCbxDropDown);
        dropdownlist_rct.size.width = g_ws.pCbxDropDown->size.width;
        dropdownlist_rct.size.height = g_ws.pCbxDropDown->combobox.dropDownSize + 1;

        if (isPointWithin(kc.mouse.col, kc.mouse.row, dropdownlist_rct))
        {
            // yes -> replace data for processing with g_ds.pDropDownCombo
            p_wgt = g_ws.pCbxDropDown;
            rct.coord = getScreenCoord(g_ws.pCbxDropDown);
            rct.size = g_ws.pCbxDropDown->size;
        }
        else
        {
            if (kc.mouse.btn == MouseBtn::ButtonLeft)
                comboBoxHideList(ctx, g_ws.pCbxDropDown);
        }
    }

    if (isEnabled(ctx, p_wgt))
    {
        switch (p_wgt->type)
        {
        case Widget::TextEdit:
            processMouse_TextEdit(ctx, p_wgt, rct, kc);
            break;
        case Widget::CheckBox:
            processMouse_CheckBox(ctx, p_wgt, rct, kc);
            break;
        case Widget::Radio:
            processMouse_Radio(ctx, p_wgt, rct, kc);
            break;
        case Widget::Button:
            processMouse_Button(ctx, p_wgt, rct, kc);
            break;
        case Widget::PageCtrl:
            processMouse_PageCtrl(ctx, p_wgt, rct, kc);
            break;
        case Widget::ListBox:
            processMouse_ListBox(ctx, p_wgt, rct, kc);
            break;
        case Widget::ComboBox:
            processMouse_ComboBox(ctx, p_wgt, rct, kc);
            break;
        case Widget::CustomWgt:
            processMouse_CustomWgt(ctx, p_wgt, rct, kc);
            break;
        case Widget::TextBox:
            processMouse_TextBox(ctx, p_wgt, rct, kc);
            break;
        default:
            moveToHome();
            g_ws.pMouseDownWgt = nullptr;
            return false;
        }
    }

    if (kc.mouse.btn == MouseBtn::ButtonReleased)
        g_ws.pMouseDownWgt = nullptr;

    return true;
}

// -----------------------------------------------------------------------------
// ---- TWINS  P U B L I C  FUNCTIONS ------------------------------------------
// -----------------------------------------------------------------------------

const char * toString(Widget::Type type)
{
    #define CASE_WGT_STR(t)     case Widget::t: return #t;

    switch (type)
    {
    CASE_WGT_STR(None)
    CASE_WGT_STR(Window)
    CASE_WGT_STR(Panel)
    CASE_WGT_STR(Label)
    CASE_WGT_STR(TextEdit)
    CASE_WGT_STR(CheckBox)
    CASE_WGT_STR(Radio)
    CASE_WGT_STR(Button)
    CASE_WGT_STR(Led)
    CASE_WGT_STR(PageCtrl)
    CASE_WGT_STR(Page)
    CASE_WGT_STR(ProgressBar)
    CASE_WGT_STR(ListBox)
    CASE_WGT_STR(ComboBox)
    CASE_WGT_STR(CustomWgt)
    CASE_WGT_STR(TextBox)
    CASE_WGT_STR(Layer)
    default: return "?";
    }
}

Coord getScreenCoord(const Widget *pWgt)
{
    Coord coord = pWgt->coord;
    if (pWgt->type == Widget::Type::Window)
        pWgt->window.getState()->getWindowCoord(pWgt, coord);

    if (pWgt->link.ownIdx > 0)
    {
        // go up the widgets hierarchy
        const auto *p_parent = getParent(pWgt);

        for (;;)
        {
            if (p_parent->type == Widget::Type::Window)
            {
                Coord wnd_coord = p_parent->coord;
                // getWindowCoord is optional
                p_parent->window.getState()->getWindowCoord(p_parent, wnd_coord);
                coord += wnd_coord;
            }
            else
            {
                coord += p_parent->coord;
            }

            if (p_parent->type == Widget::Type::PageCtrl)
            {
                coord.col += p_parent->pagectrl.tabWidth;
            }

            // top-level parent reached
            if (p_parent->link.ownIdx == 0)
                break;

            p_parent = getParent(p_parent);
        }
    }

    return coord;
}

const Widget* getWidget(const Widget *pWindowWidgets, WID widgetId)
{
    CallCtx ctx(pWindowWidgets);
    return getWidgetByWID(ctx, widgetId);
}

const Widget* getWidgetParent(const Widget *pWgt)
{
    assert(pWgt);
    return getParent(pWgt);
}

bool processInput(const Widget *pWindowWidgets, const KeyCode &kc)
{
    CallCtx ctx(pWindowWidgets);
    bool key_processed = false;

    if (kc.key == Key::None)
        return true;

    // TWINS_LOG_D("---");

    if (kc.key == Key::MouseEvent)
    {
        key_processed = processMouse(ctx, kc);
    }
    else
    {
        key_processed = processKey(ctx, kc);

        if (!key_processed && kc.m_spec)
        {
            if (g_ws.pCbxDropDown)
            {
                comboBoxHideList(ctx, g_ws.pCbxDropDown);
            }

            switch (kc.key)
            {
            case Key::Esc:
            {
                auto curr_id = ctx.pState->getFocusedID();
                auto new_id = getParentToFocus(ctx, curr_id);
                key_processed = changeFocusTo(ctx, new_id);
                break;
            }
            case Key::Tab:
            {
                auto curr_id = ctx.pState->getFocusedID();
                auto new_id = getNextToFocus(ctx, curr_id, !kc.m_shift);
                key_processed = changeFocusTo(ctx, new_id);
                break;
            }
            default:
                break;
            }
        }

        if (!key_processed)
            key_processed = ctx.pState->onWindowUnhandledInputEvt(ctx.pWidgets, kc);
    }

    return key_processed;
}

bool isWidgetVisible(const Widget *pWindowWidgets, const Widget *pWgt)
{
    CallCtx ctx(pWindowWidgets);
    return isVisible(ctx, pWgt);
}

bool isWidgetEnabled(const Widget *pWindowWidgets, const Widget *pWgt)
{
    CallCtx ctx(pWindowWidgets);
    return isEnabled(ctx, pWgt);
}

void resetInternalState()
{
    g_ws.pFocusedWgt = nullptr;
    g_ws.pMouseDownWgt = nullptr;
    g_ws.pCbxDropDown = nullptr;
    g_ws.textEditState.pWgt = nullptr;
}

// -----------------------------------------------------------------------------

namespace wgt
{

WID getPageID(const Widget *pPageControl, int8_t pageIdx)
{
    assert(pPageControl);
    assert(pPageControl->type == Widget::PageCtrl);

    if (pageIdx < 0 || pageIdx >= pPageControl->link.childrenCnt)
        return WIDGET_ID_NONE;

    const Widget *p_page = pPageControl;
    p_page += (pPageControl->link.childrenIdx - pPageControl->link.ownIdx) + pageIdx;
    return p_page->id;
}

int8_t getPageIdx(const Widget *pPageControl, WID pageID)
{
    assert(pPageControl);
    assert(pPageControl->type == Widget::PageCtrl);

    const Widget *p_page = pPageControl;
    p_page += pPageControl->link.childrenIdx - pPageControl->link.ownIdx;
    assert(p_page->type == Widget::Page);

    for (uint8_t i = 0; i < pPageControl->link.childrenCnt; i++)
        if (p_page[i].id == pageID)
            return i;

    return -1;
}

void selectPage(const Widget *pWindowWidgets, WID pageControlID, WID pageID)
{
    const auto *p_pgctrl = getWidget(pWindowWidgets, pageControlID);
    int8_t pg_idx = getPageIdx(p_pgctrl, pageID);

    if (pg_idx >= 0)
    {
        CallCtx ctx(pWindowWidgets);
        ctx.pState->onPageControlPageChange(p_pgctrl, pg_idx);
        ctx.pState->invalidate(pageControlID);
    }
    else
    {
        TWINS_LOG_W("Widget Id=%d is not PageControl Id=%d page", pageID, pageControlID);
    }
}

void selectNextPage(const Widget *pWindowWidgets, WID pageControlID, bool next)
{
    CallCtx ctx(pWindowWidgets);
    const auto *p_pgctrl = getWidget(pWindowWidgets, pageControlID);
    pgControlChangePage(ctx, p_pgctrl, next);
}

void markButtonDown(const Widget *pBtn, bool isDown)
{
    if (pBtn)
    {
        g_ws.pMouseDownWgt = isDown ? pBtn : nullptr;
    }
}

// -----------------------------------------------------------------------------

} // wgt

// -----------------------------------------------------------------------------

} // twins
