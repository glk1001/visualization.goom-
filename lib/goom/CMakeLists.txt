cmake_minimum_required(VERSION 3.13)
project(goom)

if(WIN32)
 add_definitions(-D_WIN32PC)
endif()

add_library(${PROJECT_NAME} STATIC
                 src/goom_testing.c
                 src/goom_tools.c
                 src/config_param.c
                 src/convolve_fx.c
                 src/filters.c
                 src/flying_stars_fx.c
                 src/gfontlib.c
                 src/gfontrle.c
                 src/goom_core.c
                 src/goom_logging.c
                 src/graphic.c
                 src/ifs.c
                 src/lines.c
                 src/mathtools.c
                 src/sound_tester.c
                 src/surf3d.c
                 src/tentacle3d.c
                 src/plugin_info.c
                 src/v3d.c
                 src/drawmethods.c
)
		 
add_library(libs::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "Goom compile features = \"${COMMON_C_COMPILE_FEATURES}\".")
target_compile_features(${PROJECT_NAME}
	PRIVATE
		${COMMON_C_COMPILE_FEATURES} 
)

message(STATUS "Goom C warnings = \"${COMMON_C_WARNINGS}\".")
target_compile_options(${PROJECT_NAME}
    PRIVATE 
		  ${COMMON_C_WARNINGS}
)

set_property(TARGET goom PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET goom PROPERTY C_STANDARD 11)

