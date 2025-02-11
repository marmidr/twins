/******************************************************************************
 * @brief   TWins - vector container
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"

#include <utility>  // std::move
#include <memory>   // new(addr) T()
#include <initializer_list>
#include <assert.h>

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief Template for simple self-managed Vector
 */
template <class T>
class Vector
{
public:
    using ItemType = T;

    class Iter
    {
    public:
        Iter(void) = delete;
        Iter(Vector<T> &vec, uint16_t idx  = 0) { mPtr = vec.data() + idx; }

        bool operator == (const Iter &other) const { return mPtr == other.mPtr; }
        bool operator != (const Iter &other) const { return mPtr != other.mPtr; }
        T & operator *  (void) { return *mPtr; }
        T * operator -> (void) { return mPtr; }

        // ++it
        Iter& operator ++(void)
        {
            mPtr++;
            return *this;
        }

        // it++ - ugly, do not use
        // Iter operator ++(int)
        // {
        //     Iter it(*this);
        //     mPtr++;
        //     return it;
        // }
    private:
        T* mPtr;
    };

    class ConstIter
    {
    public:
        ConstIter(void) = delete;
        ConstIter(const Vector<T> &vec, uint16_t idx  = 0) { mPtr = vec.data() + idx; }

        bool operator == (const ConstIter &other) const { return mPtr == other.mPtr; }
        bool operator != (const ConstIter &other) const { return mPtr != other.mPtr; }
        const T & operator *  (void) const { return *mPtr; }
        const T * operator -> (void) const { return mPtr; }

        // ++it
        ConstIter& operator ++(void)
        {
            mPtr++;
            return *this;
        }

        // it++ - ugly, do not use
        // Iter operator ++(int)
        // {
        //     Iter it(*this);
        //     mPtr++;
        //     return it;
        // }
    protected:
        const T* mPtr;
    };

public:
    Vector() = default;

    /** @brief Constructor with initial size */
    Vector(uint16_t itemsCount)
    {
        resize(itemsCount);
    }

    /** @brief Copy constructor */
    Vector(const Vector& other)
    {
        append(other.mpItems, other.mSize);
    }

    /** @brief Constructor from array of items */
    template<typename Tv>
    Vector(const Tv* pItems, uint16_t count)
    {
        append(pItems, count);
    }

    /** @brief Constructor from initializer_list */
    template<typename Tv>
    explicit Vector(const std::initializer_list<Tv> &items)
    {
        append(items);
    }

    /** @brief Move constructor */
    Vector(Vector&& other)
    {
        std::swap(mpItems,   other.mpItems);
        std::swap(mSize,     other.mSize);
        std::swap(mCapacity, other.mCapacity);
    }

    ~Vector()
    {
        clear();
    }

    /** @brief Copy assign */
    Vector & operator = (const Vector& other)
    {
        if (&other == this)
            return *this;

        clear();
        reserve(other.size());
        mSize = other.size();
        copyContent(mpItems, other.mpItems, mSize);
        return *this;
    }

    /** @brief Move assign */
    Vector & operator = (Vector&& other)
    {
        if (&other == this)
            return *this;

        std::swap(mpItems,    other.mpItems);
        std::swap(mSize,      other.mSize);
        std::swap(mCapacity,  other.mCapacity);
        return *this;
    }

    /** @brief Equality operator */
    bool operator == (const Vector& other) const
    {
        if (mSize != other.mSize)
            return false;

        for (uint16_t i = 0; i < mSize; i++)
            if (mpItems[i] != other.mpItems[i])
                return false;

        return true;
    }

    /** @brief Direct access operator; \b Note: may lead to crash in case of invalid \p idx */
    T & operator [] (int idx)
    {
        assert(idx >= 0 && idx < mSize);
        return mpItems[idx];
    }

    const T & operator [] (int idx) const
    {
        assert(idx >= 0 && idx < mSize);
        return mpItems[idx];
    }

    /** @brief Direct safe access operator; Returns \b nullptr in case of invalid \p idx */
    T * getAt(int idx)
    {
        if (idx >= 0 && idx < mSize)
            return &mpItems[idx];
        return nullptr;
    }

    /** @brief Get last element */
    T * back(void)
    {
        if (mSize)
            return &mpItems[mSize - 1];
        return nullptr;
    }

    /** @brief Simply returns iternal poinder do array of T */
    T * data(void) { return mpItems; }
    const T * data(void) const { return mpItems; }

    /** @brief Returns vector current size (number of items) */
    uint16_t size(void) const { return mSize; }

    /** @brief Vector capacity */
    uint16_t capacity(void) const { return mCapacity; }

    /** @brief Reserve capacity (higher than current) to avoid memory reallocations */
    void reserve(uint16_t newCapacity)
    {
        if (newCapacity <= mCapacity)
            return;

        auto* p_new_items = (T*)pPAL->memAlloc(newCapacity * sizeof(T));
        moveContent(p_new_items, mpItems, mSize);
        initContent(p_new_items + mSize, newCapacity - mSize);
        pPAL->memFree(mpItems);
        mpItems = p_new_items;
        mCapacity = newCapacity;
    }

