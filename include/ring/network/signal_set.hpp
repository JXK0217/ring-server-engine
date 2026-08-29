#ifndef RING_NETWORK_SIGNAL_SET_HPP_
#define RING_NETWORK_SIGNAL_SET_HPP_

#include <functional>

#include "ring/core/export.hpp"
#include "ring/network/error_code.hpp"

namespace ring::network
{

class RING_API signal_set
{
public:
    using wait_handler = std::function<void(error_code, int32_t)>;
public:
    signal_set() = default;
    virtual ~signal_set() = default;
public:
    virtual void add(int32_t sig) = 0;
    virtual void remove(int32_t sig) = 0;
    virtual void async_wait(wait_handler handler) = 0;
    virtual void cancel() = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_SIGNAL_SET_HPP_