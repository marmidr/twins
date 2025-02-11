/******************************************************************************
 * @brief   TWins - stack template
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"

#include <utility>  // std::move
#include <memory>   // new(addr) T()

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief Template for simple self-managed Stack class able to hold any type of value
 */
template<typename T>
class Stack
{
public:
    Stack() = default;
    Stack(const Stack&) = delete;
    Stack(Stack&&) = delete;

    ~Stack()
    {
        destroyContent();
        if (mpItems)
            pPAL->memFree(mpItems);
    }

    /** @brief Push new item onto the stack by copy or move */
    template <typename Tv>
    void push(Tv && item)
    {
        if (!growAsNecessary())
            return;
        mpItems[mSize++] = std::forward<Tv>(item);
    }

    /** @brief Returns pointer to to the top item and decrease stack size, or \b nullptr if stack is empty */
    T* pop()
    {
        if (mSize)
        {
            mSize--;
            return mpItems + mSize;
        }

        return nullptr;
    }

    /** @brief Returns pointer to the top-item or \b nullptr if stack is empty */
    T* top()
    {
        if (mSize)
            return mpItems + mSize - 1;

        return nullptr;
    }

    /** @brief Remove all items and releases memory if at least 64B is to be retrieved */
    void clear(bool force = false)
    {
        if (force || (mCapacity * sizeof(T) >= 64))
        {
            destroyContent();
            pPAL->memFree(mpItems);
            mpItems = nullptr;
            mCapacity = 0;
        }
        mSize = 0;
    }

    /** @brief Return stack size */
    uint16_t size() const { return mSize; }

private:
    bool growAsNecessary()
    {
        if (mSize == mCapacity)
        {
            if (mCapacity >= 32000)
                return false;

            if (mCapacity < 8)
                mCapacity = 8;
            else
                mCapacity = (mCapacity * 13) / 8; // * 1.65

            auto* p_new = (T*)pPAL->memAlloc(mCapacity * sizeof(T));
            moveContent(p_new, mpItems, mSize);
            initContent(p_new + mSize, mCapacity - mSize);
            pPAL->memFree(mpItems);
            mpItems = p_new;
        }

        return true;
    }

    void destroyContent(void)
    {
        for (uint16_t i = 0; i < mCapacity; i++)
            mpItems[i].~T();
    }

    void copyContent(const T *pSrc, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            mpItems[i] = pSrc[i];
    }

    void moveContent(T *pDst, T *pSrc, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            new (&pDst[i]) T(std::move(pSrc[i]));
    }

    void initContent(T *pItems, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            new (&pItems[i]) T();
    }

private:
    T *         mpItems = nullptr;
    uint16_t    mSize = 0;
    uint16_t    mCapacity = 0;
};

// -----------------------------------------------------------------------------

} // twins
