cmake_minimum_required(VERSION 3.28)

project(visualization.goom-pp)

set(VIS_GOOM_TARGET_NAME "visualization.goom-pp")
set(VIS_GOOM_ADDON_VERSION "20.1.1")
string(TOLOWER ${CMAKE_SYSTEM_NAME} PLATFORM)

option(ENABLE_TESTING "Enable unit testing" OFF)

set(GOOM_LIBS_RELDIR depends/goom-libs)
set(GOOM_LIBS_DIR ${PROJECT_SOURCE_DIR}/${GOOM_LIBS_RELDIR})
set(GOOM_DIR "${GOOM_LIBS_DIR}/src/goom")
set(GOOM_INCLUDE_DIR "${GOOM_DIR}/include")
set(GOOM_ADDON_FILES_DIR "${PROJECT_SOURCE_DIR}/visualization.goom-pp")
set(GOOM_RESOURCES_DIR "${GOOM_ADDON_FILES_DIR}/resources")

if (NOT KODI_ROOT_DIR)
    set(KODI_ROOT_DIR ${PROJECT_SOURCE_DIR}/..)
endif ()
get_filename_component(KODI_ROOT_DIR "${KODI_ROOT_DIR}" ABSOLUTE)
set(KODI_INCLUDE_DIR "${KODI_ROOT_DIR}/kodi/xbmc/addons/kodi-dev-kit/include")

set(GOOM_BUILD_DIR "${CMAKE_BINARY_DIR}")
if (NOT EXISTS "${GOOM_BUILD_DIR}")
    message(FATAL_ERROR "Could not find goom build directory \"${GOOM_BUILD_DIR}\".")
endif ()

set(CMAKE_INSTALL_PREFIX "${GOOM_BUILD_DIR}/kodi-vis-goom-addon")

