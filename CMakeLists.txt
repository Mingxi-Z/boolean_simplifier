cmake_minimum_required(VERSION 3.15)
project(boolean_simplify)

set(CMAKE_CXX_STANDARD 17)
set(DCMAKE_TOOLCHAIN_FILE "conan_toolchain.cmake")

find_package(Boost)
find_package(highs)

add_executable(boolean_simplify main.cpp)
target_sources(boolean_simplify PRIVATE BoolSimplifier.hpp)
target_sources(boolean_simplify PRIVATE BoolSimplifier.cpp)


target_include_directories(boolean_simplify PRIVATE MCDC/mcdc/Include)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/ast.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/cloption.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/codegenerator.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/coverage.cpp)

target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/mcdc.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/mintermcalculator.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/parser.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/petrick.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/quinemccluskey.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/scanner.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/token.cpp)
target_sources(boolean_simplify PRIVATE MCDC/mcdc/Source/virtualmachine.cpp)

target_sources(boolean_simplify PRIVATE minbool/minbool.h)

target_link_libraries(${PROJECT_NAME} boost::boost)
target_link_libraries(${PROJECT_NAME} highs::highs)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -g")