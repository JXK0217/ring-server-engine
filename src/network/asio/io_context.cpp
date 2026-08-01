#include "io_context.hpp"

#include <asio/post.hpp>

#include "tcp_listener.hpp"
#include "tcp_connector.hpp"

namespace ring::network
{

asio_io_context::asio_io_context() :
    work_guard_(asio::make_work_guard(ctx_)) {}

void asio_io_context::run()
{
    ctx_.run();
}

void asio_io_context::stop()
{
    work_guard_.reset();
    ctx_.stop();
}

void asio_io_context::post(std::function<void()> task)
{
    asio::post(ctx_, std::move(task));
}

std::unique_ptr<tcp_listener> asio_io_context::create_tcp_listener(const endpoint& ep)
{
    return std::make_unique<asio_tcp_listener>(ctx_, ep);
}

std::unique_ptr<tcp_connector> asio_io_context::create_tcp_connector(const endpoint& ep)
{
    return std::make_unique<asio_tcp_connector>(ctx_, ep);
}

std::unique_ptr<io_context> io_context::create()
{
    return std::make_unique<asio_io_context>();
}

} // namespace ring::network
