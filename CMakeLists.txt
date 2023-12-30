# cmake version required
cmake_minimum_required(VERSION 3.25)

project(test)

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# build mlif static library
add_library(mlifio STATIC ${INC_DIR}/mlifio.h ${SRC_DIR}/mlifio.c)
target_include_directories(mlifio PUBLIC ${INC_DIR})

add_executable(test test.c)
target_link_libraries(test PUBLIC mlifio)
