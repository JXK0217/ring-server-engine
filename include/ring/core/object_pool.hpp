#ifndef __RING_CORE_OBJECT_POOL_HPP__
#define __RING_CORE_OBJECT_POOL_HPP__

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
            _storage.resize(capacity);
        }
        ~pool_chunk() = default;
    public:
        void* acquire()
        {
            return &_storage[_offset++];
        }
        bool is_full() const
        {
            return _offset >= _storage.size();
        }
        size_t offset() const
        {
            return _offset;
        }
    private:
        std::vector<storage_type> _storage;
        size_t _offset = 0;
    };
protected:
    explicit object_pool_impl(size_t chunk_capacity) :
        _chunk_capacity(chunk_capacity) {}
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
        if (_free_list.empty())
        {
            if (_chunks.empty() || _chunks.back()->is_full())
            {
                _chunks.emplace_back(std::make_unique<pool_chunk>(_chunk_capacity));
            }
            obj = reinterpret_cast<T*>(_chunks.back()->acquire());
        }
        else
        {
            obj = _free_list.top();
            _free_list.pop();
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
        _free_list.push(obj);
    }
    bool empty() const
    {
        return size() == 0;
    }
    size_t size() const
    {
        if (_chunks.empty())
        {
            return 0;
        }
        return (_chunks.size() - 1) * _chunk_capacity + _chunks.back()->offset() - _free_list.size();
    }
    size_t capacity() const
    {
        return _chunks.size() * _chunk_capacity;
    }
private:
    const size_t _chunk_capacity = 0u;
private:
    std::vector<std::unique_ptr<pool_chunk>> _chunks;
    std::stack<T*> _free_list;
};

template<typename T>
class RING_API object_pool final : protected object_pool_impl<T>
{
private:
    using _impl = object_pool_impl<T>;
public:
    object_pool(size_t chunk_capacity = _impl::default_chunk_capacity) :
        _impl(chunk_capacity) {}
    ~object_pool() = default;
public:
    template<typename... Args>
    T* acquire(Args&&... args)
    {
        return _impl::acquire(std::forward<Args>(args)...);   
    }
    void release(T *obj)
    {
        return _impl::release(obj);
    }
    bool empty() const
    {
        return _impl::empty();
    }
    size_t size() const
    {
        return _impl::size();
    }
    size_t capacity() const
    {
        return _impl::capacity();
    }
};

template<typename T>
class RING_API object_pool_mt final : protected object_pool_impl<T>
{
private:
    using _impl = object_pool_impl<T>;
public:
    object_pool_mt(size_t chunk_capacity = _impl::default_chunk_capacity) :
        _impl(chunk_capacity) {}
    ~object_pool_mt() = default;
public:
    template<typename... Args>
    T* acquire(Args&&... args)
    {
        std::lock_guard lock(_mutex);
        return _impl::acquire(std::forward<Args>(args)...);   
    }
    void release(T *obj)
    {
        std::lock_guard lock(_mutex);
        return _impl::release(obj);
    }
    bool empty() const
    {
        std::lock_guard lock(_mutex);
        return _impl::empty();
    }
    size_t size() const
    {
        std::lock_guard lock(_mutex);
        return _impl::size();
    }
    size_t capacity() const
    {
        std::lock_guard lock(_mutex);
        return _impl::capacity();
    }
private:
    std::mutex _mutex;
};

} // namespace ring::core

#endif // !__RING_CORE_OBJECT_POOL_HPP__