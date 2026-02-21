#ifndef RING_CORE_EXCEPTION_HPP_
#define RING_CORE_EXCEPTION_HPP_

#include <exception>
#include <format>
#include <source_location>
#include <string>
#include <string_view>

namespace ring::core
{

class exception: public std::exception
{
public:
    explicit exception(std::string_view message, std::source_location location = std::source_location::current())
        : message_(message)
        , location_(location) {}
public:
    const char* what() const noexcept override
    {
        return message_.c_str();
    }
    const auto& location() const noexcept
    {
        return location_;
    }
public:
    virtual std::string_view type() const noexcept
    {
        return "exception";
    }
    virtual std::string detail() const
    {
        return std::format("{}:{} [{}]: {}", location_.file_name(), location_.line(), type(), message_);
    }
private:
    std::string message_;
    std::source_location location_;
};

} // namespace ring::core

#endif // RING_CORE_EXCEPTION_HPP_