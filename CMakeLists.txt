cmake_minimum_required(VERSION 3.28)
project(ring-server-engine LANGUAGES CXX C)

# set project version
set(RING_SERVER_VERSION_MAJOR 0)
set(RING_SERVER_VERSION_MINOR 1)
set(RING_SERVER_VERSION_PATCH 0)
set(RING_SERVER_VERSION ${RING_SERVER_VERSION_MAJOR}.${RING_SERVER_VERSION_MINOR}.${RING_SERVER_VERSION_PATCH})

# include custom modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(options)
include(settings)

# check platform
if(WIN32)
    include(platforms/windows)
elseif(APPLE)
    include(platforms/darwin)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    include(platforms/linux)
elseif(CMAKE_SYSTEM_NAME MATCHES "BSD")
    include(platforms/bsd)
endif()

# check compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    include(compilers/clang)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include(compilers/gcc)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    include(compilers/msvc)
endif()

# add third-party dependencies
add_subdirectory(third_party)

add_subdirectory(src/core)
add_subdirectory(src/logging)
add_subdirectory(src/network)

add_library(ring-server)

# add enabled modules sources
foreach(MODULE ${RING_MODULES})
    target_sources(ring-server PRIVATE ${${MODULE}_SOURCES})

    if(RING_VERBOSE)
        message(STATUS "All ${MODULE} files:")
        foreach(SRC ${${MODULE}_SOURCES})
            message(STATUS "  ${SRC}")
        endforeach()
    endif()
endforeach()

# add wrapper sources
target_sources(ring-server PRIVATE src/ring.cpp)
target_precompile_headers(ring-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/ring/pch.hpp)

# set include directories
target_include_directories(ring-server
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# set compiler properties
set_target_properties(ring-server PROPERTIES
    VERSION ${RING_SERVER_VERSION}
    SOVERSION ${RING_SERVER_VERSION_MAJOR}
    OUTPUT_NAME "ring-server"
)

target_compile_options(ring-server PRIVATE ${COMPILER_WARNINGS})

# link dependencies
target_link_libraries(ring-server 
    PRIVATE 
        $<BUILD_INTERFACE:${RING_DEPENDENCIES}>
)

# add examples and tests
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# install configuration
include(GNUInstallDirs)

configure_file(
    include/ring/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/ring/version.hpp
    @ONLY
)

# install headers
install(
    DIRECTORY include/ring
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# install monolithic library
install(
    TARGETS ring-server
    EXPORT ring-server-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/ring/version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ring
)

install(
    EXPORT ring-server-targets
    FILE ring-server-targets.cmake
    NAMESPACE ring::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring-server
)
