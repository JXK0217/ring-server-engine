#ifndef RING_NETWORK_SESSION_ID_GENERATOR_HPP_
#define RING_NETWORK_SESSION_ID_GENERATOR_HPP_

#include <atomic>

namespace ring::network
{

class session_id_generator final
{
public:
    static session_id_generator& instance()
    {
        static session_id_generator instance;
        return instance;
    }
    uint64_t next_id()
    {
        return ++id_;
    }
private:
    std::atomic<uint64_t> id_{ 0 };
};

} // namespace ring::network

#endif // RING_NETWORK_SESSION_ID_GENERATOR_HPP_