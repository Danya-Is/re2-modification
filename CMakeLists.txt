cmake_minimum_required(VERSION 3.16)
project(diploma)

set(PROJECT_NAME diploma)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_STANDARD 14)

add_executable(diploma main.cpp regex/parser.cpp regex/regex.cpp regex/regex.h automata.cpp automata.h edge.h node.h bt/binary_tree.cpp bt/binary_tree.h matchers/match.cpp variable.h mfa.cpp matchers/match_mfa.cpp bt/bt_thomson.cpp bt/bt_glushkov.cpp bt/bt_mfa.cpp bt/bt_ssnf.cpp regex/bnf.cpp test/test_bnf.cpp regex/reverse.cpp regex/helpers.cpp test/examples.cpp matchers/example_runner.cpp)
#target_link_libraries(coursework PUBLIC cgraph gvc)
