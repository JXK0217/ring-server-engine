#ifndef RING_NETWORK_ASIO_TCP_CONNECTOR_HPP_
#define RING_NETWORK_ASIO_TCP_CONNECTOR_HPP_

#include "ring/network/tcp_connector.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

#include "ring/network/endpoint.hpp"
#include "ring/network/error_code.hpp"

namespace ring::network
{
    
class asio_tcp_connector final : public tcp_connector
{
private:
    enum class state
    {
        idle,
        connecting,
        connected,
    };
public:
    explicit asio_tcp_connector(asio::io_context& asio_ctx, const endpoint& ep);
    ~asio_tcp_connector() = default;
public:
    void async_connect(connect_handler handler) override;
    void cancel() override;
private:
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::endpoint ep_;
    state state_ = state::idle;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_TCP_CONNECTOR_HPP_