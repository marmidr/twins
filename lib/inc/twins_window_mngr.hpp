/******************************************************************************
 * @brief   TWins - manager of windows
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#pragma once

#include "twins.hpp"
#include "twins_vector.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

/** @brief struct holding window stack */
class WndManager
{
public:
    ~WndManager() { /* printf("~WndManager()\n");*/ }

    /** @brief show \p pWindow on top */
    void pushWnd(twins::IWindowState *pWindow);


    /** @brief hide top window */
    void popWnd();

    /** @brief return top window */
    twins::IWindowState *topWnd()
    {
        assert(mWindows.size());
        return *mWindows.back();
    }

    /** @brief return top window widgets or nullptr */
    const twins::Widget* topWndWidgets()
    {
        return mWindows.size() ? topWnd()->getWidgets() : nullptr;
    }

    /** @brief number of windows on stack */
    unsigned size() const { return mWindows.size(); }

    /** @brief redraw windows from bottom to top */
    void redrawAll();

    /** all windows iterator */
    const auto begin() { return mWindows.begin(); }
    const auto end()   { return mWindows.end(); }

private:
    twins::Vector<twins::IWindowState *> mWindows;
};

// -----------------------------------------------------------------------------

} // namespace
