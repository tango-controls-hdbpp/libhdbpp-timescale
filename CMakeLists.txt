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
set(VERSION_MAJOR "2")
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

# Find the libhdbpp headers
find_package(libhdbpp 2)

# If not installed then get them
if(NOT libhdbpp_FOUND)
    include(FetchContent)

    FetchContent_Declare(
      libhdbpp
      GIT_REPOSITORY https://github.com/tango-controls-hdbpp/libhdbpp.git
      GIT_TAG        master
    )

    FetchContent_GetProperties(libhdbpp)
    if(NOT libhdbpp_POPULATED)
        FetchContent_Populate(libhdbpp)
        add_subdirectory(${libhdbpp_SOURCE_DIR} ${libhdbpp_BINARY_DIR})
    endif()

    add_library(libhdbpp::libhdbpp_headers ALIAS libhdbpp_headers)

endif(NOT libhdbpp_FOUND)

# Thirdparty Integration -----------------------------------

# build google benchmark (target: benchmark)
# do not build tests of benchmarking lib
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory(thirdparty/google/benchmark EXCLUDE_FROM_ALL)

# build tests (targets: gtest_main, gtest)
add_subdirectory(thirdparty/google/googletest/googletest EXCLUDE_FROM_ALL)

# Include the thirdparty projects
#add_subdirectory(thirdparty/libpqxx  EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/spdlog  EXCLUDE_FROM_ALL)
set_property(TARGET spdlog PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(thirdparty/Catch2  EXCLUDE_FROM_ALL)

# Find the libpqxx package
find_package(libpqxx 6.4)

# If not installed then get them
if(NOT libpqxx_FOUND)
    include(FetchContent)

    FetchContent_Declare(
      libpqxx
      GIT_REPOSITORY https://github.com/jtv/libpqxx.git
      GIT_TAG        6.4.5
    )

    FetchContent_GetProperties(libpqxx)
    if(NOT libpqxx_POPULATED)
        FetchContent_Populate(libpqxx)
        add_subdirectory(${libpqxx_SOURCE_DIR} ${libpqxx_BINARY_DIR}
            EXCLUDE_FROM_ALL)
    endif()

    # Do not run the tests on the libpqxx build
    set(SKIP_BUILD_TEST ON)

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

endif(NOT libpqxx_FOUND)

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

# Export target name for subdirectory build
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(BACKEND_TARGET_SHARED "libhdbpp_timescale_shared_library" PARENT_SCOPE)
    set(BACKEND_TARGET_STATIC "libhdbpp_timescale_static_library" PARENT_SCOPE)
endif(hasParent)
set(BACKEND_TARGET_SHARED "libhdbpp_timescale_shared_library")
set(BACKEND_TARGET_STATIC "libhdbpp_timescale_static_library")

# Shared library --------
add_library(${BACKEND_TARGET_SHARED} SHARED ${SRC_FILES})

target_link_libraries(${BACKEND_TARGET_SHARED} 
    PUBLIC libhdbpp::libhdbpp_headers
    PRIVATE Threads::Threads pqxx_static spdlog::spdlog TangoInterfaceLibrary)

target_include_directories(${BACKEND_TARGET_SHARED} 
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        ${INCLUDE_PATHS}
        "${PROJECT_BINARY_DIR}")

set_target_properties(${BACKEND_TARGET_SHARED} 
    PROPERTIES 
        OUTPUT_NAME hdb++timescale
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        POSITION_INDEPENDENT_CODE 1
        VERSION ${VERSION_STRING}
        SOVERSION ${VERSION_MAJOR})

if(DO_CLANG_TIDY)
    set_target_properties(${BACKEND_TARGET_SHARED} 
        PROPERTIES 
            CXX_CLANG_TIDY ${DO_CLANG_TIDY})
endif(DO_CLANG_TIDY)

target_compile_options(${BACKEND_TARGET_SHARED} 
    PRIVATE "$<$<CONFIG:DEBUG>:-g>")

# Static library --------
add_library(${BACKEND_TARGET_STATIC} STATIC EXCLUDE_FROM_ALL ${SRC_FILES})

target_link_libraries(${BACKEND_TARGET_STATIC} 
    INTERFACE libhdbpp::libhdbpp_headers
    PUBLIC pqxx_static spdlog::spdlog Threads::Threads
    PRIVATE TangoInterfaceLibrary)

target_include_directories(${BACKEND_TARGET_STATIC} 
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE 
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        ${INCLUDE_PATHS}
        "${PROJECT_BINARY_DIR}")


set_target_properties(${BACKEND_TARGET_STATIC} 
    PROPERTIES 
        OUTPUT_NAME hdb++timescale
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14
        EXCLUDE_FROM_ALL 1)

target_compile_options(${BACKEND_TARGET_STATIC} 
    PRIVATE "$<$<CONFIG:DEBUG>:-g>")

# Install Config -----------------------------------
include(CMakePackageConfigHelpers)

install(
    TARGETS ${BACKEND_TARGET_SHARED}
    EXPORT libhdbpp-timescaleTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(ConfigPackageLocation lib/cmake/libhdbpp-timescale)
set(Namespace libhdbpp::)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/libhdbpp-timescale/libhdbpp-timescaleConfigVersion.cmake"
  VERSION ${VERSION_STRING}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT libhdbpp-timescaleTargets
  FILE
    "${CMAKE_CURRENT_BINARY_DIR}/libhdbpp-timescale/libhdbpp-timescaleTargets.cmake"
  NAMESPACE ${Namespace}
)


# generate the config file that is includes the exports
configure_package_config_file(cmake/libhdbpp-timescaleConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/libhdbpp-timescale/libhdbpp-timescaleConfig.cmake"
  INSTALL_DESTINATION
    ${ConfigPackageLocation}
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(EXPORT libhdbpp-timescaleTargets
  FILE
    libhdbpp-timescaleTargets.cmake
  NAMESPACE
    ${Namespace}
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/libhdbpp-timescale/libhdbpp-timescaleConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/libhdbpp-timescale/libhdbpp-timescaleConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)


# Tests -----------------------------------
if(BUILD_UNIT_TESTS)
    add_subdirectory(test)
endif(BUILD_UNIT_TESTS)

if(BUILD_BENCHMARK_TESTS)
    add_subdirectory(benchmark)
endif(BUILD_BENCHMARK_TESTS)
