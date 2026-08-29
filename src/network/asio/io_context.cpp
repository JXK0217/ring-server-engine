#include "io_context.hpp"

#include <asio/strand.hpp>

#include "executor.hpp"
#include "signal_set.hpp"
#include "tcp_connector.hpp"
#include "tcp_listener.hpp"
#include "timer.hpp"
#include "udp_socket.hpp"

namespace ring::network
{

asio_io_context::asio_io_context() :
    work_guard_(asio::make_work_guard(ctx_)),
    ex_(std::make_unique<asio_executor>(ctx_.get_executor())) {}

void asio_io_context::run()
{
    ctx_.run();
}

void asio_io_context::stop()
{
    work_guard_.reset();
    ctx_.stop();
}

executor& asio_io_context::get_executor()
{
    return *ex_;
}

std::unique_ptr<executor> asio_io_context::create_strand()
{
    return std::make_unique<asio_executor>(asio::make_strand(ctx_));
}

std::unique_ptr<signal_set> asio_io_context::create_signal_set(executor& ex)
{
    auto& asio_ex = static_cast<asio_executor&>(ex);
    return std::make_unique<asio_signal_set>(asio_ex.raw_ex());
}

std::unique_ptr<tcp_listener> asio_io_context::create_tcp_listener(executor& ex, const endpoint& ep)
{
    auto& asio_ex = static_cast<asio_executor&>(ex);
    return std::make_unique<asio_tcp_listener>(asio_ex.raw_ex(), ep);
}

std::unique_ptr<tcp_connector> asio_io_context::create_tcp_connector(executor& ex, const endpoint& ep)
{
    auto& asio_ex = static_cast<asio_executor&>(ex);
    return std::make_unique<asio_tcp_connector>(asio_ex.raw_ex(), ep);
}

std::unique_ptr<timer> asio_io_context::create_timer(executor& ex)
{
    auto& asio_ex = static_cast<asio_executor&>(ex);
    return std::make_unique<asio_timer>(asio_ex.raw_ex());
}

std::unique_ptr<udp_socket> asio_io_context::create_udp_socket(executor& ex)
{
    auto& asio_ex = static_cast<asio_executor&>(ex);
    return std::make_unique<asio_udp_socket>(asio_ex.raw_ex());
}

std::unique_ptr<io_context> io_context::create()
{
    return std::make_unique<asio_io_context>();
}

} // namespace ring::network
