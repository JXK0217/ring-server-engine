#ifndef RING_NETWORK_ASIO_TCP_LISTENER_HPP_
#define RING_NETWORK_ASIO_TCP_LISTENER_HPP_

#include "ring/network/tcp_listener.hpp"

#include <asio/ip/tcp.hpp>

namespace ring::network
{

class asio_tcp_listener final : public tcp_listener
{
public:
    explicit asio_tcp_listener(asio::any_io_executor asio_ex, const endpoint& ep);
    ~asio_tcp_listener();
public:
    void async_accept(accept_handler handler) override;
    void close() override;
    endpoint local_endpoint() const override;
private:
    void do_accept(accept_handler handler);
    void do_close();
private:
    asio::ip::tcp::acceptor acceptor_;
    bool accepting_ = false;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_TCP_LISTENER_HPP_