/******************************************************************************
 * @brief   TWins - ring buffer for eg. byte stream; only for trivial data types
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"

#include <atomic>
#include <assert.h>
#include <stdint.h>
#include <string.h>

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief Constant length ring buffer
 * @tparam T trivial type, like int of any size
 */
template <typename T>
class RingBuff
{
public:
    RingBuff() = default;
    RingBuff(const RingBuff&) = delete;
    RingBuff(RingBuff&&) = delete;

    /** @brief Initialize with external static buffer of size N */
    template<uint16_t N>
    RingBuff(T (&buffer)[N])
    {
        initStatic<N>(buffer);
    }

    ~RingBuff()
    {
        if (mpBuff && !mStaticBuff)
            pPAL->memFree(mpBuff);
    }

    /** @brief Initialize with external static buffer of size N */
    template<uint16_t N>
    void initStatic(T (&buffer)[N])
    {
        assert(!(mpBuff && !mStaticBuff));

        mpBuff = buffer;
        mCapacity = N;
        mStaticBuff = true;
    }

    /** @brief Initialize the internal buffer using twins::IOS memAlloc() */
    void init(uint16_t bufferSize)
    {
        // if static buffer given or already initialized - exit
        if (mpBuff)
            return;

        assert(pPAL);
        mpBuff = (T*)pPAL->memAlloc(bufferSize * sizeof(T));
        mCapacity = bufferSize;
    }

    /** @brief Reset the read/write pointers */
    void clear()
    {
        mSize = 0;
        mReadIdx = mWriteIdx = 0;
    }

    /** @brief Returns data length written */
    uint16_t size() const { return mSize; }

    /** @brief Returns buffer capacity */
    uint16_t capacity() const { return mCapacity; }

    /** @brief returns true if buffer is full */
    bool isFull() const { return mSize == mCapacity; }

    /** @brief Write \p data into the buffer
     *  @return false if buffer is full
     */
    bool write(T data)
    {
        assert(mpBuff);
        if (isFull())
            return false;

        mpBuff[mWriteIdx++] = data;
        if (mWriteIdx == mCapacity)
            mWriteIdx = 0;

        mSize++;
        return true;
    }

    /** @brief Write array \p data of \p dataSize elements into the buffer
     *  @return false if no space left for entire data
     */
    bool write(const T *data, uint16_t dataSize)
    {
        assert(mpBuff);
        if (mSize + dataSize > mCapacity)
            return false;

        uint32_t toCopy = dataSize;

        while (toCopy--)
        {
            mpBuff[mWriteIdx++] = *data++;
            if (mWriteIdx == mCapacity)
                mWriteIdx = 0;
        }

        mSize += dataSize;
        return true;
    }

    /** @brief Write null-terminated C string */
    //template<class = std::enable_if<std::is_same<T, char>::value>>
    bool write(const char* data)
    {
        return data ? write(data, strlen(data)) : 0;
    }

    /** @brief Returns a pointer to head element or \b nullptr if buffer is empty;
     *         moves read pointer forward
     */
    T* read()
    {
        assert(mpBuff);
        if (mSize == 0)
            return nullptr;

        T *ret = mpBuff + mReadIdx;
        mSize--;
        mReadIdx++;

        if (mReadIdx == mCapacity)
            mReadIdx = 0;

        return ret;
    }

    /** @brief Read \p count elements into the \p buffer, but no more that \p size()
     *  @return number of elements copied
     */
    uint16_t read(T *buffer, uint16_t count)
    {
        assert(mpBuff);
        if (mSize == 0)
            return 0;

        if (count > mSize)
            count = mSize;

        uint32_t toCopy = count;

        while (toCopy--)
        {
            *buffer++ = mpBuff[mReadIdx++];
            if (mReadIdx == mCapacity)
                mReadIdx = 0;
        }

        mSize -= count;
        return count;
    }

    /** @brief Copy \p count elements into the \p buffer, but no more that \p size()
     *         not changing the internal state
     *  @return number of elements copied
     */
    uint16_t copy(T *buffer, uint16_t count) const
    {
        assert(mpBuff);
        if (mSize == 0)
            return 0;

        if (count > mSize)
            count = mSize;

        uint32_t toCopy = count;
        uint16_t head = mReadIdx;

        while (toCopy--)
        {
            *buffer++ = mpBuff[head++];
            if (head == mCapacity)
                head = 0;
        }

        return count;
    }

    /** @brief Returns a pointer to contiguous data, not changing the read pointer.
     *  @param if \p dataSz is given, set value to contiguous data size
     */
    T* getContiguousData(uint16_t *dataSz = nullptr)
    {
        assert(mpBuff);
        if (mSize == 0)
            return nullptr;

        if (dataSz)
        {
            if (mReadIdx < mWriteIdx)
            {
                // ...H...T..
                *dataSz = mSize;
            }
            else
            {
                // ...T...H..
                *dataSz = mCapacity - mReadIdx;
            }
        }

        return &mpBuff[mReadIdx];
    }

    /** @brief Moves read pointer forward by \p count items, but no more than \p size elements */
    void skip(uint16_t count)
    {
        if (count > mSize)
            count = mSize;

        mSize -= count;
        mReadIdx += count;
        if (mReadIdx >= mCapacity)
            mReadIdx -= mCapacity;
    }

private:
    std::atomic_uint16_t mWriteIdx {};
    std::atomic_uint16_t mReadIdx {};
    std::atomic_uint16_t mSize {};

    uint16_t mCapacity {};
    bool     mStaticBuff {};
    T *      mpBuff {};
};

// -----------------------------------------------------------------------------

} // twins
