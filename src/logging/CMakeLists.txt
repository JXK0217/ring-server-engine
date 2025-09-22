message(STATUS "Collecting logging module")

# set logging module sources
file(GLOB_RECURSE LOGGING_SOURCES
    *.cpp
    *.c
)

# add logging module to enabled modules
list(APPEND RING_MODULES "LOGGING")
set(RING_MODULES ${RING_MODULES} PARENT_SCOPE)

# add logging module sources to the monolithic library
set(LOGGING_SOURCES ${LOGGING_SOURCES} PARENT_SCOPE)
