add_executable(conversation_history_test conversation_history.cpp)
target_link_libraries(conversation_history_test PRIVATE oai)
add_test(NAME conversation_history COMMAND conversation_history_test)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(api_modernization_test modernization.cpp transport.cpp webhooks.cpp
  sse.cpp responses_modern.cpp components_modern.cpp resources_modern.cpp decisions.cpp skills.cpp)
target_link_libraries(api_modernization_test PRIVATE oai)
add_test(NAME api_modernization
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/http_fixture.py $<TARGET_FILE:api_modernization_test>)
set_tests_properties(api_modernization PROPERTIES TIMEOUT 120)

add_executable(rerank_test rerank/rerank.cpp)
target_link_libraries(rerank_test PRIVATE oai)
add_test(NAME rerank_http COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/rerank/mock_server.py $<TARGET_FILE:rerank_test>)
set_tests_properties(rerank_http PROPERTIES TIMEOUT 45)
