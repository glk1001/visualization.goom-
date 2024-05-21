cmake_minimum_required(VERSION 3.28)

project(visualization.goom-pp)

set(VIS_GOOM_TARGET_NAME "visualization.goom-pp")
set(VIS_GOOM_ADDON_PREFIX "GOOM")

option(ENABLE_TESTING "Enable unit testing" OFF)

set(GOOM_LIBS_RELDIR depends/goom-libs)
set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR}/${GOOM_LIBS_RELDIR})
set(GOOM_INCLUDE_DIR "${GOOM_LIBS_DIR}/src/goom/include")

if (NOT KODI_ROOT_DIR)
    set(KODI_ROOT_DIR ${PROJECT_SOURCE_DIR}/..)
endif ()
get_filename_component(KODI_ROOT_DIR "${KODI_ROOT_DIR}" ABSOLUTE)

set(GOOM_BUILD_DIR "${CMAKE_BINARY_DIR}")
if (NOT EXISTS "${GOOM_BUILD_DIR}")
    message(FATAL_ERROR "Could not find goom build directory \"${GOOM_BUILD_DIR}\".")
endif ()

set(CMAKE_INSTALL_PREFIX "${KODI_ROOT_DIR}/kodi/kodi-build/addons")
if (NOT EXISTS "${CMAKE_INSTALL_PREFIX}")
    message(FATAL_ERROR "Could not find install directory \"${CMAKE_INSTALL_PREFIX}\".")
endif ()

set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")


# Executing the Kodi addon cmake script here allows this script to be the main cmake script.
# This fits in with the way CLion works.
if (EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
    message(STATUS "In build mode: Found cache file \"${CMAKE_BINARY_DIR}/CMakeCache.txt\".")
else ()
    message(STATUS "In config mode: Executing kodi addons cmake.")
    execute_process(COMMAND cmake
                    -D ADDONS_TO_BUILD=${VIS_GOOM_TARGET_NAME}
                    -D ADDON_SRC_PREFIX=${KODI_ROOT_DIR}
                    -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                    -D PACKAGE_ZIP=1
                    ${KODI_ROOT_DIR}/kodi/cmake/addons
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    RESULT_VARIABLE CMAKE_RESULT)
    if (NOT CMAKE_RESULT EQUAL "0")
        file(REMOVE "${CMAKE_BINARY_DIR}/CMakeCache.txt")
        message(FATAL_ERROR "cmake kodi-addons failed.")
    endif ()
endif ()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SCRIPTS_PATH})

include(${CMAKE_SCRIPTS_PATH}/CPM.cmake)
include(${CMAKE_SCRIPTS_PATH}/StandardProjectSettings.cmake)
include(${CMAKE_SCRIPTS_PATH}/PreventInSourceBuilds.cmake)
include(${CMAKE_SCRIPTS_PATH}/Cache.cmake)
include(${CMAKE_SCRIPTS_PATH}/CompilerWarnings.cmake)
include(${CMAKE_SCRIPTS_PATH}/Linker.cmake)


if (CMAKE_CXX_COMPILER_ID MATCHES ".*MSVC")
    set(POS_INDEP_CODE "OFF")
else ()
    set(POS_INDEP_CODE "ON")
endif ()
if (CORE_SYSTEM_NAME STREQUAL "osx")
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup -flat_namespace")
endif ()


CPMAddPackage("gh:g-truc/glm#1.0.1")
set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")

find_package(Kodi REQUIRED)
# Override Kodi's C++ standard setting.
set(CMAKE_CXX_STANDARD 20)

