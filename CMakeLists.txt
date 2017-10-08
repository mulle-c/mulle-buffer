cmake_minimum_required( VERSION 3.0)

project( mulle-buffer C)


#
# which mulle-configuration to use ?
#
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mulle-configuration/cmake)
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()


# using include_directories is a little bit shitty
set( MULLE_BUFFER_LIBRARY ON)

include( CMakeSourcesAndHeaders.txt)
include( CMakeDependencies.txt)

include_directories( ${INCLUDE_DIRS})


set( PUBLIC_HEADERS
${HEADERS}
CMakeDependencies.txt
)


set( DEPENDENCY_LIBRARIES
${MULLE_ALLOCATOR_LIBRARY}
${MULLE_TEST_ALLOCATOR_LIBRARY}
${MULLE_THREAD_LIBRARY}
)


###
#
#
add_library( mulle_buffer STATIC
${SOURCES}
)

set( STANDALONE_BASE_NAME "mulle_buffer")
set( STANDALONE_NAME "mulle_buffer_standalone")
set( STANDALONE_DEFINITIONS ${MULLE_BUFFER_DEFINITIONS})
set( STANDALONE_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle_buffer>
${DEPENDENCY_LIBRARIES}
)

include( Standalone)

INSTALL( TARGETS mulle_buffer mulle_buffer_standalone DESTINATION "lib")
INSTALL( FILES ${PUBLIC_HEADERS} DESTINATION "include/mulle_buffer")

include( MulleTest)
