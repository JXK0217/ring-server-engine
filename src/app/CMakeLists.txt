message(STATUS "Collecting app module")

# set app module sources
file(GLOB_RECURSE APP_SOURCES
    *.cpp
    *.c
)

# add app module to enabled modules
list(APPEND RING_MODULES "APP")
set(RING_MODULES ${RING_MODULES} PARENT_SCOPE)

# add core module sources to the monolithic library
set(APP_SOURCES ${APP_SOURCES} PARENT_SCOPE)
