cmake_minimum_required(VERSION 3.6)
project(benchmark-tests)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

set(BENCHMARK_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBuilderTests.cpp)

add_executable(benchmark-tests ${BENCHMARK_SOURCES})
target_compile_options(benchmark-tests PRIVATE -Wall -Wextra -g)

target_link_libraries(benchmark-tests
    PRIVATE libhdbpp_headers libhdbpp_timescale_shared_library TangoInterfaceLibrary benchmark gtest)

target_include_directories(benchmark-tests
    PRIVATE ${CMAKE_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR})

target_compile_definitions(benchmark-tests
    PRIVATE -DDEBUG_ENABLED)

set_target_properties(benchmark-tests
    PROPERTIES 
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_CLANG_TIDY ${DO_CLANG_TIDY}
        CXX_STANDARD 14)