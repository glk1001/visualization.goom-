cmake_minimum_required(VERSION 3.13)
project(goom)

if(WIN32)
 add_definitions(-D_WIN32PC)
endif()

set(GOOM_SOURCES src/goom_tools.c
                 src/config_param.c
                 src/convolve_fx.c
                 src/filters.c
                 src/flying_stars_fx.c
                 src/gfontlib.c
                 src/gfontrle.c
                 src/goom_core.c
                 src/goom_logging.c
                 src/goom_testing.c
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

set(GOOM_HEADERS src/goom.h
                 src/goom_plugin_info.h
                 src/goom_typedefs.h
                 src/goom_graphic.h
                 src/goom_config_param.h
                 src/goom_visual_fx.h
                 src/goom_filters.h
                 src/goom_logging.h
                 src/goom_testing.h
                 src/goom_tools.h
                 src/goom_config.h
                 src/tentacle3d.h
)		 

add_library(goom STATIC ${GOOM_SOURCES} ${GOOM_HEADERS})
set_property(TARGET goom PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET goom PROPERTY C_STANDARD 11)
