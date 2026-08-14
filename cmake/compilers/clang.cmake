# compilers/clang.cmake - Clang specific settings

message(STATUS "Configuring for Clang compiler")

add_library(compiler_flags INTERFACE)

# set custom macros
set(RING_COMPILER_CLANG ON)
add_definitions(-DRING_COMPILER_CLANG)

# add Clang specific compiler flags
target_compile_options(compiler_flags INTERFACE -Wall -Wextra -Werror)  # Enable all warnings and treat them as errors
target_compile_options(compiler_flags INTERFACE -Wpedantic)  # Enable warnings for unused parameters

if(BUILD_SHARED_LIBS)
    # add fpic option for shared libraries
    target_compile_options(compiler_flags INTERFACE -fPIC)  # Position-independent code for shared libraries

    # Set compiler visibility settings (for shared libraries)
    target_compile_options(compiler_flags INTERFACE -fvisibility=hidden)
    target_compile_options(compiler_flags INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)
endif()

# set default compiler and linker flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -DNDEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O2 -g -DNDEBUG")
else()
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif()

if(ENABLE_ASAN)
    target_compile_options(compiler_flags INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(compiler_flags INTERFACE -fsanitize=address)
endif()