    /** @brief Set new size (smaller or bigger) */
    void resize(uint16_t newSize)
    {
        if (newSize == mSize)
            return;

        if (newSize < mCapacity)
        {
            if (sizeof(T) * (mCapacity - newSize) < 64)
            {
                // small change in memory - do not reallocate memory
                destroyContent(mpItems + newSize, mSize-newSize);
                mSize = newSize;
                return;
            }
        }

        uint16_t to_move = MIN(mSize, newSize);
        auto* p_new_items = (T*)pPAL->memAlloc(newSize * sizeof(T));
        moveContent(p_new_items, mpItems, to_move);
        initContent(p_new_items + to_move, newSize - to_move);
        // if new size is smaller than old, not all entries was moved
        destroyContent();
        pPAL->memFree(mpItems);

        mpItems = p_new_items;
        mCapacity = mSize = newSize;
    }

    /** @brief Change buffer capacity to enough for \b size items */
    void shrink(bool force = false)
    {
        if (mCapacity == mSize)
            return;

        if (!force && ((mCapacity - mSize) * sizeof(T) < 64))
            return;

        uint16_t new_capacity = mSize;

        auto* p_new_items = (T*)pPAL->memAlloc(new_capacity * sizeof(T));
        moveContent(p_new_items, mpItems, mSize);

        destroyContent();
        pPAL->memFree(mpItems);

        mpItems = p_new_items;
        mCapacity = new_capacity;
    }

    /** @brief Insert at given position or append if idx >= size */
    template<typename Tv>
    void insert(int idx, Tv &&val)
    {
        if (idx < 0)
            return;

        if (idx >= mSize)
        {
            append(std::forward<Tv>(val));
            return;
        }

        growAsNecessary();

        for (int i = mSize; i > idx; i--)
            mpItems[i] = std::move(mpItems[i-1]);

        mpItems[idx] = std::forward<Tv>(val);
        mSize++;
    }

    /** @brief Add new element at the end by copy or move (push back) */
    template<typename Tv>
    void append(Tv &&val)
    {
        growAsNecessary();
        mpItems[mSize++] = std::forward<Tv>(val);
    }

    /** @brief Append empty element and return reference to it */
    T& append(void)
    {
        growAsNecessary();
        return mpItems[mSize++];
    }

    /** @brief Append given array of elements by copy */
    template<typename Tv>
    void append(const Tv* pItems, uint16_t count)
    {
        growAsNecessary(count);
        copyContent(mpItems + mSize, pItems, count);
        mSize += count;
    }

    /** @brief Append given initialzier list by copy */
    template<typename Tv>
    void append(const std::initializer_list<Tv> &items)
    {
        append(items.begin(), items.size());
    }

    /** @brief Delete element at \p idx */
    bool remove(int idx, bool preserveOrder = false)
    {
        if (idx < 0 || idx >= mSize)
            return false;

        if (mSize > 1)
        {
            if (preserveOrder)
            {
                for (int i = idx; i < mSize - 1; i++)
                    mpItems[i] = std::move(mpItems[i+1]);
            }
            else
            {
                swap(idx, mSize - 1);
            }
        }

        mSize--;
        resetItem(mpItems + mSize);
        return true;
    }

    /** @brief Swap two items at given positions \p idxA and \p idxB */
    bool swap(int idxA, int idxB)
    {
        if (idxA == idxB)
            return true;

        if (idxA < 0 || idxB < 0 || idxA >= mSize || idxB >= mSize)
            return false;

        std::swap(mpItems[idxA], mpItems[idxB]);
        return true;
    }

    /** @brief Search for \p val and return pointer or \b nullptr othervise  */
    T* find(const T &val, int *pIdx = nullptr)
    {
        for (uint16_t i = 0; i < mSize; i++)
        {
            if (mpItems[i] == val)
            {
                if (pIdx)
                    *pIdx = (int)i;
                return mpItems + i;
            }
        }

        return nullptr;
    }

    /** @brief Simpy check if vector contains \p val */
    bool contains(const T &val) const
    {
        const auto *p = data();
        int n = mSize;

        while (n--)
            if (*p++ == val)
                return true;

        return false;
    }

    /** @brief Remove all items and free memory */
    void clear(void)
    {
        destroyContent();
        pPAL->memFree(mpItems);
        mpItems = nullptr;
        mCapacity = 0;
        mSize = 0;
    }

    Iter begin(void) { return Iter(*this, 0); }
    Iter end(void)   { return Iter(*this, size()); }

    ConstIter begin(void) const { return ConstIter(*this, 0); }
    ConstIter end(void)   const { return ConstIter(*this, size()); }

protected:
    T *      mpItems = nullptr;
    uint16_t mSize = 0;
    uint16_t mCapacity = 0;

protected:
    void growAsNecessary(uint16_t growBy = 1)
    {
        if (mCapacity >= mSize + growBy)
            return;

        auto new_capacity = mCapacity + growBy;
        if (new_capacity < 4)
            new_capacity = 4;
        else
            new_capacity = (new_capacity * 13) / 8; // grow by 1.65

        reserve(new_capacity);
    }

    void initContent(T *pItems, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            new (&pItems[i]) T{};
    }

    void destroyContent(T *pItems = nullptr, uint16_t count = 0)
    {
        if (pItems == nullptr)
        {
            pItems = mpItems;
            count = mSize;
        }

        for (uint16_t i = 0; i < count; i++)
            pItems[i].~T();
    }

    template<typename Tv>
    void copyContent(T *pDst, const Tv *pSrc, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            pDst[i] = pSrc[i];
    }

    void moveContent(T *pDst, T *pSrc, uint16_t count)
    {
        for (uint16_t i = 0; i < count; i++)
            new (&pDst[i]) T(std::move(pSrc[i]));
    }

    void resetItem(T *pItem)
    {
        pItem->~T();
        new (pItem) T{};
    }

};

//------------------------------------------------------------------------------

} // twins
