#ifndef RING_NETWORK_MESSAGE_REGISTRY_HPP_
#define RING_NETWORK_MESSAGE_REGISTRY_HPP_

#include <functional>
#include <memory>
#include <unordered_map>

#include <google/protobuf/message_lite.h>

#include "ring/core/export.hpp"

namespace ring::network
{

class RING_API message_registry final
{
public:
    using message_factory = std::function<std::unique_ptr<google::protobuf::MessageLite>()>;
private:
    message_registry() = default;
    ~message_registry() = default;
private:
    message_registry(const message_registry&) = delete;
    message_registry& operator=(const message_registry&) = delete;
public:
    static message_registry& instance()
    {
        static message_registry instance;
        return instance;
    }
public:
    template <typename T>
    void register_message(uint32_t msg_id)
    {
        static_assert(std::is_base_of_v<google::protobuf::MessageLite, T>,
                      "T must be a Protobuf MessageLite");
        factories_[msg_id] = []() -> std::unique_ptr<google::protobuf::MessageLite>
        {
            return std::make_unique<T>();
        };
    }
    std::unique_ptr<google::protobuf::MessageLite> create(uint32_t msg_id) const
    {
        auto it = factories_.find(msg_id);
        if (it == factories_.end())
        {
            return nullptr;
        }
        return it->second();
    }
private:
    std::unordered_map<uint32_t, message_factory> factories_;
};

} // namespace ring::network

#endif // RING_NETWORK_MESSAGE_REGISTRY_HPP_