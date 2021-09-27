cmake_minimum_required(VERSION 3.12)

project(visualization.goom)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # Kodi Windows build uses Visual Studio 2017 which calls C++17 attributes errors instead of warnings.
  # Force C++17 to get it to build properly.
  set(CPP_STANDARD 17)
else()
  set(CPP_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
cmake_policy(SET CMP0077 NEW)
set(WARNINGS_AS_ERRORS OFF)

set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/Cache.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/StaticAnalyzers.cmake)


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


find_package(Kodi REQUIRED)
# Provides KODI_INCLUDE_DIR
find_package(glm REQUIRED)
# Provides GLM_INCLUDE_DIR

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()


find_package(Freetype)
if (FREETYPE_FOUND)
  message(STATUS "Found FreeType package - using supplied include directories.")
else()
  message(WARNING "Could not find FreeType package - disabling Freetype.")
  add_definitions(-DNO_FREETYPE_INSTALLED)
endif ()


set(LIBS_DIR ${CMAKE_SOURCE_DIR}/dependencies/goom-libs/lib)
set(GOOM_LIBS_DIR ${CMAKE_SOURCE_DIR}/dependencies/goom-libs/src)

message(STATUS "CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "LIBS_DIR = \"${LIBS_DIR}\".")
message(STATUS "GOOM_LIBS_DIR = \"${GOOM_LIBS_DIR}\".")
message(STATUS "FREETYPE_INCLUDE_DIRS = \"${FREETYPE_INCLUDE_DIRS}\".")
message(STATUS "GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "KODI_INCLUDE_DIR = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Main C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "C Compile features = \"${COMMON_C_COMPILE_FEATURES}\".")
message(STATUS "C++ Compile features = \"${COMMON_CXX_COMPILE_FEATURES}\".")
message(STATUS "C warnings = \"${CMAKE_COMMON_C_WARNINGS}\".")
message(STATUS "C++ warnings = \"${CMAKE_COMMON_CXX_WARNINGS}\".")


include_directories(${GOOM_LIBS_DIR}/goom/include
                    ${GOOM_LIBS_DIR}/goomutils/include
                    SYSTEM ${GOOM_LIBS_DIR}/vivid/include
                    SYSTEM ${LIBS_DIR}/fmt/include
                    # Need this '..' hack here. Not nice!
                    SYSTEM ${GLM_INCLUDE_DIR}/..
                    SYSTEM ${KODI_INCLUDE_DIR})

set(GOOM_SOURCES src/Main.cpp)
set(GOOM_HEADERS src/CircularBuffer.h
                 src/Main.h)

list(APPEND DEPLIBS 
     ${CMAKE_SOURCE_DIR}/build-goom-libs/lib/libgoom.a 
     ${CMAKE_SOURCE_DIR}/build-goom-libs/lib/libgoomutils.a
     ${CMAKE_SOURCE_DIR}/build-goom-libs/lib/libvivid.a)

set(CMAKE_CXX_STANDARD ${CPP_STANDARD})
get_project_warnings(PROJECT_WARNINGS)
add_compile_options(${PROJECT_WARNINGS})

build_addon(visualization.goom GOOM DEPLIBS)

include(CPack)
