#include <gtest/gtest.h>

#include "test_helpers.hpp"

#include "ring/app/application.hpp"

namespace ring::app
{

class AppTest : public test::TestBase
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

TEST_F(AppTest, Application)
{
    application app;
    app.run();
}

} // namespace ring::app

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
