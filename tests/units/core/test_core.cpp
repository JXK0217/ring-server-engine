#include <gtest/gtest.h>

#include "test_helpers.hpp"

#include "ring/core/initializer_registry.hpp"

namespace ring::core
{

class CoreTest : public test::TestBase
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
public:
    static void testInitialize(int priority)
    {
        std::cout << "testInitialize: " << priority << std::endl;
    }
    static void testShutdown(int priority)
    {
        std::cout << "testShutdown: " << priority << std::endl;
    }
};

TEST_F(CoreTest, InitializerRegistry)
{
    initializer_registry::instance().register_entry("test", std::bind(testInitialize, 1), std::bind(testShutdown, 1), 1);
    initializer_registry::instance().register_entry("test", std::bind(testInitialize, -1), std::bind(testShutdown, -1), -1);
    initializer_registry::instance().register_entry("test", std::bind(testInitialize, 0), std::bind(testShutdown, 0), 0);

    EXPECT_NO_THROW(initializer_registry::instance().initialize());
    EXPECT_NO_THROW(initializer_registry::instance().shutdown());
}

} // namespace ring::logging

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
