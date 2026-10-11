cmake_minimum_required(VERSION 3.21)

project(seamwork VERSION 0.1.0 LANGUAGES CXX)

option(SEAMWORK_BUILD_STUDIES "Build Seamwork C++23 design studies." ON)
option(SEAMWORK_BUILD_TESTS "Register and run Seamwork study/component tests." ON)

if(SEAMWORK_BUILD_TESTS)
    enable_testing()
endif()

if(SEAMWORK_BUILD_STUDIES)
    add_subdirectory(studies)
endif()

message(STATUS "Seamwork version: ${PROJECT_VERSION}")
message(STATUS "Seamwork studies: ${SEAMWORK_BUILD_STUDIES}")
message(STATUS "Seamwork tests: ${SEAMWORK_BUILD_TESTS}")
