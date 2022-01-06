cmake_minimum_required(VERSION 3.15)

project(visualization.goom)

if(WIN32)
  if (NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    message(WARNING "Windows SDK is not defined. Cannot tell if it's too old.")
  else()
    set(MINIMUM_SDK_VERSION "10.0.17763.0")
    if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION VERSION_LESS ${MINIMUM_SDK_VERSION})
      message(FATAL_ERROR "Windows SDK too old: installed \"${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\" < minimum \"${MINIMUM_SDK_VERSION}\"")
    endif()
  endif()
endif()

option(ENABLE_TESTING "Enable unit testing" OFF)
option(USE_MAGIC_ENUM "Use magic enum" OFF)
option(IS_KODI_MATRIX "Compatible with kodi Matrix" OFF)
if (IS_KODI_MATRIX)
  add_definitions(-DKODI_MATRIX)
endif()

set(GOOM_LIBS_TARGET_NAME "goom_libs")
set(GOOM_ROOT_DIR ${PROJECT_SOURCE_DIR}/depends/goom-libs)
set(GOOM_BUILD_ARTIFACTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/goom-libs-artifacts)
set(GOOM_INCLUDE_DIRS "${GOOM_ROOT_DIR}/src/goom/include")
set(GOOM_SYS_INC_DIRS "${GOOM_ROOT_DIR}/src/vivid/include"
                      "${GOOM_ROOT_DIR}/lib/fmt/include")

if(NOT WIN32)
  set(GOOM_BIN_LIBS ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libgoom.a
                    ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libvivid.a)
else()
  set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
  set(GOOM_BIN_LIBS ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/goom.lib
                    ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/vivid.lib)
endif()

set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")


include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)

CPMAddPackage("gh:g-truc/glm#0.9.9.8")
set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")

CPMAddPackage("gh:fmtlib/fmt#8.0.1")

<<<<<<< HEAD
list(APPEND GOOM_SYS_INC_DIRS "${fmt_SOURCE_DIR}/include")
=======
find_package(glm REQUIRED)
# Provides GLM_INCLUDE_DIRS
set(GLM_INCLUDE_DIR "${GLM_INCLUDE_DIRS}")

list(APPEND GOOM_SYS_INC_DIRS "${magic_enum_SOURCE_DIR}/include" "${fmt_SOURCE_DIR}/include")
>>>>>>> 3b530c5be (Fix kodi and glm include dirs.)


include(ExternalProject)

ExternalProject_Add(${GOOM_LIBS_TARGET_NAME}
    SOURCE_DIR       ${GOOM_ROOT_DIR}
    BUILD_ALWAYS     ON
    INSTALL_COMMAND  ""
    BUILD_BYPRODUCTS ${GOOM_BIN_LIBS}
    CMAKE_ARGS      -D CMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
                    -D CMAKE_SCRIPTS_PATH:PATH=${CMAKE_SCRIPTS_PATH}
                    -D GOOM_BUILD_ARTIFACTS_DIR:PATH=${GOOM_BUILD_ARTIFACTS_DIR}
                    -D fmt_SOURCE_DIR:PATH=${fmt_SOURCE_DIR}
                    -D GLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
                    -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    -D ENABLE_TESTING=${ENABLE_TESTING}
<<<<<<< HEAD
                    -D USE_MAGIC_ENUM=${USE_MAGIC_ENUM}
=======
                    -D GLM_INCLUDE_DIR=${GLM_INCLUDE_DIR}
>>>>>>> 3b530c5be (Fix kodi and glm include dirs.)
)


set(GOOM_TARGET_NAME "visualization.goom")
set(GOOM_ADDON_PREFIX "GOOM")


set(CPP_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/StaticAnalyzers.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*MSVC")
  option(WARNINGS_AS_ERRORS "MSVC: Problems with glm 'type_quat.hpp' and kodi 'StringUtils.h'" OFF)
  set(POS_INDEP_CODE "OFF")
else()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_COMMON_CXX_WARNINGS ${CMAKE_COMMON_CXX_WARNINGS} -Wno-c++17-extensions)
  endif()
  set(POS_INDEP_CODE "ON")
