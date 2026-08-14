# tests/units/network/CMakeLists.txt

if(BUILD_NETWORK_MODULE)
    add_executable(test_network
        test_network.cpp
    )

    target_link_libraries(test_network
        PRIVATE
            ring-server
            gmock
            compiler_flags
    )

    add_test(NAME test_network COMMAND test_network)

    set_tests_properties(test_network PROPERTIES
        TIMEOUT 60
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()