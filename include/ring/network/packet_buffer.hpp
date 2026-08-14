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
    static constexpr size_t max_size = 0x4000000;
public:
    packet_buffer() = default;
    ~packet_buffer() = default;
public:
    bool try_write(const_buffer src)
    {
        if (src.size() == 0)
        {
            return true;
        }
        if (!try_resize(src.size()))
        {
            return false;
        }
        write(src);
        return true;
    }
    size_t read(size_t len)
    {
        auto readable = std::min(readable_bytes(), len);
        read_index_ = (read_index_ + len) % data_.size();
        if (read_index_ == write_index_)
        {
            read_index_ = 0;
            write_index_ = 0;
        }
        return readable;
    }
    void reset()
    {
        read_index_ = 0;
        write_index_ = 0;
    }
    size_t readable_bytes() const
    {
        if (write_index_ < read_index_)
        {
            return data_.size() - read_index_ + write_index_;
        }
        return write_index_ - read_index_;
    }
    size_t writable_bytes() const
    {
        if (write_index_ < read_index_)
        {
            return read_index_ - write_index_;
        }
        return data_.size() - write_index_ + read_index_;
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
    void write(const_buffer src)
    {
        auto first = std::min(src.size(), data_.size() - write_index_);
        std::memcpy(data_.data() + write_index_, src.data(), first);
        if (first < src.size())
        {
            std::memcpy(data_.data(), src.data() + first, src.size() - first);
        }
        write_index_ = (write_index_ + src.size()) % data_.size();
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