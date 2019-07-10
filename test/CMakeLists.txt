cmake_minimum_required(VERSION 3.6)
project(unit-tests)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

# Prepare "Catch" library for other executables
#set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch2)
#add_library(catch_library INTERFACE)
#target_include_directories(catch_library INTERFACE ${CATCH_INCLUDE_DIR})

# Make test executable
set(TEST_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeNameTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeTraitsTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ColumnCacheTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DbConnectionTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxBaseTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxDataEventTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxDataEventErrorTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxNewAttributeTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxHistoryEventTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTxParameterEventTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBuilderTests.cpp)

add_executable(unit-tests ${TEST_SOURCES})
target_compile_options(unit-tests PRIVATE -Wall -Wextra -g)

target_link_libraries(unit-tests 
    PRIVATE libhdbpp_headers libhdbpp_timescale_shared_library Catch2 TangoInterfaceLibrary)

target_include_directories(unit-tests PRIVATE ${UNIT_TEST_INCLUDE_PATHS})
target_compile_definitions(unit-tests PRIVATE -DDEBUG_ENABLED)
#target_compile_features(libhdbpp_timescale_shared_library PRIVATE cxx_std_17)

set_target_properties(unit-tests 
    PROPERTIES 
        LINK_FLAGS "-Wl,--no-undefined"
        CXX_CLANG_TIDY ${DO_CLANG_TIDY}
        CXX_STANDARD 14)