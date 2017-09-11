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
include( CMakeSourcesAndHeaders.txt)


find_library( MULLE_ALLOCATOR_LIBRARY      NAMES mulle_allocator)
message( STATUS "MULLE_ALLOCATOR_LIBRARY is ${MULLE_ALLOCATOR_LIBRARY}")
find_library( MULLE_TEST_ALLOCATOR_LIBRARY NAMES mulle_test_allocator)
message( STATUS "MULLE_TEST_ALLOCATOR_LIBRARY is ${MULLE_TEST_ALLOCATOR_LIBRARY}")
find_library( MULLE_THREAD_LIBRARY         NAMES mulle_thread)
message( STATUS "MULLE_THREAD_LIBRARY is ${MULLE_THREAD_LIBRARY}")


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

set( TARGET_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle_buffer>
${DEPENDENCY_LIBRARIES}
)

include( Standalone)

INSTALL( TARGETS mulle_buffer mulle_buffer_standalone DESTINATION "lib")
INSTALL( FILES ${HEADERS} DESTINATION "include/mulle_buffer")
