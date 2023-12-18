#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "goom/goom_config.h"

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Paths")
{
  REQUIRE(DATA_DIR.to_string() == std::string{"data"});
  REQUIRE(FONTS_DIR.to_string() == std::string{"data"} + PATH_SEP + "fonts");
  REQUIRE(IMAGE_FX_DIR.to_string() ==
          std::string{"media"} + PATH_SEP + "images" + PATH_SEP + "image_fx");
  REQUIRE(SHADERS_DIR.to_string() ==
          std::string{"resources"} + PATH_SEP + "data" + PATH_SEP + "shaders");

  REQUIRE(join_paths(DATA_DIR, "fonts") == std::string{"data"} + PATH_SEP + "fonts");
  REQUIRE(join_paths(MEDIA_DIR, "images", "image_fx") ==
          std::string{"media"} + PATH_SEP + "images" + PATH_SEP + "image_fx");
}
// NOLINTEND(readability-function-cognitive-complexity)

} // namespace GOOM::UNIT_TESTS
