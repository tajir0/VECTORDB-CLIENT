cmake_minimum_required(VERSION 3.14)
project(vectorDB_client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 thirdparty 目录
add_subdirectory(thirdparty)

# 添加 src 目录
add_library(vector_store_lib
    src/vector_store.cpp
)

# 设置包含目录
target_include_directories(vector_store_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/installed/include/mongocxx/v_noabi
    ${CMAKE_SOURCE_DIR}/thirdparty/installed/include/bsoncxx/v_noabi
    ${CMAKE_SOURCE_DIR}/thirdparty/installed/include/libmongoc-1.0
    ${CMAKE_SOURCE_DIR}/thirdparty/installed/include/libbson-1.0
)

# 链接 MongoDB 依赖
target_link_libraries(vector_store_lib PUBLIC
    third_party_lib
)