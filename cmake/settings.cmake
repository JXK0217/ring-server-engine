# settings.cmake - Standard project settings

# set default build type to Release (if not specified)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)

    # Set possible build type options
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS 
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set install prefix (if not set)
if(NOT CMAKE_INSTALL_PREFIX)
    if(WIN32)
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/${PROJECT_NAME}" CACHE PATH "Installation directory" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory" FORCE)
    endif()
endif()

# set output directories for build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set output directories for different build types
foreach(CONFIG ${CMAKE_BUILD_TYPE})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
endforeach()

# Set default output name postfixes
set(CMAKE_DEBUG_POSTFIX "-d")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_MINSIZEREL_POSTFIX "-minsize")
set(CMAKE_RELWITHDEBINFO_POSTFIX "-reldeb")

# Set project-specific macro definitions
add_definitions(-DRING_PROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DRING_PROJECT_VERSION="${PROJECT_VERSION}")
