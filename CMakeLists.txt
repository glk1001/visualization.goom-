cmake_minimum_required(VERSION 3.15)

project(visualization.goom)

set(GOOM_TARGET_NAME "visualization.goom")
set(GOOM_ADDON_PREFIX "GOOM")

option(ENABLE_TESTING "Enable unit testing" OFF)
option(USE_MAGIC_ENUM "Use magic enum" OFF)
option(IS_KODI_MATRIX "Compatible with kodi Matrix" OFF)
if (IS_KODI_MATRIX)
    add_definitions(-DKODI_MATRIX)
endif ()

set(GOOM_LIBS_TARGET_NAME "goom")
set(GOOM_LIBS_RELDIR depends/goom-libs)
set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR}/${GOOM_LIBS_RELDIR})
set(GOOM_BUILD_ARTIFACTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/goom-libs-artifacts)
set(GOOM_INCLUDE_DIRS "${GOOM_LIBS_DIR}/src/goom/include")

if (NOT WIN32)
    set(GOOM_BIN_LIBS
            ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libgoom.a
            ${GOOM_BUILD_ARTIFACTS_DIR}/lib/libvivid.a)
else ()
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
    set(GOOM_BIN_LIBS
            ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/goom.lib
            ${GOOM_BUILD_ARTIFACTS_DIR}/lib/${CMAKE_CONFIGURATION_TYPES}/vivid.lib)
endif ()

IF (NOT KODI_ROOT_DIR)
    set(KODI_ROOT_DIR ${PROJECT_SOURCE_DIR}/..)
endif ()
set(CMAKE_INSTALL_PREFIX ${KODI_ROOT_DIR}/xbmc/kodi-build/addons)

set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")


include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)

CPMAddPackage("gh:g-truc/glm#0.9.9.8")
set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")

CPMAddPackage("gh:fmtlib/fmt#8.0.1")


if (NOT IS_THIS_MAIN_CMAKE_SCRIPT)
    message(STATUS "This cmake script is being run from the kodi addons cmake script.")
