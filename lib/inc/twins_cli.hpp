/******************************************************************************
 * @brief   TWins - command line interface
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_vector.hpp"
#include "twins_string.hpp"
#include "twins_secure_psw.hpp"
#include "twins_ringbuffer.hpp"

#include <stdint.h>
#include <functional>
#include <initializer_list>

#ifndef TWINS_CLI_LIGHTWEIGHT_CMD
# define TWINS_CLI_LIGHTWEIGHT_CMD  1
#endif

// -----------------------------------------------------------------------------

namespace twins::cli
{

using Argv = Vector<const char*>;
using CmdHandler = std::function<void(twins::cli::Argv &argv)>;

#define TWINS_CLI_HANDLER               .handler = [](twins::cli::Argv &argv)
#define TWINS_CLI_ACCESS_UNRESTRICTED   0x00
// 🔒 🔐 🔑
#define TWINS_CLI_LOCK_SYMBOL           "🔒"

/**
 * @brief Struct holding command name and pointer to handler function
 * if \p argc is 1+, the \p argv holds command arguments
 */
struct Cmd
{
    const char* name;
    const char* help;
    uint8_t access = TWINS_CLI_ACCESS_UNRESTRICTED;
    #if TWINS_CLI_LIGHTWEIGHT_CMD
    void (*handler)(twins::cli::Argv &argv);
    #else
    CmdHandler handler;
    #endif
};

// -----------------------------------------------------------------------------

using History = Vector<String>;

/** @brief Controls command parser debug output */
extern bool verbose;

/** @brief Echo NL if CR (Enter) key detected */
extern bool echoNlAfterCr;

/** @brief Used together with the password mode */
extern uint8_t accessFlags;

/**
 * @brief Reset internal state: buffers, counters, cursor
 */
void reset(void);

/**
 * @brief Sets a valid passwords to be entered; if \p passwords is empty, password mode is disabled.
 * @param onPasswordMatchPrompt - message printed when correct password is entered
 */
void passwordSet(const std::initializer_list<twins::SecurePassw> &passwords, const char *onPasswordMatchPrompt = "");

/**
 * @brief Activates/deactivates a password entering mode (echoes *** instead of letters)
 * @param en    enable/disable password mode
 */
void passwordModeEnable(bool en);

/**
 * @brief Returns state of password mode. It is automatically reset to \p false when correct password is provided.
 */
bool passwordModeIsEnabled();

/**
 * @brief Returns the entered correct password or empty object
 */
const twins::SecurePassw& passwordValue();

/**
 * @brief Process \p data, emit echo
 */
void processInput(const char* data, uint8_t dataLen = 0);

/**
 * @brief Process the ring buffer \p rb, emit echo
 */
void processInput(twins::RingBuff<char> &rb);

/**
 * @brief CRLF >
 */
void prompt(bool newLn = true);

/**
 * @brief Returns the command history
 */
History& getHistory(void);

/**
 * @brief Compares the \p cmdAccessFlags against the \c accessFlags
 * @param cmdAccessFlags    Combination of bitfields
 * @param promptAccDenied   Prompt a message that command access is forbidden
 * @param enterPasswMode    if access is forbidden and promptAccDenied==true, enable the password enter mode
 * @retval true if the command is allowed
 */
bool checkAccessGranted(uint8_t cmdAccessFlags, bool promptAccDenied = true, bool enterPasswMode = true);

/**
 * @brief If line ends with '\r', call the matching \p commands handler
 * @param pCommands array of \b Cmd, terminated with empty cmd {}
 * @param lastCommandSet set to false if number of command arrays are to be processed
 *        to avoid printing "unknown command" and clearing internal command queue.
 *        Remember to set it to true in the last call to remove command line from the internal queue.
 * @return true if command line is complete and handler was found and executed
 */
bool checkAndExec(const Cmd* pCommands, bool lastCommandSet = true);

/**
 * @brief Execute command line \p cmdline.
 * @param cmdline command and arguments; terminator \b \r not required
 * @param pCommands array of \b Cmd, terminated with empty cmd {}
 * @return true if handler was found and executed
 */
bool execLine(const char *cmdline, const Cmd* pCommands);

/**
 * @brief Set the override command handler used when \c checkAndExec() called;
 *        call again with \c {} to restore normal behavior
 * @param handler
 */
void setOverrideHandler(CmdHandler handler);

// -----------------------------------------------------------------------------

} // twins::cli
