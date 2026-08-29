#include "signal_set.hpp"

namespace ring::network
{

asio_signal_set::asio_signal_set(asio::any_io_executor ex) :
    signals_(std::move(ex)) {}

void asio_signal_set::add(int32_t sig)
{
    asio::error_code ec;
    signals_.add(sig, ec);
}

void asio_signal_set::remove(int32_t sig)
{
    asio::error_code ec;
    signals_.remove(sig, ec);
}

void asio_signal_set::async_wait(wait_handler handler)
{
    signals_.async_wait(
        [handler = std::move(handler)](asio::error_code ec, int32_t sig)
        {
            handler(ec, sig);
        });
}

void asio_signal_set::cancel()
{
    asio::error_code ec;
    signals_.cancel(ec);
}

} // namespace ring::network
