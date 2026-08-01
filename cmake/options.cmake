# options.cmake - project configuration options

# build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
# option(BUILD_MONOLITHIC_LIB "Build monolithic library" ON)
option(BUILD_MODULAR_LIBS "Build modular libraries" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build tests" ON)

# module options
option(BUILD_CORE_MODULE "Build core module" ON)
option(BUILD_LOGGING_MODULE "Build logging module" ON)
option(BUILD_NETWORK_MODULE "Build network module" ON)

# functionality options
option(ENABLE_ASAN "Enable Asan" ON)
option(ENABLE_REDIS "Enable Redis support" ON)

# network backend
set(NETWORK_BACKEND "asio" CACHE STRING "Network backend: asio")

# third-party library options
if(NETWORK_BACKEND STREQUAL "asio")
    option(USE_ASIO_STANDALONE "Use standalone ASIO" ON)
endif()
option(USE_SYSTEM_SPDLOG "Use system-installed spdlog" OFF)

# initialize module sources
set(ENABLED_MODULES)