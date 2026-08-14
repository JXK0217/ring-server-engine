#ifndef RING_NETWORK_FRAMED_PACKETIZER_HPP_
#define RING_NETWORK_FRAMED_PACKETIZER_HPP_

#include "ring/network/socket_buffer.hpp"
#include "ring/network/packet_buffer.hpp"
#include "ring/network/packet_frame.hpp"
#include "ring/network/packetizer.hpp"

namespace ring::network
{

template <packet_frame Frame>
class framed_packetizer final : public packetizer
{
public:
    framed_packetizer() = default;
    ~framed_packetizer() = default;
public:
    bool append_data(const_buffer data) override
    {
        return buffer_.try_write(data);
    }
    std::vector<packet> extract_packets() override
    {
        std::vector<packet> packets;
        while (buffer_.readable_bytes() > 0)
        {
            auto readable_span = const_buffer(buffer_.read_begin(), buffer_.readable_bytes());
            size_t consumed = frame_.append(readable_span);
            if (consumed == 0)
            {
                break;
            }
            if (!frame_.complete())
            {
                break;
            }
            packet pkt;
            pkt.msg_id = frame_.message_id();
            auto body = frame_.body();
            pkt.payload.assign(body.begin(), body.end());
            packets.emplace_back(std::move(pkt));
            buffer_.read(consumed);
            frame_.reset();
        }
        return packets;
    }
    void reset() override
    {
        buffer_.reset();
        frame_.reset();
    }
    packet_buffer& buffer() { return buffer_; }
private:
    packet_buffer buffer_;
    Frame frame_;
};

} // namespace ring::network

#endif // RING_NETWORK_FRAMED_PACKETIZER_HPP_