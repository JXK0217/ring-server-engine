#ifndef RING_NETWORK_UDP_SOCKET_HPP_
#define RING_NETWORK_UDP_SOCKET_HPP_

#include <functional>

#include "ring/core/export.hpp"
#include "ring/network/endpoint.hpp"
#include "ring/network/error_code.hpp"
#include "ring/network/socket_buffer.hpp"

namespace ring::network
{
    
class RING_API udp_socket
{
public:
    using receive_handler   = std::function<void(error_code, size_t, endpoint from)>;
    using send_handler      = std::function<void(error_code, size_t)>;
public:
    udp_socket() = default;
    virtual ~udp_socket() = default;
public:
    virtual void async_receive_from(mutable_buffer buf, receive_handler handler) = 0;
    virtual void async_send_to(const_buffer buf, const endpoint& to, send_handler handler) = 0;
    virtual void bind(const endpoint& ep) = 0;
    virtual void close() = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_UDP_SOCKET_HPP_