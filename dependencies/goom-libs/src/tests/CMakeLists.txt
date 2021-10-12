cmake_minimum_required(VERSION 3.12)

project(GoomTests LANGUAGES CXX)

if(WIN32)
    add_definitions(-D_WIN32PC)
endif()

find_package(Threads)

add_executable(goomtests
        src/test_main.cpp
        src/test_pixels.cpp
        src/color/test_colorutils.cpp
        src/draw/test_draw.cpp
        src/filters/test_filter_buffers.cpp
        src/filters/test_filter_colors_service.cpp
        src/filters/test_normalized_coords.cpp
        src/sound/test_sound_info.cpp
        src/utils/test_goomrand.cpp
        src/utils/test_parallel_utils.cpp
        src/utils/test_strutils.cpp
        src/utils/test_t_values.cpp
        src/visual_fx/test_filter_fx.cpp
)

target_link_libraries(goomtests
    PRIVATE
        goom
        vivid
        Catch2::Catch2
        ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(goomtests
    PRIVATE 
        ${GOOM_ROOT_DIR}/goom/include
        ${GOOM_ROOT_DIR}/goom/include/goom
        ${GOOM_ROOT_DIR}/goom/src
)  
target_include_directories(goomtests
    SYSTEM PRIVATE 
        ${THIRD_PARTY_INCLUDE_DIR}/fmt/include
        ${THIRD_PARTY_INCLUDE_DIR}/magic_enum/include
)

set_project_warnings(goomtests)
enable_sanitizers(goomtests)
configure_linker(goomtests)

set_target_properties(goomtests
    PROPERTIES
        CXX_STANDARD ${CPP_STANDARD}
)

catch_discover_tests(goomtests)

get_target_property(goomtests_compile_options  goomtests COMPILE_OPTIONS)
get_target_property(goomtests_compile_features goomtests COMPILE_FEATURES)
get_target_property(goomtests_link_options     goomtests LINK_OPTIONS)

message(STATUS "Goom Tests C++ standard = \"${CPP_STANDARD}\".")
message(STATUS "Goom Tests C++ compile features = \"${goomtests_compile_features}\".")
message(STATUS "Goom Tests C++ compile options = \"${goomtests_compile_options}\".")
message(STATUS "Goom Tests C++ link options = \"${goomtests_link_options}\".")
