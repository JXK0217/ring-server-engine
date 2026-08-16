#ifndef RING_NETWORK_TCP_CLIENT_HPP_
#define RING_NETWORK_TCP_CLIENT_HPP_

#include <chrono>

#include "ring/core/export.hpp"
#include "ring/network/io_context.hpp"
#include "ring/network/session.hpp"
#include "ring/network/tcp_connector.hpp"
#include "ring/network/timer.hpp"

namespace ring::network
{

class RING_API tcp_client final
{
public:
    using session_factory = std::function<std::shared_ptr<session>(
        std::unique_ptr<tcp_connection>)>;
    using connect_handler = std::function<void(const endpoint&)>;
public:
    explicit tcp_client(io_context& ctx, const endpoint& ep, session_factory factory) :
        ctx_(ctx), ep_(ep), factory_(std::move(factory)) {}
    ~tcp_client() = default;
public:
    void start()
    {
        running_ = true;
        connect();
    }
    void stop()
    {
        running_ = false;
        if (reconnect_timer_)
        {
            reconnect_timer_->cancel();
        }
        if (session_)
        {
            session_->close();
        }
    }
    void send(const_buffer payload)
    {
        if (session_)
        {
            session_->send(payload);
        }
    }
    void set_connect_handler(connect_handler handler)
    {
        on_connect_ = std::move(handler);
    }
private:
    void connect()
    {
        if (!connector_)
        {
            connector_ = ctx_.create_tcp_connector(ep_);
        }
        connector_->async_connect(
            [this](error_code ec, std::unique_ptr<tcp_connection> conn)
            {
                on_connect(ec, std::move(conn));
            });
        if (on_connect_)
        {
            on_connect_(ep_);
        }
    }
    void on_connect(error_code ec, std::unique_ptr<tcp_connection> conn)
    {
        if (!running_)
        {
            return;
        }
        if (ec)
        {
            reconnect();
            return;
        }
        
        session_ = factory_(std::move(conn));
        session_->set_close_handler(
            [this](auto)
            {
                session_.reset();
                if (running_)
                {
                    reconnect();
                }      
            });
        session_->start();
    }
    void reconnect()
    {
        if (!reconnect_timer_)
        {
            reconnect_timer_ = ctx_.create_timer();
        }
        reconnect_timer_->expires_after(reconnect_interval_);
        reconnect_timer_->async_wait(
            [this](error_code ec)
            {
                if (!running_)
                {
                    return;
                }
                if (ec)
                {
                    return;
                }
                connect();
            });
    }
private:
    io_context& ctx_;
    endpoint ep_;
    session_factory factory_;
    std::shared_ptr<session> session_;
    std::unique_ptr<tcp_connector> connector_;
    std::unique_ptr<timer> reconnect_timer_;
    std::chrono::steady_clock::duration reconnect_interval_{ std::chrono::seconds(5) };
    connect_handler on_connect_;
    bool running_ = false;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_CLIENT_HPP_