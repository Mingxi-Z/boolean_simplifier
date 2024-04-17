cmake_minimum_required(VERSION 3.15)
project(boolean_simplify)

set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
set(DCMAKE_TOOLCHAIN_FILE "conan_toolchain.cmake")

find_package(Boost)
find_package(highs)

add_executable(boolean_simplify main.cpp)

target_link_libraries(${PROJECT_NAME} boost::boost)
target_link_libraries(${PROJECT_NAME} highs::highs)