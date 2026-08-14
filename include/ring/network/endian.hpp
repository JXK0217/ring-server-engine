#ifndef RING_NETWORK_ENDIAN_HPP_
#define RING_NETWORK_ENDIAN_HPP_

#include <bit>
#include <cstdint>

namespace ring::network
{

RING_API uint32_t ntohl(uint32_t net_long)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return std::byteswap(net_long);
    }
    else
    {
        return net_long;
    }
}

RING_API uint32_t htonl(uint32_t host_long)
{
    return ntohl(host_long); 
}

} // namespace ring::network

#endif // RING_NETWORK_ENDIAN_HPP_