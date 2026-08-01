#ifndef RING_NETWORK_TCP_LISTENER_HPP_
#define RING_NETWORK_TCP_LISTENER_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/endpoint.hpp"
#include "ring/network/error_code.hpp"

namespace ring::network
{

class tcp_connection;

class RING_API tcp_listener
{
public:
    using accept_handler = std::function<void(error_code, std::unique_ptr<tcp_connection>)>;
public:
    tcp_listener() = default;
    virtual ~tcp_listener() = default;
public:
    virtual void async_accept(accept_handler handler) = 0;
    virtual void close() = 0;
    virtual endpoint local_endpoint() const = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_LISTENER_HPP_