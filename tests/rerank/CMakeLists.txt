cmake_minimum_required(VERSION 3.21)
project(rerank_regression LANGUAGES C CXX)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_subdirectory(../.. liboai)
enable_testing()
add_executable(rerank_test rerank.cpp)
target_link_libraries(rerank_test PRIVATE oai)
add_test(NAME rerank_http
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/mock_server.py $<TARGET_FILE:rerank_test>)
set_tests_properties(rerank_http PROPERTIES TIMEOUT 45)
