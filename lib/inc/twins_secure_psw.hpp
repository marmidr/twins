/******************************************************************************
 * @brief   TWins - secure password
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_hash.hpp"
#include <stdint.h>

// -----------------------------------------------------------------------------

namespace twins
{

// -----------------------------------------------------------------------------

/** @brief Class that keeps the password in mangled form */
class SecurePassw
{
public:
    /** @brief Default constructor */
    constexpr SecurePassw() {}

    /** @brief Constructor that encodes given password */
    constexpr SecurePassw(const char* psw)
    {
        // compute length
        uint16_t psw_len = 0;
        while (psw[psw_len] != '\0')
            ++psw_len;

        mPswLen = psw_len;
        mPswHash = HashDefault::hash(psw);
    }

    /** @brief Compares given string against the encoded password */
    bool operator==(const char* psw) const
    {
        // compare lengths
        uint16_t psw_len = 0;
        while (psw[psw_len] != '\0')
            ++psw_len;

        if (psw_len == 0 || psw_len != mPswLen)
            return false;

        // compare hash values
        return mPswHash == HashDefault::hash(psw);
    }

    /** @brief If valid, compares own and \p other's encoded passwords */
    bool operator==(const SecurePassw& other) const
    {
        return (mPswLen > 0) && (mPswLen == other.mPswLen) && (mPswHash == other.mPswHash);
    }

    /** @brief Check if object contains a password */
    bool hasPassword() const { return mPswLen > 0; }

private:
    uint16_t mPswLen{};
    uint32_t mPswHash{};
};

// -----------------------------------------------------------------------------

} // twins
