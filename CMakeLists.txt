# Functions and Pre-build  -----------------------------------

# Stop messy in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

# Start Build Config -----------------------------------
cmake_minimum_required(VERSION 3.6)
set(CMAKE_SKIP_RPATH true)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

# Library output name
set(LIBHDBPP_TIMESCALE_NAME "libhdb++timescale")

# and version
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "0")
set(VERSION_METADATA "")
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

project(libhdbpp_timescale VERSION ${VERSION_STRING} LANGUAGES CXX)

# Require this for the libpqxx build
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(HDBPP_TDB_BUILD_TESTS "Build unit tests" ON)
option(HDBPP_TDB_BUILD_DEBUG_SYMBOLS "Build with debug symbol" OFF)
option(HDBPP_TDB_ENABLE_TRACING "Enable tracing on all function calls" OFF)
option(HDBPP_TDB_ENABLE_CLANG "Enable clang code and layout analysis" ON)

if(HDBPP_TDB_ENABLE_TRACING)
    message(STATUS "Enabled function tracing for libhdbpp-timescale")
endif(HDBPP_TDB_ENABLE_TRACING)

if(HDBPP_TDB_BUILD_TESTS)
    message(STATUS "Unit tests will be built")
endif(HDBPP_TDB_BUILD_TESTS)

if(HDBPP_TDB_LOG_TO_TANGO)
    message(STATUS "Enable logging redirect to tango logging system")
endif(HDBPP_TDB_LOG_TO_TANGO)

# arch install definitions
include(GNUInstallDirs)

message(STATUS "Searching for libraries...")

# Variable to contain a list of all the libs we depend on
set(TDB_LIBRARIES) 

# allow pkg-config to search the CMAKE_PREFIX_PATH 
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
list(APPEND CMAKE_PREFIX_PATH "/usr")

# Find Dependencies  ---------------------
include(cmake/FindLibraries.cmake)

# Find any libraries the user requested
if(TDB_LIBRARIES)
    find_libraries(LIBRARIES ${TDB_LIBRARIES} SEARCH_PATHS ${LIBRARY_PATHS})
    set(TDB_FOUND_LIBRARIES ${FOUND_LIBRARIES})
endif(TDB_LIBRARIES)

# First find tango if it has not already been found. Returns an interface library
# called TangoInterfaceLibrary
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Tango)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Include the thirdparty projects
add_subdirectory(thirdparty/libhdbpp EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/libpqxx  EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/spdlog  EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/Catch2  EXCLUDE_FROM_ALL)

set_target_properties(pqxx_shared
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

set_target_properties(pqxx_static
    PROPERTIES
    POSITION_INDEPENDENT_CODE 1)

# Code Analysis -----------------------------------
if(HDBPP_TDB_ENABLE_CLANG)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    find_program(CLANG_TIDY_EXE 
        NAMES "clang-tidy" 
        HINTS "/users/sjames/workspace/tools/clang/bin"
        DOC "Path to clang-tidy executable")

    if(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy not found.")
    else(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif(NOT CLANG_TIDY_EXE)
endif(HDBPP_TDB_ENABLE_CLANG)

# Sub-projects  -----------------------------------
add_subdirectory(src)

# Build Targets  -----------------------------------

# Shared library --------
add_library(libhdbpp_timescale_shared_library SHARED ${SRC_FILES})

target_link_libraries(libhdbpp_timescale_shared_library 
    PUBLIC ${TDB_FOUND_LIBRARIES} pqxx_static libhdbpp_headers spdlog Threads::Threads
    PRIVATE TangoInterfaceLibrary)

target_include_directories(libhdbpp_timescale_shared_library 
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE 
        "${PROJECT_BINARY_DIR}")

set_target_properties(libhdbpp_timescale_shared_library 
    PROPERTIES 
        OUTPUT_NAME hdb++timescale
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE 1
        CXX_CLANG_TIDY ${DO_CLANG_TIDY}
        VERSION ${VERSION_STRING}
        SOVERSION ${VERSION_STRING})

target_compile_options(libhdbpp_timescale_shared_library PRIVATE "$<$<CONFIG:DEBUG>:-g>")
#target_compile_features(libhdbpp_timescale_shared_library PRIVATE cxx_std_17)

# Libhdbpp shared library --------
add_library(libhdbpp_timescale_static_library STATIC ${SRC_FILES})

target_link_libraries(libhdbpp_timescale_static_library 
    PUBLIC ${TDB_FOUND_LIBRARIES} 
    PRIVATE TangoInterfaceLibrary)

set_target_properties(libhdbpp_timescale_static_library 
    PROPERTIES 
        OUTPUT_NAME hdb++
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        EXCLUDE_FROM_ALL 1)

target_compile_options(libhdbpp_timescale_static_library PRIVATE "$<$<CONFIG:DEBUG>:-g>")
#target_compile_features(libhdbpp_timescale_shared_library PRIVATE cxx_std_17)

# Install Config -----------------------------------
# install(TARGETS libhdbpp_timescale_shared_library
    #EXPORT libhdbpp-timescale-export
    #LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

#install(EXPORT libhdbpp-timescale-export
    #FILE LibhdbppTimeacaleTargets.cmake
    #NAMESPACE LIBHDBPP::
    #DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Libhdbpp)

#install(DIRECTORY include/libhdb++ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Tests -----------------------------------
#if(HDBPP_CASS_BUILD_TESTS)
    add_subdirectory(test)
#endif(HDBPP_CASS_BUILD_TESTS)