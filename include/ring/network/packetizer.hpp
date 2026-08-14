#ifndef RING_NETWORK_PACKETIZER_HPP_
#define RING_NETWORK_PACKETIZER_HPP_

#include <cstdint>
#include <vector>

#include "ring/network/socket_buffer.hpp"

namespace ring::network
{

struct packet
{
    uint32_t msg_id = 0;
    std::vector<std::byte> payload;
};

class packetizer
{
public:
    packetizer() = default;
    virtual ~packetizer() = default;
public:
    virtual bool append_data(const_buffer data) = 0;
    virtual std::vector<packet> extract_packets() = 0;
    virtual void reset() = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_PACKETIZER_HPP_