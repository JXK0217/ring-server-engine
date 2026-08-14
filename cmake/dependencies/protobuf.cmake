# dependencies/protobuf.cmake - CMake configuration for protobuf dependency

if(NOT USE_SYSTEM_PROTOBUF)
    message(STATUS "Using bundled protobuf library")

    add_compile_options(-w)

    set(protobuf_BUILD_PROTOC_BINARIES OFF)
    set(protobuf_BUILD_LIBUPB OFF)

    # add protobuf as a subdirectory
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/protobuf)

    add_library(ring-protobuf INTERFACE)
    add_library(ring::protobuf ALIAS ring-protobuf)

    set_target_properties(ring-protobuf PROPERTIES EXCLUDE_FROM_ALL TRUE)

    get_target_property(_proto_include_dirs protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(ring-protobuf SYSTEM INTERFACE ${_proto_include_dirs})

    target_link_libraries(ring-protobuf INTERFACE protobuf::libprotobuf)
else()
    message(STATUS "Using system-installed protobuf library")

    # find protobuf package
    find_package(protobuf REQUIRED)

    add_library(ring::protobuf INTERFACE)
    target_link_libraries(ring::protobuf INTERFACE protobuf)
endif()

# add protobuf to the list of dependencies
list(APPEND RING_DEPENDENCIES ring::protobuf)
set(RING_DEPENDENCIES ${RING_DEPENDENCIES} PARENT_SCOPE)