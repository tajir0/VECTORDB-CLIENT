cmake_minimum_required(VERSION 3.10)
project(third_party_lib)

add_library(third_party_lib INTERFACE)

set(STATIC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/installed/lib)

set(LIB_LIST
    gtest
    mongoc-static-1.0
    mongocxx-static
)

foreach(lib_name IN LISTS LIB_LIST)
    set(lib_path ${STATIC_LIB_DIR}/lib${lib_name}.a)
    message(STATUS "Importing ${lib_name} ${lib_path}")
    add_library(${lib_name} STATIC IMPORTED)
    set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION ${lib_path})

    target_link_libraries(third_party_lib INTERFACE ${lib_name})
endforeach()

set(THIRD_PARTY_LIB ${THIRD_PARTY_LIB} third_party_lib PARENT_SCOPE)