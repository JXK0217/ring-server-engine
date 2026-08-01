#ifndef RING_NETWORK_ASIO_UDP_SOCKET_HPP_
#define RING_NETWORK_ASIO_UDP_SOCKET_HPP_

#include "ring/network/udp_socket.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

namespace ring::network
{

class asio_udp_socket final : public udp_socket
{
public:
    explicit asio_udp_socket(asio::io_context& asio_ctx);
    ~asio_udp_socket() = default;
public:
    void bind(const endpoint& ep) override;
    void async_receive_from(mutable_buffer buf, receive_handler handler) override;
    void async_send_to(const_buffer buf, const endpoint& to, send_handler handler) override;
    void close() override;
private:
    asio::ip::udp::socket socket_;
    asio::error_code ec_;
    bool bound_ = false;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_UDP_SOCKET_HPP_