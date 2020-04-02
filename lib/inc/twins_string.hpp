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
    String(const char*) = delete;
    String() = default;
    String(String &&other);
    ~String();

    /** @brief Append \p count of given string \p s */
    void append(const char *s, int16_t count = 1);
    /** @brief Append new string \p s, size of \p sLen bytes */
    void appendl(const char *s, int16_t sLen);
    /** @brief Append \p count of given characters \p c */
    void append(char c, int16_t count = 1);
    /** @brief Append formatted string */
    void appendFmt(const char *fmt, ...);
    /** @brief Trim string that is too long to fit; optionally append ellipsis ... at the \p trimPos */
    void trim(int16_t trimPos, bool addEllipsis = false);
    /** @brief If shorter than len - add spaces; if longer - calls trim */
    void setLength(int16_t len, bool addEllipsis = false);
    /** @brief Iterate over lines */
    //void forEachLine(std::function<void(String &line)> &callback);
    /** @brief Set size to zero; does not release buffer memory */
    void clear();
    /** @brief Return string size, in bytes */
    unsigned size() const { return mSize; }
    /** @brief Return string length, in characters, assuming UTF-8 encoding */
    unsigned utf8Len() const;
    /** @brief Return C-style string buffer */
    const char *cstr() const { return mpBuff ? mpBuff : ""; }
    /** @brief Convenient assign operators */
    String& operator=(const char *s);
    String& operator=(const String &other) { *this = other.cstr(); return *this; }
    String& operator=(String &&other);

private:
    void resize(uint16_t newCapacity);
    void free();

private:
    char *  mpBuff = nullptr;
    int16_t mCapacity = 0;
    int16_t mSize = 0;
};

// -----------------------------------------------------------------------------

} // namespace
