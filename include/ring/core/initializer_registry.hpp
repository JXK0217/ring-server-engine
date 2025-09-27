#ifndef __RING_CORE_INITIALIZER_REGISTRY_HPP__
#define __RING_CORE_INITIALIZER_REGISTRY_HPP__

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
        _entries.emplace_back(std::move(name), std::move(initialize), std::move(shutdown), priority);
    }
public:
    void initialize()
    {
        if (_initialized)
        {
            throw ring::core::exception("_initialized");
        }
        std::sort(_entries.begin(), _entries.end(), 
            [](const auto& l, const auto& r){ return l.priority < r.priority; });
        for (auto& entry: _entries)
        {
            if (entry.initialize)
            {
                entry.initialize();
            }
        }
        _initialized = true;
    }
    void shutdown()
    {
        if (!_initialized)
        {
            throw ring::core::exception("!_initialized");
        }
        for (auto& entry: _entries | std::views::reverse)
        {
            if (entry.shutdown)
            {
                entry.shutdown();
            }
        }
    }
private:
    std::vector<entry> _entries;
    bool _initialized = false;
};

} // namespace ring::core

#endif // !__RING_CORE_INITIALIZER_REGISTRY_HPP__