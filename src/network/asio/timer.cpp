#include "timer.hpp"

namespace ring::network
{

asio_timer::asio_timer(asio::io_context& asio_ctx) :
    timer_(asio_ctx) {}

void asio_timer::expires_after(std::chrono::steady_clock::duration duration)
{
    timer_.expires_after(duration);
}

void asio_timer::async_wait(wait_handler handler)
{
    timer_.async_wait(
        [h = std::move(handler)](asio::error_code ec)
        {
            h(ec);
        });
}

void asio_timer::cancel()
{
    timer_.cancel();
}

} // namespace ring::network
