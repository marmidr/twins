/******************************************************************************
 * @brief   TWins - keyboard input - POSIX
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 * @note    Most of this code comes from http://0x80.pl/articles/terminals.html
 *****************************************************************************/

#include "twins_input_posix.hpp"
#include "twins_common.hpp"

#if TWINS_ENV_LINUX_LIKE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// -----------------------------------------------------------------------------

namespace twins
{

static int         ttyFileNo;
static char        termEOFcode;
static termios     termIOs;
static uint16_t    termKeyTimeoutMs;
static char        termBuff[8];

// -----------------------------------------------------------------------------

static void readKey()
{
    // read up to 8 bytes
    int nb = read(ttyFileNo, termBuff, sizeof(termBuff)-1);
    if (nb == -1) nb = 0;
    termBuff[nb] = '\0';
    errno = 0;
}

static void waitForKey()
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(ttyFileNo, &read_set);

    // wait for key
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = termKeyTimeoutMs * 1000;
    int rc = select(ttyFileNo + 1, &read_set, nullptr, nullptr, &tv);

    if (rc > 0)
        readKey();
}

static void checkErrNo(int line)
{
    // TODO: always fail if in GDB session
    //  https://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb
    if (errno)
    {
        fprintf(stderr, "-E- %s:%u - %s", __FILE__, line, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// -----------------------------------------------------------------------------

void inputPosixInit(uint16_t timeoutMs)
{
    termKeyTimeoutMs = timeoutMs;
    errno = 0;
    ttyFileNo = open("/dev/tty", O_RDONLY | O_NONBLOCK);

    if (errno == 0)
    {
        // get terminal configuration
        tcgetattr(ttyFileNo, &termIOs);
        checkErrNo(__LINE__);
        termEOFcode = termIOs.c_cc[VEOF];

        // disable canonical mode and echo
        termIOs.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(ttyFileNo, TCSAFLUSH, &termIOs);
        checkErrNo(__LINE__);
    }
    else
    {
        fprintf(stderr, "-E- %s:%u: cannot open /dev/tty\n", __FILE__, __LINE__);
    }
}

void inputPosixFree()
{
    if (ttyFileNo != -1)
    {
        // restore terminal configuration
        termIOs.c_lflag |= (ICANON | ECHO);
        tcsetattr(ttyFileNo, TCSAFLUSH, &termIOs);
        checkErrNo(__LINE__);
        close(ttyFileNo);
    }
}

const char* inputPosixRead(bool &quitRequested)
{
    termBuff[0] = '\0';

    if (ttyFileNo != -1)
    {
        waitForKey();

        if (termBuff[1] == '\0' && termBuff[0] == termEOFcode)
            quitRequested = true;
    }

    return termBuff;
}

// -----------------------------------------------------------------------------

} // twins

#endif // TWINS_ENV_LINUX_LIKE
