#include "tcp_connector.hpp"

#include "tcp_connection.hpp"

#include "ring/core/exception.hpp"

namespace ring::network
{

asio_tcp_connector::asio_tcp_connector(asio::any_io_executor asio_ex, const endpoint& ep) :
    socket_(std::move(asio_ex))
{
    asio::error_code ec;
    ep_ = asio::ip::tcp::endpoint(asio::ip::make_address(ep.address, ec), ep.port);
    if (ec)
    {
        throw ring::core::exception("can not connect to " + ep.to_string() + ": " + ec.message());
    }
}

asio_tcp_connector::~asio_tcp_connector()
{
    do_close();
}

void asio_tcp_connector::async_connect(connect_handler handler)
{
    if (connecting_)
    {
        throw ring::core::exception("already connecting");
    }

    do_close();
    connecting_ = true;
    socket_.async_connect(ep_,
        [this, handler = std::move(handler)](asio::error_code ec)
        {
            connecting_ = false;
            if (ec)
            {
                do_close();
                handler(ec, nullptr);
                return;
            }
            handler(ec, std::make_unique<asio_tcp_connection>(std::move(socket_)));
        });
}

void asio_tcp_connector::close()
{
    do_close();
}

void asio_tcp_connector::do_close()
{
    if (socket_.is_open())
    {
        asio::error_code ec;
        socket_.close(ec);
    }
}

} // namespace ring::network
