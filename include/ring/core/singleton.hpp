#ifndef RING_CORE_SINGLETON_HPP_
#define RING_CORE_SINGLETON_HPP_

namespace ring::core
{

template <typename T>
class singleton
{
private:
    singleton() = default;
    virtual ~singleton() = default;
private:
    singleton(const singleton&) = delete;
    singleton& operator=(const singleton&) = delete;
public:
    template <typename... Args>
    static singleton& instance(Args&& ...args)
    {
        static singleton instance(std::forward<Args>(args)...);
        return instance;
    }
};

} // namespace ring::core

#endif // !RING_CORE_SINGLETON_HPP_