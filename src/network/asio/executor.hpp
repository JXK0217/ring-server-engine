#ifndef RING_NETWORK_ASIO_EXECUTOR_HPP_
#define RING_NETWORK_ASIO_EXECUTOR_HPP_

#include "ring/network/executor.hpp"

#include <asio/io_context.hpp>
#include <asio/any_io_executor.hpp>

namespace ring::network
{

class asio_executor final : public executor
{
public:
    explicit asio_executor(asio::any_io_executor asio_ex);
    ~asio_executor() = default;
public:
    void post(executor_task task) override;
    void dispatch(executor_task task) override;
public:
    asio::any_io_executor& raw_ex()
    {
        return ex_;
    }
private:
    asio::any_io_executor ex_;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_EXECUTOR_HPP_