#ifndef RING_NETWORK_ASIO_TCP_CONNECTION_HPP_
#define RING_NETWORK_ASIO_TCP_CONNECTION_HPP_

#include "ring/network/tcp_connection.hpp"

#include <asio/ip/tcp.hpp>

namespace ring::network
{

class asio_tcp_connection final : public tcp_connection
{
public:
    explicit asio_tcp_connection(asio::ip::tcp::socket socket);
    ~asio_tcp_connection() = default;
public:
    void async_read_some(mutable_buffer buf, read_handler handler) override;
    void async_write(const_buffer buf, write_handler handler) override;
    void close() override;
    endpoint local_endpoint() const override;
    endpoint remote_endpoint() const override;
private:
    asio::ip::tcp::socket socket_;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_TCP_CONNECTION_HPP_