cmake_minimum_required(VERSION 3.12)

project(GoomTests LANGUAGES CXX)

##set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address")

if(WIN32)
    add_definitions(-D_WIN32PC)
endif()

find_package(Threads)

add_executable(goomtests
        src/test_main.cpp
        src/draw/test_draw.cpp
        src/filters/test_filter_buffers.cpp
        src/filters/test_filter_colors_service.cpp
        src/filters/test_normalized_coords.cpp
        src/test_pixels.cpp
        src/test_filter_fx.cpp
        src/utils/test_colorutils.cpp
        src/utils/test_goomrand.cpp
        src/utils/test_parallel_utils.cpp
        src/utils/test_strutils.cpp
)

target_link_libraries(goomtests
    PRIVATE
        goom
        vivid
        ${CMAKE_THREAD_LIBS_INIT}
)

target_include_directories(goomtests
    PRIVATE 
        ${LIBS_DIR}/catch2/include
        ${GOOM_ROOT_DIR}/goom/include
        ${GOOM_ROOT_DIR}/goom/include/goom
        ${GOOM_ROOT_DIR}/goom/src
)  
target_include_directories(goomtests
    SYSTEM PRIVATE 
        ${LIBS_DIR}/fmt/include
        ${LIBS_DIR}/magic_enum/include
)

set_project_warnings(goomtests)

set_target_properties(goomtests
    PROPERTIES
        CXX_STANDARD ${CPP_STANDARD}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/bin
)

catch_discover_tests(goomtests)

message(STATUS "Goom Tests C++ standard = \"${CPP_STANDARD}\".")
