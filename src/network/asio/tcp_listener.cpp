#include "tcp_listener.hpp"

#include <asio/ip/tcp.hpp>

#include "tcp_connection.hpp"

namespace ring::network
{

asio_tcp_listener::asio_tcp_listener(asio::io_context& asio_ctx, const endpoint& ep) :
    acceptor_(asio_ctx)
{
    auto asio_ep = asio::ip::tcp::endpoint(asio::ip::make_address(ep.address), ep.port);
    
    acceptor_.open(asio_ep.protocol(), ec_);
    if (!ec_)
    {
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true), ec_);
    }
    if (!ec_)
    {
        acceptor_.bind(asio_ep, ec_);
    }
    if (!ec_)
    {
        acceptor_.listen(asio::socket_base::max_listen_connections, ec_);
    }
}

void asio_tcp_listener::async_accept(accept_handler handler)
{
    if (ec_)
    {
        asio::post(acceptor_.get_executor(), 
            [handler = std::move(handler), ec = ec_]()
            {
                handler(ec, nullptr);
            });
        return;
    }

    handler_ = std::move(handler);
    if (!accepting_)
    {
        accepting_ = true;
        do_accept();
    }
}

void asio_tcp_listener::close()
{
    accepting_ = false;
    acceptor_.close();
}

endpoint asio_tcp_listener::local_endpoint() const
{
    asio::error_code ec;
    auto ep = acceptor_.local_endpoint(ec);
    return { ep.address().to_string(), ep.port() };
}

void asio_tcp_listener::do_accept()
{
    auto sock = std::make_shared<asio::ip::tcp::socket>(acceptor_.get_executor());
    acceptor_.async_accept(*sock,
        [this, sock](asio::error_code ec) mutable
        {
            if (ec)
            {
                handler_(ec, nullptr);
                accepting_ = false;
                return;
            }
            handler_(ec, std::make_unique<asio_tcp_connection>(std::move(*sock)));
            if (accepting_)
            {
                do_accept();
            }
        });
}

} // namespace ring::network
