#include <gtest/gtest.h>

#include "test_helpers.hpp"

#include "ring/logging/logger.h"

namespace ring::logging
{

class LoggerTest : public test::TestBase
{
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
        log_service::instance().shutdown();
    }
protected:
    void SetUp() override
    {
        TestBase::SetUp();
    }
    void TearDown() override
    {
        log_service::instance().flush_all();

        TestBase::TearDown();
    }
protected:
    static constexpr uint32_t count = 10000;
};

TEST_F(LoggerTest, GetAndLog)
{
    auto logger = log_service::instance().get_logger("get");  
    ASSERT_NE(logger, nullptr);

    for (size_t i = 0; i < count; i++)
    {
        EXPECT_NO_THROW(logger->info("hello world, {}!, {}", "jxk", i));
    }
}

TEST_F(LoggerTest, CreateAndLog)
{
    auto logger = log_service::instance().create_logger({ .name = "create", .file = "create_log" });  
    ASSERT_NE(logger, nullptr);

    for (size_t i = 0; i < count; i++)
    {
        EXPECT_NO_THROW(logger->info("hello world, {}!, {}", "jxk", i));
    }
}

TEST_F(LoggerTest, DefaultFuncLog)
{
    for (size_t i = 0; i < count; i++)
    {
        EXPECT_NO_THROW(ring::logging::info("hello world, {}!, {}", "jxk", i));
    }
}

TEST_F(LoggerTest, DefaultMacroLog)
{
    for (size_t i = 0; i < count; i++)
    {
        EXPECT_NO_THROW(RING_INFO("hello world, {}!, {}", "jxk", i));
    }
}

} // namespace ring::logging