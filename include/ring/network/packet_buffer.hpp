#ifndef RING_NETWORK_PACKET_BUFFER_HPP_
#define RING_NETWORK_PACKET_BUFFER_HPP_

#include <cstring>
#include <vector>

#include "ring/network/socket_buffer.hpp"

namespace ring::network
{

class packet_buffer final
{
public:
    static constexpr size_t init_size = 0x1000;
    static constexpr size_t max_size = 0x40000;
public:
    packet_buffer() = default;
    ~packet_buffer() = default;
public:
    size_t try_read(mutable_buffer dst)
    {
        return read(dst);
    }
    size_t try_write(const_buffer src)
    {
        if (!try_resize(src.size()))
        {
            return 0;
        }
        return write(src);
    }
    void commit_read(size_t len)
    {
        read(len);
    }
    void commit_write(size_t len)
    {
        write(len);
    }
    void reset()
    {
        data_.resize(init_size);
        read_index_ = 0;
        write_index_ = 0;
    }
    size_t readable_bytes() const
    {
        return write_index_ - read_index_;
    }
    size_t writable_bytes() const
    {
        return data_.size() - write_index_;
    }
    const std::byte* read_begin() const
    {
        return data_.data() + read_index_;
    }
    std::byte* write_begin()
    {
        return data_.data() + write_index_;
    }
private:
    size_t read(mutable_buffer dst)
    {
        auto readable = std::min(readable_bytes(), dst.size());
        std::memcpy(dst.data(), read_begin(), readable);
        read(readable);
        return readable;
    }
    size_t write(const_buffer src)
    {
        auto writable = std::min(writable_bytes(), src.size());
        std::memcpy(write_begin(), src.data(), writable);
        write(writable);
        return writable;
    }
    void read(size_t len)
    {
        read_index_ += len;
        if (read_index_ == write_index_)
        {
            read_index_ = 0;
            write_index_ = 0;
        }
    }
    void write(size_t len)
    {
        write_index_ += len;
    }
    bool try_resize(size_t len)
    {
        if (writable_bytes() >= len)
        {
            return true;
        }
        if (read_index_ > 0)
        {
            auto readable = readable_bytes();
            if (readable > 0)
            {
                std::memmove(data_.data(), read_begin(), readable);
            }
            read_index_ = 0;
            write_index_ = readable;
            if (writable_bytes() >= len)
            {
                return true;
            }
        }
        auto new_size = std::max(data_.size() * 2, write_index_ + len);
        if (new_size > max_size)
        {
            return false;
        }
        data_.resize(new_size);
        return true;
    }
private:
    std::vector<std::byte> data_{ init_size };
    size_t read_index_ = 0;
    size_t write_index_ = 0;
};

} // namespace ring::network

#endif // RING_NETWORK_PACKET_BUFFER_HPP_