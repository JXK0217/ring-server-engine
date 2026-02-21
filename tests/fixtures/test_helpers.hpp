#ifndef RING_TESTS_FIXTURES_TEST_HELPERS_H__
#define RING_TESTS_FIXTURES_TEST_HELPERS_H__

#include <gtest/gtest.h>

namespace ring::test
{
    
class TestBase: public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

} // namespace ring::test

#endif // RING_TESTS_FIXTURES_TEST_HELPERS_H__