set(CMAKE_SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/cmake")


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


CPMAddPackage("gh:g-truc/glm#1.0.1")
set(GLM_INCLUDE_DIR "${glm_SOURCE_DIR}")

if (NOT WIN32 AND (APP_RENDER_SYSTEM STREQUAL "gl" OR NOT APP_RENDER_SYSTEM))
    find_package(OpenGl REQUIRED)
    set(DEPLIBS ${OPENGL_LIBRARIES})
    add_definitions(${OPENGL_DEFINITIONS})
else ()
    find_package(OpenGLES 3.2 REQUIRED)
    set(DEPLIBS ${OPENGLES_LIBRARIES})
    add_definitions(${OPENGLES_DEFINITIONS})
endif ()


# Let the Goom code know we're building using Kodi. There are Kodi
# headers that the Main goom code needs only when doing a Kodi build.
add_definitions(-DIS_KODI_BUILD)
if(PLATFORM STREQUAL "linux")
    add_definitions(-DTARGET_LINUX)
elseif(PLATFORM STREQUAL "windows")
    add_definitions(-DTARGET_WINDOWS)
else()
    message(FATAL "Platform \"${PLATFORM}\" not supported.")
endif()

# Set GOOM_DEBUG for non-release builds.
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_CMAKE_BUILD_TYPE)
if (UPPER_CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    message(STATUS "Vis.Goom: GOOM_DEBUG turned off.")
else ()
    add_definitions(-DGOOM_DEBUG)
    message(STATUS "Vis.Goom: GOOM_DEBUG turned on.")
endif ()


include(ProjectOptions.cmake)
vis_goom_pp_get_project_warnings(vis_goom_pp_WARNINGS_AS_ERRORS vis_goom_pp_PROJECT_WARNINGS)

# Include the Goom libs cmake files here, before the (horrible) global
# compiler settings (needed for Kodi addon builder).
add_subdirectory(${GOOM_LIBS_RELDIR})


include(${CMAKE_SCRIPTS_PATH}/CheckMinimumToolRequirements.cmake)
add_compile_options(${vis_goom_pp_PROJECT_WARNINGS})
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")


set(VIS_GOOM_SOURCES
    src/Main.cpp
    src/build_time.cpp
    src/goom_visualization.cpp
)
set(VIS_GOOM_HEADERS
    src/gl_call.h
    src/goom_gl.h
    src/Main.h
)

list(APPEND DEPLIBS goom::lib libs::vivid blend2d::blend2d)

add_library(${VIS_GOOM_TARGET_NAME} SHARED
            ${VIS_GOOM_HEADERS}
            ${VIS_GOOM_SOURCES}
)
add_dependencies(${VIS_GOOM_TARGET_NAME}
                 goom::lib
                 libs::vivid
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

target_link_libraries(${VIS_GOOM_TARGET_NAME} PRIVATE ${DEPLIBS})
target_include_directories(${VIS_GOOM_TARGET_NAME}
                           PRIVATE
                           ${GOOM_INCLUDE_DIR}
)
target_include_directories(${VIS_GOOM_TARGET_NAME}
                           SYSTEM PRIVATE
                           ${GOOM_LIBS_DIR}/lib/std_fmt/include
                           ${GLM_INCLUDE_DIR}
                           ${KODI_INCLUDE_DIR}
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

set(INSTALL_DEST_DIR ".")
install(TARGETS ${VIS_GOOM_TARGET_NAME} DESTINATION "${INSTALL_DEST_DIR}")
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/fix-vis-goom-lib-name.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/fix-vis-goom-lib-name.cmake
        @ONLY
)
install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/fix-vis-goom-lib-name.cmake)
install(DIRECTORY ${GOOM_RESOURCES_DIR} DESTINATION "${INSTALL_DEST_DIR}")
configure_file(
        ${GOOM_ADDON_FILES_DIR}/addon.xml.in
        ${CMAKE_CURRENT_BINARY_DIR}/addon.xml
        @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/addon.xml DESTINATION "${INSTALL_DEST_DIR}")


get_target_property(goom_addon_compile_options ${VIS_GOOM_TARGET_NAME} COMPILE_OPTIONS)
get_target_property(goom_addon_compile_features ${VIS_GOOM_TARGET_NAME} COMPILE_FEATURES)

message(STATUS "Vis.goom: Cmake version              = \"${CMAKE_VERSION}\"")
message(STATUS "Vis.goom: Cmake generator            = \"${CMAKE_GENERATOR}\"")
message(STATUS "Vis.goom: CMAKE_SYSTEM_NAME          = \"${CMAKE_SYSTEM_NAME}\".")
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
message(STATUS "Vis.goom: C++ standard               = \"${CMAKE_CXX_STANDARD}\".")
message(STATUS "Vis.goom: C++ standard required      = \"${CMAKE_CXX_STANDARD_REQUIRED}\".")
message(STATUS "Vis.goom: C++ extensions             = \"${CMAKE_CXX_EXTENSIONS}\".")
message(STATUS "Vis.goom: C++ compile features       = \"${goom_addon_compile_features}\".")
message(STATUS "Vis.goom: C++ compile options        = \"${goom_addon_compile_options}\".")
message(STATUS "Vis.goom: ENABLE_TESTING             = \"${ENABLE_TESTING}\".")
message(STATUS "Vis.goom: GOOM_BUILD_DIR             = \"${GOOM_BUILD_DIR}\".")
message(STATUS "Vis.goom: GOOM_INCLUDE_DIR           = \"${GOOM_INCLUDE_DIR}\".")
message(STATUS "Vis.goom: GOOM_LIBS_DIR              = \"${GOOM_LIBS_DIR}\".")
message(STATUS "Vis.goom: GOOM_ADDON_FILES_DIR       = \"${GOOM_ADDON_FILES_DIR}\".")
message(STATUS "Vis.goom: GOOM_RESOURCES_DIR         = \"${GOOM_RESOURCES_DIR}\".")
message(STATUS "Vis.goom: KODI_ROOT_DIR              = \"${KODI_ROOT_DIR}\".")
message(STATUS "Vis.goom: KODI_INCLUDE_DIR           = \"${KODI_INCLUDE_DIR}\".")
message(STATUS "Vis.Goom: OPENGL_DEFINITIONS         = \"${OPENGL_DEFINITIONS}\".")
message(STATUS "Vis.Goom: OPENGLES_DEFINITIONS       = \"${OPENGLES_DEFINITIONS}\".")
message(STATUS "Vis.goom: DEPLIBS                    = \"${DEPLIBS}\".")
