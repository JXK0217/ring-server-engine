#ifndef RING_NETWORK_PROTOBUF_CODEC_HPP_
#define RING_NETWORK_PROTOBUF_CODEC_HPP_

#include <string>

#include <google/protobuf/message_lite.h>

#include "ring/network/message_registry.hpp"
#include "ring/network/packetizer.hpp"

namespace ring::network
{

class protobuf_codec final
{
public:
    packet encode(uint32_t msg_id, const google::protobuf::MessageLite& msg)
    {
        packet pkt;
        pkt.msg_id = msg_id;
        std::string payload;
        if (!msg.SerializeToString(&payload))
        {
            return pkt;
        }
        pkt.payload.assign(reinterpret_cast<const std::byte*>(payload.data()),
                           reinterpret_cast<const std::byte*>(payload.data() + payload.size()));
        return pkt;
    }
    std::optional<std::pair<uint32_t, std::unique_ptr<google::protobuf::MessageLite>>>
        decode(const packet& pkt)
    {
        auto msg = message_registry::instance().create(pkt.msg_id);
        if (!msg)
        {
            return std::nullopt;
        }
        std::string payload(reinterpret_cast<const char*>(pkt.payload.data()), pkt.payload.size());
        if (!msg->ParseFromString(payload))
        {
            return std::nullopt;
        }
        return std::make_pair(pkt.msg_id, std::move(msg));
    }
};

} // namespace ring::network

#endif // RING_NETWORK_PROTOBUF_CODEC_HPP_