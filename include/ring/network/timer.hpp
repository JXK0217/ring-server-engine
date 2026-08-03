#ifndef RING_NETWORK_TIMER_HPP_
#define RING_NETWORK_TIMER_HPP_

#include <chrono>
#include <functional>

#include "ring/core/export.hpp"
#include "ring/network/error_code.hpp"

namespace ring::network
{

class RING_API timer
{
public:
    using wait_handler  = std::function<void(error_code)>;
public:
    timer() = default;
    virtual ~timer() = default;
public:
    virtual void expires_after(std::chrono::steady_clock::duration duration) = 0;
    virtual void async_wait(wait_handler handler) = 0;
    virtual void cancel() = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_TIMER_HPP_