cmake_minimum_required(VERSION 3.12)

project(visualization.goom)

set(ENABLE_TESTING ON)                  

set(GOOM_LIBS_TARGET_NAME "goom_libs")
set(GOOM_ROOT_DIR ${PROJECT_SOURCE_DIR}/dependencies/goom-libs)
set(GOOM_BUILD_ARTIFACTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/goom-libs-artifacts)
set(GOOM_INCLUDE_DIRS "${GOOM_ROOT_DIR}/src/goom/include")
set(GOOM_SYS_INC_DIRS "${GOOM_ROOT_DIR}/src/vivid/include"
                      "${GOOM_ROOT_DIR}/lib/fmt/include")
set(GOOM_BIN_LIBS ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libgoom.a 
                  ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libvivid.a)

set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(ExternalProject)

ExternalProject_Add(${GOOM_LIBS_TARGET_NAME}
    SOURCE_DIR       ${GOOM_ROOT_DIR}
    BUILD_ALWAYS     ON
    INSTALL_COMMAND  ""
    BUILD_BYPRODUCTS ${GOOM_BIN_LIBS}
    CMAKE_ARGS      -D CMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
                    -D CMAKE_SCRIPTS_PATH:PATH=${CMAKE_SCRIPTS_PATH}
                    -D GOOM_BUILD_ARTIFACTS_DIR:PATH=${GOOM_BUILD_ARTIFACTS_DIR}
                    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    -D ENABLE_TESTING=${ENABLE_TESTING}
)


set(GOOM_TARGET_NAME "visualization.goom")
set(GOOM_ADDON_PREFIX "GOOM")


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


include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)


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
# But override KODI_INCLUDE_DIR to use xbmc git file
set(KODI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../xbmc/xbmc/addons/kodi-dev-kit/include/kodi")

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


set(CMAKE_CXX_STANDARD ${CPP_STANDARD})
get_project_warnings(PROJECT_WARNINGS)
add_compile_options(${PROJECT_WARNINGS})

include_directories(${GOOM_INCLUDE_DIRS}
                    SYSTEM ${GOOM_SYS_INC_DIRS}
                    # Need this '..' hack here. Not nice!
                    SYSTEM ${GLM_INCLUDE_DIR}/..
                    SYSTEM ${KODI_INCLUDE_DIR})

set(${GOOM_ADDON_PREFIX}_SOURCES src/Main.cpp)
set(${GOOM_ADDON_PREFIX}_HEADERS src/CircularBuffer.h src/Main.h)

list(APPEND DEPLIBS ${GOOM_BIN_LIBS})


build_addon(${GOOM_TARGET_NAME} ${GOOM_ADDON_PREFIX} DEPLIBS)
add_dependencies(${GOOM_TARGET_NAME} ${GOOM_LIBS_TARGET_NAME})


include(CPack)


get_target_property(goom_addon_compile_options  ${GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Visualization.goom C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Visualization.goom C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Visualization.goom C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Visualization.goom C++ compile features = \"${goom_addon_compile_features}\".")
message(STATUS "Visualization.goom C++ compile options = \"${goom_addon_compile_options}\".")
message(STATUS "GOOM_ROOT_DIR = \"${GOOM_ROOT_DIR}\".")
message(STATUS "GOOM_BUILD_ARTIFACTS_DIR = \"${GOOM_BUILD_ARTIFACTS_DIR}\".")
message(STATUS "GOOM_INCLUDE_DIRS = \"${GOOM_INCLUDE_DIRS}\".")
message(STATUS "GOOM_SYS_INC_DIRS = \"${GOOM_SYS_INC_DIRS}\".")
message(STATUS "GOOM_BIN_LIBS = \"${GOOM_BIN_LIBS}\".")
message(STATUS "GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "KODI_INCLUDE_DIR = \"${KODI_INCLUDE_DIR}\".")
