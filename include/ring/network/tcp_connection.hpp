#ifndef RING_NETWORK_TCP_CONNECTION_HPP_
#define RING_NETWORK_TCP_CONNECTION_HPP_

#include <functional>

#include "ring/core/export.hpp"
#include "ring/network/endpoint.hpp"
#include "ring/network/error_code.hpp"
#include "ring/network/socket_buffer.hpp"

namespace ring::network
{

class RING_API tcp_connection
{
public:
    using read_handler  = std::function<void(error_code, size_t)>;
    using write_handler = std::function<void(error_code, size_t)>;
public:
    tcp_connection() = default;
    virtual ~tcp_connection() = default;
public:
    virtual void async_read_some(mutable_buffer buf, read_handler handler) = 0;
    virtual void async_write(const_buffer buf, write_handler handler) = 0;
    virtual void close() = 0;
    virtual endpoint local_endpoint() const = 0;
    virtual endpoint remote_endpoint() const = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_CONNECTION_HPP_