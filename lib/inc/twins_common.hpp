/******************************************************************************
 * @brief   TWins - common definitions
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include <stdint.h>
#include <stdarg.h>

// -----------------------------------------------------------------------------

#if defined __linux__ || defined __CYGWIN__ || defined __MSYS__
# define TWINS_ENV_LINUX_LIKE   1
#else
# define TWINS_ENV_LINUX_LIKE   0
#endif


#ifndef MIN
# define MIN(x, y)              (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
# define MAX(x, y)              (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
# define ABS(a)                 ((a) >= 0 ? (a) : (-(a)))
#endif

#ifndef BIT
# define BIT(n)                  (1 << (n))
#endif

#ifndef INRANGE
# define INRANGE(val, min, max)  (((val) >= (min)) && ((val) <= (max)))
#endif

namespace twins
{

// forward decl
class String;


/**
 * @brief Template returning length of array of type T
 */
template<unsigned N, typename T>
unsigned arrSize(const T (&)[N]) { return N; }


/**
 * @brief array template usefull in const expressions
 */
template <typename T, unsigned N>
struct Array
{
    constexpr       T& operator[](unsigned i)       { return data[i]; }
    constexpr const T& operator[](unsigned i) const { return data[i]; }
    constexpr const T* begin()                const { return data; }
    constexpr const T* end()                  const { return data + N; }
    constexpr unsigned size()                 const { return N; }

    T data[N] = {};
};


/** @brief Structure holding pointer to first array element and the array size */
template <typename T>
struct Span
{
    T* data = {};
    unsigned size = {};
};

/** @brief View on contiguous array of C strings */
using CStrView = twins::Span<const char>;

/**
 * @brief Platform Abstraction Layer for easy porting
 */
struct IPal
{
    struct Stats
    {
        uint16_t memChunks;
        uint16_t memChunksMax;
        int32_t  memAllocated;
        int32_t  memAllocatedMax;
    };

    virtual ~IPal() = default;
    //
    virtual int writeChar(char c, int16_t repeat = 1) = 0;
    virtual int writeStr(const char *s, int16_t repeat = 1) = 0;
    virtual int writeStrLen(const char *s, uint16_t sLen) = 0;
    virtual int writeStrVFmt(const char *fmt, va_list ap) = 0;
    virtual void flushBuff() = 0;
    virtual void setLogging(bool on) = 0;
    virtual void promptPrinted() = 0;
    //
    virtual void *memAlloc(uint32_t sz) = 0;
    virtual void  memFree(void *ptr) = 0;
    //
    virtual void  sleep(uint16_t ms) = 0;
    virtual uint16_t getLogsRow() = 0;
    virtual uint32_t getTimeStamp() = 0;
    virtual uint32_t getTimeDiff(uint32_t timestamp) = 0;
    //
    virtual bool lock(bool wait = true) = 0;
    virtual void unlock() = 0;
    //
    virtual void wgtDrawBegin(const void *pWgt) = 0;
    virtual void wgtDrawEnd(const void *pWgt) = 0;
};


// pointer set by init()
extern IPal *pPAL;



/**
 * @brief ANSI codes
 */
enum class Ansi : uint8_t
{
    NUL = 0x00,  // Null
    SOH = 0x01,  // Start of Header
    STX = 0x02,  // Start of Text
    ETX = 0x03,  // End of Text
    EOT = 0x04,  // End of Transmission
    ENQ = 0x05,  // Enquiry
    ACK = 0x06,  // Acknowledgment
    BEL = 0x07,  // Bell
    BS  = 0x08,  // Backspace
    HT  = 0x09,  // Horizontal Tab
    LF  = 0x0A,  // Line Feed
    VT  = 0x0B,  // Vertical Tab
    FF  = 0x0C,  // Form Feed
    CR  = 0x0D,  // Carriage Return
    SO  = 0x0E,  // Shift Out
    SI  = 0x0F,  // Shift In
    DLE = 0x10,  // Data Link Escape
    DC1 = 0x11,  // XONDevice Control 1
    DC2 = 0x12,  // Device Control 2
    DC3 = 0x13,  // XOFFDevice Control 3
    DC4 = 0x14,  // Device Control 4
    NAK = 0x15,  // Negative Ack.
    SYN = 0x16,  // Synchronous Idle
    ETB = 0x17,  // End of Trans. Block
    CAN = 0x18,  // Cancel
    EM  = 0x19,  // End of Medium
    SUB = 0x1A,  // Substitute
    ESC = 0x1B,  // Escape
    FS  = 0x1C,  // File Separator
    GS  = 0x1D,  // Group Separator
    RS  = 0x1E,  // Record Separator
    US  = 0x1F,  // Unit Separator
    DEL = 0x7F   // Delete
};


/** Special keys */
enum class Key : uint8_t
{
    None,
    Esc,
    Tab,
    Enter,
    Backspace,
    Pause,
    //
    Up,
    Down,
    Left,
    Right,
    //
    Insert,
    Delete,
    Home,
    End,
    PgUp,
    PgDown,
    //
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    //
    MouseEvent
};

/** Mouse button click events */
enum class MouseBtn : uint8_t
{
    None,
    ButtonLeft,
    ButtonMid,
    ButtonRight,
    ButtonGoBack,
    ButtonGoForward,
    ButtonReleased,
    WheelUp,
    WheelDown,
};

/** Key modifiers */
#define KEY_MOD_NONE    0
#define KEY_MOD_CTRL    1
#define KEY_MOD_ALT     2
#define KEY_MOD_SHIFT   4
#define KEY_MOD_SPECIAL 8

/**
 * @brief Decoded terminal key
 */
struct KeyCode
{
    union
    {
        /** used for regular text input */
        char    utf8[5];    // NUL terminated UTF-8 code: 'a', '4', 'Ł'
        /** used for special keys */
        Key     key = {};   // 'F1', 'Enter'
        /** used for mouse events (when key == Key::MouseClick) */
        struct
        {
            // same as key above
            Key      key;
            /** button or wheel event */
            MouseBtn btn;
            /** 1:1 based terminal coordinates of the event */
            uint8_t  col;
            uint8_t  row;
        } mouse;
    };

    union
    {
        uint8_t mod_all = 0;    // KEY_MOD_CTRL | KEY_MOD_SHIFT
        struct
        {
            uint8_t m_ctrl  : 1; // KEY_MOD_CTRL
            uint8_t m_alt   : 1; // KEY_MOD_ALT
            uint8_t m_shift : 1; // KEY_MOD_SHIFT
            uint8_t m_spec  : 1; // KEY_MOD_SPECIAL
        };
    };

    const char *name = nullptr;
};

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};

// -----------------------------------------------------------------------------

} // twins
