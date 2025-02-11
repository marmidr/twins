/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_input_posix.hpp"

// -----------------------------------------------------------------------------

TEST(INP_POSIX, getkey)
{
    twins::inputPosixInit(50);
    bool qreq = false;
    twins::inputPosixRead(qreq);
    twins::inputPosixFree();
}
