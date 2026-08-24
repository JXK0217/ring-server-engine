#ifndef RING_NETWORK_TCP_CLIENT_HPP_
#define RING_NETWORK_TCP_CLIENT_HPP_

#include <chrono>

#include "ring/core/exception.hpp"
#include "ring/core/export.hpp"
#include "ring/network/io_context.hpp"
#include "ring/network/session.hpp"
#include "ring/network/tcp_connector.hpp"
#include "ring/network/timer.hpp"

namespace ring::network
{

class RING_API tcp_client final : public std::enable_shared_from_this<tcp_client>
{
private:
    enum class state
    {
        idle,
        running,
        stopped,
    };
public:
    using session_factory       = std::function<std::shared_ptr<session>(std::unique_ptr<tcp_connection>)>;
    using connecting_handler    = std::function<void(const endpoint&)>;
public:
    static std::shared_ptr<tcp_client> create(io_context& ctx,
                                              const endpoint& ep,
                                              session_factory factory)
    {
        return std::shared_ptr<tcp_client>(new tcp_client(ctx, ep, std::move(factory)));
    }
private:
    explicit tcp_client(io_context& ctx, const endpoint& ep, session_factory factory) :
        ctx_(ctx), ex_(ctx_.create_strand()), ep_(ep), factory_(std::move(factory)),
        connector_(ctx_.create_tcp_connector(*ex_, ep_)), reconnect_timer_(ctx_.create_timer(*ex_)) {}
public:
    ~tcp_client() = default;
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
    void send(payload frame)
    {
        ex_->dispatch(
            [this, self = shared_from_this(), frame = std::move(frame)] mutable
            {
                do_send(std::move(frame));
            });
    }
    void set_connecting_handler(connecting_handler handler)
    {
        on_connecting_ = std::move(handler);
    }
private:
    void do_start()
    {
        if (state_ != state::idle)
        {
            throw ring::core::exception("can not start tcp_client again");
        }
        
        state_ = state::running;
        connect();
    }
    void do_stop()
    {
        if (state_ != state::running)
        {
            return;
        }
        
        state_ = state::stopped;
        if (session_)
        {
            session_->close();
            session_.reset();
        }
        connector_->close();
        reconnect_timer_->cancel();
    }
    void do_send(payload frame)
    {
        if (state_ != state::running)
        {
            return;
        }
        
        if (session_)
        {
            session_->send(std::move(frame));
        }
    }
    void connect()
    {
        connector_->async_connect(
            [this, self = shared_from_this()](error_code ec, std::unique_ptr<tcp_connection> conn)
            {
                on_connect(ec, std::move(conn));
            });
        if (on_connecting_)
        {
            on_connecting_(ep_);
        }
    }
    void on_connect(error_code ec, std::unique_ptr<tcp_connection> conn)
    {
        if (state_ != state::running)
        {
            return;
        }
        if (ec)
        {
            reconnect();
            return;
        }
        
        session_ = factory_(std::move(conn));
        if (!session_)
        {
            reconnect();
            return;
        }

        session_->set_close_handler(
            [weak = weak_from_this()](auto)
            {
                auto self = weak.lock();
                if (!self)
                {
                    return;
                }

                self->ex_->dispatch(
                    [self = std::move(self)]
                    {
                        self->session_.reset();
                        if (self->state_ == state::running)
                        {
                            self->reconnect();
                        }
                    });
            });
        session_->start();
    }
    void reconnect()
    {
        reconnect_timer_->expires_after(reconnect_interval_);
        reconnect_timer_->async_wait(
            [this, self = shared_from_this()](error_code ec)
            {
                on_reconnect(ec);
            });
    }
    void on_reconnect(error_code ec)
    {
        if (state_ != state::running)
        {
            return;
        }
        if (ec)
        {
            return;
        }
        connect();
    }
private:
    io_context& ctx_;
    std::unique_ptr<executor> ex_;
    endpoint ep_;
    session_factory factory_;
    std::shared_ptr<session> session_;
    std::unique_ptr<tcp_connector> connector_;
    std::unique_ptr<timer> reconnect_timer_;
    std::chrono::steady_clock::duration reconnect_interval_{ std::chrono::seconds(3) };
    connecting_handler on_connecting_;
    state state_ = state::idle;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_CLIENT_HPP_