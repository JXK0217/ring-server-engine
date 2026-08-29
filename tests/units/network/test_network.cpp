#include <gtest/gtest.h>

#include <cstring>

#include "test_helpers.hpp"

#include "ring/network/io_context.hpp"
#include "ring/network/signal_monitor.hpp"
#include "ring/network/signal_set.hpp"
#include "ring/network/tcp_client.hpp"
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

TEST_F(NetworkTest, Signal)
{
    auto ctx = io_context::create();

    auto signal_set = ctx->create_signal_set(ctx->get_executor());
    signal_set->add(SIGHUP);
    signal_set->add(SIGINT);
    signal_set->add(SIGTERM);

    signal_set::wait_handler func_signal; 
    func_signal = 
        [&](error_code ec, int32_t sig)
        {
            std::cout << "signal caught: " << sig << ", ec: " << ec << std::endl;
        
            signal_set->async_wait(func_signal);
            if (sig == SIGINT)
            {
                ctx->stop();
                return;
            }
        };

    signal_set->async_wait(func_signal);
    
    ctx->run();
}

TEST_F(NetworkTest, Listen)
{
    auto ctx = io_context::create();

    auto listener = ctx->create_tcp_listener(ctx->get_executor(), { "0.0.0.0", 8080 });
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

    auto connector = ctx->create_tcp_connector(ctx->get_executor(), { "127.0.0.1", 8080 });
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

    auto timer = ctx->create_timer(ctx->get_executor());
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

    auto server_socket = ctx->create_udp_socket(ctx->get_executor());
    auto client_socket = ctx->create_udp_socket(ctx->get_executor());

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

TEST_F(NetworkTest, TcpClient)
{
    auto ctx = io_context::create();

    int client_num = 20000;
    std::vector<std::shared_ptr<tcp_client>> clients(client_num);
    std::vector<uint32_t> counts(client_num);
    std::atomic<int> total_count{ 0 };
    for (auto i = 0; i < client_num; i++)
    {
        auto& client = clients[i];
        client = tcp_client::create(*ctx, endpoint{ "127.0.0.1", 8080 },
            [&](auto conn)
            {
                class test_session final : public session
                {
                public:
                    test_session(io_context& ctx, std::unique_ptr<tcp_connection> conn)
                        : session(ctx, std::move(conn)) {}
                    ~test_session() = default;
                public:
                    void on_start() override
                    {
                        std::cout << std::format("Session[{}] Started", id()) << std::endl;
                        std::string text = "hello world";
                        payload frame(text.size());
                        std::memcpy(frame.data(), text.data(), text.size());
                        send(std::move(frame));
                    }
                    void on_message(payload payload) override
                    {
                        std::string_view sv(reinterpret_cast<const char*>(payload.data()), payload.size());
                        std::cout << std::format("Session[{}] Received {} bytes: {}",
                                    id(), payload.size(), sv)
                                    << std::endl;
                        // send(std::move(payload));
                        close();
                    }
                    void on_error(error_code ec) override
                    {
                        std::cerr << std::format("Session[{}] Error: {}", id(), ec.message()) << std::endl;
                    }
                    void on_close() override
                    {
                        std::cout << std::format("Session[{}] Closed", id()) << std::endl;
                    }
                };
                return std::make_shared<test_session>(*ctx, std::move(conn));
            });
        auto& count = counts[i];
        client->set_connecting_handler(
            [&](const auto &ep)
            {
                std::cout << std::format("Connecting {}", ep) << std::endl;
                if (++count > 1)
                {
                    client->stop();
                    ++total_count;
                    if (total_count >= client_num)
                    {
                        ctx->stop();
                    }
                }
            });
        client->start();
    }

    int thread_count = std::thread::hardware_concurrency() / 4;
    std::vector<std::thread> threads;
    for (auto i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(
            [&]
            {
                ctx->run();
            });
    }
    for (auto& t : threads)
    {
        t.join();
    }
    // ctx->run();
}

TEST_F(NetworkTest, TcpServer)
{
    auto ctx = io_context::create();

    auto server = tcp_server::create(*ctx, { "0.0.0.0", 8080 },
        [&](auto conn)
        {
            class test_session final : public session
            {
            public:
                test_session(io_context& ctx, std::unique_ptr<tcp_connection> conn)
                    : session(ctx, std::move(conn)) {}
                ~test_session() = default;
            public:
                void on_start() override
                {
                    std::cout << std::format("Session[{}] Started", id()) << std::endl;
                }
                void on_message(payload payload) override
                {
                    std::string_view sv(reinterpret_cast<const char*>(payload.data()), payload.size());
                    std::cout << std::format("Session[{}] Received {} bytes: {}", 
                                 id(), payload.size(), sv)
                                 << std::endl;
                    send(std::move(payload));
                    // close();
                }
                void on_error(error_code ec) override
                {
                    std::cerr << std::format("Session[{}] Error: {}", id(), ec.message()) << std::endl;
                }
                void on_close() override
                {
                    std::cout << std::format("Session[{}] Closed", id()) << std::endl;
                }
            };
            return std::make_shared<test_session>(*ctx, std::move(conn));
        });
    server->set_stop_handler([&]
    {
        ctx->stop();
    });
    server->start();

    int thread_count = std::thread::hardware_concurrency() / 4;
    std::vector<std::thread> threads;
    for (auto i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(
            [&]
            {
                ctx->run();
            });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

TEST_F(NetworkTest, SignalMonitor)
{
    auto ctx = io_context::create();

    auto monitor = signal_monitor::create(*ctx);
    monitor->subscribe(SIGHUP,
        [](int32_t sig)
        {
            std::cout << "signal caught: " << sig << std::endl;
        });
    monitor->subscribe(SIGHUP,
        [](int32_t sig)
        {
            std::cout << "signal caught: " << sig << ", notice twice" << std::endl;
        });
    monitor->subscribe(SIGINT,
        [&](int32_t sig)
        {
            std::cout << "signal caught: " << sig << std::endl;
            ctx->stop();
        });
    monitor->start();
    
    ctx->run();
}

} // namespace ring::network

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
