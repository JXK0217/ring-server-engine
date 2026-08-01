#ifndef RING_NETWORK_ENDPOINT_HPP_
#define RING_NETWORK_ENDPOINT_HPP_

#include <cstdint>
#include <string>

#include "ring/core/export.hpp"

namespace ring::network
{

struct RING_API endpoint final
{
    std::string address;
    uint16_t    port = 0;    
};
    
} // namespace ring::network

#endif // RING_NETWORK_ENDPOINT_HPP_