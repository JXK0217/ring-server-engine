# tests/units/app/CMakeLists.txt

if(BUILD_APP_MODULE)
    add_executable(test_app
        test_app.cpp
    )

    target_link_libraries(test_app
        PRIVATE
            ring-server
            gmock
            compiler_flags
    )

    add_test(NAME test_app COMMAND test_app)

    set_tests_properties(test_app PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()