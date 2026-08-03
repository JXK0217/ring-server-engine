#ifndef RING_NETWORK_ASIO_TIMER_HPP_
#define RING_NETWORK_ASIO_TIMER_HPP_

#include "ring/network/timer.hpp"

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

namespace ring::network
{

class asio_timer final : public timer
{
public:
    explicit asio_timer(asio::io_context& asio_ctx);
    ~asio_timer() = default;
public:
    void expires_after(std::chrono::steady_clock::duration duration) override;
    void async_wait(wait_handler handler) override;
    void cancel() override;
private:
    asio::steady_timer timer_;
};
    
} // namespace ring::network

#endif // RING_NETWORK_ASIO_TIMER_HPP_