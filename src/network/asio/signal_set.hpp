#ifndef RING_NETWORK_ASIO_SIGNAL_SET_HPP_
#define RING_NETWORK_ASIO_SIGNAL_SET_HPP_

#include "ring/network/signal_set.hpp"

#include <asio/executor.hpp>
#include <asio/signal_set.hpp>

namespace ring::network
{

class asio_signal_set final : public signal_set
{
public:
    explicit asio_signal_set(asio::any_io_executor asio_ex);
    ~asio_signal_set() = default;
public:
    void add(int32_t sig) override;
    void remove(int32_t sig) override;
    void async_wait(wait_handler handler) override;
    void cancel() override;
private:
    asio::signal_set signals_;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_SIGNAL_SET_HPP_