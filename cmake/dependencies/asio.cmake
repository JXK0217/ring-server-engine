# dependencies/asio.cmake - ASIO library configuration

# configure standalone asio
if(USE_ASIO_STANDALONE)
    # set asio path
    set(ASIO_SOURCE_DIR ${PROJECT_SOURCE_DIR}/third_party/asio)
    set(ASIO_INCLUDE_DIR ${ASIO_SOURCE_DIR}/asio/include)
    
    message(STATUS "Using standalone Asio from: ${ASIO_SOURCE_DIR}")

    # create interface library target
    add_library(ring-asio INTERFACE)

    # add include directories
    target_include_directories(ring-asio INTERFACE ${ASIO_INCLUDE_DIR})

    # add compile definitions (ensure standalone mode)
    target_compile_definitions(ring-asio INTERFACE ASIO_STANDALONE)

    # platform-specific settings
    if(WIN32)
        # windows specific settings
        target_compile_definitions(ring-asio INTERFACE 
            _WIN32_WINNT=0x0A00  # Windows 10 or later
        )
        target_link_libraries(ring-asio INTERFACE ws2_32 mswsock)
    else()
        # unix-like system settings
        target_link_libraries(ring-asio INTERFACE Threads::Threads)
        
        # check if additional libraries (such as rt) are needed
        if(CMAKE_SYSTEM_NAME MATCHES "Linux")
            target_link_libraries(ring-asio INTERFACE rt)
        endif()
    endif()
else()
    message(STATUS "Using Boost.Asio")

    find_package(Boost REQUIRED COMPONENTS system)

    # create interface library target
    add_library(asio INTERFACE)
    target_link_libraries(asio INTERFACE Boost::system)

    # add compile definitions (ensure Boost mode)
    target_compile_definitions(asio INTERFACE ASIO_DISABLE_STANDALONE)
endif()

# create alias
add_library(ring::asio ALIAS ring-asio)

# add to global dependencies
list(APPEND RING_DEPENDENCIES ring::asio)
set(RING_DEPENDENCIES ${RING_DEPENDENCIES} PARENT_SCOPE)
