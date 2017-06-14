cmake_minimum_required (VERSION 3.0)


# set(CMAKE_OSX_SYSROOT "macosx10.11")   # means current OS X


project (mulle-buffer)


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


add_definitions( -DMULLE_ABA_TRACE_SWAP
                 -DMULLE_ALLOCATOR_EXTERN_GLOBAL=extern)


# using include_directories is a little bit shitty
include_directories( src
)

include_directories( SYSTEM
dependencies/include
)

set( HEADERS
src/_mulle_buffer.h
src/mulle_buffer.h
)


set( SOURCES
src/_mulle_buffer.c
src/mulle_buffer.c
)


add_library(mulle_buffer STATIC
${SOURCES}
)

INSTALL( FILES ${HEADERS} DESTINATION "include/mulle_buffer")
INSTALL( TARGETS mulle_buffer DESTINATION "lib")

#
# standalone shared library (for tests)
#

if( MSVC)
  set( DEF_FILE mulle_buffer_standalone.def)
  set_source_files_properties( ${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
  set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEF:${DEF_FILE}")
  # /NODEFAULTLIB:MSVCRT")
endif()


add_library( mulle_buffer_standalone SHARED
src/mulle_buffer_standalone.c
${DEF_FILE}
)

add_dependencies( mulle_buffer_standalone mulle_buffer)


find_library( MULLE_ALLOCATOR_LIBRARY      NAMES mulle_allocator)
find_library( MULLE_TEST_ALLOCATOR_LIBRARY NAMES mulle_test_allocator)
find_library( MULLE_THREAD_LIBRARY         NAMES mulle_thread)


set( DEPENDENCY_LIBRARIES
${MULLE_ALLOCATOR_LIBRARY}
${MULLE_TEST_ALLOCATOR_LIBRARY}
${MULLE_THREAD_LIBRARY}
)


if( MSVC)
   add_custom_command( OUTPUT ${DEF_FILE}
                       COMMAND mulle-mingw-dumpdef.bat -o "${DEF_FILE}" --prefix "mulle" ${DEPENDENCY_LIBRARIES} $<TARGET_FILE:mulle_buffer>
                       DEPENDS "${DEPENDENCY_LIBRARIES}" mulle_buffer
                       VERBATIM)
endif()


add_dependencies( mulle_buffer_standalone mulle_buffer)

TARGET_LINK_LIBRARIES( mulle_buffer_standalone
${BEGIN_ALL_LOAD}
mulle_buffer
${DEPENDENCY_LIBRARIES}
${END_ALL_LOAD}
)


INSTALL( TARGETS mulle_buffer mulle_buffer_standalone DESTINATION "lib")
INSTALL( FILES ${HEADERS} DESTINATION "include/mulle_buffer")
