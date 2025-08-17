#include "kv_storage.h"
#include <tuple>
#include <iostream>

int main()
{
    using Clock = SteadyClockWrapper;
    std::tuple<std::string, std::string, uint32_t> init[] = {
        {"a", "1", 0},
        {"b", "2", 5},
    };
    KVStorage<Clock> store{std::span{init}};
    if (auto v = store.get("a"))
    {
        std::cout << "a=" << *v << "\n";
    }
    else
    {
        std::cout << "a not found\n";
    }
    return 0;
}
