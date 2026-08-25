#ifndef RING_APP_MODULE_HPP_
#define RING_APP_MODULE_HPP_

#include "ring/core/export.hpp"

namespace ring::app
{
    
class RING_API module
{
public:
    module() = default;
    virtual ~module() = default;
public:
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // namespace ring::app

#endif // RING_APP_MODULE_HPP_