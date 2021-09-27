cmake_minimum_required(VERSION 3.12)

project(visualization.goom.lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Kodi Windows build uses Visual Studio 2017 which calls C++17 attributes errors instead of warnings.
  # Force C++17 to get it to build properly.
  set(CPP_STANDARD 17)
else()
  set(CPP_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# enable cache system
include(cmake/Cache.cmake)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

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

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/lib)
set(GOOM_ROOT_DIR ${CMAKE_SOURCE_DIR}/src)


message(STATUS "CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "GOOM_ROOT_DIR = \"${GOOM_ROOT_DIR}\".")
message(STATUS "LIBS_DIR = \"${LIBS_DIR}\".")
message(STATUS "FREETYPE_INCLUDE_DIRS = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY = \"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}\".")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY = \"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}\".")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY = \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\".")
message(STATUS "Main C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")


include(CTest)
include(Catch)

add_subdirectory(src)
