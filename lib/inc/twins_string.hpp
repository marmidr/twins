/******************************************************************************
 * @brief   TWins - string class for our needs
 * @author  Mariusz Midor
 *          https://bitbucket.org/mmidor/twins
 *****************************************************************************/

#pragma once
#include <stdint.h>
#include <stdarg.h>

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief
 */
class String
{
public:
    // avoid memory allocation in constructor
    String() = default;
    ~String();

    void append(const char *s);
    void append(char c, uint16_t count = 1);
    void appendFmt(const char *fmt, ...);
    void trim(uint16_t trimPos, bool addEllipsis = false);
    void clear();
    String& operator=(const char *s);
    int size() const { return mSize; }
    int utf8Len() const;
    const char *cstr() const { return mpBuff ? mpBuff : ""; }

private:
    void resize(uint16_t newCapacity);

private:
    char *  mpBuff = nullptr;
    int16_t mCapacity = 0;
    int16_t mSize = 0;
};

// -----------------------------------------------------------------------------

} // namespace
