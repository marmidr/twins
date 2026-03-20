/******************************************************************************
 * @brief   TWins - secure password
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include <stdint.h>

// -----------------------------------------------------------------------------

namespace twins
{

// -----------------------------------------------------------------------------

/** @brief Class that keeps the password in mangled form */
class SecurePassw
{
public:
    static constexpr uint16_t PSW_MAX_LEN = 20;

    /** @brief Constructor that encodes given password */
    constexpr SecurePassw(const char* psw)
    {
        // compute length
        uint16_t psw_len = 0;
        while (psw[psw_len] != '\0')
            ++psw_len;

        if (psw_len >= PSW_MAX_LEN)
        {
            // error - provided password is too long
            return;
        }

        mPswLen = psw_len;

        // fill permutation map
        for (uint16_t k = 0; k < mPswLen; ++k)
            mPswMap[k] = permute(k);

        // store scrambled password
        for (uint16_t k = 0; k < mPswLen; ++k)
            mScrambledPsw[ mPswMap[k] ] = psw[k];
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

        // compare characters using reverse mapping
        for (uint16_t k = 0; k < mPswLen; ++k)
        {
            uint8_t pos = mPswMap[k];
            if (mScrambledPsw[pos] != psw[k])
                return false;
        }

        return true;
    }

    /** @brief Check if object contains a password */
    bool hasPassword() const { return mPswLen > 0; }

private:
    uint16_t mPswLen{};
    uint8_t  mPswMap[PSW_MAX_LEN]{};   // permutation mapping
    char     mScrambledPsw[PSW_MAX_LEN]{};

    // simple constexpr pseudo-random permutation generator
    static constexpr uint8_t permute(uint8_t i)
    {
        // cheap reversible mixing function
        return (i * 17 + 23) % PSW_MAX_LEN;
    }
};

// -----------------------------------------------------------------------------

} // twins
