# tests/units/logging/CMakeLists.txt

if(BUILD_LOGGING_MODULE)
    add_executable(test_logging
        test_logger.cpp
    )

    target_link_libraries(test_logging
        PRIVATE
            ring-logging
            gmock
    )

    add_test(NAME test_logging COMMAND test_logging)

    set_tests_properties(test_logging PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()