#ifndef __RING_CORE_EXCEPTION_HPP__
#define __RING_CORE_EXCEPTION_HPP__

#include <exception>
#include <format>
#include <source_location>
#include <string>
#include <string_view>

#include "ring/core/type_traits.hpp"

namespace ring::core
{

class exception: public std::exception
{
public:
    explicit exception(std::string_view message, std::source_location location = std::source_location::current())
        : _message(message)
        , _location(location) {}
public:
    const char* what() const noexcept override
    {
        return _message.c_str();
    }
    const auto& location() const noexcept
    {
        return _location;
    }
public:
    virtual std::string_view type() const noexcept
    {
        return "exception";
    }
    virtual std::string detail() const
    {
        return std::format("{}:{} [{}]: {}", _location.file_name(), _location.line(), type(), _message);
    }
private:
    std::string _message;
    std::source_location _location;
};

} // namespace ring::core

#endif // !__RING_CORE_EXCEPTION_HPP__