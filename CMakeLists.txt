# Functions and Pre-build  -----------------------------------

# Stop messy in source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

# Start Build Config -----------------------------------
cmake_minimum_required(VERSION 3.11)
set(CMAKE_SKIP_RPATH true)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

# Library output name for the final binaries
set(LIBHDBPP_TIMESCALE_NAME "libhdb++timescale")

# Versioning
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_PATCH "0")
set(VERSION_METADATA "")
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# Add any include paths from the command line
list(APPEND INCLUDE_PATHS ${CMAKE_INCLUDE_PATH})
list(APPEND INCLUDE_PATHS ${CMAKE_SOURCE_DIR})
list(APPEND LIBRARY_PATHS ${CMAKE_LIBRARY_PATH})

# Start the project
project(libhdbpp_timescale VERSION ${VERSION_STRING} LANGUAGES CXX)

# Require this for the libpqxx build, also this project is c++14 minimum
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
set(FETCH_LIBHDBPP_TAG "exp-refactor" CACHE STRING "Libhdbpp branch/tag to clone 'master'")
option(FETCH_LIBHDBPP "Download and build using a local copy of libhdb++" ON)
option(FETCH_LIBHDBPP_TAG "When FETCH_LIBHDBPP is enabled, this is the tag fetch ('master')")
option(BUILD_UNIT_TESTS "Build unit tests" OFF)
option(BUILD_BENCHMARK_TESTS "Build benchmarking tests (Forces RELEASE build)" OFF)
option(ENABLE_CLANG "Enable clang code and layout analysis" OFF)

if(BUILD_UNIT_TESTS)
    message(STATUS "Unit tests will be built")
endif(BUILD_UNIT_TESTS)

if(BUILD_BENCHMARK_TESTS)
    message(STATUS "Benchmark tests will be built (Forces RELEASE build)")
    set(CMAKE_BUILD_TYPE "Release")
endif(BUILD_BENCHMARK_TESTS)

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

# First find tango if it has not already been found. Returns an interface library
# called TangoInterfaceLibrary
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Tango)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Attempt to find the various libraries the project is dependent on
if(TDB_LIBRARIES)
    find_libraries(LIBRARIES ${TDB_LIBRARIES} SEARCH_PATHS ${LIBRARY_PATHS})
    set(TDB_FOUND_LIBRARIES ${FOUND_LIBRARIES})
endif(TDB_LIBRARIES)

# Thirdparty Integration -----------------------------------

# build google benchmark (target: benchmark)
# do not build tests of benchmarking lib
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory(thirdparty/google/benchmark EXCLUDE_FROM_ALL)

# build tests (targets: gtest_main, gtest)
add_subdirectory(thirdparty/google/googletest/googletest EXCLUDE_FROM_ALL)

# Include the thirdparty projects
add_subdirectory(thirdparty/libpqxx  EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/spdlog  EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/Catch2  EXCLUDE_FROM_ALL)

# For ease of use, we set the libpqxx output to our projects build directory,
# then if we build libpqxx as a shared library its easy to find 
set_target_properties(pqxx_shared
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Currently we link libpqxx into our library as a static object, to ease 
# deployment, so we make this small change here.
set_target_properties(pqxx_static
    PROPERTIES
    POSITION_INDEPENDENT_CODE 1)

# Code Analysis -----------------------------------
if(ENABLE_CLANG)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # To find clang, find_program will search your PATH environment variable.
    # Ensure if you have a non-standard clang install, that it has been added
    # to your path.
    find_program(CLANG_TIDY_EXE 
        NAMES "clang-tidy" 
        DOC "Path to clang-tidy executable")

    if(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy not found.")
    else(NOT CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif(NOT CLANG_TIDY_EXE)
endif(ENABLE_CLANG)

# Source -----------------------------------

add_subdirectory(src)

# Build Targets  -----------------------------------

# Shared library --------
add_library(libhdbpp_timescale_shared_library SHARED ${SRC_FILES})

target_link_libraries(libhdbpp_timescale_shared_library 
    PUBLIC ${TDB_FOUND_LIBRARIES} pqxx_static spdlog::spdlog_header_only Threads::Threads
    PRIVATE TangoInterfaceLibrary)

target_include_directories(libhdbpp_timescale_shared_library 
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        ${INCLUDE_PATHS}
        "${PROJECT_BINARY_DIR}")

set_target_properties(libhdbpp_timescale_shared_library 
    PROPERTIES 
        OUTPUT_NAME hdb++timescale
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE 1
        VERSION ${VERSION_STRING}
        SOVERSION ${VERSION_MAJOR})

if(DO_CLANG_TIDY)
    set_target_properties(libhdbpp_timescale_shared_library 
        PROPERTIES 
            CXX_CLANG_TIDY ${DO_CLANG_TIDY})
endif(DO_CLANG_TIDY)

target_compile_options(libhdbpp_timescale_shared_library 
    PRIVATE "$<$<CONFIG:DEBUG>:-g>")

# Static library --------
add_library(libhdbpp_timescale_static_library STATIC EXCLUDE_FROM_ALL ${SRC_FILES})

target_link_libraries(libhdbpp_timescale_static_library 
    PUBLIC ${TDB_FOUND_LIBRARIES} pqxx_static spdlog Threads::Threads
    PRIVATE TangoInterfaceLibrary)

target_include_directories(libhdbpp_timescale_static_library 
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        ${INCLUDE_PATHS}
        "${PROJECT_BINARY_DIR}")


set_target_properties(libhdbpp_timescale_static_library 
    PROPERTIES 
        OUTPUT_NAME hdb++timescale
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        EXCLUDE_FROM_ALL 1)

target_compile_options(libhdbpp_timescale_static_library 
    PRIVATE "$<$<CONFIG:DEBUG>:-g>")

# Install Config -----------------------------------
install(
    TARGETS libhdbpp_timescale_shared_library
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
    PATTERN HdbppTimescaleDb.hpp)

# Tests -----------------------------------
if(BUILD_UNIT_TESTS)
    add_subdirectory(test)
endif(BUILD_UNIT_TESTS)

if(BUILD_BENCHMARK_TESTS)
    add_subdirectory(benchmark)
endif(BUILD_BENCHMARK_TESTS)
