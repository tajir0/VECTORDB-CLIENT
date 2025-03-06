cmake_minimum_required(VERSION 3.14)
project(vector_store_test)

# 启用 C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置库文件路径
set(VECTOR_STORE_LIB ${CMAKE_SOURCE_DIR}/../build/libvector_store_lib.a)

# 添加测试可执行文件
add_executable(vector_store_test
    test_vector_store.cpp
)

# 设置包含目录
target_include_directories(vector_store_test PRIVATE
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/include/mongocxx/v_noabi
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/include/bsoncxx/v_noabi
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/include/libmongoc-1.0
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/include/libbson-1.0
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/include
)

# 设置库文件路径
link_directories(
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib
)

# 链接测试库
target_link_libraries(vector_store_test PRIVATE
    ${VECTOR_STORE_LIB}
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libgtest.a
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libgtest_main.a
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libmongocxx-static.a
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libbsoncxx-static.a
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libmongoc-static-1.0.a
    ${CMAKE_SOURCE_DIR}/../thirdparty/installed/lib/libbson-static-1.0.a
    "-framework CoreFoundation"
    "-framework Security"
    "-framework LDAP"
    "-framework GSS"
    "-framework SystemConfiguration"
    pthread
    resolv
    sasl2
)

# 添加测试
add_test(NAME vector_store_test COMMAND vector_store_test)