message(STATUS "Collecting core module")

# set core module sources
file(GLOB_RECURSE CORE_SOURCES
    *.cpp
    *.c
)

# add core module to enabled modules
list(APPEND RING_MODULES "CORE")
set(RING_MODULES ${RING_MODULES} PARENT_SCOPE)

# add core module sources to the monolithic library
set(CORE_SOURCES ${CORE_SOURCES} PARENT_SCOPE)
