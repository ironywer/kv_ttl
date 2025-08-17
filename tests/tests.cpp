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