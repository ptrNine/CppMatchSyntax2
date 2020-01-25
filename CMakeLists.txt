cmake_minimum_required(VERSION 3.14)
project(CppMatchSyntax2)

set(CMAKE_CXX_STANDARD 17)

if (BUILD_TESTS)
    add_executable(tests tests.cpp)
endif()

if (BUILD_EXAMPLES)
    add_executable(examples examples.cpp)
endif()

#add_executable(CppMatchSyntax2 main.cpp)