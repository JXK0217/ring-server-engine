# compilers/clang.cmake - Clang specific settings

message(STATUS "Configuring for Clang compiler")

# set custom macros
set(RING_COMPILER_CLANG ON)
add_definitions(-DRING_COMPILER_CLANG)

# add Clang specific compiler flags
add_compile_options(-Wall -Wextra -Werror)  # Enable all warnings and treat them as errors
add_compile_options(-Wpedantic)  # Enable warnings for unused parameters

# add fpic option for shared libraries
if(BUILD_SHARED_LIBS)
    add_compile_options(-fPIC)  # Position-independent code for shared libraries
endif()

# Set compiler visibility settings (for shared libraries)
if(BUILD_SHARED_LIBS)
    add_compile_options(-fvisibility=hidden)
    add_compile_options(-fvisibility-inlines-hidden)
endif()
