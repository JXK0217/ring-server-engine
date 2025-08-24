# tests/unit/CMakeLists.txt - build configuration for unit tests

if(BUILD_CORE_MODULE)
    add_subdirectory(core)
endif()

if(BUILD_LOGGING_MODULE)
    add_subdirectory(logging)
endif()

if(BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif()
