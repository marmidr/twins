/******************************************************************************
 * @brief   TWins - string class for our needs
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins_common.hpp"
#include "twins_string.hpp"
#include "twins_utf8str.hpp"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <utility> //std::swap

// -----------------------------------------------------------------------------

namespace twins
{

String::String(const char *s)
{
    assert(pPAL);
    append(s);
}

String::String(String &&other) noexcept
{
    *this = std::move(other);
}

String::~String()
{
    if (mpBuff)
        pPAL->memFree(mpBuff);
}

String& String::append(const char *s, int16_t repeat)
{
    if (repeat <= 0 || !s || sourceIsOurs(s))
        return *this;

    unsigned sl = strlen(s);
    reserve(mSize + repeat * sl);
    char *p = mpBuff + mSize;
    mSize += repeat * sl;
    while (repeat--)
        p = strcat(p, s);
    mpBuff[mSize] = '\0';
    return *this;
}

String& String::appendLen(const char *s, int16_t sLen)
{
    if (sLen <= 0 || !s || sourceIsOurs(s))
        return *this;

    reserve(mSize + sLen);
    strncat(mpBuff + mSize, s, sLen);
    mSize += sLen;
    mpBuff[mSize] = '\0';
    return *this;
}

String& String::append(char c, int16_t repeat)
{
    if (repeat <= 0) return *this;
    reserve(mSize + repeat);
    char *p = mpBuff + mSize;
    mSize += repeat;
    while (repeat--)
        *p++ = c;
    mpBuff[mSize] = '\0';
    return *this;
}

String& String::appendFmt(const char *fmt, ...)
{
    if (!fmt) return *this;

    va_list ap;
    va_start(ap, fmt);
    appendVFmt(fmt, ap);
    va_end(ap);
    return *this;
}

void String::appendVFmt(const char *fmt, va_list ap)
{
    // https://en.cppreference.com/w/cpp/io/c/fprintf
    uint8_t retry = 1;

    do
    {
        va_list ap_copy;
        va_copy(ap_copy, ap);
        int freespace = mCapacity - mSize;
        int n = vsnprintf(mpBuff + mSize, freespace, fmt, ap_copy);

        if (n > 0)
        {
            // everything ok
            if (n < freespace)
            {
                mSize += n;
                break;
            }

            // printf("too small buffer\n");
            reserve(mCapacity + n + 10);
        }
        else
        {
            // printf("err\n");
            break;
        }

    } while (retry--);
}

String& String::trim(int16_t trimPos, bool addEllipsis, bool ignoreESC)
{
    if (trimPos < 0 || trimPos >= mSize)
        return *this;

    if (addEllipsis && trimPos > 0)
        trimPos--;

    char *p = mpBuff;

    if (ignoreESC)
    {
        p = const_cast<char*>(u8skip(p, trimPos));
    }
    else
    {
        for (int i = 0; i < trimPos; i++)
        {
            int seqLen = utf8seqlen(p);
            if (seqLen <= 0) break;
            p += seqLen;
        }
    }

    if (p >= mpBuff + mSize)
        return *this;

    mSize = p - mpBuff;
    char last = mpBuff[mSize];

    if (addEllipsis && last == ' ')
        mSize++;
    mpBuff[mSize] = '\0';
    if (addEllipsis && last != ' ')
        append("…");
    return *this;
}

String& String::erase(int16_t pos, int16_t len)
{
    if (pos < 0 || pos >= mSize || len <= 0)
        return *this;

    char *p = mpBuff;

    for (int i = 0; i < pos; i++)
    {
        int seqLen = utf8seqlen(p);
        if (seqLen <= 0) break;
        p += seqLen;
    }

    if (p >= mpBuff + mSize)
        return *this;

    char *erase_at = p;
    unsigned bytes_to_erase = 0;

    for (int i = 0; i < len; i++)
    {
        int seqLen = utf8seqlen(p);
        if (seqLen <= 0) break;
        bytes_to_erase += seqLen;
        p += seqLen;
        if (p >= mpBuff + mSize)
            break;
    }

    memmove(erase_at, erase_at + bytes_to_erase, mSize - (erase_at - mpBuff));
    mSize -= bytes_to_erase;
    mpBuff[mSize] = '\0';
    return *this;
}

String& String::insert(int16_t pos, const char *s, int16_t repeat)
{
    if (pos < 0 || (!s || !*s) || repeat < 1 || sourceIsOurs(s))
        return *this;

    if ((unsigned)pos >= u8len())
    {
        append(s, repeat);
        return *this;
    }

    char *p = mpBuff;

    for (int i = 0; i < pos; i++)
    {
        int seqLen = utf8seqlen(p);
        if (seqLen <= 0) break;
        p += seqLen;
    }

    if (p >= mpBuff + mSize)
        return *this;

    char *insert_at = p;
    unsigned src_len = strlen(s);
    unsigned bytes_to_insert = src_len * repeat;

    reserve(mSize + bytes_to_insert);
    memmove(insert_at + bytes_to_insert, insert_at, mSize - (insert_at - mpBuff));
    while (repeat--)
    {
        memmove(insert_at, s, src_len);
        insert_at += src_len;
    }
    mSize += bytes_to_insert;
    mpBuff[mSize] = '\0';
    return *this;
}

void String::setWidth(int16_t newWidth, bool addEllipsis)
{
    if (newWidth < 0)
        return;

    int w = width();

    if (w <= newWidth)
        append(' ', newWidth - w);
    else
        trim(newWidth, addEllipsis, true);
}

String& String::clear(uint16_t threshordToFree)
{
    if (mCapacity > alignCapacity(threshordToFree))
        freeBuff();

    mSize = 0;
    if (mpBuff)
        *mpBuff = '\0';

    return *this;
}

String& String::operator=(const char *s)
{
    if (!sourceIsOurs(s))
    {
        clear();
        append(s);
    }
    return *this;
}

String& String::operator =(const String &other)
{
    if (this != &other)
        *this = other.cstr();

    return *this;
}

String& String::operator=(String &&other)
{
    if (this != &other)
    {
        std::swap(mpBuff,    other.mpBuff);
        std::swap(mCapacity, other.mCapacity);
        std::swap(mSize,     other.mSize);
    }
    return *this;
}

bool String::operator==(const char *str) const
{
    if (!str)
        return false;

    return strcmp(cstr(), str) == 0;
}

bool String::startsWith(const char *str) const
{
    if (!str || !*str)
        return false;

    unsigned sl = strlen(str);
    return strncmp(cstr(), str, sl) == 0;
}

bool String::endsWith(const char *str) const
{
    if (!str || !*str)
        return false;

    unsigned sl = strlen(str);
    if (sl > mSize)
        return false;

    return strcmp(cstr() + mSize - sl, str) == 0;
}

int String::find(const char *str) const
{
    if (!str || !*str)
        return -1;

    const char *p = strstr(cstr(), str);
    if (!p)
        return -1;

    return p - cstr();
}

uint16_t String::u8len(bool ignoreESC, bool realWidth) const
{
    return u8len(mpBuff, mpBuff + mSize, ignoreESC, realWidth);
}

uint16_t String::alignCapacity(uint16_t newCapacity) const
{
    // extra byte for NUL
    newCapacity++;
    if (newCapacity & (32-1))
    {
        // avoid allocating every time 1-byte more
        newCapacity &= ~(32-1);
        newCapacity += 32;
    }

    return newCapacity;
}

void String::reserve(uint16_t newCapacity)
{
    newCapacity = alignCapacity(newCapacity);

    if (newCapacity < mCapacity)
        return;

    if (!mpBuff)
    {
        // first-time buffer allocation
        mpBuff = (char*)pPAL->memAlloc(newCapacity);
        mCapacity = newCapacity;
        *mpBuff = '\0';
        mSize = 0;
        return;
    }

    if (newCapacity > mCapacity)
    {
        // reallocation needed
        char *pnew = (char*)pPAL->memAlloc(newCapacity);
        mCapacity = newCapacity;
        memcpy(pnew, mpBuff, mSize+1);
        pPAL->memFree(mpBuff);
        mpBuff = pnew;
    }
}

void String::freeBuff()
{
    if (mpBuff) pPAL->memFree(mpBuff);
    mpBuff = nullptr;
    mCapacity = 0;
    mSize = 0;
}

uint16_t String::escLen(const char *str, const char *strEnd)
{
    // ESC sequence always ends with:
    // - A..Z
    // - a..z
    // - @, ^, ~

    // ESC_BG_RGB(r,g,b)    \e[48;2;255;255;255m
    constexpr uint8_t esc_max_seq_len = 20;

    if (str && *str == '\e')
    {
        unsigned sl = 1;
        unsigned max_sl;

        if (strEnd && (strEnd - str < esc_max_seq_len))
            max_sl = strEnd - str;
        else
            max_sl = esc_max_seq_len;

        while (sl < max_sl)
        {
            const char c = str[sl];

            switch (c)
            {
            case '\0':
                return 0;
            case '@':
            case '^':
            case '~':
                return sl + 1;
            case 'M':
                return ((max_sl >= 6) && (strnlen(str, 6) >= 6)) ? 6 : 0;
            default:
                if (c >= 'A' && c <= 'Z' && c != 'O')
                    return sl + 1;
                if (c >= 'a' && c <= 'z')
                    return sl + 1;
                break;
            }

            sl++;
        }
    }

    return 0;
}

struct UnicodeBlockRange
{
    long first, last;
};

// not accurate subset, but works in most cases
const UnicodeBlockRange wideSymbolRanges[] =
{
    // https://www.compart.com/en/unicode/block/U+2600
    { 0x02600, 0x026FF },
    // https://www.compart.com/en/unicode/block/U+2700
    { 0x02700, 0x027BF },
    // https://www.compart.com/en/unicode/block/U+1F300
    { 0x1F300, 0x1F5FF },
    // https://www.compart.com/en/unicode/block/U+1F600
    { 0x1F600, 0x1F64F },
    // https://www.compart.com/en/unicode/block/U+1F680
    { 0x1F680, 0x1F6FF },
    // https://www.compart.com/en/unicode/block/U+1F900
    { 0x1F900, 0x1F9FF },
};

uint16_t String::u8len(const char *str, const char *strEnd, bool ignoreESC, bool realWidth)
{
    if (!str || !*str)
        return 0;

    if (!(ignoreESC | realWidth))
        return (unsigned)utf8len(str);

    if (!strEnd)
        strEnd = str + strlen(str);

    uint16_t len = 0;

    while (str < strEnd)
    {
        bool seq_found = false;

        if (ignoreESC)
        {
            uint16_t esc_len = escLen(str, strEnd);
            seq_found = esc_len > 0;

            while (esc_len && (str < strEnd))
            {
                str += esc_len;
                esc_len = escLen(str, strEnd);
            }
        }

        if (str < strEnd)
        {
            long u8ch = utf8getchar(str);

            if (int u8_len = utf8charlen(u8ch))
            {
                seq_found = true;
                len++;
                str += u8_len;

                if (realWidth && u8ch >= 0x02600)
                {
                    for (const auto &r : wideSymbolRanges)
                        if (u8ch >= r.first && u8ch <= r.last)
                            len++;
                }
            }
        }

        // nothing recognized? - string illformed
        if (!seq_found)
            break;
    }

    return len;
}

const char* String::u8skip(const char *str, unsigned toSkip, bool ignoreESC)
{
    if (!str || !*str)
        return "";

    const char *str_end = str + strlen(str);
    unsigned skipped = 0;

    while (str < str_end && skipped < toSkip)
    {
        bool seq_found = false;

        if (ignoreESC)
        {
            uint16_t esc_len = escLen(str);
            seq_found = esc_len > 0;

            for (; esc_len && (str < str_end); )
            {
                str += esc_len;
                esc_len = escLen(str);
            }
        }

        if (str < str_end)
        {
            if (int u8_len = utf8seqlen(str))
            {
                seq_found = true;
                skipped++;
                str += u8_len;
            }
        }

        // nothing recognized? - string illformed
        if (!seq_found)
            break;
    }

    return str;
}

// -----------------------------------------------------------------------------

} // twins
