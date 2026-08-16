#ifndef RING_NETWORK_TCP_SERVER_HPP_
#define RING_NETWORK_TCP_SERVER_HPP_

#include <functional>
#include <mutex>
#include <unordered_map>

#include "ring/network/session.hpp"

namespace ring::network
{

class tcp_server final
{
public:
    using session_factory = std::function<std::shared_ptr<session>(
        std::unique_ptr<tcp_connection>)>;
public:
    explicit tcp_server(io_context& ctx, const endpoint& ep, session_factory factory)
        : ctx_(ctx), listener_(ctx.create_tcp_listener(ep)), factory_(std::move(factory)) {}
    ~tcp_server() = default;
public:
    void start()
    {
        do_accept();
    }
    void stop()
    {
        listener_->close();
        std::lock_guard lock(mutex_);
        for (auto& [_, session] : sessions_)
        {
            session->close();
        }
        sessions_.clear();
    }
private:
    void do_accept()
    {
        listener_->async_accept([this](error_code ec, std::unique_ptr<tcp_connection> conn)
            {
                if (ec)
                {
                    return;
                }

                auto session = factory_(std::move(conn));
                if (session)
                {
                    session->set_close_handler([this](auto s)
                    {
                        std::lock_guard lock(mutex_);
                        sessions_.erase(s->id());
                    });

                    session->start();
                    std::lock_guard lock(mutex_);
                    sessions_.emplace(session->id(), session);
                }
                do_accept();
            });
    }
private:
    io_context& ctx_;
    std::unique_ptr<tcp_listener> listener_;
    session_factory factory_;
    std::unordered_map<uint64_t, std::shared_ptr<session>> sessions_;
    std::mutex mutex_;
};

} // namespace ring::network

#endif // RING_NETWORK_TCP_SERVER_HPP_