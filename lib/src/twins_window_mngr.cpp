/******************************************************************************
 * @brief   TWins - manager of windows
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins_window_mngr.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

// -----------------------------------------------------------------------------

void WndManager::show(twins::IWindowState *pWndState, bool bringToTop)
{
    int idx = -1;

    if (mWndStates.find(pWndState, &idx))
    {
        // is on the list
        if (idx < (int)(mWndStates.size())-1)
        {
            // ...and is not on top
            if (bringToTop)
            {
                mWndStates.remove(idx, true);
                mWndStates.append(pWndState);
                pWndState->onBeforeShow();
                redrawAll();
            }
        }
        else
        {
            twins::drawWidget(pWndState->getWidgets());
        }
    }
    else if (pWndState)
    {
        mWndStates.append(pWndState);
        twins::resetInternalState();
        pWndState->onBeforeShow();
        twins::drawWidget(pWndState->getWidgets());
    }
}

void WndManager::hide(twins::IWindowState *pWndState, bool redrawAllWindows)
{
    int idx = -1;

    if (mWndStates.find(pWndState, &idx))
    {
        twins::Rect popup_rect = {
            .coord = twins::getScreenCoord(pWndState->getWidgets()),
            .size = pWndState->getWidgets()->size
        };

        mWndStates.remove(idx);
        twins::resetInternalState();

        if (mWndStates.size())
        {
            if (redrawAllWindows)
            {
                redrawAll();
            }
            else
            {
                if (auto p_wgt = twins::findCoveringWidget(topWnd()->getWidgets(), popup_rect))
                    topWnd()->invalidate(p_wgt->id);
                else
                    redrawAll();
            }
        }
        else
        {
            twins::screenClrAll();
            twins::flushBuffer();
        }
    }
}

bool WndManager::visible(twins::IWindowState *pWndState) const
{
    return pWndState ? mWndStates.contains(pWndState) : false;
}

const twins::Widget* WndManager::topWndWidgets()
{
    return mWndStates.size() ? topWnd()->getWidgets() : nullptr;
}

void WndManager::redrawAll()
{
    for (auto p_wstate : mWndStates)
    {
        twins::drawWidget(p_wstate->getWidgets());
        // signal that invalidate list must be cleared
        p_wstate->invalidate(WIDGET_ID_NONE);
    }

    twins::flushBuffer();
}

// -----------------------------------------------------------------------------

} // twins
