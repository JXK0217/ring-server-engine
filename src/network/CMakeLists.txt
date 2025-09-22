message(STATUS "Collecting network module")

# set network module sources
file(GLOB_RECURSE NETWORK_SOURCES
    *.cpp
    *.c
)

# add network module to enabled modules
list(APPEND RING_MODULES "NETWORK")
set(RING_MODULES ${RING_MODULES} PARENT_SCOPE)

# add network module sources to the monolithic library
set(NETWORK_SOURCES ${NETWORK_SOURCES} PARENT_SCOPE)
