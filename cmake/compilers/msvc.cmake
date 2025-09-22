# compilers/msvc.cmake - MSVC specific settings

message(STATUS "Configuring for MSVC compiler")

# set custom macros
set(RING_COMPILER_MSVC ON)
add_definitions(-DRING_COMPILER_MSVC)

# add MSVC specific compiler flags
add_compile_options(/W4 /WX)  # Enable warnings and treat them as errors
add_compile_options(/MP)  # Enable multi-processor compilation
add_compile_options(/wd4251)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # Disable secure warnings
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)  # Disable deprecated warnings for Winsock
add_definitions(-D_WIN32_WINNT=0x0A00)  # Set minimum Windows version to Windows 10

# Set character set to Unicode
add_definitions(-DUNICODE -D_UNICODE)

# set default compiler and linker flags
set(HAS_DEBUG_SYMBOLS FALSE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /Od")
    set(HAS_DEBUG_SYMBOLS TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /O1 /DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /O1 /DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Zi /DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /O2 /Zi /DNDEBUG")
    set(HAS_DEBUG_SYMBOLS TRUE)
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

if(HAS_DEBUG_SYMBOLS AND ENABLE_ASAN)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
endif()
