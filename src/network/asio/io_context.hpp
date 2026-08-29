#ifndef RING_NETWORK_ASIO_IO_CONTEXT_HPP_
#define RING_NETWORK_ASIO_IO_CONTEXT_HPP_

#include "ring/network/io_context.hpp"

#include <asio/io_context.hpp>

namespace ring::network
{

class asio_io_context final : public io_context
{
public:
    asio_io_context();
    ~asio_io_context() = default;
public:
    void run() override;
    void stop() override;
    executor& get_executor() override;
public:
    std::unique_ptr<executor> create_strand() override;
    std::unique_ptr<signal_set> create_signal_set(executor& ex) override;
    std::unique_ptr<tcp_connector> create_tcp_connector(executor& ex, const endpoint& ep) override;
    std::unique_ptr<tcp_listener> create_tcp_listener(executor& ex, const endpoint& ep) override;
    std::unique_ptr<timer> create_timer(executor& ex) override;
    std::unique_ptr<udp_socket> create_udp_socket(executor& ex) override;
private:
    asio::io_context ctx_;
    asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
    std::unique_ptr<executor> ex_;
};

} // namespace ring::network

#endif // RING_NETWORK_ASIO_IO_CONTEXT_HPP_