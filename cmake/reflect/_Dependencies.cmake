# This file will be regenerated by `mulle-sourcetree-to-cmake` via
# `mulle-sde reflect` and any edits will be lost.
#
# This file will be included by cmake/share/Files.cmake
#
# Disable generation of this file with:
#
# mulle-sde environment set MULLE_SOURCETREE_TO_CMAKE_DEPENDENCIES_FILE DISABLE
#
if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# Generated from sourcetree: B8174030-DEEE-41DE-BACD-D93F25A0315A;mulle-allocator;no-all-load,no-cmake-inherit,no-import,no-singlephase;
# Disable with : `mulle-sourcetree mark mulle-allocator no-link`
# Disable for this platform: `mulle-sourcetree mark mulle-allocator no-cmake-platform-${MULLE_UNAME}`
# Disable for a sdk: `mulle-sourcetree mark mulle-allocator no-cmake-sdk-<name>`
#
if( NOT MULLE_ALLOCATOR_LIBRARY)
   find_library( MULLE_ALLOCATOR_LIBRARY NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}mulle-allocator${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_PREFIX}mulle-allocator${CMAKE_STATIC_LIBRARY_SUFFIX} mulle-allocator NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
   message( STATUS "MULLE_ALLOCATOR_LIBRARY is ${MULLE_ALLOCATOR_LIBRARY}")
   #
   # The order looks ascending, but due to the way this file is read
   # it ends up being descending, which is what we need.
   #
   if( MULLE_ALLOCATOR_LIBRARY)
      #
      # Add MULLE_ALLOCATOR_LIBRARY to DEPENDENCY_LIBRARIES list.
      # Disable with: `mulle-sourcetree mark mulle-allocator no-cmake-add`
      #
      set( DEPENDENCY_LIBRARIES
         ${DEPENDENCY_LIBRARIES}
         ${MULLE_ALLOCATOR_LIBRARY}
         CACHE INTERNAL "need to cache this"
      )
      # intentionally left blank
   else()
      # Disable with: `mulle-sourcetree mark mulle-allocator no-require-link`
      message( FATAL_ERROR "MULLE_ALLOCATOR_LIBRARY was not found")
   endif()
endif()


#
# Generated from sourcetree: 1B1CB2BA-0620-4D84-AC63-CF78411EEF1F;mulle-data;no-all-load,no-cmake-inherit,no-import,no-singlephase;
# Disable with : `mulle-sourcetree mark mulle-data no-link`
# Disable for this platform: `mulle-sourcetree mark mulle-data no-cmake-platform-${MULLE_UNAME}`
# Disable for a sdk: `mulle-sourcetree mark mulle-data no-cmake-sdk-<name>`
#
if( NOT MULLE_DATA_LIBRARY)
   find_library( MULLE_DATA_LIBRARY NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}mulle-data${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_STATIC_LIBRARY_PREFIX}mulle-data${CMAKE_STATIC_LIBRARY_SUFFIX} mulle-data NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
   message( STATUS "MULLE_DATA_LIBRARY is ${MULLE_DATA_LIBRARY}")
   #
   # The order looks ascending, but due to the way this file is read
   # it ends up being descending, which is what we need.
   #
   if( MULLE_DATA_LIBRARY)
      #
      # Add MULLE_DATA_LIBRARY to DEPENDENCY_LIBRARIES list.
      # Disable with: `mulle-sourcetree mark mulle-data no-cmake-add`
      #
      set( DEPENDENCY_LIBRARIES
         ${DEPENDENCY_LIBRARIES}
         ${MULLE_DATA_LIBRARY}
         CACHE INTERNAL "need to cache this"
      )
      # intentionally left blank
   else()
      # Disable with: `mulle-sourcetree mark mulle-data no-require-link`
      message( FATAL_ERROR "MULLE_DATA_LIBRARY was not found")
   endif()
endif()
