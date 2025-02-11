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
    static uint32_t bernsteinHashImpl(const void *data, unsigned length, uint32_t seed = 5381)
    {
        const char *p = (const char*)data;
        // modified Dan Bernstein hash function for strings
        uint32_t hash = seed;
        for (unsigned i = 0; i < length; i++)
            hash = ((hash << 5) + hash) ^ *p++; // (hash * 33) ^ *p
        return hash;
    }

    template <typename Tp, typename std::enable_if<
        std::is_same<const char*, Tp>::value || std::is_same<char*, Tp>::value, int>::type = 0>
    static uint16_t hash(Tp v)
    {
        return bernsteinHashImpl(v, strnlen(v, 15));
    }

    template <typename Tp, typename std::enable_if<
        std::is_integral<Tp>::value || std::is_enum<Tp>::value, int>::type = 0>
    static uint16_t hash(Tp v)
    {
        return bernsteinHashImpl(&v, sizeof(v));
    }

    template <typename Tp, typename std::enable_if<
        std::is_floating_point<Tp>::value, int>::type = 0>
    static uint16_t hash(Tp v)
    {
        // for both -0.0 and 0.0
        return v != Tp{} ? bernsteinHashImpl(&v, sizeof(v)) : 0;
    }

    static uint16_t hash(bool v)
    {
        return static_cast<uint16_t>(v);
    }

};

// -----------------------------------------------------------------------------

} // twins
