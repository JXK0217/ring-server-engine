#ifndef RING_NETWORK_TCP_SERVER_HPP_
#define RING_NETWORK_TCP_SERVER_HPP_

#include <functional>
#include <unordered_map>

#include "ring/core/exception.hpp"
#include "ring/core/export.hpp"
#include "ring/network/session.hpp"
#include "ring/network/tcp_listener.hpp"

namespace ring::network
{

class RING_API tcp_server final : public std::enable_shared_from_this<tcp_server>
{
private:
    enum class state
    {
        idle,
        running,
        stopped,
    };
public:
    using session_factory   = std::function<std::shared_ptr<session>(std::unique_ptr<tcp_connection>)>;
    using stop_handler      = std::function<void()>;
    using error_handler     = std::function<void(error_code)>;
public:
    static std::shared_ptr<tcp_server> create(io_context& ctx,
                                              const endpoint& ep,
                                              session_factory factory)
    {
        return std::shared_ptr<tcp_server>(new tcp_server(ctx, ep, std::move(factory)));
    }
private:
    explicit tcp_server(io_context& ctx, const endpoint& ep, session_factory factory)
        : ctx_(ctx), ex_(ctx_.create_strand()),
        listener_(ctx.create_tcp_listener(*ex_, ep)), factory_(std::move(factory)) {}
public:
    ~tcp_server() = default;
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
    void set_stop_handler(stop_handler handler)
    {
        on_stop_ = std::move(handler);
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
            throw ring::core::exception("can not start tcp_server again");
        }
        
        state_ = state::running;
        do_accept();
    }
    void do_stop()
    {
        if (state_ != state::running)
        {
            return;
        }
        
        state_ = state::stopped;
        listener_->close();

        for (auto& [_, session] : sessions_)
        {
            session->close();
        }
        sessions_.clear();

        if (on_stop_)
        {
            on_stop_();
        }
    }
    void do_accept()
    {
        listener_->async_accept(
            [this, self = shared_from_this()](error_code ec, std::unique_ptr<tcp_connection> conn)
            {
                on_accept(ec, std::move(conn));
            });
    }
    void on_accept(error_code ec, std::unique_ptr<tcp_connection> conn)
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
            do_stop();
            return;
        }

        auto session = factory_(std::move(conn));
        if (session)
        {
            session->set_close_handler(
                [weak = weak_from_this()](auto s)
                {
                    auto self = weak.lock();
                    if (!self)
                    {
                        return;
                    }
                    self->ex_->dispatch(
                        [self = std::move(self), id = s->id()]
                        {
                            self->sessions_.erase(id);
                        });
                });

            session->start();
            sessions_.emplace(session->id(), session);
        }
    }
private:
    io_context& ctx_;
    std::unique_ptr<executor> ex_;
    std::unique_ptr<tcp_listener> listener_;
    session_factory factory_;
    std::unordered_map<uint64_t, std::shared_ptr<session>> sessions_;
    stop_handler on_stop_;
    error_handler on_error_;
    state state_ = state::idle;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_SERVER_HPP_