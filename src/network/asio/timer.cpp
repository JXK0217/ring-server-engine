#include "timer.hpp"

namespace ring::network
{

asio_timer::asio_timer(asio::any_io_executor asio_ex) :
    timer_(std::move(asio_ex)) {}

void asio_timer::expires_after(std::chrono::steady_clock::duration duration)
{
    timer_.expires_after(duration);
}

void asio_timer::async_wait(wait_handler handler)
{
    timer_.async_wait(
        [handler = std::move(handler)](asio::error_code ec)
        {
            handler(ec);
        });
}

void asio_timer::cancel()
{
    timer_.cancel();
}

} // namespace ring::network
