cmake_minimum_required(VERSION 3.23)
project(coursework)

set(PROJECT_NAME coursework)

set(CMAKE_CXX_STANDARD 14)

add_executable(coursework main.cpp parser.cpp regex.cpp regex.h automata.cpp automata.h edge.h node.h binary_tree.cpp binary_tree.h match.cpp variable.h mfa.cpp)
target_link_libraries(coursework PUBLIC cgraph gvc)
