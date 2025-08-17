#include "kv_storage.h"
#include <gtest/gtest.h>
#include <tuple>
#include <optional>
#include <string>
#include <span>

struct TestClock
{
    using underlying = std::chrono::steady_clock;
    using time_point = underlying::time_point;
    using duration = underlying::duration;

    static time_point &ref_now()
    {
        static time_point t = underlying::now();
        return t;
    }
    static time_point now() { return ref_now(); }
    static void advance(std::chrono::seconds s) { ref_now() += s; }
};

using Store = KVStorage<TestClock>;

TEST(KVStorage_Basics, InitFromSpan_ProvidesData)
{
    std::tuple<std::string, std::string, uint32_t> init[] = {
        {"a", "1", 0},
        {"b", "2", 0}};
    Store s{std::span{init}};

    auto va = s.get("a");
    auto vb = s.get("b");
    EXPECT_TRUE(va.has_value());
    EXPECT_TRUE(vb.has_value());
    EXPECT_EQ(*va, "1");
    EXPECT_EQ(*vb, "2");
}

TEST(KVStorage_Basics, GetMissing_ReturnsNullopt)
{
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});
    EXPECT_EQ(s.get("nope"), std::nullopt);
}

TEST(KVStorage_Basics, SetThenGet_Works)
{
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});
    s.set("k", "v", 0);
    auto v = s.get("k");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "v");
}

TEST(KVStorage_Basics, SetOverwrite_ReplacesValue)
{
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});
    s.set("x", "old", 0);
    s.set("x", "new", 0);
    auto v = s.get("x");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "new");
}

TEST(KVStorage_TTL, ExpireAndRemoveOne)
{
    TestClock::ref_now() = TestClock::underlying::now();
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});

    s.set("t", "val", 2);
    EXPECT_EQ(s.get("t"), std::optional<std::string>("val"));

    TestClock::advance(std::chrono::seconds(3));
    EXPECT_EQ(s.get("t"), std::nullopt);

    auto removed = s.removeOneExpiredEntry();
    ASSERT_TRUE(removed.has_value());
    EXPECT_EQ(removed->first, "t");
    EXPECT_EQ(removed->second, "val");

    EXPECT_EQ(s.removeOneExpiredEntry(), std::nullopt);
}

TEST(KVStorage_API, RemoveExistingAndMissing)
{
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});
    s.set("x", "1", 0);

    EXPECT_TRUE(s.remove("x"));
    EXPECT_FALSE(s.remove("x"));
    EXPECT_EQ(s.get("x"), std::nullopt);
}

TEST(KVStorage_Range, GetManySortedSkipsExpired)
{
    TestClock::ref_now() = TestClock::underlying::now();
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});

    s.set("a", "1", 0);
    s.set("b", "2", 1);
    s.set("c", "3", 0);
    s.set("d", "4", 2);

    auto v1 = s.getManySorted("b", 3);
    ASSERT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1[0].first, "b");
    EXPECT_EQ(v1[1].first, "c");
    EXPECT_EQ(v1[2].first, "d");

    TestClock::advance(std::chrono::seconds(2));
    auto v2 = s.getManySorted("a", 5);
    ASSERT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2[0].first, "a");
    EXPECT_EQ(v2[1].first, "c");
}

TEST(KVStorage_TTL, RemoveSeveralExpiredInAnyOrder)
{
    TestClock::ref_now() = TestClock::underlying::now();
    Store s(std::span<std::tuple<std::string, std::string, uint32_t>>{});

    s.set("a", "1", 1);
    s.set("b", "2", 2);
    s.set("c", "3", 3);

    TestClock::advance(std::chrono::seconds(5));

    std::set<std::string> got;
    for (int i = 0; i < 3; ++i)
    {
        auto r = s.removeOneExpiredEntry();
        ASSERT_TRUE(r.has_value());
        got.insert(r->first);
    }
    EXPECT_TRUE(got.count("a"));
    EXPECT_TRUE(got.count("b"));
    EXPECT_TRUE(got.count("c"));
    EXPECT_EQ(s.removeOneExpiredEntry(), std::nullopt);
}
