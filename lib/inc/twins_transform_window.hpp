/******************************************************************************
 * @brief   TWins - window definition constexpr transformation
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *          Proof of concept: https://godbolt.org/z/TkMfcC
 *****************************************************************************/

#pragma once

// field set in user Widget definition;
#define __TWINS_LINK_SECRET     const Widget *pChildren

#include "twins.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

// ad-hoc solution for constexpr assert() resulting compilation error if ill-configured widget is found;
// assert and printf are non-constexpr, so...
// error: array subscript value ‘-49’ is outside the bounds of array type ‘const char [5]’ --> problem with widget ID=49
constexpr char cexpr_assert(const twins::Widget *pWgt, bool cond)
{
    return cond ? '1' : "ID"[-pWgt->id];
}

constexpr void checkWidgetParams(const twins::Widget *pWgt) noexcept
{
    if (pWgt->type == Widget::ComboBox)
    {
        cexpr_assert(pWgt, pWgt->size.height == 1);
        cexpr_assert(pWgt, pWgt->combobox.dropDownSize > 0);
    }

    if (pWgt->type == Widget::ProgressBar)
        cexpr_assert(pWgt, pWgt->size.height == 1);

    // if (pWgt->type == Widget::Button)
    //     cexpr_assert(pWgt, pWgt->button.text || (pWgt->size.width >= 2));
}

/**
 * @brief Count all widgets in pointed window (or any other type of widget)
 */
constexpr int getWgtsCount(const twins::Widget *pWgt)
{
    int n = 1;
    checkWidgetParams(pWgt);

    for (const auto *ch = pWgt->link.pChildren; ch && ch->id != twins::WIDGET_ID_NONE; ch++)
        n += getWgtsCount(ch);

    return n;
}

/**
 * @brief Count all Pages in pointed window
 */
constexpr int getPagesCount(const twins::Widget *pWgt)
{
    int n = pWgt->type == twins::Widget::Page;

    for (const auto *ch = pWgt->link.pChildren; ch && ch->id != twins::WIDGET_ID_NONE; ch++)
        n += getWgtsCount(ch);

    return n;
}

/**
 * @brief Recurrent widget transformation function
 *        Widgets that belongs to the same parent are put together, next to each other, to ease TAB navigation.
 * @par arr Destination array of widgets
 * @par pWgt Pointer to Window - the root widget
 */
template<unsigned N>
constexpr int transformWidgetTreeToArray(twins::Array<twins::Widget, N> &arr, const twins::Widget* pWgt, const int wgtIdx, int freeSlotIdx)
{
    int child_idx = freeSlotIdx;

    // copy widget
    arr[wgtIdx] = *pWgt;
    arr[wgtIdx].link = {};
    arr[wgtIdx].link.ownIdx = wgtIdx;

    // count the children
    int n_children = 0;
    for (auto *p_child = pWgt->link.pChildren; p_child && p_child->id != twins::WIDGET_ID_NONE; p_child++)
        n_children++;

    arr[wgtIdx].link.childrenIdx = child_idx;
    arr[wgtIdx].link.childrenCnt = n_children;
    freeSlotIdx += n_children;

    // copy children
    for (auto *p_child = pWgt->link.pChildren; p_child && p_child->id != twins::WIDGET_ID_NONE; p_child++)
    {
        freeSlotIdx = transformWidgetTreeToArray<N>(arr, p_child, child_idx, freeSlotIdx);
        arr[child_idx].link.parentIdx = wgtIdx;

        if (arr[child_idx].type == Widget::Layer)
        {
            // layer coord is always 0,0
            arr[child_idx].coord = {};
            // layer size is always it's parent size
            arr[child_idx].size = arr[wgtIdx].size;
        }

        child_idx++;
    }

    return freeSlotIdx;
}

template<const twins::Widget *pWINDOW, int N = getWgtsCount(pWINDOW) + 1>
constexpr twins::Array<twins::Widget, N> transforWindowDefinition()
{
    static_assert(N < 65000, "Limit of widgets per window reached");
    twins::Array<twins::Widget, N> arr;

    transformWidgetTreeToArray<N>(arr, pWINDOW, 0, 1);
    return arr;
}

// -----------------------------------------------------------------------------

} // twins
