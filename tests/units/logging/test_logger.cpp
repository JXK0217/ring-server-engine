#include <gtest/gtest.h>

#include "test_helpers.hpp"

#include "ring/logging/logger.h"

namespace ring::logging
{

class LoggerTest : public test::TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();

        handler = &log_service::instance();
    }
    void TearDown() override
    {
        TestBase::TearDown();
    }
protected:
    log_service *handler = nullptr;
};

TEST_F(LoggerTest, GetAndLog)
{
    auto logger = handler->get_logger("t1");  
    ASSERT_NE(logger, nullptr);

    for (size_t i = 0; i < 1000000; i++)
    {
        EXPECT_NO_THROW(logger->info("hello world, {}!, {}", "jxk", i));
    }
}

} // namespace ring::logging