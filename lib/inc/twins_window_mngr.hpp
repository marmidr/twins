/******************************************************************************
 * @brief   TWins - manager of windows
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
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

    /** @brief show \p pWnd if not visible */
    void show(twins::IWindowState *pWnd, bool bringToTop = false);

    /** @brief hide window */
    void hide(twins::IWindowState *pWnd);

    /** @brief return top window */
    twins::IWindowState *topWnd()
    {
        assert(mWindows.size());
        return *mWindows.back();
    }

    /** @brief check if given window is on the list */
    bool visible(twins::IWindowState *pWnd) const;

    /** @brief return top window widgets or nullptr */
    const twins::Widget* topWndWidgets();

    /** @brief number of windows on stack */
    unsigned size() const { return mWindows.size(); }

    /** @brief redraw windows from bottom to top */
    void redrawAll();

    /** all windows iterator */
    auto begin() { return mWindows.begin(); }
    auto end()   { return mWindows.end(); }

private:
    twins::Vector<twins::IWindowState*> mWindows;
};

// -----------------------------------------------------------------------------

} // twins
