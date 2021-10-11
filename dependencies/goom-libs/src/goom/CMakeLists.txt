cmake_minimum_required(VERSION 3.12)

project(Goom LANGUAGES CXX)

if(WIN32)
    add_definitions(-D_WIN32PC)
endif()
if ($ENV{DO_GOOM_LOGGING})
    ADD_DEFINITIONS(-DDO_GOOM_LOGGING)
    MESSAGE("Added 'DO_GOOM_LOGGING' #define.")
endif()

add_library(goom
    STATIC
        src/color/color_data/color_data_maps.cpp
        src/color/color_data/extra_color_groups.cpp
        src/color/colormaps.cpp
        src/color/colorutils.cpp
        src/color/random_colormaps.cpp
        src/color/random_colormaps_manager.cpp
        src/utils/goomrand.cpp
        src/utils/graphics/image_bitmaps.cpp
        src/utils/graphics/small_image_bitmaps.cpp
        src/utils/logging.cpp
        src/utils/mathutils.cpp
        src/utils/name_value_pairs.h
        src/utils/strutils.cpp
        src/utils/t_values.cpp
        src/utils/thread_pool.cpp
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
        src/draw/goom_draw.cpp
        src/draw/goom_draw_to_buffer.cpp
        src/draw/goom_draw_to_container.cpp
        src/draw/goom_draw_to_many.cpp
        src/draw/text_draw.cpp
        src/goom_control.cpp
        src/goom_graphic.cpp
        src/sound_info.cpp
        src/visual_fx/filters/amulet.cpp
        src/visual_fx/filters/crystal_ball.cpp
        src/visual_fx/filters/filter_buffers.cpp
        src/visual_fx/filters/filter_buffers_service.cpp
        src/visual_fx/filters/filter_colors_service.cpp
        src/visual_fx/filters/filter_settings_service.cpp
        src/visual_fx/filters/filter_zoom_vector.cpp
        src/visual_fx/filters/filter_zoom_vector_effects.cpp
        src/visual_fx/filters/hypercos.cpp
        src/visual_fx/filters/image_displacement.cpp
        src/visual_fx/filters/image_displacement_list.cpp
        src/visual_fx/filters/image_speed_coeffs.cpp
        src/visual_fx/filters/image_velocity.cpp
        src/visual_fx/filters/noise.cpp
        src/visual_fx/filters/normalized_coords.cpp
        src/visual_fx/filters/planes.cpp
        src/visual_fx/filters/rotation.cpp
        src/visual_fx/filters/scrunch.cpp
        src/visual_fx/filters/simple_speed_coefficients_effect.h
        src/visual_fx/filters/speed_coefficients_effect.h
        src/visual_fx/filters/speedway.cpp
        src/visual_fx/filters/tan_effect.cpp
        src/visual_fx/filters/wave.cpp
        src/visual_fx/filters/y_only.cpp
        src/visual_fx/ifs/colorizer.cpp
        src/visual_fx/ifs/fractal.cpp
        src/visual_fx/ifs/low_density_blurrer.cpp
        src/visual_fx/tentacles/tentacle_driver.cpp
        src/visual_fx/tentacles/tentacles.cpp
        src/visual_fx/tubes/tubes.cpp
        src/visual_fx/flying_stars_fx.cpp
        src/visual_fx/goom_dots_fx.cpp
        src/visual_fx/goom_visual_fx.h
        src/visual_fx/ifs_dancers_fx.cpp
        src/visual_fx/image_fx.cpp
        src/visual_fx/lines_fx.cpp
        src/visual_fx/tentacles_fx.cpp
        src/visual_fx/tube_fx.cpp
        src/visual_fx/zoom_filter_fx.cpp
        src/visual_fx/image_fx.h src/visual_fx/image_fx.cpp)

target_include_directories(goom
    PUBLIC
        ${GOOM_ROOT_DIR}/goom/include
        ${GOOM_ROOT_DIR}/utils/include
    PRIVATE
        ${GOOM_ROOT_DIR}/goom/include/goom
        ${GOOM_ROOT_DIR}/goom/src
)

target_include_directories(goom
    SYSTEM PRIVATE
        ${GOOM_ROOT_DIR}/vivid/include
        ${THIRD_PARTY_INCLUDE_DIR}/fmt/include
        ${THIRD_PARTY_INCLUDE_DIR}/magic_enum/include
        ${FREETYPE_INCLUDE_DIRS}
)

set_project_warnings(goom)
enable_sanitizers(goom)

set_target_properties(goom
    PROPERTIES
        CXX_STANDARD ${CPP_STANDARD}
        POSITION_INDEPENDENT_CODE ${POS_INDEP_CODE}
)

get_target_property(goom_compile_options  goom COMPILE_OPTIONS)
get_target_property(goom_compile_features goom COMPILE_FEATURES)

message(STATUS "Goom C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Goom C++ compile features = \"${goom_compile_features}\".")
message(STATUS "Goom C++ compile options = \"${goom_compile_options}\".")
