#ifndef RING_NETWORK_SESSION_HPP_
#define RING_NETWORK_SESSION_HPP_

#include <deque>
#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/error_code.hpp"
#include "ring/network/executor.hpp"
#include "ring/network/packet_framer.hpp"
#include "ring/network/session_id_generator.hpp"
#include "ring/network/tcp_connection.hpp"
#include "ring/network/timer.hpp"

namespace ring::network
{

class RING_API session : public std::enable_shared_from_this<session>
{
private:
    enum class state
    {
        idle,
        running,
        stopped,
    };
public:
    using close_handler = std::function<void(std::shared_ptr<session>)>;
public:
    explicit session(io_context& ctx, std::unique_ptr<tcp_connection> conn) :
        ctx_(ctx), ex_(ctx_.create_strand()),
        conn_(std::move(conn)), id_(session_id_generator::instance().next_id()) {}
    ~session() = default;
public:
    virtual void on_start() {}
    virtual void on_message(payload) {}
    virtual void on_error(error_code) {}
    virtual void on_close() {}
public:
    void start()
    {
        ex_->post(
            [this, self = shared_from_this()]
            {
                do_start();
            });
    }
    void send(payload frame)
    {
        ex_->dispatch(
            [this, self = shared_from_this(), frame = std::move(frame)]
            {
                if (state_ != state::running)
                {
                    return;
                }

                write_queue_.emplace_back(framer_.pack({ frame.begin(), frame.end() }));
                if (!writing_)
                {
                    writing_ = true;
                    do_write();
                }
            });
    }
    void close()
    {
        ex_->dispatch(
            [this, self = shared_from_this()]
            {
                do_close();
            });
    }
    void set_close_handler(close_handler handler)
    {
        on_close_ = std::move(handler);
    }
public:
    uint64_t id() const
    {
        return id_;
    }
private:
    void do_start()
    {
        if (state_ != state::idle)
        {
            return;
        }
        
        state_ = state::running;
        on_start();
        do_read();
    }
    void do_read()
    {
        if (state_ != state::running)
        {
            return;
        }

        conn_->async_read_some(framer_.write_buffer(),
            [this, self = shared_from_this()](error_code ec, std::size_t n)
            {
                on_read(ec, n);
            });
    }
    void on_read(error_code ec, std::size_t n)
    {
        if (state_ != state::running)
        {
            return;
        }
        if (ec)
        {
            on_error(ec);
            do_close();
            return;
        }

        framer_.commit_write(n);
        auto frames = framer_.unpack();
        for (auto& frame : frames)
        {
            on_message(std::move(frame));
        }

        do_read();
    }
    void do_write()
    {
        if (write_queue_.empty())
        {
            return;
        }

        auto buf = std::move(write_queue_.front());
        write_queue_.pop_front();
        auto span = const_buffer(buf.data(), buf.size());
        conn_->async_write(span,
            [this, self = shared_from_this(), buf = std::move(buf)](error_code ec, std::size_t)
            {
                writing_ = false;
                if (state_ != state::running)
                {
                    return;
                }
                if (ec)
                {
                    on_error(ec);
                    do_close();
                    return;
                }

                if (!write_queue_.empty())
                {
                    writing_ = true;
                    do_write();
                }
            });
    }
    void do_close()
    {
        if (state_ != state::running)
        {
            return;
        }
        
        state_ = state::stopped;
        write_queue_.clear();
        writing_ = false;
        conn_->close();

        on_close();
        if (on_close_)
        {
            on_close_(shared_from_this());
        }
    }
private:
    io_context& ctx_;
    std::unique_ptr<executor> ex_;
    std::unique_ptr<tcp_connection> conn_;
    uint64_t id_;
    std::deque<payload> write_queue_;
    bool writing_ = false;
    packet_framer framer_;
    close_handler on_close_;
    state state_ = state::idle;
};

} // namespace ring::network

#endif // RING_NETWORK_SESSION_HPP_