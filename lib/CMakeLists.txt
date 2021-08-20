add_subdirectory(goom)
add_subdirectory(goomutils)
add_subdirectory(vivid)

#if($ENV{GoomTests})
    message(STATUS "Building GoomTests.")
    add_subdirectory(goomtests)
#endif()
