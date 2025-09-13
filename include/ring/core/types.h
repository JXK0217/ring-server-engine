#ifndef __RING_CORE_TYPES_H__
#define __RING_CORE_TYPES_H__

#include <cstdint>

namespace ring::types
{

using int8  = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8     = std::uint8_t;
using uint16    = std::uint16_t;
using uint32    = std::uint32_t;
using uint64    = std::uint64_t;

} // namespace ring::types

#if !defined(int8)
    using int8  = ring::types::int8;
#endif
#if !defined(int16)
    using int16 = ring::types::int16;
#endif
#if !defined(int32)
    using int32 = ring::types::int32;
#endif
#if !defined(int64)
    using int64 = ring::types::int64;
#endif

#if !defined(uint8)
    using uint8     = ring::types::uint8;
#endif
#if !defined(uint16)
    using uint16    = ring::types::uint16;
#endif
#if !defined(uint32)
    using uint32    = ring::types::uint32;
#endif
#if !defined(uint64)
    using uint64    = ring::types::uint64;
#endif

#endif // !__RING_CORE_TYPES_H__