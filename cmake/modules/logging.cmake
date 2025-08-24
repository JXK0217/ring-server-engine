# modules/logging.cmake - Logging module CMake configuration

if(BUILD_LOGGING_MODULE)
    message(STATUS "Building logging module")

    # set logging module sources
    file(GLOB_RECURSE LOGGING_SOURCES
        src/logging/*.cpp
        src/logging/*.c
    )

    # build logging module library
    if(BUILD_MODULAR_LIBS)
        add_library(ring-logging)

        target_sources(ring-logging PRIVATE ${LOGGING_SOURCES})

        # set include directories for the logging module
        target_include_directories(ring-logging 
            PUBLIC 
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            )

        # set compiler properties for the logging module
        set_target_properties(ring-logging PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            VERSION ${RING_SERVER_VERSION}
            SOVERSION ${RING_SERVER_VERSION_MAJOR}
            OUTPUT_NAME "ring-logging"
        )

        target_compile_options(ring-logging PRIVATE ${COMPILER_WARNINGS})

        # link dependencies
        target_link_libraries(ring-logging PRIVATE ${RING_DEPENDENCIES})
        # add logging module to enabled modules
        list(APPEND ENABLED_MODULES "ring-logging")

        # create an alias for the logging module
        add_library(ring::logging ALIAS ring-logging)
    endif()

    # add logging module sources to the monolithic library
    set(LOGGING_SOURCES ${LOGGING_SOURCES})
endif()