/******************************************************************************
 * @brief   TWins - keyboard input - POSIX
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"

#include <stdint.h>

// -----------------------------------------------------------------------------

#if TWINS_ENV_LINUX_LIKE
namespace twins
{

/** @brief Initialze keyboard device reader */
void inputPosixInit(uint16_t timeoutMs);

/** @brief Free keyboard device reader resources */
void inputPosixFree();

/** @brief Read codes from keyboard and returns pointer to nul-terminated buffer
  *        If \p quitRequested is set, application is requested to end
  * @return nul-terminated C-string
  */
const char * inputPosixRead(bool &quitRequested);

// -----------------------------------------------------------------------------

} // twins

#endif // TWINS_ENV_LINUX_LIKE
