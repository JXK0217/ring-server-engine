# tests/units/core/CMakeLists.txt

if(BUILD_CORE_MODULE)
    add_executable(test_core
        test_core.cpp
    )

    target_link_libraries(test_core
        PRIVATE
            ring-server
            gmock
    )

    add_test(NAME test_core COMMAND test_core)

    set_tests_properties(test_core PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()