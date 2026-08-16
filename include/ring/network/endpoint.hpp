#ifndef RING_NETWORK_ENDPOINT_HPP_
#define RING_NETWORK_ENDPOINT_HPP_

#include <cstdint>
#include <string>

#include "ring/core/export.hpp"
#include "ring/core/format.hpp"

namespace ring::network
{

struct RING_API endpoint final
{
    std::string address;
    uint16_t port = 0;

    std::string to_string() const
    {
        return std::format("{}:{}", address, port);
    }
};
    
} // namespace ring::network

#endif // RING_NETWORK_ENDPOINT_HPP_