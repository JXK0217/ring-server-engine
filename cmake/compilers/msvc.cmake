# compilers/msvc.cmake - MSVC specific settings

message(STATUS "Configuring for MSVC compiler")

# set custom macros
set(RING_COMPILER_MSVC ON)
add_definitions(-DRING_COMPILER_MSVC)

# add MSVC specific compiler flags
add_compile_options(/W4 /WX)  # Enable warnings and treat them as errors
add_compile_options(/MP)  # Enable multi-processor compilation
add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # Disable secure warnings
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)  # Disable deprecated warnings for Winsock
add_definitions(-D_WIN32_WINNT=0x0601)  # Set minimum Windows version to Windows 7
add_definitions(-DNTDDI_VERSION=0x0601)  # Set NTDLL version to Windows 7

# Set character set to Unicode
add_definitions(-DUNICODE -D_UNICODE)
