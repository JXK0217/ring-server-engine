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
    explicit asio_udp_socket(asio::any_io_executor asio_ex);
    ~asio_udp_socket();
public:
    void bind(const endpoint& ep) override;
    void async_receive_from(mutable_buffer buf, receive_handler handler) override;
    void async_send_to(const_buffer buf, const endpoint& to, send_handler handler) override;
    void close() override;
private:
    void do_close();
private:
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint remote_ep_;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_UDP_SOCKET_HPP_