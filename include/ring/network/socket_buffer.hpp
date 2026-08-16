#ifndef RING_NETWORK_SOCKET_BUFFER_HPP_
#define RING_NETWORK_SOCKET_BUFFER_HPP_

#include <cstddef>
#include <span>

namespace ring::network
{

using const_buffer      = std::span<const std::byte>;
using mutable_buffer    = std::span<std::byte>;

using payload           = std::vector<std::byte>;

} // namespace ring::network

#endif // RING_NETWORK_SOCKET_BUFFER_HPP_