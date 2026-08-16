#ifndef RING_NETWORK_SESSION_HPP_
#define RING_NETWORK_SESSION_HPP_

#include <functional>
#include <memory>

#include "ring/core/export.hpp"
#include "ring/network/error_code.hpp"
#include "ring/network/packet_framer.hpp"
#include "ring/network/session_id_generator.hpp"
#include "ring/network/tcp_connection.hpp"
#include "ring/network/timer.hpp"

namespace ring::network
{

class RING_API session : public std::enable_shared_from_this<session>
{
public:
    using close_handler = std::function<void(std::shared_ptr<session>)>;
public:
    explicit session(io_context& ctx, std::unique_ptr<tcp_connection> conn) :
        ctx_(ctx), conn_(std::move(conn)), id_(session_id_generator::instance().next_id()) {}
    virtual ~session() = default;
public:
    virtual void on_start() {}
    virtual void on_message(payload) {}
    virtual void on_error(error_code) {}
    virtual void on_close() {}
public:
    void start()
    {
        on_start();

        do_read();
    }
    void send(const_buffer frame)
    {
        // TODO use a buffer pool to avoid frequent allocations
        auto frame_data = std::make_shared<std::vector<std::byte>>(framer_.pack(frame));
        auto span = const_buffer(frame_data->data(), frame_data->size());
        conn_->async_write(span, [frame_data](error_code, std::size_t) {});
    }
    void close()
    {
        conn_->close();
        on_close();
        if (close_handler_)
        {
            close_handler_(shared_from_this());
        }
    }
    void set_close_handler(close_handler handler)
    {
        close_handler_ = std::move(handler);
    }
public:
    uint64_t id() const
    {
        return id_;
    }
private:
    void do_read()
    {
        conn_->async_read_some(framer_.write_buffer(),
            [self = shared_from_this()](error_code ec, std::size_t n)
            {
                self->on_read(ec, n);
            });
    }
    void on_read(error_code ec, std::size_t n)
    {
        if (ec)
        {
            on_error(ec);
            close();
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
private:
    io_context& ctx_;
    std::unique_ptr<tcp_connection> conn_;
    uint64_t id_;
    packet_framer framer_;
    close_handler close_handler_;
};

} // namespace ring::network

#endif // RING_NETWORK_SESSION_HPP_