#include "udp_socket.hpp"

#include "ring/core/exception.hpp"

namespace ring::network
{

asio_udp_socket::asio_udp_socket(asio::any_io_executor asio_ex) :
    socket_(std::move(asio_ex)) {}

asio_udp_socket::~asio_udp_socket()
{
    do_close();
}

void asio_udp_socket::bind(const endpoint& ep)
{
    asio::error_code ec;
    auto asio_ep = asio::ip::udp::endpoint(asio::ip::make_address(ep.address, ec), ep.port);
    if (!ec)
    {   
        socket_.open(asio_ep.protocol(), ec);
    }
    if (!ec)
    {
        socket_.bind(asio_ep, ec);
    }
    if (ec)
    {
        asio::error_code e;
        socket_.close(e);

        throw ring::core::exception("can not bind on " + ep.to_string() + ": " + ec.message());
    }
}

void asio_udp_socket::async_receive_from(mutable_buffer buf, receive_handler handler)
{
    if (!socket_.is_open())
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::not_connected), 0, {});
            });
        return;
    }
    
    socket_.async_receive_from(asio::buffer(buf.data(), buf.size()), remote_ep_,
        [this, handler = std::move(handler)](asio::error_code ec, size_t n)
        {
            endpoint from{ remote_ep_.address().to_string(), remote_ep_.port() };
            handler(ec, n, from);
        });
}

void asio_udp_socket::async_send_to(const_buffer buf, const endpoint& to, send_handler handler)
{
    if (!socket_.is_open())
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::not_connected), 0);
            });
        return;
    }

    asio::error_code ec;
    auto ep = asio::ip::udp::endpoint(asio::ip::make_address(to.address, ec), to.port);
    if (ec)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::invalid_argument), 0);
            });
        return;
    }
    
    socket_.async_send_to(asio::buffer(buf.data(), buf.size()), ep,
        [handler = std::move(handler)](asio::error_code ec, size_t n)
        {
            handler(ec, n);
        });
}

void asio_udp_socket::close()
{
    do_close();
}

void asio_udp_socket::do_close()
{
    if (socket_.is_open())
    {   
        asio::error_code ec;
        socket_.close(ec);
    }
}

} // namespace ring::network
