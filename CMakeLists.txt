# cmake version required
cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# build mlif static library
add_library(mlifio STATIC ${SRC_DIR}/mlifio.c)
target_include_directories(mlifio PUBLIC ${INC_DIR})

project(test)

add_executable(test ${SRC_DIR}/test.c)
target_link_libraries(test PUBLIC mlifio)
