/******************************************************************************
 * @brief   TWins - string class for our needs
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <utility> // move

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief The heart of this library
 */
class String
{
public:
    String(const char *s);
    String() = default;
    String(const String &other) : String(other.cstr()) {}
    String(String &&other) noexcept;
    ~String();

    /** @brief Append \p repeat of given string \p s */
    String& append(const char *s, int16_t repeat = 1);
    /** @brief Append new string \p s, size of \p sLen bytes */
    String& appendLen(const char *s, int16_t sLen);
    /** @brief Append \p repeat of given characters \p c */
    String& append(char c, int16_t repeat = 1);
    /** @brief Append new string \p s */
    inline String& append(const String &s) { return append(s.cstr()); }
    /** @brief Append formatted string */
    String& appendFmt(const char *fmt, ...);
    void appendVFmt(const char *fmt, va_list ap);
    /** @brief Trim string that is too long to fit; optionally append ellipsis ... at the \p trimPos */
    String& trim(int16_t trimPos, bool addEllipsis = false, bool ignoreESC = false);
    /** @brief Erase \p len characters from string at \p pos */
    String& erase(int16_t pos, int16_t len = 1);
    /** @brief Insert string \p s at \p pos */
    String& insert(int16_t pos, const char *s, int16_t repeat = 1);
    /** @brief If shorter than \p newWidth - add spaces; if longer - calls trim */
    void setWidth(int16_t newWidth, bool addEllipsis = false);
    /** @brief Set size to zero; release buffer memory only if capacity >= \p threshordToFree */
    String& clear(uint16_t threshordToFree = 500);
    /** @brief Return string size, in bytes */
    inline uint16_t size() const { return mSize; }
    /** @brief Return length of UTF-8 string, ignoring ESC sequences inside it
     *         and recognizing double-width glyphs */
    uint16_t u8len(bool ignoreESC = false, bool realWidth = false) const;
    /** @brief Text width on terminal */
    inline uint16_t width() const { return u8len(true, true); }
    /** @brief Return C-style string buffer */
    inline const char* cstr() const { return mpBuff ? mpBuff : ""; }
    /** @brief Reserve buffer if u know the string size in advance */
    void reserve(uint16_t newCapacity);
    /** @brief Useful tests */
    bool startsWith(const char *str) const;
    bool endsWith(const char *str) const;
    int  find(const char *str) const;
    bool contains(const char *str) const { return find(str) >= 0; }
    /** @brief Convenient assign operators */
    String& operator =(const char *s);
    String& operator =(const String &other);
    String& operator =(String &&other);
    /** @brief Convenient append operator */
    String& operator <<(char c) { return append(c); }
    String& operator <<(const char *s) { return append(s); }
    String& operator <<(const String &s) { return appendLen(s.cstr(), s.size()); }
    /** @brief Equality operator */
    bool operator==(const String &s) const { return operator==(s.cstr()); }
    bool operator==(const char *str) const;

    /** @brief Return ESC sequence length starting at \p str */
    static uint16_t escLen(const char *str, const char *strEnd = nullptr);
    /** @brief Return length of UTF-8 string \p str, ignoring ESC sequences inside it
     *         and recognizing double-width glyphs */
    static uint16_t u8len(const char *str, const char *strEnd = nullptr, bool ignoreESC = false, bool realWidth = false);
    /** @brief Text width on terminal */
    static inline uint16_t width(const char *str, const char *strEnd = nullptr) { return u8len(str, strEnd, true, true); }
    /** @brief Return pointer to \p str moved by \p toSkip UTF-8 characters, omitting ESC sequences */
    static const char* u8skip(const char *str, unsigned toSkip, bool ignoreESC = true);

protected:
    void freeBuff();
    uint16_t alignCapacity(uint16_t newCapacity) const;
    bool sourceIsOurs(const char *s) const { return (s >= mpBuff) && (s < mpBuff + mCapacity); }

    char* mpBuff = nullptr;
    uint16_t mCapacity = 0;
    uint16_t mSize = 0;
};


/**
 * @brief String with write-access
 */
class StringBuff : public String
{
public:
    StringBuff() = default;
    StringBuff(const char *s) : String(s) {}
    StringBuff(const StringBuff&) = delete;
    StringBuff(StringBuff&&) = delete;

    StringBuff(String &&other) noexcept
    {
        String::operator=(std::move(other));
    }

    StringBuff& operator =(String &&other)
    {
        String::operator=(std::move(other));
        return *this;
    }

    StringBuff& operator =(const String &other)
    {
        String::operator=(other);
        return *this;
    }

    /**
     * @brief Returns pointer to writable internal buffer limited by \b sz.size()
     */
    char* data()
    {
        if (!mpBuff)
            append("");
        return mpBuff;
    }

    /**
     * @brief Unsafe access operator
     */
    char& operator[](uint16_t idx)
    {
        return mpBuff[idx];
    }
};

// -----------------------------------------------------------------------------

} // twins
