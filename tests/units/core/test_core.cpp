#include <gtest/gtest.h>

#include <thread>

#include "test_helpers.hpp"

#include "ring/core/exception.hpp"
#include "ring/core/initializer_registry.hpp"
#include "ring/core/lockfree_queue.hpp"
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

template <bool WithString>
struct TestItemImpl
{
    TestItemImpl(size_t producer_id, size_t sequence) :
        producer_id(producer_id), sequence(sequence)
    {
        if constexpr (WithString)
        {
            str = std::format("item_{}_{}", producer_id, sequence);
        }
    }
    TestItemImpl() = default;

    size_t producer_id;
    size_t sequence;
    std::conditional_t<WithString, std::string, std::monostate> str;
};

constexpr bool test_item_with_string = true;
constexpr size_t queue_capacity = 65536;
constexpr size_t items_total = 10000000;
constexpr size_t producer_count = 4;
constexpr size_t consumer_count = 4;
constexpr size_t producer_batch = 1024;
constexpr size_t consumer_batch = 4096;

using TestItem = TestItemImpl<test_item_with_string>;

TEST_F(CoreTest, SPSCQueue)
{
    std::atomic<size_t> total_pushed{ 0 };
    std::atomic<size_t> total_popped{ 0 };
    std::atomic<bool> error_flag{ false };

    ring::core::spsc_queue<TestItem> queue(queue_capacity);

    std::thread producer([&]()
        {
            for (size_t i = 0; i < items_total; )
            {
                auto batch_size = std::min(producer_batch, items_total - i);
                std::array<TestItem, producer_batch> items;
                for (size_t j = 0; j < batch_size; ++j)
                {
                    items[j] = TestItem(0, i + j);
                }
                queue.push_batch(items.begin(), std::next(items.begin(), batch_size));
                total_pushed.fetch_add(batch_size, std::memory_order_relaxed);
                i += batch_size;
            }
            std::cout << "Producer Over" << std::endl;
        });
    std::thread consumer([&]()
        {
            std::array<TestItem, consumer_batch> items;
            while (!error_flag)
            {
                size_t count = queue.try_pop_batch(items.data(), items.size());
                if (count == 0)
                {
                    std::this_thread::yield();
                    continue;
                }
                total_popped.fetch_add(count, std::memory_order_relaxed);
            }
        });
    
    producer.join();
    std::cout << "Producers finished. Pushed: " << total_pushed.load() << std::endl;

    while (total_popped.load() < total_pushed.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    error_flag = true;
    consumer.join();
    std::cout << "Consumers finished. Popped: " << total_popped.load() << std::endl;

    assert(total_pushed.load() == total_popped.load());
    assert(queue.empty());
    std::cout << "Stress Test Passed: No data loss, no crash." << std::endl;
}

TEST_F(CoreTest, MPSCQueue)
{
    std::atomic<size_t> total_pushed{ 0 };
    std::atomic<size_t> total_popped{ 0 };
    std::atomic<bool> error_flag{ false };

    mpsc_queue<TestItem> queue(queue_capacity);

    std::vector<std::thread> producers;
    for (size_t k = 0; k < producer_count; ++k)
    {
        size_t max_count = (k == producer_count - 1) ?
                            (items_total - items_total / producer_count * k) :
                            items_total / producer_count;
        producers.emplace_back([&, k, max_count]()
            {
                for (size_t i = 0; i < max_count; )
                {
                    auto batch_size = std::min(producer_batch, max_count - i);
                    std::array<TestItem, producer_batch> items;
                    for (size_t j = 0; j < batch_size; ++j)
                    {
                        items[j] = TestItem(k, i + j);
                    }
                    queue.push_batch(items.begin(), std::next(items.begin(), batch_size));
                    total_pushed.fetch_add(batch_size, std::memory_order_relaxed);
                    i += batch_size;
                }
            });
    }
    std::thread consumer([&]()
        {
            std::array<TestItem, consumer_batch> items;
            while (!error_flag)
            {
                size_t count = queue.try_pop_batch(items.data(), items.size());
                if (count == 0)
                {
                    std::this_thread::yield();
                    continue;
                }
                total_popped.fetch_add(count, std::memory_order_relaxed);
            }
        });

    for (auto& t : producers)
    {
        t.join();
    }
    std::cout << "Producers finished. Pushed: " << total_pushed.load() << std::endl;

    while (total_popped.load() < total_pushed.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    error_flag = true;
    consumer.join();
    std::cout << "Consumers finished. Popped: " << total_popped.load() << std::endl;

    assert(total_pushed.load() == total_popped.load());
    assert(queue.empty());
    std::cout << "Stress Test Passed: No data loss, no crash." << std::endl;
}

TEST_F(CoreTest, MPMCQueue)
{
    std::atomic<size_t> total_pushed{ 0 };
    std::atomic<size_t> total_popped{ 0 };
    std::atomic<bool> error_flag{false};

    mpmc_queue<TestItem> queue(queue_capacity);
    
    std::vector<std::thread> producers;
    for (size_t k = 0; k < producer_count; ++k)
    {
        size_t max_count = (k == producer_count - 1) ?
                            (items_total - items_total / producer_count * k) :
                            items_total / producer_count;
        producers.emplace_back([&, k, max_count]()
            {
                for (size_t i = 0; i < max_count; )
                {
                    auto batch_size = std::min(producer_batch, max_count - i);
                    std::array<TestItem, producer_batch> items;
                    for (size_t j = 0; j < batch_size; ++j)
                    {
                        items[j] = TestItem(k, i + j);
                    }
                    queue.push_batch(items.begin(), std::next(items.begin(), batch_size));
                    total_pushed.fetch_add(batch_size, std::memory_order_relaxed);
                    i += batch_size;
                }
            });
    }
    std::vector<std::thread> consumers;
    for (size_t i = 0; i < consumer_count; ++i)
    {
        consumers.emplace_back([&, i]()
            {
                std::array<TestItem, consumer_batch> items;
                while (!error_flag)
                {
                    size_t count = queue.try_pop_batch(items.data(), items.size());
                    if (count == 0)
                    {
                        std::this_thread::yield();
                        continue;
                    }
                    total_popped.fetch_add(count, std::memory_order_relaxed);
                }
            });
    }

    for (auto& t : producers)
    {
        t.join();
    }
    std::cout << "Producers finished. Pushed: " << total_pushed.load() << std::endl;

    while (total_popped.load() < total_pushed.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    error_flag = true;
    for (auto& t : consumers)
    {
        t.join();
    }
    std::cout << "Consumers finished. Popped: " << total_popped.load() << std::endl;

    assert(total_pushed.load() == total_popped.load());
    assert(queue.empty());
    std::cout << "Stress Test Passed: No data loss, no crash." << std::endl;
}

} // namespace ring::core

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
