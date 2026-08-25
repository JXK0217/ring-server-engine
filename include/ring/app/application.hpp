#ifndef RING_APP_APPLICATION_HPP_
#define RING_APP_APPLICATION_HPP_

#include <memory>
#include <ranges>
#include <vector>

#include "ring/app/module.hpp"
#include "ring/core/export.hpp"
#include "ring/network/io_context.hpp"

namespace ring::app
{

class RING_API application final
{
public:
    application() :
        ctx_(ring::network::io_context::create()) {}
    ~application() = default;
public:
    void run()
    {
        for (auto& module : modules_)
        {
            module->start();
        }
        ctx_->run();
        for (auto& module : modules_ | std::views::reverse)
        {
            module->stop();
        }
    }
private:
    // TODO just a demo
    std::unique_ptr<ring::network::io_context> ctx_;
    std::vector<std::unique_ptr<module>> modules_;
};

} // namespace ring::app

#endif // RING_APP_APPLICATION_HPP_