#include "tcp_connector.hpp"

#include "tcp_connection.hpp"

namespace ring::network
{

asio_tcp_connector::asio_tcp_connector(asio::io_context& asio_ctx, const endpoint& ep) :
    socket_(asio_ctx), ep_(asio::ip::make_address(ep.address), ep.port) {}

void asio_tcp_connector::async_connect(connect_handler handler)
{
    if (state_ == state::connected)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::already_connected), nullptr);
            });
        return;
    }
    if (state_ == state::connecting)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::device_or_resource_busy), nullptr);
            });
        return;
    }

    state_ = state::connecting;
    socket_.async_connect(ep_,
        [this, handler = std::move(handler)](asio::error_code ec) mutable
        {
            if (ec)
            {
                state_ = state::idle;
                handler(ec, nullptr);
                return;
            }
            state_ = state::connected;
            handler(ec, std::make_unique<asio_tcp_connection>(std::move(socket_)));
        });
}

void asio_tcp_connector::cancel()
{
    asio::error_code ec;
    socket_.cancel(ec);
}

} // namespace ring::network
