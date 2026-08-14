#include <gtest/gtest.h>

#include <cstring>

#include "test_helpers.hpp"

#include "ring/network/io_context.hpp"
#include "ring/network/tcp_connection.hpp"
#include "ring/network/tcp_connector.hpp"
#include "ring/network/tcp_listener.hpp"
#include "ring/network/tcp_server.hpp"
#include "ring/network/timer.hpp"
#include "ring/network/udp_socket.hpp"

namespace ring::network
{

class NetworkTest : public test::TestBase
{
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
    }
protected:
    void SetUp() override
    {
        TestBase::SetUp();
    }
    void TearDown() override
    {
        TestBase::TearDown();
    }
};

TEST_F(NetworkTest, Listen)
{
    auto ctx = io_context::create();

    auto listener = ctx->create_tcp_listener({ "0.0.0.0", 8080 });
    listener->async_accept([&](error_code ec, std::unique_ptr<tcp_connection> conn)
    {
        if (!ec)
        {
            std::cout << "New connection from " << conn->remote_endpoint().address << std::endl;
        }
        ctx->stop();
    });

    ctx->run();
}

TEST_F(NetworkTest, Connect)
{
    auto ctx = io_context::create();

    auto connector = ctx->create_tcp_connector({ "127.0.0.1", 8080 });
    connector->async_connect([&](error_code ec, std::unique_ptr<tcp_connection> conn)
    {
        if (!ec)
        {
            std::cout << "New connection to " << conn->local_endpoint().address << std::endl;
        }
        ctx->stop();
    });

    ctx->run();
}

TEST_F(NetworkTest, Timer)
{
    auto ctx = io_context::create();

    using namespace std::chrono_literals;

    auto timer = ctx->create_timer();
    timer->expires_after(5s);
    timer->async_wait([&](error_code ec)
        {
            if (!ec)
            {
                std::cout << "Timer fired!" << std::endl;
            }
            ctx->stop();
        });

    ctx->run();
}

TEST_F(NetworkTest, Udp)
{
    auto ctx = io_context::create();

    auto server_socket = ctx->create_udp_socket();
    auto client_socket = ctx->create_udp_socket();

    auto server_ep = endpoint{ "127.0.0.1", 8080 };
    server_socket->bind(server_ep);
    auto client_ep = endpoint{ "127.0.0.1", 0 };
    client_socket->bind(client_ep);

    std::string msg = "hello udp";
    std::byte recv_buf[1024];
    server_socket->async_receive_from(recv_buf, 
        [&](error_code ec, size_t n, endpoint from)
        {
            if (!ec)
            {
                std::cout << "Server received " << n << " bytes from "
                          << from.address << ":" << from.port << ": "
                          << std::string(reinterpret_cast<const char*>(recv_buf), n)
                          << std::endl;
            }
            ctx->stop();
        });

    auto span = std::span{ reinterpret_cast<const std::byte*>(msg.data()), msg.size() };
    client_socket->async_send_to(span, server_ep,
        [](error_code ec, size_t n)
        {
            if (!ec)
            {
                std::cout << "Client sent " << n << " bytes." << std::endl;
            }
        });

    ctx->run();
}

auto make_session(io_context& ctx, std::unique_ptr<tcp_connection> conn)
{
    auto pktzr = std::make_unique<framed_packetizer<length_prefix_frame>>();
    auto codec = std::make_unique<protobuf_codec>();
    auto s = std::make_shared<session>(ctx, std::move(conn), std::move(pktzr), std::move(codec));
    s->set_message_handler(
        [](auto, uint32_t msg_id, auto)
        {
            std::cout << "Received msg id: " << msg_id << std::endl;
        });
    s->set_error_handler(
        [](auto, error_code ec)
        {
            std::cerr << "Session error: " << ec.message() << std::endl;
        });
    return s;
}

TEST_F(NetworkTest, TcpServer)
{
    auto ctx = io_context::create();

    tcp_server server(*ctx, {"0.0.0.0", 8080},
        [&](auto conn)
        {
            return make_session(*ctx, std::move(conn));
        });
    server.start();

    ctx->run();
}

} // namespace ring::network

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
