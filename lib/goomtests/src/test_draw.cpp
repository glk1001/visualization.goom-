#include "catch2/catch.hpp"
#include "draw/goom_draw_to_container.h"
#include "goom_graphic.h"

#include <cstdint>
#include <format>
#include <vector>

using GOOM::Pixel;
using GOOM::DRAW::GoomDrawToContainer;
using ColorsList = GoomDrawToContainer::ColorsList;

constexpr uint32_t WIDTH = 100;
constexpr uint32_t HEIGHT = 100;


struct PixelInfo
{
  int32_t x{};
  int32_t y{};
  std::vector<Pixel> colors{};
};

void CheckPixels(const std::vector<PixelInfo>& changedPixels,
                 const std::vector<PixelInfo>& expectedPixels)
{
  // '1' is old, 'expectedPixels.size() - 1' is new.
  for (size_t i = 0; i < expectedPixels.size(); i++)
  {
    const int32_t x = expectedPixels[i].x;
    const int32_t y = expectedPixels[i].y;
    const std::vector<Pixel>& colors = expectedPixels[i].colors;

    const PixelInfo coords = changedPixels[i];

    INFO(
        std20::format("i = {}, coords = ({}, {}), (x, y) = ({}, {})", i, coords.x, coords.y, x, y));
    REQUIRE(coords.x == x);
    REQUIRE(coords.y == y);

    REQUIRE(coords.colors.size() == colors.size());
    for (size_t c = 0; c < colors.size(); c++)
    {
      INFO(std20::format("coords.colors[{}] = ({}, {}, {}, {}), colors[{}] = ({}, {}, {}, {})", c,
                         coords.colors[c].R(), coords.colors[c].G(), coords.colors[c].B(),
                         coords.colors[c].A(), c, colors[c].R(), colors[c].G(), colors[c].B(),
                         colors[c].A()));
      REQUIRE(coords.colors[c] == colors[c]);
    }
  }
}

void CheckContainer(const GoomDrawToContainer& draw, const std::vector<PixelInfo>& expectedPixels)
{
  INFO(std20::format("draw.GetNumChangedCoords() = {}", draw.GetNumChangedCoords()));
  REQUIRE(draw.GetNumChangedCoords() == expectedPixels.size());

  std::vector<PixelInfo> changedPixels{};
  const auto emplaceCoords = [&](const int32_t x, const int32_t y, const ColorsList& colorsList) {
    changedPixels.emplace_back(PixelInfo{x, y, colorsList[0]});
  };
  draw.IterateChangedCoordsNewToOld(emplaceCoords);
  REQUIRE(changedPixels.size() == expectedPixels.size());

  CheckPixels(changedPixels, expectedPixels);
}

auto FillDrawContainer(GoomDrawToContainer* draw, const size_t numChanged) -> std::vector<PixelInfo>
{
  std::vector<PixelInfo> pixelsNewToOld{};
  // Add some changed coords - '1' is old, 'numChanged' is new.
  for (size_t i = 1; i <= numChanged; i++)
  {
    const auto x = static_cast<int32_t>(i);
    const auto y = static_cast<int32_t>(i);
    const auto c0 = static_cast<uint8_t>(i);
    const auto c1 = static_cast<uint8_t>(i + 1);
    const Pixel color0{{c0, c0, c0, 255U}};
    const Pixel color1{{c1, c1, c1, 0U}};
    const std::vector<Pixel> colors{color0, color1};

    pixelsNewToOld.emplace_back(PixelInfo{x, y, colors});

    draw->DrawPixels(x, y, colors);
    REQUIRE(draw->GetPixels(x, y)[0] == color0);
    REQUIRE(draw->GetPixels(x, y)[1] == color1);
  }
  std::reverse(pixelsNewToOld.begin(), pixelsNewToOld.end());
  REQUIRE(pixelsNewToOld.size() == numChanged);

  return pixelsNewToOld;
}

TEST_CASE("Test Draw to Container", "[GoomDrawToContainer]")
{
  GoomDrawToContainer draw{WIDTH, HEIGHT};

  draw.SetBuffIntensity(1.0F);
  draw.SetAllowOverexposed(true);

  constexpr size_t NUM_CHANGED_COORDS = 5;
  std::vector<PixelInfo> pixelsNewToOld = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  int32_t i = NUM_CHANGED_COORDS;
  for (const auto& p : pixelsNewToOld)
  {
    REQUIRE(p.x == i);
    REQUIRE(p.y == i);
    i--;
  }

  CheckContainer(draw, pixelsNewToOld);

  const size_t NEW_SIZE = NUM_CHANGED_COORDS / 2;
  draw.ResizeChangedCoordsKeepingNewest(NEW_SIZE);
  pixelsNewToOld.resize(NEW_SIZE);
  i = NUM_CHANGED_COORDS;
  for (const auto& p : pixelsNewToOld)
  {
    REQUIRE(p.x == i);
    REQUIRE(p.y == i);
    i--;
  }
  CheckContainer(draw, pixelsNewToOld);
}

TEST_CASE("Test Draw to Container with Duplicates", "[GoomDrawToContainerDuplicates]")
{
  GoomDrawToContainer draw{WIDTH, HEIGHT};

  draw.SetBuffIntensity(1.0F);
  draw.SetAllowOverexposed(true);

  constexpr size_t NUM_CHANGED_COORDS = 5;
  std::vector<PixelInfo> pixelsNewToOld = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  const Pixel color0{{10, 10, 10, 255U}};
  const Pixel color1{{11, 11, 11, 0U}};
  const std::vector<Pixel> colors{color0, color1};
  PixelInfo oldest = pixelsNewToOld.back();
  draw.DrawPixels(oldest.x, oldest.y, colors);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS);

  const ColorsList& colorsListOldest = draw.GetColorsList(oldest.x, oldest.y);
  REQUIRE(colorsListOldest.size() == 2);

  const GoomDrawToContainer::Coords& coords0 = draw.GetChangedCoordsList()[0];
  const ColorsList& colorsList0 = draw.GetColorsList(coords0.x, coords0.y);
  REQUIRE(colorsListOldest == colorsList0);

  draw.ResizeChangedCoordsKeepingNewest(NUM_CHANGED_COORDS - 1);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS - 1);
  REQUIRE(draw.GetColorsList(oldest.x, oldest.y).empty());
}

TEST_CASE("Test Draw ClearAll", "[GoomDrawToContainerClearAll]")
{
  GoomDrawToContainer draw{WIDTH, HEIGHT};

  draw.SetBuffIntensity(1.0F);
  draw.SetAllowOverexposed(true);

  constexpr size_t NUM_CHANGED_COORDS = 5;
  std::vector<PixelInfo> pixelsNewToOld = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const ColorsList& colorsList = draw.GetColorsList(pixelInfo.x, pixelInfo.y);
    REQUIRE(!colorsList.empty());
  }

  draw.ClearAll();

  REQUIRE(draw.GetNumChangedCoords() == 0);
  REQUIRE(draw.GetChangedCoordsList().empty());

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const ColorsList& colorsList = draw.GetColorsList(pixelInfo.x, pixelInfo.y);
    REQUIRE(colorsList.empty());
  }
}
