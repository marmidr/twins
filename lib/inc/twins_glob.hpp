/******************************************************************************
 * @brief   TWins - global namespace (not used by the TWins itself)
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"
#include "twins_window_mngr.hpp"

// -----------------------------------------------------------------------------

namespace twins
{

namespace glob
{
/** @brief reference to actual PAL */
extern twins::IPal& pal;
/** @brief window manager */
extern twins::WndManager& wMngr;
/** @brief bottom-most window widgets */
extern const twins::Widget* pMainWindowWgts;
} // glob

// -----------------------------------------------------------------------------

} // twins
