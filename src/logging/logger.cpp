#include "ring/logging/logger.hpp"

#include <memory>
#include <unordered_map>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#include "ring/core/exception.hpp"

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

struct log_service_config
{
    size_t queue_size = 8192;
};

class log_service::impl final
{
public:
    impl()
    {
        initialize();
    }
    ~impl()
    {
        shutdown();
    }
public:
    void initialize()
    {
        std::lock_guard lock(_mutex);

        spdlog::drop_all();
        spdlog::flush_on(spdlog::level::warn);
        spdlog::flush_every(std::chrono::seconds(3));

        auto logger = __create_logger({ .name = "", .pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v" });
        _default_logger = logger;
    }
    void shutdown()
    {
        std::lock_guard lock(_mutex);

        _loggers.clear();
        _default_logger.reset();
        spdlog::shutdown();
    }
    std::shared_ptr<logger> create_logger(const logger_config& config)
    {
        std::lock_guard lock(_mutex);

        auto it = _loggers.find(config.name);
        if(it != _loggers.end())
        {
            throw ring::core::exception("Logger with name '" + config.name + "' already exists.");
        }
        return __create_logger(config);
    }
    void set_default_logger(std::shared_ptr<logger> logger)
    {
        std::lock_guard lock(_mutex);

        _default_logger = logger;
    }
    std::shared_ptr<logger> get_default_logger()
    {
        std::lock_guard lock(_mutex);

        return _default_logger;
    }
    std::shared_ptr<logger> get_logger(std::string_view name)
    {
        std::lock_guard lock(_mutex);
        
        auto it = _loggers.find(name.data());
        if (it != _loggers.end())
        {
            return it->second;
        }
        auto logger = __create_logger({ .name = name.data() });
        return logger;
    }
    void flush_all()
    {
        std::lock_guard lock(_mutex);
        
        for (auto& [_, logger] : _loggers)
        {
            logger->flush();
        }
    }
private:
    std::shared_ptr<logger> __create_logger(const logger_config& config)
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
            if (!spdlog::thread_pool())
            {
                spdlog::init_thread_pool(_service_config.queue_size, 1);
            }

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

        auto logger = std::make_shared<ring::logging::logger>(spd_logger->name());
        _loggers[config.name] = logger;
        return logger;
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
    std::mutex _mutex;
    log_service_config _service_config;
    std::unordered_map<std::string, std::shared_ptr<logger>, string_hash, string_equal> _loggers;
    std::shared_ptr<logger> _default_logger;
};

log_service::log_service() :
    _impl(std::make_unique<impl>())
{}

log_service::~log_service()
{}

void log_service::initialize()
{
    _impl->initialize();
}

void log_service::shutdown()
{
    _impl->shutdown();
}

std::shared_ptr<logger> log_service::create_logger(const logger_config& config)
{
    return _impl->create_logger(config);
}

std::shared_ptr<logger> log_service::get_default_logger()
{
    return _impl->get_default_logger();
}

std::shared_ptr<logger> log_service::get_logger(std::string_view name)
{
    return _impl->get_logger(name);
}

void log_service::flush_all()
{
    _impl->flush_all();
}

class logger::impl final
{
public:
    impl(const std::string& name) :
        _name(name)
    {
        auto spd_logger = spdlog::get(_name);
        if (!spd_logger)
        {
            throw ring::core::exception("spdlog logger with name '" + _name + "' does not exist.");
        }
        _spd_logger = spd_logger;
    }
    ~impl()
    {
        if (_spd_logger)
        {
            spdlog::drop(_spd_logger->name());
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

logger::logger(const std::string &name) :
    _impl(std::make_unique<impl>(name))
{
}

logger::~logger()
{
}

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
