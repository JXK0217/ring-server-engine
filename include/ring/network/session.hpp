#ifndef RING_NETWORK_SESSION_HPP_
#define RING_NETWORK_SESSION_HPP_

#include <functional>
#include <memory>

#include "ring/network/error_code.hpp"
#include "ring/network/framed_packetizer.hpp"
#include "ring/network/protobuf_codec.hpp"
#include "ring/network/tcp_connection.hpp"
#include "ring/network/timer.hpp"

namespace ring::network
{

class session : public std::enable_shared_from_this<session>
{
public:
    using message_handler = std::function<void(std::shared_ptr<session>,
                                               uint32_t msg_id,
                                               std::unique_ptr<google::protobuf::MessageLite>)>;
    using error_handler = std::function<void(std::shared_ptr<session>, error_code)>;
public:
    session(io_context& ctx,
            std::unique_ptr<tcp_connection> conn,
            std::unique_ptr<packetizer> pktzr,
            std::unique_ptr<protobuf_codec> codec);
    ~session();
public:
    void start();
    void send(uint32_t msg_id, const google::protobuf::MessageLite& msg);
    void close();
public:
    void set_message_handler(message_handler handler);
    void set_error_handler(error_handler handler);
private:
    void do_read();
    void on_read(error_code ec, std::size_t n);
    void process_packets();
    void schedule_retry_read();
private:
    io_context& ctx_;
    std::unique_ptr<tcp_connection> conn_;
    std::unique_ptr<packetizer> pktzr_;
    std::unique_ptr<protobuf_codec> codec_;
    message_handler on_msg_;
    error_handler on_err_;
    std::array<std::byte, 4096> read_buf_;
};

inline session::session(io_context& ctx,
                        std::unique_ptr<tcp_connection> conn,
                        std::unique_ptr<packetizer> pktzr,
                        std::unique_ptr<protobuf_codec> codec)
    : ctx_(ctx)
    , conn_(std::move(conn))
    , pktzr_(std::move(pktzr))
    , codec_(std::move(codec))
{}

inline session::~session() = default;

inline void session::start()
{
    do_read();
}

inline void session::send(uint32_t msg_id, const google::protobuf::MessageLite& msg)
{
    auto pkt = codec_->encode(msg_id, msg);
    uint32_t body_len = 4 + static_cast<uint32_t>(pkt.payload.size());
    uint32_t net_body_len = htonl(body_len);
    uint32_t net_msg_id = htonl(msg_id);

    auto frame = std::make_shared<std::vector<std::byte>>(4 + body_len);
    std::memcpy(frame->data(), &net_body_len, 4);
    std::memcpy(frame->data() + 4, &net_msg_id, 4);
    if (!pkt.payload.empty())
    {
        std::memcpy(frame->data() + 8, pkt.payload.data(), pkt.payload.size());
    }
    auto span = std::span<const std::byte>(frame->data(), frame->size());
    conn_->async_write(span, [frame](error_code, std::size_t) {});
}

inline void session::close()
{
    conn_->close();
}

inline void session::set_message_handler(message_handler h) { on_msg_ = std::move(h); }
inline void session::set_error_handler(error_handler h) { on_err_ = std::move(h); }

inline void session::do_read()
{
    auto self = shared_from_this();
    auto buf = mutable_buffer(read_buf_.data(), read_buf_.size());
    conn_->async_read_some(buf,
        [self](error_code ec, std::size_t n)
        {
            self->on_read(ec, n);
        });
}

inline void session::on_read(error_code ec, std::size_t n)
{
    if (ec)
    {
        if (on_err_)
        {
            on_err_(shared_from_this(), ec);
        }
        return;
    }
    if (!pktzr_->append_data(std::span<const std::byte>(read_buf_.data(), n)))
    {
        schedule_retry_read();
        return;
    }
    process_packets();
    do_read();
}

inline void session::process_packets()
{
    auto packets = pktzr_->extract_packets();
    for (auto& pkt : packets)
    {
        auto result = codec_->decode(pkt);
        if (result && on_msg_)
        {
            auto [msg_id, msg] = std::move(*result);
            on_msg_(shared_from_this(), msg_id, std::move(msg));
        }
    }
}

inline void session::schedule_retry_read()
{
    auto timer = ctx_.create_timer();
    timer->expires_after(std::chrono::milliseconds(10));
    auto self = shared_from_this();
    timer->async_wait(
        [self](error_code ec)
        {
            if (!ec)
            {
                self->do_read();
            }
        });
}

} // namespace ring::network

#endif // RING_NETWORK_SESSION_HPP_