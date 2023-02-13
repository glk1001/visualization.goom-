#include "catch2/catch.hpp"
#include "draw/goom_draw.h"
#include "draw/goom_draw_to_container.h"
#include "goom_graphic.h"
#include "point2d.h"

#include <format>
#include <vector>

namespace GOOM::UNIT_TESTS
{

using DRAW::GoomDrawToContainer;
using DRAW::MultiplePixels;
using ColorsList = GoomDrawToContainer::ColorsList;

static constexpr uint32_t WIDTH  = 100;
static constexpr uint32_t HEIGHT = 100;

struct PixelInfo
{
  Point2dInt point{};
  MultiplePixels colors{};
};

void CheckPixels(const std::vector<PixelInfo>& changedPixels,
                 const std::vector<PixelInfo>& expectedPixels)
{
  // '1' is old, 'expectedPixels.size() - 1' is new.
  for (size_t i = 0; i < expectedPixels.size(); ++i)
  {
    const Point2dInt& point      = expectedPixels[i].point;
    const MultiplePixels& colors = expectedPixels[i].colors;

    const PixelInfo& coords = changedPixels[i];

    INFO(std20::format("i = {}, coords = ({}, {}), (x, y) = ({}, {})",
                       i,
                       coords.point.x,
                       coords.point.y,
                       point.x,
                       point.y));
    REQUIRE(coords.point.x == point.x);
    REQUIRE(coords.point.y == point.y);

    INFO(std20::format("coords.colors[{}] = ({}, {}, {}, {}), colors[{}] = ({}, {}, {}, {})",
                       0,
                       coords.colors.color1.R(),
                       coords.colors.color1.G(),
                       coords.colors.color1.B(),
                       coords.colors.color1.A(),
                       0,
                       colors.color1.R(),
                       colors.color1.G(),
                       colors.color1.B(),
                       colors.color1.A()));
    REQUIRE(coords.colors.color1 == colors.color1);
    REQUIRE(coords.colors.color2 == BLACK_PIXEL);
  }
}

void CheckContainer(const GoomDrawToContainer& draw, const std::vector<PixelInfo>& expectedPixels)
{
  INFO(std20::format("draw.GetNumChangedCoords() = {}", draw.GetNumChangedCoords()));
  REQUIRE(draw.GetNumChangedCoords() == expectedPixels.size());

  std::vector<PixelInfo> changedPixels{};
  const auto emplaceCoords = [&](const Point2dInt& point, const ColorsList& colorsList)
  {
    changedPixels.emplace_back(PixelInfo{
        point, {colorsList.colorsArray[0], BLACK_PIXEL}
    });
  };
  draw.IterateChangedCoordsNewToOld(emplaceCoords);
  REQUIRE(changedPixels.size() == expectedPixels.size());

  CheckPixels(changedPixels, expectedPixels);
}

auto FillDrawContainer(GoomDrawToContainer* const draw, const size_t numChanged)
    -> std::vector<PixelInfo>
{
  std::vector<PixelInfo> pixelsNewToOld{};
  // Add some changed coords - '1' is old, 'numChanged' is new.
  for (size_t i = 1; i <= numChanged; ++i)
  {
    const auto point = Point2dInt{static_cast<int32_t>(i), static_cast<int32_t>(i)};
    const auto chan0 = static_cast<PixelChannelType>(i);
    const auto chan1 = static_cast<PixelChannelType>(i + 1);
    const Pixel color0{
        {chan0, chan0, chan0, 255U}
    };
    const Pixel color1{
        {chan1, chan1, chan1, 0U}
    };
    const MultiplePixels colors{color0, color1};

    pixelsNewToOld.emplace_back(PixelInfo{point, colors});

    draw->DrawPixels(point, colors);
    REQUIRE(draw->GetPixels(point).color1 == color0);
    REQUIRE(draw->GetPixels(point).color2 == BLACK_PIXEL);
  }
  std::reverse(begin(pixelsNewToOld), end(pixelsNewToOld));

  REQUIRE(pixelsNewToOld.size() == numChanged);
  REQUIRE(pixelsNewToOld.front().point.x == static_cast<int32_t>(numChanged));
  REQUIRE(pixelsNewToOld.front().point.y == static_cast<int32_t>(numChanged));
  REQUIRE(pixelsNewToOld.back().point.x == 1);
  REQUIRE(pixelsNewToOld.back().point.y == 1);

  return pixelsNewToOld;
}

TEST_CASE("Test DrawMovingText to Container", "[GoomDrawToContainer]")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr auto NUM_CHANGED_COORDS = 5U;
  std::vector<PixelInfo> pixelsNewToOld    = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  int32_t i = NUM_CHANGED_COORDS;
  for (const auto& pixelInfo : pixelsNewToOld)
  {
    REQUIRE(pixelInfo.point.x == i);
    REQUIRE(pixelInfo.point.y == i);
    --i;
  }

