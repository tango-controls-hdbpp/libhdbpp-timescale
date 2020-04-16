# Stop messy in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

# Start Build Config -----------------------------------
cmake_minimum_required(VERSION 3.0)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE   ON)

project(libhdbpp)

# We only support Release for now
#set(CMAKE_BUILD_TYPE "Release")

include(cmake/ReleaseVersion.cmake)

# arch install definitions
include(GNUInstallDirs)

# Create a list of the source files for this build to be given to
# the target later
set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/LibHdb++.cpp)

# Set the output directory to lib to stay consistent with the old build
set(OUTPUT_DIR "lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${OUTPUT_DIR})

# set up a configuration file to pass variables into the build
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/LibHdb++Config.h.in"
    "${PROJECT_BINARY_DIR}/LibHdb++Config.h")

message(STATUS "Searching for libraries...")

# Variable to contain a list of all the libs we depend on
set(HDBPP_LIBRARIES dl) 

# allow pkg-config to search the CMAKE_PREFIX_PATH 
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
list(APPEND CMAKE_PREFIX_PATH "/usr")

# Find Dependencies  ---------------------
include(cmake/FindLibraries.cmake)

# Find any libraries the user requested
if(HDBPP_LIBRARIES)
    find_libraries(LIBRARIES ${HDBPP_LIBRARIES})
    set(HDBPP_FOUND_LIBRARIES ${FOUND_LIBRARIES})
endif(HDBPP_LIBRARIES)

# First find tango if it has not already been found. Returns an interface library
# called TangoInterfaceLibrary
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Tango)

# Build Targets  -----------------------------------

# Libhdbpp header library --------
add_library(libhdbpp_headers INTERFACE)

target_include_directories(libhdbpp_headers 
    INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Libhdbpp shared library --------
add_library(libhdbpp_shared_library SHARED ${SRC_FILES})

target_link_libraries(libhdbpp_shared_library 
    PUBLIC ${HDBPP_FOUND_LIBRARIES} 
    PRIVATE TangoInterfaceLibrary)

target_include_directories(libhdbpp_shared_library 
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE 
        "${PROJECT_BINARY_DIR}")

set_target_properties(libhdbpp_shared_library 
    PROPERTIES 
        OUTPUT_NAME hdb++
        LINK_FLAGS "-Wl,--no-undefined"
        POSITION_INDEPENDENT_CODE 1
        VERSION ${LIBRARY_VERSION_STRING}
        SOVERSION ${LIBRARY_VERSION_MAJOR})

# Libhdbpp shared library --------
add_library(libhdbpp_static_library STATIC ${SRC_FILES})

target_link_libraries(libhdbpp_static_library 
    PUBLIC ${HDBPP_FOUND_LIBRARIES} 
    PRIVATE TangoInterfaceLibrary)

set_target_properties(libhdbpp_static_library 
    PROPERTIES 
        OUTPUT_NAME hdb++
        LINK_FLAGS "-Wl,--no-undefined"
        EXCLUDE_FROM_ALL 1)

# Install Config -----------------------------------
install(TARGETS libhdbpp_shared_library
    EXPORT libhdbpp-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT libhdbpp-export
    FILE LibhdbppTargets.cmake
    NAMESPACE LIBHDBPP::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Libhdbpp)

install(DIRECTORY include/libhdb++ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
