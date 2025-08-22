# definitions.cmake - CMake definitions

# check architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(RING_ARCH_X64 ON)
    add_definitions(-DRING_ARCH_X64)
else()
    set(RING_ARCH_X32 ON)
    add_definitions(-DRING_ARCH_X32)
endif()
