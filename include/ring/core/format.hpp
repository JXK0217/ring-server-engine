#ifndef RING_CORE_FORMAT_HPP_
#define RING_CORE_FORMAT_HPP_

#include <format>
#include <string>
#include <iterator>
#include <concepts>

namespace ring::core
{

namespace detail
{

template <typename T>
concept has_member_to_string = requires(const T& t)
{
    { t.to_string() } -> std::convertible_to<std::string>;
};

} // namespace detail

} // namespace ring::core

template <ring::core::detail::has_member_to_string T>
struct std::formatter<T> : std::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const T& t, FormatContext& ctx) const
    {
        return formatter<std::string>::format(t.to_string(), ctx);
    }
};

template <ring::core::detail::has_member_to_string T>
std::ostream& operator<<(std::ostream& os, const T& t)
{
    return os << t.to_string();
}

#endif // RING_CORE_FORMAT_HPP_
