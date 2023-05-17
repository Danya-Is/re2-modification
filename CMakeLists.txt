cmake_minimum_required(VERSION 3.23)
project(coursework)

set(PROJECT_NAME coursework)

set(CMAKE_CXX_STANDARD 14)

add_executable(coursework main.cpp parser.cpp regex.cpp regex.h automata.cpp automata.h edge.h node.h bt/binary_tree.cpp bt/binary_tree.h matchers/match.cpp variable.h mfa.cpp matchers/match_mfa.cpp test.h bt/bt_thomson.cpp bt/bt_glushkov.cpp bt/bt_mfa.cpp bt/bt_ssnf.cpp)
#target_link_libraries(coursework PUBLIC cgraph gvc)
