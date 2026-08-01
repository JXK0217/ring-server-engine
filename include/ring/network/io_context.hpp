#ifndef RING_NETWORK_IO_CONTEXT_HPP_
#define RING_NETWORK_IO_CONTEXT_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/endpoint.hpp"

namespace ring::network
{

class tcp_listener;
class tcp_connector;

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
    virtual void post(std::function<void()> task) = 0;
public:
    virtual std::unique_ptr<tcp_listener> create_tcp_listener(const endpoint& ep) = 0;
    virtual std::unique_ptr<tcp_connector> create_tcp_connector(const endpoint& ep) = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_IO_CONTEXT_HPP_