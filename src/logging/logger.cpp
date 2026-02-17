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
        std::lock_guard lock(mutex_);

        spdlog::drop_all();
        spdlog::flush_on(spdlog::level::warn);
        spdlog::flush_every(std::chrono::seconds(3));

        auto logger = create_logger_impl({ .name = "", .pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v" });
        default_logger_ = logger;
    }
    void shutdown()
    {
        std::lock_guard lock(mutex_);

        loggers_.clear();
        default_logger_.reset();
        spdlog::shutdown();
    }
    std::shared_ptr<logger> create_logger(const logger_config& config)
    {
        std::lock_guard lock(mutex_);

        auto it = loggers_.find(config.name);
        if (it != loggers_.end())
        {
            throw ring::core::exception("Logger with name '" + config.name + "' already exists.");
        }
        return create_logger_impl(config);
    }
    void set_default_logger(std::shared_ptr<logger> logger)
    {
        std::lock_guard lock(mutex_);

        default_logger_ = logger;
    }
    std::shared_ptr<logger> get_default_logger()
    {
        std::lock_guard lock(mutex_);

        return default_logger_;
    }
    std::shared_ptr<logger> get_logger(std::string_view name)
    {
        std::lock_guard lock(mutex_);
        
        auto it = loggers_.find(name.data());
        if (it != loggers_.end())
        {
            return it->second;
        }
        auto logger = create_logger_impl({ .name = name.data() });
        return logger;
    }
    void flush_all()
    {
        std::lock_guard lock(mutex_);
        
        for (auto& [_, logger] : loggers_)
        {
            logger->flush();
        }
    }
private:
    std::shared_ptr<logger> create_logger_impl(const logger_config& config)
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
                spdlog::init_thread_pool(service_config_.queue_size, 1);
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
        loggers_[config.name] = logger;
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
    std::mutex mutex_;
    log_service_config service_config_;
    std::unordered_map<std::string, std::shared_ptr<logger>, string_hash, string_equal> loggers_;
    std::shared_ptr<logger> default_logger_;
};

log_service::log_service() :
    impl_(std::make_unique<impl>()) {}

log_service::~log_service() {}

void log_service::initialize()
{
    impl_->initialize();
}

void log_service::shutdown()
{
    impl_->shutdown();
}

std::shared_ptr<logger> log_service::create_logger(const logger_config& config)
{
    return impl_->create_logger(config);
}

std::shared_ptr<logger> log_service::get_default_logger()
{
    return impl_->get_default_logger();
}

std::shared_ptr<logger> log_service::get_logger(std::string_view name)
{
    return impl_->get_logger(name);
}

void log_service::flush_all()
{
    impl_->flush_all();
}

class logger::impl final
{
public:
    impl(const std::string& name) :
        name_(name)
    {
        auto spd_logger = spdlog::get(name_);
        if (!spd_logger)
        {
            throw ring::core::exception("spdlog logger with name '" + name_ + "' does not exist.");
        }
        spd_logger_ = spd_logger;
    }
    ~impl()
    {
        if (spd_logger_)
        {
            spdlog::drop(spd_logger_->name());
        }
    }
public:
    bool should_log(log_level level) const
    {
        return spd_logger_->should_log(to_spdlog_level(level));
    }
    void set_level(log_level level)
    {
        spd_logger_->set_level(to_spdlog_level(level));
    }
    log_level level() const
    {
        return from_spdlog_level(spd_logger_->level());
    }
    const std::string& name() const
    {
        return name_;
    }
    void flush()
    {
        spd_logger_->flush();
    }
public:
    void log(log_level level, const std::string& str)
    {
        spd_logger_->log(to_spdlog_level(level), str);
    }
private:
    std::string name_;
    std::shared_ptr<spdlog::logger> spd_logger_;
};

logger::logger(const std::string &name) :
    impl_(std::make_unique<impl>(name)) {}

logger::~logger() {}

bool logger::should_log(log_level level) const
{
    return impl_->should_log(level);
}

void logger::set_level(log_level level)
{
    impl_->set_level(level);
}

log_level logger::level() const
{
    return impl_->level();
}

const std::string& logger::name() const
{
    return impl_->name();
}

void logger::flush()
{
    impl_->flush();
}

void logger::log(log_level level, const std::string& str)
{
    impl_->log(level, str);
}

} // namespace ring::logging
