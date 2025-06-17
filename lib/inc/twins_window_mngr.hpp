/******************************************************************************
 * @brief   TWins - manager of windows
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins.hpp"
#include "twins_vector.hpp"

#include <assert.h>

// -----------------------------------------------------------------------------

namespace twins
{

/** @brief Struct holding windows' state stack */
class WndManager
{
public:
    ~WndManager() { /* printf("~WndManager()\n");*/ }

    /** @brief show \p pWndState if not visible */
    void show(twins::IWindowState *pWndState, bool bringToTop = false);

    /** @brief hide the \p pWndState window, redrawing the content underneath */
    void hide(twins::IWindowState *pWndState, bool redrawAllWindows = false);

    /** @brief returns the top window; throw assertion if the list is empty */
    twins::IWindowState *topWnd()
    {
        assert(mWndStates.size());
        return *mWndStates.back();
    }

    /** @brief check if given window is on the list */
    bool visible(twins::IWindowState *pWndState) const;

    /** @brief return the top window widgets or nullptr */
    const twins::Widget* topWndWidgets();

    /** @brief number of windows on stack */
    unsigned size() const { return mWndStates.size(); }

    /** @brief redraw all windows from bottom to top */
    void redrawAll();

    /** all windows iterator */
    auto begin() { return mWndStates.begin(); }
    auto end()   { return mWndStates.end(); }

private:
    twins::Vector<twins::IWindowState*> mWndStates;
};

// -----------------------------------------------------------------------------

} // twins
