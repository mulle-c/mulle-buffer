cmake_minimum_required( VERSION 3.0)

project( mulle-buffer C)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)


### Files

include( PreFiles OPTIONAL)

set( MULLE_BUFFER_LIBRARY ON)

#
# Include all non underscored .cmake files in ./cmake
#
file( GLOB LOCAL_CMAKE_INCLUDES
   "cmake/[A-Za-z0-9]*.cmake"
)

foreach( FILE ${LOCAL_CMAKE_INCLUDES} )
   include( "${FILE}")
endforeach()


include_directories( ${INCLUDE_DIRS})

include( PostFiles OPTIONAL)

### Library

include( PreLibrary OPTIONAL)

add_library( "mulle-buffer"
${SOURCES}
${PUBLIC_HEADERS}
${PRIVATE_HEADERS}
${CMAKE_EDITABLE_FILES}
)

if( BUILD_SHARED_LIBS)
   target_link_libraries( mulle-buffer
${DEPENDENCY_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)
endif()

include( PostLibrary OPTIONAL)


set( STANDALONE_BASE_NAME "mulle-buffer")
set( STANDALONE_NAME "mulle-buffer-standalone")
set( STANDALONE_DEFINITIONS ${MULLE_BUFFER_DEFINITIONS})
set( STANDALONE_DEPENDENCIES
mulle-buffer
)

set( STANDALONE_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle-buffer>
${DEPENDENCY_LIBRARIES}
${OPTIONAL_DEPENDENCY_LIBRARIES}
)

set( STANDALONE_NORMAL_LOAD_LIBRARIES
${OS_SPECIFIC_LIBRARIES}
)

include( Standalone)

### Install

include( PreInstall OPTIONAL)

install( TARGETS "mulle-buffer" "mulle-buffer-standalone" DESTINATION "lib")
install( FILES ${PUBLIC_HEADERS}  DESTINATION "include/mulle-buffer")
install( FILES ${PRIVATE_HEADERS} DESTINATION "include/mulle-buffer/private")

install( FILES ${CMAKE_INCLUDES}  DESTINATION "include/mulle-buffer/cmake")

include( PostInstall OPTIONAL)
