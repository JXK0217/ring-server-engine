# src/CMakeLists.txt - main build configuration for ring-server

# gather all source files
set(ALL_SOURCES)

if(BUILD_MONOLITHIC_LIB)
    list(APPEND ALL_SOURCES ring.cpp)
endif()

# export all sources to parent scope
set(ALL_RESOURCES ${ALL_SOURCES} PARENT_SCOPE)

if(RING_VERBOSE)
    message(STATUS "All source files:")
    foreach(SRC ${ALL_SOURCES})
        message(STATUS "  ${SRC}")
    endforeach()
endif()