endif()


find_package(Kodi REQUIRED)
# Provides KODI_INCLUDE_DIR but out by one!
set(KODI_PARENT_INCLUDE_DIR ${KODI_INCLUDE_DIR}/..)

if(NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
  find_package(OpenGl REQUIRED)
  set(DEPLIBS ${OPENGL_LIBRARIES})
  set(includes ${OPENGL_INCLUDE_DIR})
  add_definitions(${OPENGL_DEFINITIONS})
else()
  find_package(OpenGLES 3.2 REQUIRED)
  set(DEPLIBS ${OPENGLES_LIBRARIES})
  set(includes ${OPENGLES_INCLUDE_DIR})
  add_definitions(${OPENGLES_DEFINITIONS})
endif()


set(CMAKE_CXX_STANDARD ${CPP_STANDARD})
get_project_warnings(PROJECT_WARNINGS)
add_compile_options(${PROJECT_WARNINGS})

include_directories(${GOOM_INCLUDE_DIRS}
                    SYSTEM ${GOOM_SYS_INC_DIRS}
                    SYSTEM ${GLM_INCLUDE_DIR}
                    SYSTEM ${KODI_INCLUDE_DIR}
<<<<<<< HEAD
                    SYSTEM ${KODI_PARENT_INCLUDE_DIR}
=======
>>>>>>> 3b530c5be (Fix kodi and glm include dirs.)
)

set(${GOOM_ADDON_PREFIX}_SOURCES src/Main.cpp)
set(${GOOM_ADDON_PREFIX}_HEADERS src/CircularBuffer.h src/Main.h)

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif()

list(APPEND DEPLIBS ${GOOM_BIN_LIBS})

if (CORE_SYSTEM_NAME STREQUAL "osx")
  set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup -flat_namespace")
endif()

build_addon(${GOOM_TARGET_NAME} ${GOOM_ADDON_PREFIX} DEPLIBS)
add_dependencies(${GOOM_TARGET_NAME} ${GOOM_LIBS_TARGET_NAME})

include(CPack)


get_target_property(goom_addon_compile_options  ${GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "Visualization.goom: CORE_SYSTEM_NAME = \"${CORE_SYSTEM_NAME}\".")
message(STATUS "Visualization.goom: CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Visualization.goom: CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Visualization.goom: CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Visualization.goom: WARNINGS_AS_ERRORS = \"${WARNINGS_AS_ERRORS}\".")
if(WIN32)
  message(STATUS "Visualization.goom: CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION = \"${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\".")
endif()
message(STATUS "Visualization.goom: CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Visualization.goom: CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Visualization.goom: C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Visualization.goom: C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Visualization.goom: C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Visualization.goom: C++ compile features = \"${goom_addon_compile_features}\".")
message(STATUS "Visualization.goom: C++ compile options = \"${goom_addon_compile_options}\".")
message(STATUS "Visualization.goom: ENABLE_TESTING = \"${ENABLE_TESTING}\".")
message(STATUS "Visualization.goom: IS_KODI_MATRIX = \"${IS_KODI_MATRIX}\".")
message(STATUS "Visualization.goom: includes = \"${includes}\".")
message(STATUS "Visualization.goom: GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_INCLUDE_DIR = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_PARENT_INCLUDE_DIR = \"${KODI_PARENT_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: GOOM_ROOT_DIR = \"${GOOM_ROOT_DIR}\".")
message(STATUS "Visualization.goom: GOOM_INCLUDE_DIRS = \"${GOOM_INCLUDE_DIRS}\".")
message(STATUS "Visualization.goom: GOOM_SYS_INC_DIRS = \"${GOOM_SYS_INC_DIRS}\".")
message(STATUS "Visualization.goom: GOOM_BIN_LIBS = \"${GOOM_BIN_LIBS}\".")
message(STATUS "Visualization.goom: GOOM_BUILD_ARTIFACTS_DIR = \"${GOOM_BUILD_ARTIFACTS_DIR}\".")
