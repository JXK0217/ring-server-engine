#include "ring/logging/logger.h"

#include <memory>
#include <unordered_map>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

namespace ring::logging
{

spdlog::level::level_enum to_spdlog_level(log_level level)
{
    switch (level)
    {
    case log_level::trace:      return spdlog::level::trace;
    case log_level::debug:      return spdlog::level::debug;
    case log_level::info:       return spdlog::level::info;
    case log_level::warn:       return spdlog::level::warn;
    case log_level::error:      return spdlog::level::err;
    case log_level::critical:   return spdlog::level::critical;
    case log_level::off:        return spdlog::level::off;
    default:                    return spdlog::level::info;
    }
}

log_level from_spdlog_level(spdlog::level::level_enum level)
{
    switch (level)
    {
    case spdlog::level::trace:      return log_level::trace;
    case spdlog::level::debug:      return log_level::debug;
    case spdlog::level::info:       return log_level::info;
    case spdlog::level::warn:       return log_level::warn;
    case spdlog::level::err:        return log_level::error;
    case spdlog::level::critical:   return log_level::critical;
    case spdlog::level::off:        return log_level::off;
    default:                        return log_level::info;
    }
}

class log_service::impl final
{
public:
    impl() : _default_level(log_level::info) {}
    ~impl()
    {
        shutdown();
    }
public:
    void initialize(const logger_config& config)
    {
        std::lock_guard lock(_mutex);

        if (config.async)
        {
            spdlog::init_thread_pool(config.queue_size, 1);
        }
        
        __create_logger(config);
    }
    void shutdown()
    {
        std::lock_guard lock(_mutex);

        _loggers.clear();
        spdlog::shutdown();
    }
    std::shared_ptr<logger> get_logger(std::string_view name)
    {
        std::lock_guard lock(_mutex);
        
        auto it = _loggers.find(name.data());
        if (it != _loggers.end())
        {
            return it->second;
        }

        auto logger = std::make_shared<ring::logging::logger>(std::string(name));
        _loggers[name.data()] = logger;
        return logger;
    }
    void set_default_level(log_level level)
    {
        std::lock_guard lock(_mutex);
        
        _default_level = level;
    }
    void flush_all()
    {
        std::lock_guard lock(_mutex);
        
        for (auto& [name, logger] : _loggers)
        {
            logger->flush();
        }
    }
private:
    void __create_logger(const logger_config& config)
    {
        std::vector<spdlog::sink_ptr> sinks;
        
        if (config.console)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(config.pattern);
            sinks.push_back(console_sink);
        }
        
        if (!config.file.empty())
        {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config.file, config.max_file_size, config.max_files);
            file_sink->set_pattern(config.pattern);
            sinks.push_back(file_sink);
        }
        
        std::shared_ptr<spdlog::logger> spd_logger;
        
        if (config.async)
        {
            spd_logger = std::make_shared<spdlog::async_logger>(
                config.name, sinks.begin(), sinks.end(), 
                spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        }
        else
        {
            spd_logger = std::make_shared<spdlog::logger>(config.name, sinks.begin(), sinks.end());
        }
        
        spd_logger->set_level(to_spdlog_level(config.level));
        spdlog::register_logger(spd_logger);
        
        auto logger = std::make_shared<ring::logging::logger>(config.name);
        _loggers[config.name] = logger;
    }
private:
    struct string_hash
    {
        size_t operator()(std::string_view view) const
        {
            return std::hash<std::string_view>{}(view);
        }
    };
    struct string_equal
    {
        bool operator()(std::string_view l, std::string_view r) const
        {
            return l == r;
        }
    };
private:
    std::unordered_map<std::string, std::shared_ptr<logger>, string_hash, string_equal> _loggers;
    log_level _default_level;
    std::mutex _mutex;
};

log_service::log_service() :
    _impl(std::make_unique<impl>())
{}

log_service::~log_service()
{}

void log_service::initialize(const logger_config& config)
{
    _impl->initialize(config);
}

void log_service::shutdown()
{
    _impl->shutdown();
}

std::shared_ptr<logger> log_service::get_logger(std::string_view name)
{
    return _impl->get_logger(name);
}

void log_service::set_default_level(log_level level)
{
    _impl->set_default_level(level);
}

void log_service::flush_all()
{
    _impl->flush_all();
}

class logger::impl final
{
public:
    impl(const std::string& name) : _name(name)
    {
        _spd_logger = spdlog::get(_name);
        if (!_spd_logger)
        {
            _spd_logger = spdlog::stdout_color_mt(_name);
        }
    }
public:
    bool should_log(log_level level) const
    {
        return _spd_logger->should_log(to_spdlog_level(level));
    }
    void set_level(log_level level)
    {
        _spd_logger->set_level(to_spdlog_level(level));
    }
    log_level level() const
    {
        return from_spdlog_level(_spd_logger->level());
    }
    const std::string& name() const
    {
        return _name;
    }
    void flush()
    {
        _spd_logger->flush();
    }
public:
    void log(log_level level, const std::string& str)
    {
        _spd_logger->log(to_spdlog_level(level), str);
    }
private:
    std::string _name;
    std::shared_ptr<spdlog::logger> _spd_logger;
};

logger::logger(const std::string& name) :
    _impl(std::make_unique<impl>(name))
{}

logger::~logger()
{}

bool logger::should_log(log_level level) const
{
    return _impl->should_log(level);
}

void logger::set_level(log_level level)
{
    _impl->set_level(level);
}

log_level logger::level() const
{
    return _impl->level();
}

const std::string& logger::name() const
{
    return _impl->name();
}

void logger::flush()
{
    _impl->flush();
}

void logger::log(log_level level, const std::string& str)
{
    _impl->log(level, str);
}

} // namespace ring::logging
