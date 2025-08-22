# platforms/windows.cmake - Windows platform specific settings

message(STATUS "Configuring for Windows platform")

# set custom macros
set(RING_PLATFORM_WINDOWS ON)
add_definitions(-DRING_PLATFORM_WINDOWS)

# add platform-specific source files
add_subdirectory(platform/windows)

# windows library dependencies
set(PLATFORM_LIBS
    ws2_32  # Winsock library
    crypt32  # Cryptography API
)
