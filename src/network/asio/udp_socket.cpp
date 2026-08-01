#include "udp_socket.hpp"

namespace ring::network
{

asio_udp_socket::asio_udp_socket(asio::io_context& asio_ctx) :
    socket_(asio_ctx) {}

void asio_udp_socket::bind(const endpoint& ep)
{
    auto asio_ep = asio::ip::udp::endpoint(asio::ip::make_address(ep.address), ep.port);

    socket_.open(asio_ep.protocol(), ec_);
    if (!ec_)
    {
        socket_.bind(asio_ep, ec_);
    }
    bound_ = true;
}

void asio_udp_socket::async_receive_from(mutable_buffer buf, receive_handler handler)
{
    if (!bound_)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::invalid_argument), 0, {});
            });
        return;
    }
    if (ec_)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler), ec = ec_]()
            {
                handler(ec, 0, {});
            });
        return;
    }
    
    auto ep = std::make_shared<asio::ip::udp::endpoint>();
    socket_.async_receive_from(asio::buffer(buf.data(), buf.size()), *ep,
        [h = std::move(handler), ep](asio::error_code ec, size_t n)
        {
            endpoint from{ ep->address().to_string(), ep->port() };
            h(ec, n, from);
        });
}

void asio_udp_socket::async_send_to(const_buffer buf, const endpoint& to, send_handler handler)
{
    if (!bound_)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler)]()
            {
                handler(std::make_error_code(std::errc::invalid_argument), 0);
            });
        return;
    }
    if (ec_)
    {
        asio::post(socket_.get_executor(),
            [handler = std::move(handler), ec = ec_]()
            {
                handler(ec, 0);
            });
        return;
    }

    socket_.async_send_to(asio::buffer(buf.data(), buf.size()),
        asio::ip::udp::endpoint(asio::ip::make_address(to.address), to.port),
        [h = std::move(handler)](asio::error_code ec, size_t n)
        {
            h(ec, n);
        });
}

void asio_udp_socket::close()
{
    asio::error_code ec;
    socket_.close(ec);
}

} // namespace ring::network
