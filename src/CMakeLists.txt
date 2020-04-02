cmake_minimum_required(VERSION 3.6)

# source files
set(LOCAL_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeName.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeName.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeTraits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTimescaleDbApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LibUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DbConnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PqxxExtension.cpp)

if(NOT BYPASS_LIBHDBPP)
    set(LOCAL_SRC_FILES ${LOCAL_SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTimescaleDb.cpp)
endif()

if(BYPASS_LIBHDBPP)
    set(LOCAL_SRC_FILES ${LOCAL_SRC_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/HdbClient.cpp)
endif()

set(SRC_FILES 
    ${SRC_FILES} 
    ${LOCAL_SRC_FILES} 
    PARENT_SCOPE)
