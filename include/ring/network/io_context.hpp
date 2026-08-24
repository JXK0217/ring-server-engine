#ifndef RING_NETWORK_IO_CONTEXT_HPP_
#define RING_NETWORK_IO_CONTEXT_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/endpoint.hpp"
#include "ring/network/executor.hpp"

namespace ring::network
{

class executor;
class tcp_connector;
class tcp_listener;
class timer;
class udp_socket;

class RING_API io_context
{
public:
    static std::unique_ptr<io_context> create();
public:
    io_context() = default;
    virtual ~io_context() = default;
public:
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual executor& get_executor() = 0;
public:
    virtual std::unique_ptr<executor> create_strand() = 0;
    virtual std::unique_ptr<tcp_connector> create_tcp_connector(executor &ex, const endpoint& ep) = 0;
    virtual std::unique_ptr<tcp_listener> create_tcp_listener(executor &ex, const endpoint& ep) = 0;
    virtual std::unique_ptr<timer> create_timer(executor &ex) = 0;
    virtual std::unique_ptr<udp_socket> create_udp_socket(executor &ex) = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_IO_CONTEXT_HPP_