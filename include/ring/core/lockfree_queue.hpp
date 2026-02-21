#ifndef RING_LOCKFREE_QUEUE_HPP_
#define RING_LOCKFREE_QUEUE_HPP_

#include <algorithm>
#include <atomic>
#include <iterator>
#include <thread>

#include "ring/core/cache_line.hpp"
#include "ring/core/export.hpp"

namespace ring::core
{

namespace detail
{

template <typename T, bool WithSequences>
class memory_block;

template <typename T>
class memory_block<T, false>
{
public:
    explicit memory_block(size_t capacity) :
        capacity(capacity),
        data(make_unique_buffer_aligned(capacity * sizeof(T))) {}
public:
    T& operator[](size_t i)
    {
        return *reinterpret_cast<T*>(&data[i * sizeof(T)]);
    }
public:
    cache_aligned<std::atomic<size_t>> head{ 0 };
    cache_aligned<std::atomic<size_t>> tail{ 0 };
public:
    const size_t capacity;
    buffer_aligned data;
};

template <typename T>
class memory_block<T, true>
{
public:
    explicit memory_block(size_t capacity) :
        capacity(capacity),
        data(make_unique_buffer_aligned(capacity * sizeof(T))),
        sequences(std::make_unique<std::atomic<size_t>[]>(capacity))
    {
        for (size_t i = 0; i < capacity; ++i)
        {
            sequences[i].store(i, std::memory_order_relaxed);
        }
    }
public:
    T& operator[](size_t i)
    {
        return *reinterpret_cast<T*>(&data[i * sizeof(T)]);
    }
public:
    cache_aligned<std::atomic<size_t>> head{ 0 };
    cache_aligned<std::atomic<size_t>> tail{ 0 };
public:
    const size_t capacity;
    buffer_aligned data;
    std::unique_ptr<std::atomic<size_t>[]> sequences;
};

template <typename T>
class spsc_queue final
{
private:
    using block_type = memory_block<T, false>;
public:
    explicit spsc_queue(size_t capacity) :
        block_(capacity) {}

    ~spsc_queue()
    {
        size_t head = block_.head->load(std::memory_order_relaxed);
        size_t tail = block_.tail->load(std::memory_order_relaxed);
        while (head < tail)
        {
            size_t slot = head % block_.capacity;
            block_[slot].~T();
            ++head;
        }
    }
public:
    bool try_push(T&& value)
    {
        T temp(std::move(value));
        return try_push_batch(&temp, &temp + 1) == 1;
    }

    bool try_pop(T& result)
    {
        return try_pop_batch(&result, 1) == 1;
    }

    template <typename InputIt>
    size_t try_push_batch(InputIt first, InputIt last)
    {
        size_t tail = block_.tail->load(std::memory_order_relaxed);
        size_t head = block_.head->load(std::memory_order_acquire);
        size_t capacity = block_.capacity;
        size_t available = head + capacity - tail;
        if (available == 0)
        {
            return 0;
        }
        size_t count = 0;
        for (auto it = first; count < available && it != last; ++it, ++count)
        {
            size_t slot = (tail + count) % capacity;
            new (&block_[slot]) T(std::move(*it));
        }
        block_.tail->store(tail + count, std::memory_order_release);
        return count;
    }

    template <typename OutputIt>
    size_t try_pop_batch(OutputIt first, size_t max_count)
    {
        size_t head = block_.head->load(std::memory_order_relaxed);
        size_t tail = block_.tail->load(std::memory_order_acquire);
        size_t available = tail - head;
        if (available == 0)
        {
            return 0;
        }
        size_t count = std::min(available, max_count);
        size_t capacity = block_.capacity;
        for (size_t i = 0; i < count; ++i)
        {
            size_t slot = (head + i) % capacity;
            *first++ = std::move(block_[slot]);
            block_[slot].~T();
        }
        block_.head->store(head + count, std::memory_order_release);
        return count;
    }

    size_t size() const
    {
        size_t head = block_.head->load(std::memory_order_acquire);
        size_t tail = block_.tail->load(std::memory_order_acquire);
        return tail < head ? 0 : tail - head;
    }
    
    size_t capacity() const
    {
        return block_.capacity;
    }
    
