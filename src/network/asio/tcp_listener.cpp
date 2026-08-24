#include "tcp_listener.hpp"

#include <asio/ip/tcp.hpp>

#include "tcp_connection.hpp"

#include "ring/core/exception.hpp"

namespace ring::network
{

namespace detail
{

bool should_try_accept_again(const asio::error_code &ec)
{
    return ec == asio::error::connection_aborted
        || ec == asio::error::connection_reset
        || ec == asio::error::interrupted
        || ec == asio::error::network_down
        || ec == asio::error::network_reset
        || ec == asio::error::network_unreachable;
}

} // namespace detail

asio_tcp_listener::asio_tcp_listener(asio::any_io_executor asio_ex, const endpoint& ep) :
    acceptor_(std::move(asio_ex))
{
    asio::error_code ec;
    auto asio_ep = asio::ip::tcp::endpoint(asio::ip::make_address(ep.address, ec), ep.port);
    if (!ec)
    {
        acceptor_.open(asio_ep.protocol(), ec);
    }
    if (!ec)
    {
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec);
    }
    if (!ec)
    {
        acceptor_.bind(asio_ep, ec);
    }
    if (!ec)
    {
        acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    }
    if (ec)
    {
        asio::error_code e;
        acceptor_.close(e);

        throw ring::core::exception("can not listen on " + ep.to_string() + ": " + ec.message());
    }
}

asio_tcp_listener::~asio_tcp_listener()
{
    do_close();
}

void asio_tcp_listener::async_accept(accept_handler handler)
{
    if (accepting_)
    {
        throw ring::core::exception("already accepting");
    }

    accepting_ = true;
    do_accept(std::move(handler));
}

void asio_tcp_listener::close()
{
    do_close();
}

endpoint asio_tcp_listener::local_endpoint() const
{
    asio::error_code ec;
    auto ep = acceptor_.local_endpoint(ec);
    return ec ? endpoint{} : endpoint{ ep.address().to_string(), ep.port() };
}

void asio_tcp_listener::do_accept(accept_handler handler)
{
    if (!acceptor_.is_open())
    {
        accepting_ = false;
        asio::post(acceptor_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::connection_aborted), nullptr);
            });
        return;
    }

    auto sock = std::make_shared<asio::ip::tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*sock,
        [this, sock, handler = std::move(handler)](asio::error_code ec) mutable
        {
            if (ec)
            {
                if (detail::should_try_accept_again(ec))
                {
                    if (accepting_ && acceptor_.is_open())
                    {
                        do_accept(std::move(handler));
                    }
                    else
                    {
                        accepting_ = false;
                    }
                }
                else
                {
                    accepting_ = false;
                    handler(ec, nullptr);
                }
                return;
            }

            handler(ec, std::make_unique<asio_tcp_connection>(std::move(*sock)));
            if (accepting_ && acceptor_.is_open())
            {
                do_accept(std::move(handler));
            }
            else
            {
                accepting_ = false;
            }
        });
}

void asio_tcp_listener::do_close()
{
    accepting_ = false;
    if (acceptor_.is_open())
    {   
        asio::error_code ec;
        acceptor_.close(ec);
    }
}

} // namespace ring::network
