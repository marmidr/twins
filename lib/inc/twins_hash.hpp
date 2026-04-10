/******************************************************************************
 * @brief   TWins - hashing functions
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include <stdint.h>
#include <type_traits>

// -----------------------------------------------------------------------------

namespace twins
{

struct HashDefault
{
    // due to the pointer-casting, cannot be constexpr
    static uint32_t bernsteinHashImpl(const void *data, uint32_t length, uint32_t seed = 5381)
    {
        // modified Dan Bernstein hash function for strings
        uint32_t hash = seed;
        for (uint32_t i = 0; i < length; i++)
            hash = ((hash << 5) + hash) ^ static_cast<const uint8_t*>(data)[i]; // (hash * 33) ^ *p
        return hash;
    }

    template <typename Tp, typename std::enable_if<
        std::is_same<Tp, char*>::value || std::is_same<Tp, const char*>::value, int>::type = 0>
    constexpr static uint32_t hash(Tp str)
    {
        uint32_t hash = 5381;
        uint32_t i = 0;
        while (str[i] && i < 30)
        {
            hash = ((hash << 5) + hash) ^ static_cast<uint8_t>(str[i]); // (hash * 33) ^ v[i]
            ++i;
        }
        return hash;
    }

    template <typename Tp, typename std::enable_if<
        std::is_integral<Tp>::value || std::is_enum<Tp>::value || std::is_floating_point<Tp>::value, int>::type = 0>
    constexpr static uint32_t hash(Tp v)
    {
        if (v == Tp{})
            return 0;

        const auto* bytes = reinterpret_cast<const uint8_t*>(&v);
        uint32_t hash = 5381;
        for (uint32_t i = 0; i < sizeof(v); ++i)
        {
            hash = ((hash << 5) + hash) ^ bytes[i];
        }
        return hash;
    }
};

// -----------------------------------------------------------------------------

} // twins
