cmake_minimum_required(VERSION 3.12)

project(Goom LANGUAGES CXX)

##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
##set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
##set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
# To test Kodi, run:
#
#  LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.6 ASAN_OPTIONS=allocator_may_return_null=1:detect_leaks=0unmap_shadow_on_exit=1 kodi &

if(WIN32)
    add_definitions(-D_WIN32PC)
endif()
if ($ENV{DO_GOOM_LOGGING})
    ADD_DEFINITIONS(-DDO_GOOM_LOGGING)
    MESSAGE("Added 'DO_GOOM_LOGGING' #define.")
endif()

add_library(goom
    STATIC
        src/control/goom_all_visual_fx.cpp
        src/control/goom_all_visual_fx_color_maps.cpp
        src/control/goom_events.cpp
        src/control/goom_image_buffers.cpp
        src/control/goom_message_displayer.cpp
        src/control/goom_music_lines_reactor.cpp
        src/control/goom_music_settings_reactor.cpp
        src/control/goom_states.cpp
        src/control/goom_title_displayer.cpp
        src/draw/draw_methods.cpp
        src/draw/goom_draw_to_buffer.cpp
        src/draw/goom_draw_to_container.cpp
        src/draw/goom_draw_to_many.cpp
        src/draw/text_draw.cpp
        src/filters/amulet.cpp
        src/filters/crystal_ball.cpp
        src/filters/filter_buffers.cpp
        src/filters/filter_buffers_service.cpp
        src/filters/filter_colors_service.cpp
        src/filters/filter_settings_service.cpp
        src/filters/filter_zoom_vector.cpp
        src/filters/filter_zoom_vector_effects.cpp
        src/filters/hypercos.cpp
        src/filters/image_displacement.cpp
        src/filters/image_displacement_list.cpp
        src/filters/image_speed_coeffs.cpp
        src/filters/image_velocity.cpp
        src/filters/noise.cpp
        src/filters/normalized_coords.cpp
        src/filters/planes.cpp
        src/filters/rotation.cpp
        src/filters/scrunch.cpp
        src/filters/simple_speed_coefficients_effect.h
        src/filters/speed_coefficients_effect.h
        src/filters/speedway.cpp
        src/filters/tan_effect.cpp
        src/filters/wave.cpp
        src/filters/y_only.cpp
        src/goom_control.cpp
        src/goom_draw.cpp
        src/goom_graphic.cpp
        src/ifs/colorizer.cpp
        src/ifs/fractal.cpp
        src/ifs/low_density_blurrer.cpp
        src/sound_info.cpp
        src/tentacles/tentacle_driver.cpp
        src/tentacles/tentacles.cpp
        src/tubes/tubes.cpp
        src/visual_fx/flying_stars_fx.cpp
        src/visual_fx/goom_dots_fx.cpp
        src/visual_fx/ifs_dancers_fx.cpp
        src/visual_fx/goom_visual_fx.h
        src/visual_fx/lines_fx.cpp
        src/visual_fx/tentacles_fx.cpp
        src/visual_fx/tube_fx.cpp
        src/visual_fx/zoom_filter_fx.cpp
        )

#set_source_files_properties(src/flying_stars_fx.cpp PROPERTIES COMPILE_FLAGS -fsanitize=address)

target_include_directories(goom
    PUBLIC
        ${LIBS_DIR}/goom/include
        ${LIBS_DIR}/goomutils/include
    PRIVATE
        ${LIBS_DIR}/goom/include/goom
)

target_include_directories(goom
    SYSTEM PRIVATE
        ${LIBS_DIR}/vivid/include
        ${LIBS_DIR}/fmt/include
        ${LIBS_DIR}/magic_enum/include
        ${FREETYPE_INCLUDE_DIRS}
        # Need this '..' hack here. Not nice!
        SYSTEM ${GLM_INCLUDE_DIR}/..
)

set_target_properties(goom
    PROPERTIES
        CXX_STANDARD ${CPP_STANDARD}
        POSITION_INDEPENDENT_CODE ${POS_INDEP_CODE}
)

message(STATUS "Goom C++ standard = \"${CPP_STANDARD}\".")