  CheckContainer(draw, pixelsNewToOld);

  static constexpr auto NEW_SIZE = NUM_CHANGED_COORDS / 2U;
  draw.ResizeChangedCoordsKeepingNewest(NEW_SIZE);
  pixelsNewToOld.resize(NEW_SIZE);
  i = NUM_CHANGED_COORDS;
  for (const auto& pixelInfo : pixelsNewToOld)
  {
    REQUIRE(pixelInfo.point.x == i);
    REQUIRE(pixelInfo.point.y == i);
    --i;
  }
  CheckContainer(draw, pixelsNewToOld);
}

TEST_CASE("Test DrawMovingText to Container with Duplicates", "[GoomDrawToContainerDuplicates]")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr size_t NUM_CHANGED_COORDS = 5;
  std::vector<PixelInfo> pixelsNewToOld      = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  const Pixel color0{
      {10, 10, 10, 255U}
  };
  const Pixel color1{
      {11, 11, 11, 0U}
  };
  const MultiplePixels colors{color0, color1};
  const PixelInfo oldest = pixelsNewToOld.back();
  draw.DrawPixels(oldest.point, colors);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS);

  const ColorsList& colorsListOldest = draw.GetColorsList(oldest.point);
  REQUIRE(2 == colorsListOldest.count);

  const Point2dInt& coords0     = draw.GetChangedCoordsList()[0];
  const ColorsList& colorsList0 = draw.GetColorsList(coords0);
  REQUIRE(colorsListOldest.count == colorsList0.count);
  REQUIRE(colorsListOldest.colorsArray == colorsList0.colorsArray);

  draw.ResizeChangedCoordsKeepingNewest(NUM_CHANGED_COORDS - 1);
  REQUIRE(draw.GetNumChangedCoords() == NUM_CHANGED_COORDS - 1);
  REQUIRE(0 == draw.GetColorsList(oldest.point).count);
}

TEST_CASE("Test DrawMovingText ClearAll", "[GoomDrawToContainerClearAll]")
{
  GoomDrawToContainer draw{
      {WIDTH, HEIGHT}
  };

  draw.SetBuffIntensity(1.0F);

  static constexpr size_t NUM_CHANGED_COORDS  = 5;
  const std::vector<PixelInfo> pixelsNewToOld = FillDrawContainer(&draw, NUM_CHANGED_COORDS);

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const ColorsList& colorsList = draw.GetColorsList(pixelInfo.point);
    REQUIRE(0 != colorsList.count);
  }

  draw.ClearAll();

  REQUIRE(draw.GetNumChangedCoords() == 0);
  REQUIRE(draw.GetChangedCoordsList().empty());

  for (const auto& pixelInfo : pixelsNewToOld)
  {
    const ColorsList& colorsList = draw.GetColorsList(pixelInfo.point);
    REQUIRE(0 == colorsList.count);
  }
}

} // namespace GOOM::UNIT_TESTS