    bool empty() const
    {
        return size() == 0;
    }
private:
    block_type block_;
};

template <typename T>
class mpsc_queue final
{
private:
    using block_type = memory_block<T, true>;
public:
    explicit mpsc_queue(size_t capacity) :
        block_(capacity) {}

    ~mpsc_queue()
    {
        size_t head = block_.head->load(std::memory_order_relaxed);
        size_t tail = block_.tail->load(std::memory_order_relaxed);
        while (head < tail)
        {
            size_t slot = head % block_.capacity;
            block_[slot].~T();
            ++head;
        }
    }
public:
    bool try_push(T&& value)
    {
        T temp(std::move(value));
        return try_push_batch(&temp, &temp + 1) == 1;
    }

    bool try_pop(T& result)
    {
        return try_pop_batch(&result, 1) == 1;
    }

    template <typename InputIt>
    size_t try_push_batch(InputIt first, InputIt last)
    {
        size_t pos = block_.tail->load(std::memory_order_relaxed);
        size_t head = block_.head->load(std::memory_order_acquire);
        size_t capacity = block_.capacity;
        size_t available = head + capacity - pos;
        if (available == 0)
        {
            return 0;
        }
        size_t requested = std::distance(first, last);
        if (requested == 0)
        {
            return 0;
        }
        size_t max_count = std::min(requested, available);
        size_t count = 0;
        for ( ; count < max_count; ++count)
        {
            size_t slot = (pos + count) % capacity;
            if (block_.sequences[slot].load(std::memory_order_acquire) != pos + count)
            {
                break;
            }
        }
        if (count == 0)
        {
            return 0;
        }
        if (!block_.tail->compare_exchange_weak(pos, pos + count,
            std::memory_order_release, std::memory_order_relaxed))
        {
            return 0;
        }
        for (size_t i = 0; i < count; ++i, ++first)
        {
            size_t slot = (pos + i) % capacity;
            new (&block_[slot]) T(std::move(*first));
            block_.sequences[slot].store(pos + i + 1, std::memory_order_release);
        }
        return count;
    }

    template <typename OutputIt>
    size_t try_pop_batch(OutputIt first, size_t max_count)
    {
        size_t head = block_.head->load(std::memory_order_relaxed);
        size_t tail = block_.tail->load(std::memory_order_acquire);
        size_t available = tail - head;
        if (available == 0)
        {
            return 0;
        }
        size_t capacity = block_.capacity;
        size_t count = 0;
        for (; count < std::min(available, max_count); ++count)
        {
            size_t slot = (head + count) % capacity;
            if (block_.sequences[slot].load(std::memory_order_acquire) != head + count + 1)
            {
                break;
            }
        }
        if (count == 0)
        {
            return 0;
        }
        for (size_t i = 0; i < count; ++i)
        {
            size_t slot = (head + i) % capacity;
            *first++ = std::move(block_[slot]);
            block_[slot].~T();
            block_.sequences[slot].store(head + i + capacity, std::memory_order_release);
        }
        block_.head->store(head + count, std::memory_order_release);
        return count;
    }

    size_t size() const
    {
        size_t head = block_.head->load(std::memory_order_acquire);
        size_t tail = block_.tail->load(std::memory_order_acquire);
        return tail < head ? 0 : tail - head;
    }
    
    size_t capacity() const
    {
        return block_.capacity;
    }
    
    bool empty() const
    {
        return size() == 0;
    }
private:
    block_type block_;
};

template <typename T>
class mpmc_queue final
{
private:
    using block_type = memory_block<T, true>;
public:
    explicit mpmc_queue(size_t capacity) :
        block_(capacity) {}

