#include <gtest/gtest.h>

#include <thread>

#include "test_helpers.hpp"

#include "ring/core/exception.hpp"
#include "ring/core/initializer_registry.hpp"
#include "ring/core/object_pool.hpp"

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
    EXPECT_NO_THROW(initializer_registry::instance().initialize());
    
    EXPECT_NO_THROW(initializer_registry::instance().shutdown());
    EXPECT_NO_THROW(initializer_registry::instance().shutdown());
}

class TestException final: public ring::core::exception
{
public:
    explicit TestException(std::string_view message, std::source_location location = std::source_location::current())
        : exception(message, location) {}
public:
    std::string_view type() const noexcept override
    {
        return "TestException";
    }
};

TEST_F(CoreTest, Exception)
{
    {
        ring::core::exception e("exception");
        std::cout << e.detail() << std::endl;
    }
    {
        TestException e("Test");
        std::cout << e.detail() << std::endl;
    }
    throw ring::core::exception("throw");
}

TEST_F(CoreTest, ObjectPool)
{
    struct TestObject
    {
        TestObject(std::string str) :
            str(std::move(str)) {}
        std::string str;
    };
    
    {
        ring::core::object_pool<TestObject> pool;
        auto *obj = pool.acquire("hello world");
        std::cout << obj->str << std::endl;
        pool.release(obj);
    }
    {
        struct Context
        {
            ring::core::object_pool_mt<TestObject> pool;
            std::vector<TestObject*> objects;
            std::mutex mutex;
        } context;
        constexpr size_t threads_size = 20000;
        std::vector<std::thread> threads;
        threads.reserve(threads_size);
        for (auto i = 0ul; i < threads_size; ++i)
        {
            threads.emplace_back([](Context *context, size_t i)
            {
                auto* obj = context->pool.acquire(std::format("hello world: {}", i));
                std::lock_guard lock(context->mutex);
                context->objects.push_back(obj);
            }, &context, i);
        }
        for (auto i = 0ul; i < threads.size(); ++i)
        {
            threads[i].join();
        }
        for (auto i = 0ul; i < context.objects.size(); ++i)
        {
            std::cout << context.objects[i]->str << std::endl;
            context.pool.release(context.objects[i]);
        }
    }
}

} // namespace ring::logging

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
