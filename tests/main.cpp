// tests/main.cpp
#include <gtest/gtest.h>

#ifdef BUILD_CORE_MODULE
#include "ring/core/singleton.hpp"
#endif

#ifdef BUILD_NETWORK_MODULE
#endif

#ifdef BUILD_LOGGING_MODULE
#include "ring/logging/logger.h"
#endif

std::string GetTestFilter()
{
    std::string filter;
    
#ifdef BUILD_CORE_MODULE
    filter += "Core*:";
#endif
    
#ifdef BUILD_NETWORK_MODULE
    filter += "Network*:";
#endif
    
#ifdef BUILD_LOGGING_MODULE
    filter += "Logging*:";
#endif
    
    if (!filter.empty())
    {
        filter.pop_back();
    }
    return filter;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    
    ::testing::GTEST_FLAG(filter) = GetTestFilter();
    
    return RUN_ALL_TESTS();
}
