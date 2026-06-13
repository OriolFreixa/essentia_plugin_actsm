cmake_minimum_required(VERSION 3.22)
project(DependenciesSuperbuild NONE)

include(ExternalProject)

# 1) Global settings once for all externals
set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../CMakeModules" CACHE PATH "")

# macOS SDK target (if needed)
if(APPLE)
  execute_process(COMMAND xcrun --show-sdk-version
                  OUTPUT_VARIABLE SDK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET ${SDK_VERSION} CACHE STRING "" FORCE)
endif()

# 2) Shared install tree
set(DEPS_INSTALL_DIR ${CMAKE_BINARY_DIR}/install)

# 3) Common CMake args for all ExternalProject_Add
set(DEFAULT_CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DBUILD_SHARED_LIBS=OFF        # static everywhere
#   -DCMAKE_SKIP_RPATH=ON
)

# 4) JUCE
if(EXISTS "${CMAKE_SOURCE_DIR}/../external/JUCE/CMakeLists.txt")
  message(STATUS "Using local JUCE")
  ExternalProject_Add(
    JUCE
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/../external/JUCE"
    BINARY_DIR "${CMAKE_BINARY_DIR}/JUCE-build"
    INSTALL_DIR "${DEPS_INSTALL_DIR}"
    CMAKE_ARGS ${DEFAULT_CMAKE_ARGS}
  )
else()
  message(STATUS "Cloning JUCE from Git")
  ExternalProject_Add(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    PREFIX         "${CMAKE_BINARY_DIR}/JUCE"
    INSTALL_DIR    "${DEPS_INSTALL_DIR}"
    CMAKE_ARGS     ${DEFAULT_CMAKE_ARGS}
  )
endif()

# 5) googletest (same pattern)
ExternalProject_Add(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG         v1.17.0
  PREFIX         "${CMAKE_BINARY_DIR}/gtest"
  INSTALL_DIR    "${DEPS_INSTALL_DIR}"
  CMAKE_ARGS     ${DEFAULT_CMAKE_ARGS}
)

# # 6) Essentia
# ExternalProject_Add(
#   essentia
#   GIT_REPOSITORY https://github.com/MTG/essentia.git
#   GIT_TAG        v2.1.1
#   PREFIX         "${CMAKE_BINARY_DIR}/essentia"
#   INSTALL_DIR    "${DEPS_INSTALL_DIR}"
#   CMAKE_ARGS     ${DEFAULT_CMAKE_ARGS}
#                   -DESSENTIA_STATIC=ON
#                   -DESSENTIA_PYTHON=OFF
# )

add_dependencies(googletest JUCE)