# modules/network.cmake - Network module CMake configuration

if(BUILD_NETWORK_MODULE)
    message(STATUS "Building network module")

    # set network module sources
    file(GLOB_RECURSE NETWORK_SOURCES
        src/network/*.cpp
        src/network/*.c
    )

    # build network module library
    if(BUILD_MODULAR_LIBS)
        add_library(ring-network)

        target_sources(ring-network PRIVATE ${NETWORK_SOURCES})

        # set include directories for the network module
        target_include_directories(ring-network 
            PUBLIC 
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )

        # set compiler properties for the network module
        set_target_properties(ring-network PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            VERSION ${RING_SERVER_VERSION}
            SOVERSION ${RING_SERVER_VERSION_MAJOR}
            OUTPUT_NAME "ring-network"
        )

        target_compile_options(ring-network PRIVATE ${COMPILER_WARNINGS})

        # link dependencies
        target_link_libraries(ring-network PRIVATE ${RING_DEPENDENCIES})
        
        # add network module to enabled modules
        list(APPEND ENABLED_MODULES "ring-network")

        # create an alias for the network module
        add_library(ring::network ALIAS ring-network)
        target_precompile_headers(ring-network PRIVATE ${CMAKE_SOURCE_DIR}/include/ring/pch.h)
    endif()

    # add network module sources to the monolithic library
    set(NETWORK_SOURCES ${NETWORK_SOURCES})
endif()