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

# include modules
include(modules/core)
include(modules/logging)
include(modules/network)

# build monolithic library
if(BUILD_MONOLITHIC_LIB)
    add_library(ring-server)

    # add enabled modules sources
    foreach(MODULE ${ENABLED_MODULES})
        target_sources(ring-server PRIVATE ${${MODULE}_SOURCES})
    endforeach()

    # add wrapper sources
    target_sources(ring-server PRIVATE src/ring.cpp)
    target_precompile_headers(ring-server PRIVATE ${CMAKE_SOURCE_DIR}/include/ring/pch.h)
  
    # set include directories
    target_include_directories(ring-server
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
  
    # set compiler properties
    set_target_properties(ring-server PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        VERSION ${RING_SERVER_VERSION}
        SOVERSION ${RING_SERVER_VERSION_MAJOR}
        OUTPUT_NAME "ring-server"
    )
  
    target_compile_options(ring-server PRIVATE ${COMPILER_WARNINGS})
  
    # link dependencies
    target_link_libraries(ring-server PRIVATE ${RING_DEPENDENCIES})
endif()

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

# install headers
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install monolithic library
if(BUILD_MONOLITHIC_LIB)
    install(
        TARGETS ring-server
        EXPORT ring-server-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# install individual modules
foreach(MODULE ${ENABLED_MODULES})
    install(
        TARGETS ${MODULE}
        EXPORT ring-server-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endforeach()

# generate configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ring-server-config-version.cmake"
    VERSION ${RING_SERVER_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT ring-server-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/ring-server-targets.cmake"
    NAMESPACE ring::
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ring-server-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ring-server-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring-server
)

install(
    EXPORT ring-server-targets
    FILE ring-server-targets.cmake
    NAMESPACE ring::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring-server
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ring-server-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ring-server-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ring-server
)
