# modules/core.cmake - Core module CMake configuration

if(BUILD_CORE_MODULE)
    message(STATUS "Building core module")

    # set core module sources
    file(GLOB_RECURSE CORE_SOURCES
        src/core/*.cpp
        src/core/*.c
    )

    # build core module library
    if(BUILD_MODULAR_LIBS)
        add_library(ring-core)

        target_sources(ring-core PRIVATE ${CORE_SOURCES})

        # set include directories for the core module
        target_include_directories(ring-core 
            PUBLIC 
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )

        # set compiler properties for the core module
        set_target_properties(ring-core PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            VERSION ${RING_SERVER_VERSION}
            SOVERSION ${RING_SERVER_VERSION_MAJOR}
            OUTPUT_NAME "ring-core"
        )

        target_compile_options(ring-core PRIVATE ${COMPILER_WARNINGS})

        # link dependencies
        target_link_libraries(ring-core PRIVATE ${RING_DEPENDENCIES})
        # add core module to enabled modules
        list(APPEND ENABLED_MODULES "ring-core")

        # create an alias for the core module
        add_library(ring::core ALIAS ring-core)
    endif()

    # add core module sources to the monolithic library
    set(CORE_SOURCES ${CORE_SOURCES})
endif()