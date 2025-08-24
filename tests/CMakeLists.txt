# tests/CMakeLists.txt - build configuration for tests

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    ${CMAKE_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
)

# add test units
add_subdirectory(units)

# add test integration
add_subdirectory(integration)

# add test performance
add_subdirectory(performance)

# add test fixtures
add_subdirectory(fixtures)

add_executable(test_runner
    main.cpp
)

if(TARGET test_logging)
    add_dependencies(test_runner test_logging)
endif()

target_link_libraries(test_runner
    PRIVATE
        gtest
        gmock
)

add_test(NAME test_runner COMMAND test_runner)

set_tests_properties(test_runner PROPERTIES
    TIMEOUT 300
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
