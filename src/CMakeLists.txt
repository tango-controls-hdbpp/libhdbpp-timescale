cmake_minimum_required(VERSION 3.6)

# source files
set(SRC_FILES ${SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeName.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeName.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AttributeTraits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HdbppTimescaleDb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LibUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DbConnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QueryBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PqxxExtension.cpp
    PARENT_SCOPE)