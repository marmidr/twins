/******************************************************************************
 * @brief   TWins - utility code
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#include "twins_utils.hpp"
#include "twins.hpp"

#include <string.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------

namespace twins::util
{

const char* strnchr(const char *str, int strSz, char c)
{
    while (strSz-- > 0)
    {
        if (*str == c) return str;
        str++;
    }

    return nullptr;
}

const char* strechr(const char *str, const char *estr, char c)
{
    while (str < estr)
    {
        if (*str == c) return str;
        str++;
    }

    return nullptr;
}

Vector<CStrView> splitWords(const char *str, const char *delim, bool storeDelim)
{
    if (!str || !*str)
        return {};
    if (!delim || !*delim)
        return {};

    unsigned n_words = 1;

    Vector<CStrView> out;
    unsigned span = 0;

    const char *pstr = str;
    // skip whitespaces
    while ((span = strspn(pstr, delim)) > 0)
        pstr += span;

    // count words
    while ((span = strcspn(pstr, delim)) > 0)
    {
        n_words++;
        pstr += span;
        pstr += strspn(pstr, delim);
    }

    // prepare output
    if (storeDelim) n_words *= 2;
    out.reserve(n_words);
    pstr = str;

    // skip whitespaces
    while ((span = strspn(pstr, delim)) > 0)
        pstr += span;

    if (storeDelim && (pstr > str))
        out.append(CStrView{str, unsigned(pstr - str)});

    // put each word into separate cell
    while ((span = strcspn(pstr, delim)) > 0)
    {
        if (const char *pesc = strnchr(pstr, span, '\e'))
        {
            // span contains ESC sequence - store it separately

            // append text before ESC
            if (pesc > pstr)
                out.append(CStrView{pstr, unsigned(pesc - pstr)});
            auto esc_len = String::escLen(pesc);

            // append ESC seq
            out.append(CStrView{pesc, esc_len});

            // append text after ESC
            pesc += esc_len;
            if (pesc < pstr + span)
                out.append(CStrView{pesc, unsigned(pstr + span - pesc)});
        }
        else
        {
            // store word
            out.append(CStrView{pstr, span});
        }

        pstr += span;
        // check delimiters length
        span = strspn(pstr, delim);
        // store delimiter and skip it
        if (storeDelim) out.append(CStrView{pstr, span});
        pstr += span;
    }

    return out;
}

String wordWrap(const char *str, uint16_t areaWidth, const char *delim, const char *separator)
{
    if (!str || !*str)
        return {};
    if (areaWidth < 1)
        return {};
    if (!delim || !*delim)
        return {};
    if (!separator || !*separator)
        return {};

    auto words = splitWords(str, delim, true);
    unsigned line_len = 0;
    String out;
    out.reserve(strlen(str) + words.size());

    for (auto &w : words)
    {
        // w is an ESC sequence
        if (*w.data == '\e')
        {
            // append entire sequence and treat it as zero-length
            out.appendLen(w.data, w.size);
            continue;
        }

        // w is a delimiter char or sequence
        if (strchr(delim, *w.data))
        {
            // check if w contains \n and adjust line length accordingly
            if (const char *nl = strnchr(w.data, w.size, '\n'))
            {
                out.appendLen(w.data, w.size);
                line_len = w.size - (nl - w.data);
                continue;
            }

            if (line_len > 0)
            {
                out.appendLen(w.data, w.size);
                line_len += w.size;
            }
            continue;
        }

        auto word_width = String::width(w.data, w.data + w.size);

        if (line_len + word_width <= areaWidth)
        {
            out.appendLen(w.data, w.size);
            line_len += word_width;
        }
        else
        {
            if (word_width > areaWidth)
            {
                if (line_len > 0)
                    out << separator;

                out.appendLen(w.data, areaWidth-1);
                out << "…" << separator;
                line_len = 0;
                continue;
            }

            if (out.size() > 0)
                out << separator;

            out.appendLen(w.data, w.size);
            line_len = word_width;
        }
    }

    return out;
}

Vector<CStrView> splitLines(const char *str)
{
    if (!str || !*str)
        return {};

    Vector<CStrView> out;

    while (const char *nl = strchr(str, '\n'))
    {
        // possible and allowed: nl == str
        out.append(CStrView{str, (unsigned)(nl-str)});
        str = nl+1;
    }

    if (*str)
        out.append(CStrView{str, (unsigned)strlen(str)});

    return out;
}

twins::String centerText(const char *str, uint16_t areaWidth)
{
    if (!str) str = "";
    auto str_width = twins::String::width(str);
    twins::String out;

    if (areaWidth - str_width > 0)
    {
        uint16_t leading_spaces = (areaWidth - str_width)/2;
        // note that for reservation a buffer length is needed, not visual text width
        out.reserve(strlen(str) + (areaWidth - str_width));
        out.append(' ', leading_spaces)
           .append(str)
           .append(' ', areaWidth - str_width - leading_spaces);
    }
    else
    {
        out = str;
    }

    return out;
}

// -----------------------------------------------------------------------------

bool numEditInputEvt(const twins::KeyCode &kc, twins::String &str, int16_t &cursorPos, int64_t limitMin, int64_t limitMax, bool wrap)
{
    if (kc.mod_all == KEY_MOD_NONE)
    {
        // reject non-digits and avoid too long numbers
        // 0x7fffffffffffffff = 9223372036854775807
        if (kc.utf8[0] < '0' || kc.utf8[0] > '9' || str.size() >= 19)
        {
            twins::writeStr(ESC_BELL);
            twins::flushBuffer();
            return true;
        }
    }

    if (kc.m_spec)
    {
        switch (kc.key)
        {
        case twins::Key::Enter:
        {
            int64_t n = atoll(str.cstr());
            if (n < limitMin) n = limitMin;
            if (n > limitMax) n = limitMax;
            str.clear();
            str.appendFmt("%lld", n);
            return false;
        }
        case twins::Key::Esc:
            return false;
        case twins::Key::Up:
        case twins::Key::Down:
        {
            int64_t n = atoll(str.cstr());
            int delta = kc.m_shift ? 100 : (kc.m_ctrl ? 10 : 1);
            if (kc.key == twins::Key::Down) delta *= -1;
            n += delta;
            if (n < limitMin) n = wrap ? limitMax : limitMin;
            if (n > limitMax) n = wrap ? limitMin : limitMax;

            str.clear();
            str.appendFmt("%lld", n);
            cursorPos = str.size();
            return true;
        }
        default:
            break;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

} // twins::util
