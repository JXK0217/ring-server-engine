#ifndef RING_APP_SERVICE_HPP_
#define RING_APP_SERVICE_HPP_

#include "ring/app/module.hpp"

#include "ring/core/export.hpp"
#include "ring/network/io_context.hpp"

namespace ring::app
{
    
class RING_API service : public module
{
public:
    service(ring::network::io_context& ctx) :
        ctx_(ctx) {}
    ~service() = default;
public:
    ring::network::io_context& ctx_;
};

} // namespace ring::app

#endif // RING_APP_SERVICE_HPP_