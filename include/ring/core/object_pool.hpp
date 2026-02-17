#ifndef RING_CORE_OBJECT_POOL_HPP_
#define RING_CORE_OBJECT_POOL_HPP_

#include <memory>
#include <mutex>
#include <stack>
#include <vector>

#include "ring/core/export.hpp"

namespace ring::core
{

template<typename T>
class object_pool_impl
{
protected:
    static constexpr size_t default_chunk_capacity = 1024;
private:
    class pool_chunk
    {
    private:
        struct storage_type
        {
            alignas(alignof(T)) std::byte data[sizeof(T)];
        };
    public:
        pool_chunk(size_t capacity)
        {
            storage_.resize(capacity);
        }
        ~pool_chunk() = default;
    public:
        void* acquire()
        {
            return &storage_[offset_++];
        }
        bool is_full() const
        {
            return offset_ >= storage_.size();
        }
        size_t offset() const
        {
            return offset_;
        }
    private:
        std::vector<storage_type> storage_;
        size_t offset_ = 0;
    };
protected:
    explicit object_pool_impl(size_t chunk_capacity) :
        chunk_capacity_(chunk_capacity) {}
    ~object_pool_impl() = default;
protected:
    object_pool_impl(const object_pool_impl&) = delete;
    object_pool_impl(object_pool_impl&&) = delete;
    object_pool_impl& operator=(const object_pool_impl&) = delete;
    object_pool_impl& operator=(object_pool_impl&&) = delete;
protected:
    template<typename... Args>
    T* acquire(Args&&... args)
    {
        T *obj = nullptr;
        if (free_list_.empty())
        {
            if (chunks_.empty() || chunks_.back()->is_full())
            {
                chunks_.emplace_back(std::make_unique<pool_chunk>(chunk_capacity_));
            }
            obj = reinterpret_cast<T*>(chunks_.back()->acquire());
        }
        else
        {
            obj = free_list_.top();
            free_list_.pop();
        }
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }
    void release(T* obj)
    {
        if (!obj)
        {
            return;
        }
        obj->~T();
        free_list_.push(obj);
    }
    bool empty() const
    {
        return size() == 0;
    }
    size_t size() const
    {
        if (chunks_.empty())
        {
            return 0;
        }
        return (chunks_.size() - 1) * chunk_capacity_ + chunks_.back()->offset() - free_list_.size();
    }
    size_t capacity() const
    {
        return chunks_.size() * chunk_capacity_;
    }
private:
    const size_t chunk_capacity_ = 0u;
private:
    std::vector<std::unique_ptr<pool_chunk>> chunks_;
    std::stack<T*> free_list_;
};

template<typename T>
class RING_API object_pool final : protected object_pool_impl<T>
{
private:
    using impl_ = object_pool_impl<T>;
public:
    object_pool(size_t chunk_capacity = impl_::default_chunk_capacity) :
        impl_(chunk_capacity) {}
    ~object_pool() = default;
public:
    template<typename... Args>
    T* acquire(Args&&... args)
    {
        return impl_::acquire(std::forward<Args>(args)...);   
    }
    void release(T *obj)
    {
        return impl_::release(obj);
    }
    bool empty() const
    {
        return impl_::empty();
    }
    size_t size() const
    {
        return impl_::size();
    }
    size_t capacity() const
    {
        return impl_::capacity();
    }
};

template<typename T>
class RING_API object_pool_mt final : protected object_pool_impl<T>
{
private:
    using impl_ = object_pool_impl<T>;
public:
    object_pool_mt(size_t chunk_capacity = impl_::default_chunk_capacity) :
        impl_(chunk_capacity) {}
    ~object_pool_mt() = default;
public:
    template<typename... Args>
    T* acquire(Args&&... args)
    {
        std::lock_guard lock(_mutex);
        return impl_::acquire(std::forward<Args>(args)...);   
    }
    void release(T *obj)
    {
        std::lock_guard lock(_mutex);
        return impl_::release(obj);
    }
    bool empty() const
    {
        std::lock_guard lock(_mutex);
        return impl_::empty();
    }
    size_t size() const
    {
        std::lock_guard lock(_mutex);
        return impl_::size();
    }
    size_t capacity() const
    {
        std::lock_guard lock(_mutex);
        return impl_::capacity();
    }
private:
    std::mutex _mutex;
};

} // namespace ring::core

#endif // !RING_CORE_OBJECT_POOL_HPP_