if (NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
    find_package(OpenGl REQUIRED)
    set(DEPLIBS ${OPENGL_LIBRARIES})
    add_definitions(${OPENGL_DEFINITIONS})
else ()
    find_package(OpenGLES 3.2 REQUIRED)
    set(DEPLIBS ${OPENGLES_LIBRARIES})
    add_definitions(${OPENGLES_DEFINITIONS})
endif ()


# Let the Goom code know we're building from Kodi. There are Kodi
# headers that the Main goom code needs only when doing a Kodi build.
add_definitions(-DIS_KODI_BUILD)

# Set GOOM_DEBUG for non-release builds.
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    MESSAGE(STATUS "Vis.Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    MESSAGE(STATUS "Vis.Goom: GOOM_DEBUG turned on.")
endif ()


include(ProjectOptions.cmake)
vis_goom_pp_get_project_warnings(vis_goom_pp_WARNINGS_AS_ERRORS vis_goom_pp_PROJECT_WARNINGS)

# Include the Goom libs cmake files here, before the (horrible) global
# compiler settings (needed for Kodi addon builder).
add_subdirectory(${GOOM_LIBS_RELDIR})


add_compile_options(${vis_goom_pp_PROJECT_WARNINGS})
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    # glm has many 'unsafe-buffer-usage' warnings.
    message(STATUS "Turning off 'unsafe-buffer-usage' for Clang.")
    add_compile_options(-Wno-unknown-warning-option -Wno-unsafe-buffer-usage)
endif ()


# Setup the includes and other dependencies the addon builder needs.
include_directories(BEFORE
                    ${GOOM_INCLUDE_DIR}
)
include_directories(BEFORE
                    SYSTEM
                    ${GOOM_LIBS_DIR}/lib/std_fmt/include
                    ${GLM_INCLUDE_DIR}
                    ${KODI_INCLUDE_DIR}
                    ${KODI_FIXED_INCLUDE_DIR}
)

set(${VIS_GOOM_ADDON_PREFIX}_SOURCES
    src/Main.cpp
    src/build_time.cpp
    src/goom_visualization.cpp
)
set(${VIS_GOOM_ADDON_PREFIX}_HEADERS
    src/gl_call.h
    src/goom_gl.h
    src/Main.h
)

# Let kodi set this:
set(CMAKE_INSTALL_LIBDIR "")

list(APPEND DEPLIBS goom::lib libs::vivid LSys::lib blend2d::blend2d)
build_addon(${VIS_GOOM_TARGET_NAME} ${VIS_GOOM_ADDON_PREFIX} DEPLIBS)
add_dependencies(${VIS_GOOM_TARGET_NAME}
                 goom::lib
                 libs::vivid
                 LSys::lib
                 blend2d::blend2d
)

target_sources(${VIS_GOOM_TARGET_NAME}
               PRIVATE
               FILE_SET CXX_MODULES FILES
               src/build_time.cppm
               src/displacement_filter.cppm
               src/gl_render_types.cppm
               src/gl_2d_textures.cppm
               src/gl_utils.cppm
               src/glsl_program.cppm
               src/glsl_shader_file.cppm
               src/goom_visualization.cppm
               src/scene.cppm
               src/slot_producer_consumer.cppm
)


# Force the build time into the compiled binaries.
set(GOOM_BUILD_TIME_FILES
    ${GOOM_LIBS_DIR}/src/goom/src/utils/build_time.cppm
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

include(${CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)


get_target_property(goom_addon_compile_options ${VIS_GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${VIS_GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "Vis.goom: Cmake version              = \"${CMAKE_VERSION}\"")
message(STATUS "Vis.goom: Cmake generator            = \"${CMAKE_GENERATOR}\"")
message(STATUS "Vis.goom: CORE_SYSTEM_NAME           = \"${CORE_SYSTEM_NAME}\".")
message(STATUS "Vis.goom: CMAKE_CXX_COMPILER_ID      = \"${CMAKE_CXX_COMPILER_ID}\".")
message(STATUS "Vis.goom: CMAKE_CXX_COMPILER_VERSION = \"${CMAKE_CXX_COMPILER_VERSION}\".")
message(STATUS "Vis.goom: CMAKE_BUILD_TYPE           = \"${CMAKE_BUILD_TYPE}\".")
message(STATUS "Vis.goom: WARNINGS_AS_ERRORS         = \"${vis_goom_pp_WARNINGS_AS_ERRORS}\".")
if (WIN32)
    message(STATUS "Vis.goom: CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION ="
            " \"${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}\".")
endif ()
message(STATUS "Vis.goom: CMAKE_SOURCE_DIR           = \"${CMAKE_SOURCE_DIR}\".")
message(STATUS "Vis.goom: PROJECT_SOURCE_DIR         = \"${PROJECT_SOURCE_DIR}\".")
message(STATUS "Vis.goom: CMAKE_BINARY_DIR           = \"${CMAKE_BINARY_DIR}\".")
message(STATUS "Vis.goom: CMAKE_MODULE_PATH          = \"${CMAKE_MODULE_PATH}\".")
message(STATUS "Vis.goom: CMAKE_SCRIPTS_PATH         = \"${CMAKE_SCRIPTS_PATH}\".")
message(STATUS "Vis.goom: CMAKE_INSTALL_PREFIX       = \"${CMAKE_INSTALL_PREFIX}\".")
message(STATUS "Vis.goom: CMAKE_INSTALL_LIBDIR       = \"${CMAKE_INSTALL_LIBDIR}\".")
message(STATUS "Vis.goom: CMAKE_INSTALL_DATADIR      = \"${CMAKE_INSTALL_DATADIR}\".")
message(STATUS "Vis.goom: C++ standard               = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Vis.goom: C++ standard required      = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Vis.goom: C++ extensions             = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Vis.goom: C++ compile features       = \"${goom_addon_compile_features}\".")
message(STATUS "Vis.goom: C++ compile options        = \"${goom_addon_compile_options}\".")
message(STATUS "Vis.goom: ENABLE_TESTING             = \"${ENABLE_TESTING}\".")
message(STATUS "Vis.goom: GOOM_BUILD_DIR             = \"${GOOM_BUILD_DIR}\".")
message(STATUS "Vis.goom: GOOM_INCLUDE_DIR           = \"${GOOM_INCLUDE_DIR}\".")
message(STATUS "Vis.goom: GOOM_LIBS_DIR              = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Vis.goom: KODI_ROOT_DIR              = \"${KODI_ROOT_DIR}\".")
message(STATUS "Vis.goom: KODI_CONFIG_DIR            = \"${KODI_CONFIG_DIR}\".")
message(STATUS "Vis.goom: KODI_INCLUDE_DIR           = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Vis.goom: KODI_FIXED_INCLUDE_DIR     = \"${KODI_FIXED_INCLUDE_DIR}\".")
message(STATUS "Vis.goom: DEPLIBS                    = \"${DEPLIBS}\".")
