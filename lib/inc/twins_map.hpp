/******************************************************************************
 * @brief   TWins - hash map template
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *          https://github.com/marmidr/twins
 *****************************************************************************/

#pragma once

#include "twins_common.hpp"
#include "twins_vector.hpp"
#include "twins_hash.hpp"

#include <utility>  // std::move
#include <memory>   // new(addr) T()
#include <type_traits>

// -----------------------------------------------------------------------------

namespace twins
{

/**
 * @brief Simple hashing map for Key-Value pairs
 */
template<typename K, typename V, typename H = HashDefault>
class Map
{
public:
    using Hash = uint16_t;

    struct Node
    {
        Hash    hash;
        K       key;
        V       val;
    };

    using Bucket = Vector<Node>;
    using Buckets = Vector<Bucket>;

    class Iter
    {
    public:
        struct NodeIdx
        {
            uint16_t bktIdx;
            uint16_t itemIdx;
            bool operator ==(const NodeIdx& other) const { return bktIdx == other.bktIdx && itemIdx == other.itemIdx; }
        };

    public:
        Iter(void) = delete;

        Iter(const Map<K, V, H> &map, bool begin)
            : mBkts(map.mBuckets)
        {
            if (begin)
            {
                mIdx.bktIdx = 0;
                goToNextNonemptyBucket();
            }
            else
            {
                mIdx.bktIdx = mBkts.size();
            }

            mIdx.itemIdx = 0;
        }

        Iter(const Iter &other)
            : mBkts(other.mBkts), mIdx(other.mIdx)
        {}

        bool operator == (const Iter &other) const { return mIdx == other.mIdx; }
        bool operator != (const Iter &other) const { return !(mIdx == other.mIdx); }
        const Node * operator -> (void) const { return &operator*(); }
        const Node & operator * (void)  const { return mBkts[mIdx.bktIdx][mIdx.itemIdx]; }

        // ++it
        const Iter& operator ++(void) const
        {
            if (mIdx.bktIdx < mBkts.size())
            {
                if (++mIdx.itemIdx >= mBkts[mIdx.bktIdx].size())
                {
                    mIdx.itemIdx = 0;
                    mIdx.bktIdx++;
                    goToNextNonemptyBucket();
                }
            }

            return *this;
        }

    private:
        void goToNextNonemptyBucket() const
        {
            while (mIdx.bktIdx < mBkts.size() && mBkts[mIdx.bktIdx].size() == 0)
                mIdx.bktIdx++;
        }

    protected:
        const Buckets &mBkts;
        mutable NodeIdx mIdx;
    };

public:
    Map() = default;
    ~Map() = default;

    /** @brief Direct access operator; return existing value, creates new otherwise */
    V& operator[](const K &key)
    {
        if (mBuckets.size() == 0)
        {
            // initial buckets count
            mBuckets.resize(4);
        }

        if (mNodes >= (mBuckets.size() * 4))
            growBuckets();

        auto &node = getNode(key);
        return node.val;
    }

    /** @brief Check if given key exists */
    bool contains(const K &key) const
    {
        if (!mBuckets.size())
            return false;

        auto hash = H::hash(key);
        auto bidx = getBucketIdx(hash);
        const auto &bkt = mBuckets[bidx];

        for (auto &node : bkt)
            if ((node.hash == hash) && keysEqual(node.key, key, key_is_cstr{}))
                return true;

        return false;
    }

    /** @brief Remove entry */
    void remove(const K &key)
    {
        if (!mBuckets.size())
            return;

        auto hash = H::hash(key);
        auto &bkt = mBuckets[getBucketIdx(hash)];

        for (uint16_t i = 0; i < bkt.size(); i++)
        {
            if ((bkt[i].hash == hash) && keysEqual(bkt[i].key, key, key_is_cstr{}))
            {
                bkt.remove(i);
                mNodes--;
                break;
            }
        }
    }

    /** @brief Return number of key-value pairs */
    uint16_t size() const
    {
        return mNodes;
    }

    /** @brief Clear all map entries */
    void clear()
    {
        if (mBuckets.size())
        {
            mNodes = 0;
            mBuckets.resize(4);

            for (auto &bkt : mBuckets)
                bkt.clear();
        }
    }

    /** @brief Number of buckets - for test purposes */
    uint16_t bucketsCount() const
    {
        return mBuckets.size();
    }

    /** @brief Return given bucket - for test purposes */
    const Bucket* bucket(int16_t idx) const
    {
        if (idx < 0 || (uint16_t)idx > bucketsCount())
            return nullptr;

        return &mBuckets[idx];
    }

    /** @brief Return elements distribution 0 (worse)..100% (best) */
    uint8_t distribution()
    {
        if (mNodes < 2)
            return 100;

        unsigned expected_nodes_per_bkt = mNodes / mBuckets.size();
        unsigned over_expected = 0;

        if (expected_nodes_per_bkt < 2)
            return 100;

        for (const auto &bkt : mBuckets)
        {
            if (bkt.size() > expected_nodes_per_bkt)
                over_expected += bkt.size() - expected_nodes_per_bkt;
        }

        return 100 - (100 * over_expected / mNodes);
    }

    Iter begin(void) const  { return Iter(*this, true); }
    Iter end(void)   const  { return Iter(*this, false); }

private:
    using key_is_cstr = typename std::conditional<
                            std::is_same<const char*, K>::value || std::is_same<char*, K>::value,
                            std::true_type, std::false_type
                        >::type;

    inline uint16_t getBucketIdx(Hash hash) const
    {
        // mBuckets.size() must be power of 2
        return hash & (mBuckets.size() - 1);
    }

    template<typename Key>
    inline bool keysEqual(Key k1, Key k2, std::true_type) const
    {
        return strcmp(k1, k2) == 0;
    }

    template<typename Key>
    inline bool keysEqual(const Key &k1, const Key &k2, std::false_type) const
    {
        return k1 == k2;
    }

    Node& getNode(const K &key)
    {
        auto hash = H::hash(key);
        auto bidx = getBucketIdx(hash);
        auto &bkt = mBuckets[bidx];

        for (auto &node : bkt)
            if ((node.hash == hash) && keysEqual(node.key, key, key_is_cstr{}))
                return node;

        auto &node = mBuckets[bidx].append();
        mNodes++;
        node.hash = hash;
        node.key = key;
        return node;
    }

    void growBuckets()
    {
        auto old_buckets = std::move(mBuckets);
        mBuckets.resize(old_buckets.size() * 2);

        for (const auto &old_bkt : old_buckets)
        {
            for (auto &old_node : old_bkt)
            {
                auto hash = H::hash(old_node.key);
                auto bidx = getBucketIdx(hash);
                auto &node = mBuckets[bidx].append();
                node.hash = hash;
                node.key = old_node.key;
                node.val = std::move(old_node.val);
            }
        }
    }

private:
    Buckets  mBuckets;
    uint16_t mNodes = 0;
};

// -----------------------------------------------------------------------------

} // twins
