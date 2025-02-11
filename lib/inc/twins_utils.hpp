/******************************************************************************
 * @brief   TWins - utility code
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_string.hpp"
#include "twins_vector.hpp"
#include "twins_common.hpp"

#include <assert.h>
#include <limits.h>

// -----------------------------------------------------------------------------

namespace twins::util
{

/** @brief Like \b strchr() but with limited length */
const char* strnchr(const char *str, int strSz, char c);
/** @brief Like \b strchr() but with pointed end of string */
const char* strechr(const char *str, const char *estr, char c);

/** @brief Split string into separate words using any of \p delim characters as delimiters
 *  @param str input string
 *  @param delim set of whitespace characters delimiting words
 *  @param storeDelim if true, every second entry returned is delimiter before next word
 *  @return vector of pointers to the beginning of text lines
 */
twins::Vector<twins::CStrView> splitWords(const char *str, const char *delim = " \t\n", bool storeDelim = false);

/** @brief Insert \p newLine and ellipsis to ensure the line length is always < \p maxLineLen
 *  @param str input string
 *  @param areaWidth max number of printable characters that fits into line of text
 *  @param delim set of characters where word wrap can occur
 *  @param separator sequence to insert to break too long line
 */
twins::String wordWrap(const char *str, uint16_t areaWidth, const char *delim = " \t\n", const char *separator = "\n");

/** @brief Split lines using newline character
 *  @param str input string
 *  @return vector of pointers to the beginning of text lines
 */
twins::Vector<twins::CStrView> splitLines(const char *str);

/** @brief Prepend single line \p str with spaces and append spaces to make it centered on \p areaWidth .
 *         ESC sequences are ignored
 */
twins::String centerText(const char *str, uint16_t areaWidth);

// -----------------------------------------------------------------------------

/** @brief String helper holding text wrapped and splitted into lines */
struct WrappedString
{
    WrappedString() = default;

    /**
     * @brief Set maximum width the individual line will never break;
     * @param maxWidth maximum line width
     * @param delim characters that the line can break at
     * @param sep lines separator
     */
    void config(uint16_t maxWidth = 250, const char* delim = " \t\n", const char* sep = "\n")
    {
        mMaxWidth = maxWidth;
        if (delim) mpDelim = delim;
        if (sep) mpSep = sep;
        mDirty = true;
    }

    /**
     * @brief Parse input string and wrap it
     * @param newContent optional new content; can be null if the same content is to be used with new wrapping configuration
     */
    void updateLines(const char *newContent = nullptr)
    {
        if (newContent)
        {
            mSourceStr.clear();
            mSourceStr.append(newContent);
        }
        mWrappedStr = wordWrap(mSourceStr.cstr(), mMaxWidth, mpDelim, mpSep);
        mLines = splitLines(mWrappedStr.cstr());
        mDirty = false;
    }

    /** @brief Returns vector of lines */
    const Vector<CStrView>& getLines()
    {
        if (mDirty)
            updateLines();
        return mLines;
    }

    /** @brief Returns reference to source string */
    String& getSourceStr()
    {
        return mSourceStr;
    }

    /** @brief Returns reference to wrapped string */
    const String& getWrappedStr() const
    {
        return mWrappedStr;
    }

    /** @brief Returns true if source or configuration was changed so the wrapped string is deprecated */
    bool isDirty() const { return mDirty; }

    /** @brief */
    WrappedString& operator =(const char *str)
    {
        mLines.clear();
        mSourceStr = str;
        mDirty = true;
        return *this;
    }

    /** @brief */
    WrappedString& operator =(const String &other)
    {
        *this = other.cstr();
        return *this;
    }

    /** @brief */
    WrappedString& operator =(String &&other)
    {
        mLines.clear();
        mSourceStr = std::move(other);
        mDirty = true;
        return *this;
    }

private:
    bool        mDirty = false;
    uint16_t    mMaxWidth = 250;
    const char* mpDelim = " \t\n";
    const char* mpSep = "\n";
    String      mSourceStr;
    String      mWrappedStr;
    Vector<CStrView> mLines;
};

// -----------------------------------------------------------------------------

/** @brief Default twins::Edit key handler for NumEdit */
bool numEditInputEvt(const twins::KeyCode &kc, twins::String &str, int16_t &cursorPos,
                    int64_t limitMin = LONG_MIN, int64_t limitMax = LONG_MAX, bool wrap = false);

// -----------------------------------------------------------------------------

} // twins::util
