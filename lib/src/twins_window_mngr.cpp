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

void WndManager::show(twins::IWindowState *pWnd, bool bringToTop)
{
    int idx = -1;

    if (mWindows.find(pWnd, &idx))
    {
        // is on the list
        if (idx < (int)(mWindows.size())-1)
        {
            // ...and is not on top
            if (bringToTop)
            {
                mWindows.remove(idx, true);
                mWindows.append(pWnd);
                redrawAll();
            }
        }
        else
        {
            twins::drawWidget(pWnd->getWidgets());
        }
    }
    else if (pWnd)
    {
        mWindows.append(pWnd);
        twins::resetInternalState();
        twins::drawWidget(pWnd->getWidgets());
    }
}

void WndManager::hide(twins::IWindowState *pWnd)
{
    int idx = -1;

    if (mWindows.find(pWnd, &idx))
    {
        mWindows.remove(idx);

        if (mWindows.size())
        {
            redrawAll();
        }
        else
        {
            twins::screenClrAll();
            twins::flushBuffer();
        }
    }
}

bool WndManager::visible(twins::IWindowState *pWnd) const
{
    return pWnd ? mWindows.contains(pWnd) : false;
}

const twins::Widget* WndManager::topWndWidgets()
{
    return mWindows.size() ? topWnd()->getWidgets() : nullptr;
}

void WndManager::redrawAll()
{
    for (auto p_wnd : mWindows)
    {
        twins::drawWidget(p_wnd->getWidgets());
        // signal that invalidate list must be cleared
        p_wnd->invalidate(WIDGET_ID_NONE);
    }

    twins::flushBuffer();
}

// -----------------------------------------------------------------------------

} // twins
