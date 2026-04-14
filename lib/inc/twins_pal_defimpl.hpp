/******************************************************************************
 * @brief   TWins - default implementation of platform abstraction layer
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#ifndef TWINS_PAL_FULLIMPL
# define TWINS_PAL_FULLIMPL     1
#endif

// -----------------------------------------------------------------------------

#include "twins_common.hpp"
#include "twins_string.hpp"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#if TWINS_ENV_LINUX_LIKE
# include <time.h>
# include <unistd.h>
#else
# define malloc_usable_size(x)  0
#endif

// -----------------------------------------------------------------------------

namespace twins
{

struct DefaultPAL : public twins::IPal
{
    DefaultPAL() {}
    ~DefaultPAL() {}

    int writeChar(char c, int16_t repeat) override
    {
        auto sz = lineBuff.size();
        lineBuff.append(c, repeat);
        return lineBuff.size() - sz;
    }

    int writeStr(const char *s, int16_t repeat) override
    {
        auto sz = lineBuff.size();
        lineBuff.append(s, repeat);
        return lineBuff.size() - sz;
    }

    int writeStrLen(const char *s, uint16_t sLen) override
    {
        lineBuff.appendLen(s, sLen);
        return sLen;
    }

    int writeStrVFmt(const char *fmt, va_list ap) override
    {
        auto sz = lineBuff.size();
        lineBuff.appendVFmt(fmt, ap);
        return lineBuff.size() - sz;
    }

    void flushBuff() override
    {
    #if TWINS_PAL_FULLIMPL
        if (lineBuff.size())
        {
            fwrite(lineBuff.cstr(), 1, lineBuff.size(), stdout);

            if (lineBuff.size() > lineBuffMaxSize)
            {
                lineBuffMaxSize = lineBuff.size();
            }

            lineBuff.clear(1000);
            lineBuff.reserve(1000);
        }

        fflush(stdout);
    #else
        lineBuff.clear(1000);
    #endif
    }

    void setLogging(bool /* on */) override {}

    void promptPrinted() override {}

    void* memAlloc(uint32_t size) override
    {
    #if TWINS_PAL_FULLIMPL
        void *ptr = malloc(size);
        auto allocated = malloc_usable_size(ptr);

        stats.memAllocated += allocated;
        if (stats.memAllocated > stats.memAllocatedMax)
            stats.memAllocatedMax = stats.memAllocated;

        stats.memChunks++;
        if (stats.memChunks > stats.memChunksMax)
            stats.memChunksMax = stats.memChunks;

        return ptr;
    #else
        assert(!"memAlloc() must be implemented");
        return nullptr;
    #endif
    }

    void memFree(void *ptr) override
    {
    #if TWINS_PAL_FULLIMPL
        if (ptr)
        {
            auto allocated = malloc_usable_size(ptr);
            stats.memAllocated -= allocated;
            stats.memChunks--;
            free(ptr);
        }
    #else
        (void)ptr;
        assert(!"memFree() must be implemented");
    #endif
    }

    void sleep(uint16_t ms) override
    {
    #if TWINS_ENV_LINUX_LIKE && TWINS_PAL_FULLIMPL
        //*
        timespec ts = {};
        ts.tv_sec = ms / 1000;
        ms %= 1000;
        ts.tv_nsec = 1'000'000 * ms;
        nanosleep(&ts, nullptr);
        //*/

        //usleep(ms * 1000);
    #else
        (void)ms;
    #endif
    }

    uint16_t getLogsRow() override
    {
        return 0;
    }

    uint32_t getTimeStamp() override
    {
    #if TWINS_ENV_LINUX_LIKE && TWINS_PAL_FULLIMPL
        static timespec ts_at_start;
        if (ts_at_start.tv_sec == 0)
            clock_gettime(CLOCK_MONOTONIC, &ts_at_start);

        timespec ts_now;
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        uint32_t sec_diff = ts_now.tv_sec - ts_at_start.tv_sec;
        auto msec_diff = (ts_now.tv_nsec - ts_at_start.tv_nsec) / 1'000'000;
        return sec_diff * 1000 + msec_diff;
    #else
        return 0;
    #endif
    }

    uint32_t getTimeDiff(uint32_t timestamp) override
    {
        auto now = getTimeStamp();
        return now - timestamp;
    }

    bool lock(bool) override
    {
        return true;
    }

    void unlock() override {}

    void wgtDrawBegin(const void */* pWgt */) override {}

    void wgtDrawEnd(const void */* pWgt */) override {}

protected:
    // called before PAL is unregistered
    void deinit()
    {
        lineBuff.clear(0);
    }

public:
    String lineBuff;
    uint32_t lineBuffMaxSize = 0;

    // statistics
    Stats stats = {};
};

// -----------------------------------------------------------------------------

} // twins
