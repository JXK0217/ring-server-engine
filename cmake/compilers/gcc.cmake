# compilers/gcc.cmake - GNU Compiler Collection (GCC) specific settings

message(STATUS "Configuring for GCC compiler")

# set custom macros
set(RING_COMPILER_GCC ON)
add_definitions(-DRING_COMPILER_GCC)

# add GCC specific compiler flags
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
