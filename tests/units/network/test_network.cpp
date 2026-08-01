#include <gtest/gtest.h>

#include "test_helpers.hpp"

#include "ring/network/io_context.hpp"
#include "ring/network/tcp_connection.hpp"
#include "ring/network/tcp_connector.hpp"
#include "ring/network/tcp_listener.hpp"

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

} // namespace ring::network

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
