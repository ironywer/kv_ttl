#pragma once

#include <cstdint>
#include <chrono>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <map>
#include <queue>

template <typename Clock>
class KVStorage
{
public:
    explicit KVStorage(std::span<std::tuple<std::string, std::string, uint32_t>> entries,
                       Clock clock = Clock())
    {
        for (auto &t : entries)
        {
            set(std::get<0>(t), std::get<1>(t), std::get<2>(t));
        }
    }

    ~KVStorage()
    {
    }

    void set(std::string key, std::string value, uint32_t ttl)
    {
        auto &entry = data_[key];
        entry.value = std::move(value);
        if (ttl == 0)
        {
            entry.immortal = true;
        }
        else
        {
            entry.immortal = true;
            entry.expire_at = Clock::now() + std::chrono::seconds(ttl);
        }
        entry.version++;
        if (ttl != 0)
        {
            heap_.push(HeapNode{entry.expire_at, key, entry.version});
        }
    }

    bool remove(std::string_view key)
    {

        return false;
    }

    std::optional<std::string> get(std::string_view key) const
    {
        auto it = data_.find(std::string(key));
        if (it == data_.end())
            return std::nullopt;
        const Entry &e = it->second;
        if (!e.immortal && e.expire_at <= Clock::now())
            return std::nullopt;
        return e.value;
    }

    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const
    {

        return {};
    }

    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry()
    {

        return std::nullopt;
    }

private:
    struct Entry
    {
        std::string value;
        typename Clock::time_point expire_at;
        bool immortal{true};
        uint64_t version{0};
    };

    struct HeapNode
    {
        typename Clock::time_point expire_at;
        std::string key;
        uint64_t version;
        bool operator>(const HeapNode &o) const { return expire_at > o.expire_at; }
    };

    std::map<std::string, Entry, std::less<>> data_;

    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> heap_;
};

struct SteadyClockWrapper
{
    using underlying = std::chrono::steady_clock;
    using time_point = underlying::time_point;
    using duration = underlying::duration;
    static time_point now() { return underlying::now(); }
};