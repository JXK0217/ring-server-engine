# compilers/clang.cmake - Clang specific settings

message(STATUS "Configuring for Clang compiler")

# set custom macros
set(RING_COMPILER_CLANG ON)
add_definitions(-DRING_COMPILER_CLANG)

# add Clang specific compiler flags
add_compile_options(-Wall -Wextra -Werror)  # Enable all warnings and treat them as errors
add_compile_options(-Wpedantic)  # Enable warnings for unused parameters

# add fpic option for shared libraries
if(BUILD_SHARED_LIBS)
    add_compile_options(-fPIC)  # Position-independent code for shared libraries
endif()

# Set compiler visibility settings (for shared libraries)
if(BUILD_SHARED_LIBS)
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-fvisibility-inlines-hidden)
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
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