else ()
    # Executing the Kodi addon cmake script here allows this script to be the main cmake script.
    # This fits in with the way CLion works.
    if (EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
        message(STATUS "In build mode: Found cache file ${CMAKE_BINARY_DIR}/CMakeCache.txt.")
    else ()
        message(STATUS "In config mode: Executing kodi addons cmake.")
        execute_process(COMMAND cmake
                -D ADDONS_TO_BUILD=${GOOM_TARGET_NAME}
                -D ADDON_SRC_PREFIX=${KODI_ROOT_DIR}
                -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                -D PACKAGE_ZIP=1
                ${KODI_ROOT_DIR}/xbmc/cmake/addons
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                RESULT_VARIABLE CMAKE_RESULT)
        if (NOT CMAKE_RESULT EQUAL "0")
            message(FATAL_ERROR "cmake kodi-addons failed.")
        endif ()
    endif ()
endif ()


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
include(${CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)


if (CMAKE_CXX_COMPILER_ID MATCHES ".*MSVC")
    option(WARNINGS_AS_ERRORS "MSVC: Problems with glm 'type_quat.hpp' and kodi 'StringUtils.h'" OFF)
    set(POS_INDEP_CODE "OFF")
else ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CMAKE_COMMON_CXX_WARNINGS ${CMAKE_COMMON_CXX_WARNINGS} -Wno-c++17-extensions)
    endif ()
    set(POS_INDEP_CODE "ON")
endif ()

if (CORE_SYSTEM_NAME STREQUAL "osx")
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup -flat_namespace")
endif ()


find_package(Kodi PATHS ${CMAKE_BINARY_DIR}/build/depends/lib/kodi NO_DEFAULT_PATH REQUIRED)
# Provides KODI_INCLUDE_DIR but it's out by one!
set(KODI_FIXED_INCLUDE_DIR ${KODI_INCLUDE_DIR}/..)

if (NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
    find_package(OpenGl REQUIRED)
    set(DEPLIBS ${OPENGL_LIBRARIES})
    set(includes ${OPENGL_INCLUDE_DIR})
    add_definitions(${OPENGL_DEFINITIONS})
else ()
    find_package(OpenGLES 3.2 REQUIRED)
    set(DEPLIBS ${OPENGLES_LIBRARIES})
    set(includes ${OPENGLES_INCLUDE_DIR})
    add_definitions(${OPENGLES_DEFINITIONS})
endif ()


set(CMAKE_CXX_STANDARD ${CPP_STANDARD})
get_project_warnings(PROJECT_WARNINGS)
add_compile_options(${PROJECT_WARNINGS})

include_directories(${GOOM_INCLUDE_DIRS}
        SYSTEM ${GOOM_LIBS_DIR}/src/vivid/include
        SYSTEM ${GOOM_LIBS_DIR}/lib/fmt/include
        SYSTEM ${fmt_SOURCE_DIR}/include
        SYSTEM ${GLM_INCLUDE_DIR}
        SYSTEM ${KODI_INCLUDE_DIR}
        SYSTEM ${KODI_FIXED_INCLUDE_DIR}
        )

set(${GOOM_ADDON_PREFIX}_SOURCES src/Main.cpp src/build_time.cpp)
set(${GOOM_ADDON_PREFIX}_HEADERS src/CircularBuffer.h src/Main.h src/build_time.h)

string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Goom: GOOM_DEBUG turned on.")
endif ()

list(APPEND DEPLIBS ${GOOM_BIN_LIBS})

add_subdirectory(${GOOM_LIBS_RELDIR})

build_addon(${GOOM_TARGET_NAME} ${GOOM_ADDON_PREFIX} DEPLIBS)
add_dependencies(${GOOM_TARGET_NAME} ${GOOM_LIBS_TARGET_NAME} vivid)


# Move the external goom libs Dart file into the main build directory so CLion is happy.
set(GOOM_LIBS_DART_DIR ${CMAKE_BINARY_DIR}/${GOOM_LIBS_RELDIR})
add_custom_command(
        TARGET ${GOOM_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${GOOM_LIBS_DART_DIR}/DartConfiguration.tcl
        ${CMAKE_BINARY_DIR}/DartConfiguration.tcl)

# Force the build time into the compiled binaries.
set(GOOM_BUILD_TIME_FILES
        ${GOOM_LIBS_DIR}/src/goom/src/utils/build_time.cpp
        ${PROJECT_SOURCE_DIR}/src/build_time.cpp
        )
add_custom_target(goom_build_time ALL DEPENDS ${GOOM_BUILD_TIME_FILES})
add_custom_command(TARGET goom_build_time
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E touch ${GOOM_BUILD_TIME_FILES}
        COMMENT "Touching build files."
        VERBATIM
        )


include(CPack)


get_target_property(goom_addon_compile_options ${GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "Visualization.goom: CORE_SYSTEM_NAME = \"${CORE_SYSTEM_NAME}\".")
message(STATUS "Visualization.goom: CMAKE_CXX_COMPILER_ID = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Visualization.goom: CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Visualization.goom: CMAKE_BUILD_TYPE = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Visualization.goom: WARNINGS_AS_ERRORS = \"${WARNINGS_AS_ERRORS}\".")
if (WIN32)
    message(STATUS "Visualization.goom: CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION = \"${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\".")
endif ()
message(STATUS "Visualization.goom: CMAKE_SOURCE_DIR   = \"${CMAKE_SOURCE_DIR}\".")
message(STATUS "Visualization.goom: PROJECT_SOURCE_DIR = \"${PROJECT_SOURCE_DIR}\".")
message(STATUS "Visualization.goom: CMAKE_BINARY_DIR   = \"${CMAKE_BINARY_DIR}\".")
message(STATUS "Visualization.goom: CMAKE_MODULE_PATH = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Visualization.goom: CMAKE_SCRIPTS_PATH = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Visualization.goom: CMAKE_INSTALL_PREFIX = \"${CMAKE_INSTALL_PREFIX}\".")
message(STATUS "Visualization.goom: C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Visualization.goom: C++ standard required = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Visualization.goom: C++ extensions = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Visualization.goom: C++ compile features = \"${goom_addon_compile_features}\".")
message(STATUS "Visualization.goom: C++ compile options = \"${goom_addon_compile_options}\".")
message(STATUS "Visualization.goom: ENABLE_TESTING = \"${ENABLE_TESTING}\".")
message(STATUS "Visualization.goom: IS_KODI_MATRIX = \"${IS_KODI_MATRIX}\".")
message(STATUS "Visualization.goom: includes = \"${includes}\".")
message(STATUS "Visualization.goom: GOOM_LIBS_DIR = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Visualization.goom: GOOM_INCLUDE_DIRS = \"${GOOM_INCLUDE_DIRS}\".")
message(STATUS "Visualization.goom: GOOM_BIN_LIBS = \"${GOOM_BIN_LIBS}\".")
message(STATUS "Visualization.goom: GOOM_BUILD_ARTIFACTS_DIR = \"${GOOM_BUILD_ARTIFACTS_DIR}\".")
message(STATUS "Visualization.goom: GOOM_LIBS_DART_DIR = \"${GOOM_LIBS_DART_DIR}\".")
message(STATUS "Visualization.goom: GLM_INCLUDE_DIR = \"${GLM_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_ROOT_DIR = \"${KODI_ROOT_DIR}\".")
message(STATUS "Visualization.goom: KODI_INCLUDE_DIR = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Visualization.goom: KODI_FIXED_INCLUDE_DIR = \"${KODI_FIXED_INCLUDE_DIR}\".")
