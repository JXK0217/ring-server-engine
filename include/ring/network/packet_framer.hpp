#ifndef RING_NETWORK_PACKET_FRAMER_HPP_
#define RING_NETWORK_PACKET_FRAMER_HPP_

#include <cstdint>
#include <memory>

#include "ring/core/exception.hpp"
#include "ring/network/endian.hpp"
#include "ring/network/packet_buffer.hpp"

namespace ring::network
{

class packet_framer final
{
public:
    static constexpr size_t head_len = sizeof(uint32_t);
public:
    packet_framer() = default;
    ~packet_framer() = default;
public:
    payload pack(const_buffer frame)
    {
        if (frame.size() > std::numeric_limits<uint32_t>::max())
        {
            throw ring::core::exception("frame size exceeds maximum limit");
        }

        uint32_t body_len = static_cast<uint32_t>(frame.size());
        uint32_t net_body_len = htonl(body_len);

        payload frame_data(head_len + body_len);
        std::memcpy(frame_data.data(), &net_body_len, head_len);
        std::memcpy(frame_data.data() + head_len, frame.data(), body_len);
        return frame_data;
    }
    std::vector<payload> unpack()
    {
        std::vector<payload> frames;
        while (true)
        {
            if (buffer_.readable_bytes() < head_len)
            {
                break;
            }
            uint32_t body_len;
            std::memcpy(&body_len, buffer_.read_begin(), head_len);
            body_len = ntohl(body_len);
            if (buffer_.readable_bytes() < head_len + body_len)
            {
                break;
            }
            buffer_.commit_read(head_len);
            payload body(body_len);
            auto read_len = buffer_.try_read(mutable_buffer(body.data(), body_len));
            if (read_len < body_len)
            {
                throw ring::core::exception("failed to read message from buffer");
            }
            frames.emplace_back(std::move(body));
        }
        return frames;
    }
    mutable_buffer write_buffer()
    {
        return mutable_buffer(buffer_.write_begin(), buffer_.writable_bytes());
    }
    void commit_write(size_t len)
    {
        buffer_.commit_write(len);
    }
private:
    packet_buffer buffer_;
};

} // namespace ring::network

#endif // RING_NETWORK_PACKET_FRAMER_HPP_