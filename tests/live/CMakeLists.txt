foreach(suite responses components resources extensions)
  add_executable(liboai_live_${suite} ${suite}.cpp)
  target_link_libraries(liboai_live_${suite} PRIVATE oai)
endforeach()
# Deliberately not registered with CTest. Execution requires run.py --allow-live.
