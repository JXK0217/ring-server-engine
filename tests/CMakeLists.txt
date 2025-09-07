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
