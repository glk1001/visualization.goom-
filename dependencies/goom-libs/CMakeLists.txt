cmake_minimum_required(VERSION 3.12)

project(visualization.goom.lib)

if(NOT DEFINED GOOM_BUILD_ARTIFACTS_DIR)
  set(GOOM_BUILD_ARTIFACTS_DIR "${CMAKE_BINARY_DIR}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GOOM_BUILD_ARTIFACTS_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Kodi Windows build uses Visual Studio 2017 which calls C++17 attributes errors instead of warnings.
  # Force C++17 to get it to build properly.
  set(CPP_STANDARD 17)
else()
  set(CPP_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/Linker.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/Sanitizers.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_COMMON_CXX_WARNINGS /W2)
  # Windows linker seems to be broken - possibly with multi-threading.
  set(POS_INDEP_CODE "OFF")
else()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_COMMON_CXX_WARNINGS ${CMAKE_COMMON_CXX_WARNINGS} -Wno-c++17-extensions)
  endif()
  set(POS_INDEP_CODE "ON")
endif()


find_package(Freetype)
if (FREETYPE_FOUND)
  message(STATUS "Found FreeType package - using supplied include directories.")
else()
  message(WARNING "Could not find FreeType package - disabling Freetype.")
  add_definitions(-DNO_FREETYPE_INSTALLED)
endif ()

find_package(Catch2 REQUIRED)

set(THIRD_PARTY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(GOOM_ROOT_DIR ${CMAKE_SOURCE_DIR}/src)


message(STATUS "CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "Goom-libs C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Goom-libs C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Goom-libs C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "GOOM_ROOT_DIR = \"${GOOM_ROOT_DIR}\".")
message(STATUS "THIRD_PARTY_INCLUDE_DIR = \"${THIRD_PARTY_INCLUDE_DIR}\".")
message(STATUS "FREETYPE_INCLUDE_DIRS = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY = \"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}\".")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY = \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\".")


include(CTest)
include(Catch)

add_subdirectory(src)
