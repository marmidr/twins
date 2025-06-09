/******************************************************************************
 * @brief   TWins - base implementation of IWindowState
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins.hpp"
#include "twins_vector.hpp"

#include <functional>

// -----------------------------------------------------------------------------

namespace twins
{

/** @brief Basic, common implementation of the WindowState interface */
class WindowStateBase : public IWindowState
{
public:
    WindowStateBase() {}
    ~WindowStateBase() {}

    void init(const twins::Widget* pWindowWgts) override
    {
        mFocusedId = WIDGET_ID_NONE;
        mpWgts = pWindowWgts;
    }

    const twins::Widget* getWidgets() const override
    {
        return mpWgts;
    }

    twins::WID& getFocusedID() override
    {
        return mFocusedId;
    }

    bool isFocused(const twins::Widget* pWgt) override
    {
        return pWgt->id == mFocusedId;
    }

    void invalidateImpl(const twins::WID *pId, uint16_t count, bool instantly) override
    {
        (void)instantly;

        if (count == 1 && *pId == twins::WIDGET_ID_NONE)
            return;

        // state or focus changed - widget must be repainted
        if (getWidgets())
        {
            twins::drawWidgets(getWidgets(), pId, count);
            twins::flushBuffer();
        }
        else
        {
            TWINS_LOG_E("Window state not initialized");
        }
    }

    /** Iterates through the direct children widgets */
    void forEachChild(twins::WID parentID, std::function<void(const twins::Widget* pWgt)> cbk)
    {
        const auto *p_wnd_children = getWidgets();
        const auto *p_parent = twins::getWidget(p_wnd_children, parentID);
        if (p_parent)
        {
            for (unsigned i = p_parent->link.childrenIdx; i < p_parent->link.childrenIdx + p_parent->link.childrenCnt; i++)
                cbk(p_wnd_children + i);
        }
    }

public:
    twins::Vector<twins::WID> invalidatedWgts;

protected:
    WID mFocusedId;
    const Widget* mpWgts = nullptr;
};

//------------------------------------------------------------------------------

} // twins
