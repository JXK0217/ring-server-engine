# platforms/linux.cmake - Linux platform specific settings

message(STATUS "Configuring for Linux platform")

# set custom macros
set(RING_PLATFORM_LINUX ON)
add_definitions(-DRING_PLATFORM_LINUX)

# add platform-specific 
add_subdirectory(platform/linux)

# linux library dependencies
find_package(Threads REQUIRED)  # Find the Threads package
set(PLATFORM_LIBS
    ${CMAKE_THREAD_LIBS_INIT}  # Thread library
    dl  # Dynamic linking library
)