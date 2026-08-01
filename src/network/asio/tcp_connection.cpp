#include "tcp_connection.hpp"

#include <asio/write.hpp>
#include <asio/ip/tcp.hpp>

namespace ring::network
{

asio_tcp_connection::asio_tcp_connection(asio::ip::tcp::socket socket) :
    socket_(std::move(socket)) {}

void asio_tcp_connection::async_read_some(mutable_buffer buf, read_handler handler)
{
    socket_.async_read_some(asio::buffer(buf.data(), buf.size()),
        [h = std::move(handler)](asio::error_code ec, size_t n)
        {
            h(ec, n);
        });
}

void asio_tcp_connection::async_write(const_buffer buf, write_handler handler)
{
    asio::async_write(socket_, asio::buffer(buf.data(), buf.size()),
        [h = std::move(handler)](asio::error_code ec, size_t n)
        {
            h(ec, n);
        });
}

void asio_tcp_connection::close()
{
    asio::error_code ec;
    socket_.close(ec);
}

endpoint asio_tcp_connection::local_endpoint() const
{
    asio::error_code ec;
    auto ep = socket_.local_endpoint(ec);
    return { ep.address().to_string(), ep.port() };
}

endpoint asio_tcp_connection::remote_endpoint() const
{
    asio::error_code ec;
    auto ep = socket_.remote_endpoint(ec);
    return { ep.address().to_string(), ep.port() };
}

} // namespace ring::network
