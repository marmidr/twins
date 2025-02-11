/******************************************************************************
 * @brief   TWins - base implementation of IWindowState
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins.hpp"

 // -----------------------------------------------------------------------------

namespace twins
{

/** @brief Basic, common implementation of interface */
class WindowStateBase : public IWindowState
{
public:
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

protected:
    WID mFocusedId;
    const Widget* mpWgts = nullptr;
};

//------------------------------------------------------------------------------

} // twins
