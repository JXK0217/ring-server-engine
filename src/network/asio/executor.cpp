#include "executor.hpp"

#include <asio/dispatch.hpp>
#include <asio/post.hpp>

namespace ring::network
{

asio_executor::asio_executor(asio::any_io_executor asio_ex) :
    ex_(std::move(asio_ex)) {}

void asio_executor::post(executor_task task)
{
    asio::post(ex_, std::move(task));
}

void asio_executor::dispatch(executor_task task)
{
    asio::dispatch(ex_, std::move(task));
}

} // namespace ring::network
