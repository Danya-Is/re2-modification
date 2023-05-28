cmake_minimum_required(VERSION 3.23)
project(coursework)

#find_package(Catch2 CONFIG REQUIRED)
## These tests can use the Catch2-provided main
#add_executable(tests test.cpp)
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

## These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)
#
Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests test/test_bnf.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
#
## These tests need their own main
#add_executable(custom-main-tests test/test_bnf.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)

set(PROJECT_NAME coursework)

set(CMAKE_CXX_STANDARD 14)

add_executable(coursework main.cpp regex/parser.cpp regex/regex.cpp regex/regex.h automata.cpp automata.h edge.h node.h bt/binary_tree.cpp bt/binary_tree.h matchers/match.cpp variable.h mfa.cpp matchers/match_mfa.cpp bt/bt_thomson.cpp bt/bt_glushkov.cpp bt/bt_mfa.cpp bt/bt_ssnf.cpp regex/bnf.cpp test/test_bnf.cpp regex/reverse.cpp)
#target_link_libraries(coursework PUBLIC cgraph gvc)
