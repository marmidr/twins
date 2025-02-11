/******************************************************************************
 * @brief   TWins - unit tests
 * @author  Mariusz Midor
 *          https://bitbucket.org/marmidr/twins
 *****************************************************************************/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "twins_map.hpp"
#include <string>

// -----------------------------------------------------------------------------

const char *itostr(int i)
{
    static char buff[12];
    snprintf(buff, sizeof(buff), "%d", i);
    return buff;
}

TEST(MAP, add_many)
{
    twins::Map<short, long long> m;
    EXPECT_EQ(0, m.size());
    const int step = 3;
    const int nodes = 300;

    for (int i = 1; i < nodes; i += step)
        m[i] = i;

    EXPECT_EQ(nodes/step, m.size());
    EXPECT_EQ(32, m.bucketsCount());

    for (int i = 1; i < nodes; i += step)
        EXPECT_EQ(i, m[i]);

    fprintf(stderr, "distro: %d\n", m.distribution());
    EXPECT_GE(m.distribution(), 85);

    // for (unsigned i = 0; i < m.bucketsCount(); i++)
    // {
    //     const auto *p = m.bucket(i);
    //     printf("bucket[%u] = %u\n", i, p->size());
    // }
}

TEST(MAP, add_many_strings)
{
    twins::Map<short, std::string> m;
    EXPECT_EQ(0, m.size());
    const int nodes = 100;

    for (int i = 0; i < nodes; i ++)
        m[i] = itostr(i);

    EXPECT_EQ(nodes, m.size());
    EXPECT_EQ(32, m.bucketsCount());
    EXPECT_STREQ("42", m[42].c_str());
}

TEST(MAP, remove_clear)
{
    twins::Map<short, int> m;
    EXPECT_EQ(0, m.size());

    for (int i = 0; i < 10; i++)
        m[i] = i;

    EXPECT_EQ(10, m.size());
    EXPECT_EQ(5, m[5]);
    EXPECT_TRUE(m.contains(5));
    EXPECT_FALSE(m.contains(111));

    m.remove(5);
    EXPECT_EQ(9, m.size());
    EXPECT_FALSE(m.contains(5));
    EXPECT_EQ(0, m[5]);
    EXPECT_TRUE(m.contains(5));

    m.clear();
    EXPECT_EQ(0, m.size());
}

TEST(MAP, key_arithmetic)
{
    {
        twins::Map<bool, int> m;
        EXPECT_EQ(0, m.size());

        m[false] = 1;
        m[true] = 2;
        EXPECT_EQ(2, m[true]);
    }

    {
        twins::Map<double, bool> m;

        EXPECT_FALSE(m[3.14]);
        m[3.14] = true;
        EXPECT_TRUE(m[3.14]);

        EXPECT_FALSE(m[0.0]);
        EXPECT_FALSE(m[-0.0]);
        m[-0.0] = true;
        EXPECT_TRUE(m[0.0]);
    }
}

TEST(MAP, key_enum)
{
    {
        enum Valves {VLV_None, VLV_Forward, VLV_Backward} ;
        twins::Map<Valves, bool> m;

        EXPECT_FALSE(m.contains(VLV_None));
        EXPECT_FALSE(m[VLV_Forward]);
        m[VLV_Forward] = true;
        EXPECT_TRUE(m[VLV_Forward]);
    }

    {
        enum class Valves {None, Forward, Backward} ;
        twins::Map<Valves, int> m;
        m[Valves::Forward] = true;
    }
}

TEST(MAP, key_string)
{
    {
        twins::Map<const char*, bool> m;
        const char *key = "Göbekli Tepe";
        EXPECT_FALSE(m.contains(key));
        EXPECT_FALSE(m[key]);
        EXPECT_TRUE(m.contains(key));

        m[key] = true;
        std::string k = key; // same key, different pointer value
        EXPECT_TRUE(m[k.c_str()]);
        EXPECT_EQ(1, m.size());
    }

    {
        twins::Map<char*, bool> m;
        char key[10]; strcpy(key, "*key");

        EXPECT_FALSE(m[key]);
        m[key] = true;
        EXPECT_TRUE(m[key]);
    }

    // example of own hashing object for not embedded key type
    {
        struct HashStdStr { static uint16_t hash(const std::string& str) { return twins::HashDefault::bernsteinHashImpl(str.data(), str.size()); } };
        twins::Map<std::string, bool, HashStdStr> m;

        EXPECT_FALSE(m["exists"]);
        m["exists"] = true;
        EXPECT_TRUE(m["exists"]);
    }
}

TEST(MAP, iterators_small)
{
    twins::Map<short, int> m;
    EXPECT_EQ(0, m.size());

    // map empty
    {
        int n = 0;
        for (auto it : m)
            (void)it, n++;
        EXPECT_EQ(0, n);
    }

    // 1 element
    {
        m[0] = 0;
        EXPECT_EQ(1, m.size());
        EXPECT_EQ(4, m.bucketsCount());

        for (auto it : m)
        {
            printf("m[%d | %04x] : %d\n", it.key, it.hash, it.val);
            it.val++;
        }
    }
}

TEST(MAP, iterators_big)
{
    twins::Map<short, int> m;
    EXPECT_EQ(0, m.size());

    for (int i = 4; i < 10; i++)
        m[i] = i*10;

    {
        auto b = m.begin();
        auto e = m.end();

        while (b != e)
        {
            // b->hash;
            ++b;
        }
    }

    {
        int n = 0;
        for (auto &it : m)
        {
            printf("m[%d | %04x] : %d\n", it.key, it.hash, it.val);
            n++;
        }

        EXPECT_EQ(m.size(), n);
    }

    // iterating const-map
    {
        int x = 0;
        const auto &map = m;
        for (auto &it : map)
        {
            // it.val++; error -> ok
            x += it.val;
        }
    }
}
