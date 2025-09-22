# third_party/CMakeLists.txt - CMake configuration for third-party dependencies

# include dependencies
include(../cmake/dependencies/asio)
include(../cmake/dependencies/spdlog)

if(BUILD_TESTS)
    set(INSTALL_GTEST OFF)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/googletest)
endif()