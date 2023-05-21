macro(vis_goom_pp_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

vis_goom_pp_supports_sanitizers()
option(vis_goom_pp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
# option(vis_goom_pp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
option(vis_goom_pp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
option(vis_goom_pp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
option(vis_goom_pp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
option(vis_goom_pp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)

if (NOT SUPPORTS_UBSAN
    OR vis_goom_pp_ENABLE_SANITIZER_UNDEFINED
    OR vis_goom_pp_ENABLE_SANITIZER_ADDRESS
    OR vis_goom_pp_ENABLE_SANITIZER_THREAD
    OR vis_goom_pp_ENABLE_SANITIZER_LEAK)
    set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
else ()
    set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
endif ()

vis_goom_pp_enable_hardening(global ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
vis_goom_pp_assure_out_of_source_builds()
vis_goom_pp_enable_cache()

option(vis_goom_pp_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
