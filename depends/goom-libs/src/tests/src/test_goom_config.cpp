#include <array>
#include <catch2/catch_test_macros.hpp>
#include <string>

import Goom.Lib.GoomConfigPaths;
import Goom.Lib.GoomPaths;
import Goom.Lib.GoomUtils;

namespace GOOM::UNIT_TESTS
{

// NOLINTBEGIN(bugprone-chained-comparison): Catch2 needs to fix this.
// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("Paths")
{
  static constexpr auto TEST_STR           = "test"_cts;
  static constexpr auto TEST_STR_ARRAY     = TEST_STR.buffer;
  static constexpr auto EXPECTED_STR_ARRAY = std::to_array({'t', 'e', 's', 't', '\0'});
  REQUIRE(TEST_STR_ARRAY == EXPECTED_STR_ARRAY);

  const auto pathSep = GetPathSep().to_string();
  REQUIRE(not pathSep.empty());

  REQUIRE(join_paths(DATA_DIR, "fonts") == std::string{"data"} + pathSep + "fonts");
  REQUIRE(join_paths(MEDIA_DIR, "images", "image_fx") ==
          std::string{"media"} + pathSep + "images" + pathSep + "image_fx");

  REQUIRE(DATA_DIR.to_string() == std::string{"data"});
  REQUIRE(FONTS_DIR.to_string() == join_paths("data", "fonts"));
  REQUIRE(IMAGE_FX_DIR.to_string() == join_paths("media", "images", "image_fx"));
  REQUIRE(SHADERS_DIR.to_string() == join_paths("resources", "data", "shaders"));
}
// NOLINTEND(readability-function-cognitive-complexity)
// NOLINTEND(bugprone-chained-comparison)

} // namespace GOOM::UNIT_TESTS
