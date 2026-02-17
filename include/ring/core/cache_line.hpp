#ifndef RING_CORE_CACHE_LINE_HPP_
#define RING_CORE_CACHE_LINE_HPP_

#include <cstdint>
#include <new>

#include "ring/core/export.hpp"

namespace ring::core
{

namespace detail
{

constexpr size_t cache_line_size = 64;
constexpr std::align_val_t cache_line_alignment{ cache_line_size };

struct aligned_deleter
{
    void operator()(void* ptr) const
    {
        ::operator delete(ptr, cache_line_alignment);
    }
};

template<size_t N>
class padding
{
private:
    char pad_[N];
};

template<>
class padding<0> {};

} // namespace detail

template<typename T, size_t Alignment = detail::cache_line_size>
class RING_API cache_aligned final
{
    static_assert(Alignment >= sizeof(T) && Alignment >= alignof(T), 
        "Alignment must accommodate type size and alignment");
public:
    template<typename... Args>
    explicit cache_aligned(Args&&... args) :
        value_(std::forward<Args>(args)...) {}
public:
    T* operator->() & noexcept { return &value_; }
    const T* operator->() const & noexcept { return &value_; }
    T& operator*() & noexcept { return value_; }
    const T& operator*() const & noexcept { return value_; }
    T&& operator*() && noexcept { return std::move(value_); }
private:
    alignas(Alignment) T value_;
    [[no_unique_address]] detail::padding<Alignment - sizeof(T)> pad_;
};

std::unique_ptr<char[], detail::aligned_deleter> make_unique_buffer_aligned(size_t size)
{
    void* mem = ::operator new(size, detail::cache_line_alignment);
    return std::unique_ptr<char[], detail::aligned_deleter>(static_cast<char*>(mem));
}

using buffer_aligned = std::unique_ptr<char[], detail::aligned_deleter>;

} // namespace ring::core

#endif // !RING_CORE_CACHE_LINE_HPP_