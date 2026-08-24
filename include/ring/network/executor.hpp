#ifndef RING_NETWORK_EXECUTOR_HPP_
#define RING_NETWORK_EXECUTOR_HPP_

#include <functional>

#include "ring/core/export.hpp"

namespace ring::network
{
    
class RING_API executor
{
public:
    using executor_task = std::function<void()>;
public:
    executor() = default;
    virtual ~executor() = default;
public:
    virtual void post(executor_task task) = 0;
    virtual void dispatch(executor_task task) = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_EXECUTOR_HPP_