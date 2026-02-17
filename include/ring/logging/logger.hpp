#ifndef RING_LOGGING_LOGGER_HPP_
#define RING_LOGGING_LOGGER_HPP_

#include <format>
#include <memory>
#include <string>

#include "ring/core/export.hpp"

namespace ring::logging
{

enum class log_level
{
    trace,
    debug,
    info,
    warn,
    error,
    critical,
    off
};

struct logger_config
{
    std::string name = "default";
    log_level level = log_level::info;
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] [%n] %v";
    size_t max_file_size = 100ul * 1024 * 1024;
    size_t max_files = 1024;
    bool console = true;
    std::string file = "";
    bool async = true;
};

class logger;

class RING_API log_service final
{
private:
    log_service();
    ~log_service();
private:
    log_service(const log_service&) = delete;
    log_service& operator=(const log_service&) = delete;
public:
    static log_service& instance()
    {
        static log_service instance;
        return instance;
    }
public:
    void initialize();
    void shutdown();
    std::shared_ptr<logger> create_logger(const logger_config& config);
    std::shared_ptr<logger> get_default_logger();
    std::shared_ptr<logger> get_logger(std::string_view name);
    void flush_all();
public:
    class impl;
    std::unique_ptr<impl> impl_;
};

class RING_API logger final
{
public:
    logger(const std::string& name);
    ~logger();
private:
    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;
public:
    bool should_log(log_level level) const;
    void set_level(log_level level);
    log_level level() const;
    const std::string& name() const;
    void flush();
public:
    void log(log_level level, const std::string& message);
    template <typename... Args>
    void log(log_level level, std::format_string<Args...> fmt, Args&&... args)
    {
        if (should_log(level))
        {
            log(level, std::format(fmt, std::forward<Args>(args)...));
        }
    }
    template <typename... Args>
    void trace(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::trace, std::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::debug, std::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::info, std::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::warn, std::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::error, std::format(fmt, std::forward<Args>(args)...));
    }
    template <typename... Args>
    void critical(std::format_string<Args...> fmt, Args&&... args)
    {
        log(log_level::critical, std::format(fmt, std::forward<Args>(args)...));
    }
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

template <typename... Args>
void trace(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->trace(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->debug(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->info(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void warn(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->warn(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->error(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void critical(std::format_string<Args...> fmt, Args&&... args)
{
    log_service::instance().get_default_logger()->critical(fmt, std::forward<Args>(args)...);
}

} // namespace ring::logging

#define RING_TRACE(...)     ring::logging::trace(__VA_ARGS__)
#define RING_DEBUG(...)     ring::logging::debug(__VA_ARGS__)
#define RING_INFO(...)      ring::logging::info(__VA_ARGS__)
#define RING_WARN(...)      ring::logging::warn(__VA_ARGS__)
#define RING_ERROR(...)     ring::logging::error(__VA_ARGS__)
#define RING_CRITICAL(...)  ring::logging::critical(__VA_ARGS__)

#endif // !RING_LOGGING_LOGGER_HPP_