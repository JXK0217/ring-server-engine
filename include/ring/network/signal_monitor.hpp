#ifndef RING_NETWORK_SIGNAL_MONITOR_HPP_
#define RING_NETWORK_SIGNAL_MONITOR_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/executor.hpp"
#include "ring/network/io_context.hpp"
#include "ring/network/signal_set.hpp"

namespace ring::network
{

class RING_API signal_monitor final : public std::enable_shared_from_this<signal_monitor>
{
public:
    enum class state
    {
        idle,
        running,
        stopped,
    };
public:
    using signal_handler    = std::function<void(int32_t)>;
    using error_handler     = std::function<void(error_code)>;
public:
    static std::shared_ptr<signal_monitor> create(io_context& ctx)
    {
        return std::shared_ptr<signal_monitor>(new signal_monitor(ctx));
    }
private:
    explicit signal_monitor(io_context& ctx) :
        ctx_(ctx), ex_(ctx_.create_strand()), signals_(ctx_.create_signal_set(*ex_)) {}
public:
    ~signal_monitor() = default;
public:
    void start()
    {
        ex_->post(
            [this, self = shared_from_this()]
            {
                do_start();
            });
    }
    void stop()
    {
        ex_->dispatch(
            [this, self = shared_from_this()]
            {
                do_stop();
            });
    }
    void subscribe(int32_t sig, signal_handler handler)
    {
        ex_->dispatch(
            [this, self = shared_from_this(), sig, handler = std::move(handler)] mutable
            {
                if (state_ == state::stopped)
                {
                    return;
                }
                
                handlers_[sig].emplace_back(std::move(handler));
                signals_->add(sig);
            });
    }
    void unsubscribe(int32_t sig)
    {
        ex_->dispatch(
            [this, self = shared_from_this(), sig]
            {
                if (state_ == state::stopped)
                {
                    return;
                }

                handlers_.erase(sig);
                signals_->remove(sig);
            });
    }
    void set_error_handler(error_handler handler)
    {
        on_error_ = std::move(handler);
    }
private:
    void do_start()
    {
        if (state_ != state::idle)
        {
            return;
        }
        
        state_ = state::running;
        do_wait();
    }
    void do_wait()
    {
        signals_->async_wait(
            [this, self = shared_from_this()](error_code ec, int32_t sig)
            {
                if (state_ != state::running)
                {
                    return;
                }
                if (ec)
                {
                    if (on_error_)
                    {
                        on_error_(ec);
                    }
                    return;
                }

                do_wait();
                if(auto it = handlers_.find(sig); it != handlers_.end())
                {
                    for (auto& handler : it->second)
                    {
                        handler(it->first);
                    }
                }
            });
    }
    void do_stop()
    {
        if (state_ != state::running)
        {
            return;
        }
        
        state_ = state::stopped;
        signals_->cancel();

        handlers_.clear();
    }
private:
    io_context& ctx_;
    std::unique_ptr<executor> ex_;
    std::unique_ptr<signal_set> signals_;
    std::unordered_map<int32_t, std::vector<signal_handler>> handlers_;
    error_handler on_error_;
    state state_ = state::idle;
};

} // namespace ring::network

#endif // RING_NETWORK_SIGNAL_MONITOR_HPP_