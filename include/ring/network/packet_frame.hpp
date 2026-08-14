#ifndef RING_NETWORK_PACKET_FRAME_HPP_
#define RING_NETWORK_PACKET_FRAME_HPP_

#include <concepts>
#include <cstdint>
#include <cstring>
#include <vector>

#include <stdexcept>

#include "ring/network/endian.hpp"
#include "ring/network/socket_buffer.hpp"

namespace ring::network
{
    
template <typename T>
concept packet_frame = requires(T t, const_buffer data)
{
    { t.append(data) }  -> std::same_as<size_t>;
    { t.complete() }    -> std::same_as<bool>;
    { t.message_id() }  -> std::same_as<uint32_t>;
    { t.body() }        -> std::same_as<const_buffer>;
    { t.reset() }       -> std::same_as<void>;
};

class length_prefix_frame
{
public:
    static constexpr size_t header_size = 4;
    static constexpr size_t max_body_size = 0x1000000;
public:
    length_prefix_frame() = default;
    ~length_prefix_frame() = default;
public:
    size_t append(const_buffer data)
    {
        if (complete_)
        {
            return 0;
        }
        if (data.size() < header_size)
        {
            return 0;
        }
        uint32_t net_len = 0;
        std::memcpy(&net_len, data.data(), header_size);
        auto body_len = ntohl(net_len);
        if (body_len < sizeof(uint32_t) || body_len > max_body_size)
        {
            return 0;
        }
        auto total = header_size + body_len;
        if (data.size() < total)
        {
            return 0;
        }
        uint32_t net_id = 0;
        std::memcpy(&net_id, data.data() + header_size, sizeof(uint32_t));
        msg_id_ = ntohl(net_id);
        body_view_ = const_buffer(data.data() + header_size + sizeof(uint32_t), body_len - sizeof(uint32_t));
        complete_ = true;
        return total;
    }
    bool complete() const
    {
        return complete_;
    }
    uint32_t message_id() const
    {
        return msg_id_;
    }
    const_buffer body() const
    {
        return body_view_;
    }
    void reset()
    {
        complete_ = false;
        msg_id_ = 0;
        body_view_ = {};
    }
private:
    bool complete_ = false;
    uint32_t msg_id_ = 0;
    const_buffer body_view_;
};

static_assert(packet_frame<length_prefix_frame>);

} // namespace ring::network

#endif // RING_NETWORK_PACKET_FRAME_HPP_