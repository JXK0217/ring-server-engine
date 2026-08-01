#ifndef RING_NETWORK_TCP_CONNECTOR_HPP_
#define RING_NETWORK_TCP_CONNECTOR_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/error_code.hpp"

namespace ring::network
{

class tcp_connection;

class RING_API tcp_connector
{
public:
    using connect_handler = std::function<void(error_code, std::unique_ptr<tcp_connection>)>;
public:
    tcp_connector() = default;
    virtual ~tcp_connector() = default;
public:
    virtual void async_connect(connect_handler handler) = 0;
    virtual void cancel() = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_CONNECTOR_HPP_