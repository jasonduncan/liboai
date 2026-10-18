cmake_minimum_required(VERSION 3.21)

# Toolchains must be selected before project() initializes the compiler.
# Retain the historical Windows vcpkg convenience for top-level consumers;
# explicit toolchains and parent projects remain in control of their setup.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT DEFINED CMAKE_TOOLCHAIN_FILE
    AND NOT DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  if(DEFINED VCPKG_CMAKE_PATH AND NOT VCPKG_CMAKE_PATH STREQUAL "")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_CMAKE_PATH}" CACHE FILEPATH "CMake toolchain")
  elseif(CMAKE_HOST_WIN32 AND DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "CMake toolchain")
  endif()
endif()

project(liboai)

option(BUILD_EXAMPLES "Build example applications" OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(liboai)

if(BUILD_EXAMPLES)
  add_subdirectory(documentation)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT oai)

option(LIBOAI_BUILD_TESTS "Build liboai regression tests" OFF)
if(LIBOAI_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
