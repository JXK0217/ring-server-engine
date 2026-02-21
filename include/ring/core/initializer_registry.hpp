#ifndef RING_CORE_INITIALIZER_REGISTRY_HPP_
#define RING_CORE_INITIALIZER_REGISTRY_HPP_

#include <algorithm>
#include <functional>
#include <ranges>
#include <string>
#include <vector>

#include "ring/core/exception.hpp"
#include "ring/core/export.hpp"

namespace ring::core
{

class RING_API initializer_registry final
{
private:
    using handle = std::function<void()>;
private:
    struct entry
    {
        std::string name;
        handle initialize;
        handle shutdown;
        int priority = 0;
    };
private:
    initializer_registry() = default;
    ~initializer_registry() = default;
private:
    initializer_registry(const initializer_registry&) = delete;
    initializer_registry& operator=(const initializer_registry&) = delete;
public:
    static initializer_registry& instance()
    {
        static initializer_registry instance;
        return instance;
    }
public:
    void register_entry(std::string name, handle initialize, handle shutdown, int priority = 0)
    {
        entries_.emplace_back(std::move(name), std::move(initialize), std::move(shutdown), priority);
    }
public:
    void initialize()
    {
        if (initialized_)
        {
            throw ring::core::exception("already initialized");
        }
        std::sort(entries_.begin(), entries_.end(), 
            [](const auto& l, const auto& r){ return l.priority < r.priority; });
        for (auto& entry: entries_)
        {
            if (entry.initialize)
            {
                entry.initialize();
            }
        }
        initialized_ = true;
    }
    void shutdown()
    {
        if (!initialized_)
        {
            throw ring::core::exception("not initialized");
        }
        for (auto& entry: entries_ | std::views::reverse)
        {
            if (entry.shutdown)
            {
                entry.shutdown();
            }
        }
    }
private:
    std::vector<entry> entries_;
    bool initialized_ = false;
};

} // namespace ring::core

#endif // RING_CORE_INITIALIZER_REGISTRY_HPP_