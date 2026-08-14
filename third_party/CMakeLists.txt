# third_party/CMakeLists.txt - CMake configuration for third-party dependencies

# include dependencies
if(NETWORK_BACKEND STREQUAL "asio")
    include(../cmake/dependencies/asio)
endif()
include(../cmake/dependencies/protobuf)
include(../cmake/dependencies/spdlog)

if(BUILD_TESTS)
    set(INSTALL_GTEST OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/googletest)
endif()