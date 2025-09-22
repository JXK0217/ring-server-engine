# dependencies/spdlog.cmake - CMake configuration for spdlog dependency

if(NOT USE_SYSTEM_SPDLOG)
    message(STATUS "Using bundled spdlog library")

    # add spdlog as a subdirectory
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/spdlog)

    add_library(ring-spdlog INTERFACE)
    add_library(ring::spdlog ALIAS ring-spdlog)

    set_target_properties(ring-spdlog PROPERTIES EXCLUDE_FROM_ALL TRUE)
    
    if(TARGET spdlog)
        target_link_libraries(ring-spdlog INTERFACE spdlog)
    else()
        message(FATAL_ERROR "spdlog target not found. Ensure spdlog is correctly added as a subdirectory.")
    endif()
else()
    message(STATUS "Using system-installed spdlog library")

    # find spdlog package
    find_package(spdlog REQUIRED)

    add_library(ring::spdlog INTERFACE)
    target_link_libraries(ring::spdlog INTERFACE spdlog::spdlog)
endif()

# add spdlog to the list of dependencies
list(APPEND RING_DEPENDENCIES ring::spdlog)
set(RING_DEPENDENCIES ${RING_DEPENDENCIES} PARENT_SCOPE)

if(NOT SPDLOG_FMT_EXTERNAL AND TARGET fmt)
    list(APPEND RING_DEPENDENCIES fmt::fmt)
    set(RING_DEPENDENCIES ${RING_DEPENDENCIES} PARENT_SCOPE)
endif()