    ~mpmc_queue()
    {
        size_t head = block_.head->load(std::memory_order_relaxed);
        size_t tail = block_.tail->load(std::memory_order_relaxed);
        while (head < tail)
        {
            size_t slot = head % block_.capacity;
            block_[slot].~T();
            ++head;
        }
    }
public:
    bool try_push(T&& value)
    {
        size_t pos = block_.tail->load(std::memory_order_relaxed);
        size_t capacity = block_.capacity;
        if (pos + 1 > block_.head->load(std::memory_order_acquire) + capacity)
        {
            return false;
        }
        size_t slot = pos % capacity;
        size_t seq = block_.sequences[slot].load(std::memory_order_acquire);
        if (seq != pos)
        {
            return false;
        }
        if (!block_.tail->compare_exchange_weak(pos, pos + 1, 
            std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return false;
        }
        new (&block_[slot]) T(std::move(value));
        block_.sequences[slot].store(pos + 1, std::memory_order_release);
        return true;
    }

    bool try_pop(T& result)
    {
        size_t pos = block_.head->load(std::memory_order_relaxed);
        if (pos == block_.tail->load(std::memory_order_acquire))
        {
            return false;
        }
        size_t capacity = block_.capacity;
        size_t slot = pos % capacity;
        size_t seq = block_.sequences[slot].load(std::memory_order_acquire);
        if (seq != pos + 1)
        {
            return false;
        }
        if (!block_.head->compare_exchange_weak(pos, pos + 1,
            std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return false;
        }
        result = std::move(block_[slot]);
        block_[slot].~T();
        block_.sequences[slot].store(pos + capacity, std::memory_order_release);
        return true;
    }

    template <typename InputIt>
    size_t try_push_batch(InputIt first, InputIt last)
    {
        size_t pushed = 0;
        for (auto it = first; it != last; ++it)
        {
            if (!try_push(std::move(*it)))
            {
                break;
            }
            pushed++;
        }
        return pushed;
    }

    template <typename OutputIt>
    size_t try_pop_batch(OutputIt first, size_t max_count)
    {
        size_t popped = 0;
        for (size_t i = 0; i < max_count; ++i)
        {
            if (!try_pop(*first++))
            {
                break;
            }
            popped++;
        }
        return popped;
    }

    size_t size() const
    {
        size_t head = block_.head->load(std::memory_order_acquire);
        size_t tail = block_.tail->load(std::memory_order_acquire);
        return tail < head ? 0 : tail - head;
    }
    
    size_t capacity() const
    {
        return block_.capacity;
    }
    
    bool empty() const
    {
        return size() == 0;
    }
private:
    block_type block_;
};

} // namespace detail

template <typename T, template <typename> class Queue>
class RING_API lockfree_queue final
{
public:
    explicit lockfree_queue(size_t capacity) :
        impl_(capacity) {}
    ~lockfree_queue() = default;
private:
    lockfree_queue(const lockfree_queue&) = delete;
    lockfree_queue& operator=(const lockfree_queue&) = delete;
public:
    bool try_push(T&& value)
    {
        return impl_.try_push(std::move(value));
    }

    bool try_push(const T& value)
    {
        return try_push(T(value));
    }

    bool try_pop(T& result)
    {
        return impl_.try_pop(result);
    }

    template <typename InputIt>
    size_t try_push_batch(InputIt first, InputIt last)
    {
        return impl_.try_push_batch(first, last);
    }

    template <typename OutputIt>
    size_t try_pop_batch(OutputIt first, size_t max_count)
    {
        return impl_.try_pop_batch(first, max_count);
    }

    void push(T&& value)
    {
        while (!try_push(std::move(value)))
        {
            std::this_thread::yield();
        }
    }

    void push(const T& value)
    {
        push(T(value));
    }

    void pop(T& result)
    {
        while (!try_pop(result))
        {
            std::this_thread::yield();
        }
    }

    template <typename InputIt>
    void push_batch(InputIt first, InputIt last)
    {
        auto it = first;
        while (it != last)
        {
            size_t pushed = try_push_batch(it, last);
            if (pushed)
            {
                std::advance(it, pushed);
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }

    template <typename OutputIt>
    void pop_batch(OutputIt first, size_t max_count)
    {
        size_t total_popped = 0;
        while (total_popped < max_count)
        {
            size_t popped = try_pop_batch(std::next(first, total_popped), max_count - total_popped);
            if (popped)
            {
                total_popped += popped;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }

    size_t size() const
    {
        return impl_.size();
    }

    size_t capacity() const
    {
        return impl_.capacity();
    }

    bool empty() const
    {
        return impl_.empty();
    }
private:
    Queue<T> impl_;
};

template <typename T>
using spsc_queue = lockfree_queue<T, detail::spsc_queue>;

template <typename T>
using mpsc_queue = lockfree_queue<T, detail::mpsc_queue>;

template <typename T>
using mpmc_queue = lockfree_queue<T, detail::mpmc_queue>;

} // namespace ring::core

#endif // RING_LOCKFREE_QUEUE_HPP_