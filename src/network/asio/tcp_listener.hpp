#ifndef RING_NETWORK_ASIO_TCP_LISTENER_HPP_
#define RING_NETWORK_ASIO_TCP_LISTENER_HPP_

#include "ring/network/tcp_listener.hpp"

#include <asio/ip/tcp.hpp>

namespace ring::network
{

class asio_tcp_listener final : public tcp_listener
{
public:
    explicit asio_tcp_listener(asio::io_context& asio_ctx, const endpoint& ep);
    ~asio_tcp_listener() = default;
public:
    void async_accept(accept_handler handler) override;
    void close() override;
    endpoint local_endpoint() const override;
private:
    void do_accept();
private:
    asio::ip::tcp::acceptor acceptor_;
    asio::error_code ec_;
    accept_handler handler_;
    bool accepting_ = false;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_TCP_LISTENER_HPP_