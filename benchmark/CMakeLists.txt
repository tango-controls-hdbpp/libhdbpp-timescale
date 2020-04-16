cmake_minimum_required(VERSION 3.6)
project(benchmark-tests)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

add_executable(query-builder-tests ${CMAKE_CURRENT_SOURCE_DIR}/QueryBuilderTests.cpp)
target_compile_options(query-builder-tests PRIVATE -Wall -Wextra -g)

target_link_libraries(query-builder-tests
    PRIVATE 
        libhdbpp_timescale_static_library 
        TangoInterfaceLibrary 
        benchmark 
        benchmark_main 
        gtest
        test-utils)

target_include_directories(query-builder-tests
    PRIVATE ${CMAKE_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR})

target_compile_definitions(query-builder-tests
    PRIVATE -DDEBUG_ENABLED)

set_target_properties(query-builder-tests
    PROPERTIES 
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14)

if(DO_CLANG_TIDY)
    set_target_properties(query-builder-tests
        PROPERTIES 
        CXX_CLANG_TIDY ${DO_CLANG_TIDY})
endif(DO_CLANG_TIDY)

add_executable(db-insert-tests ${CMAKE_CURRENT_SOURCE_DIR}/DbInsertionTests.cpp)
target_compile_options(db-insert-tests PRIVATE -Wall -Wextra -g)

target_link_libraries(db-insert-tests
    PRIVATE 
        libhdbpp_timescale_static_library 
        TangoInterfaceLibrary 
        benchmark 
        benchmark_main 
        gtest
        test-utils)

target_include_directories(db-insert-tests
    PRIVATE ${CMAKE_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR})

target_compile_definitions(db-insert-tests
    PRIVATE -DDEBUG_ENABLED)

set_target_properties(db-insert-tests
    PROPERTIES 
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_STANDARD 14)

if(DO_CLANG_TIDY)
    set_target_properties(db-insert-tests
        PROPERTIES 
        CXX_CLANG_TIDY ${DO_CLANG_TIDY})
endif(DO_CLANG_TIDY)
