cmake_minimum_required(VERSION 3.12)
project(sockery C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-g -Wall -Wextra)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>4.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GTK4)
