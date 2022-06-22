#include "star_drawer.h"

#include "color/color_utils.h"
#include "draw/goom_draw.h"
#include "goom_config.h"
#include "goom_graphic.h"
#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"

namespace GOOM::VISUAL_FX::FLYING_STARS
{

using COLOR::GetColorMultiply;
using DRAW::IGoomDraw;
using DRAW::MultiplePixels;
using UTILS::TValue;
using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

StarDrawer::StarDrawer(IGoomDraw& draw,
                       const IGoomRand& goomRand,
                       const SmallImageBitmaps& smallBitmaps,
                       const GetMixedColorsFunc& getMixedColorsFunc) noexcept
  : m_draw{draw},
    m_goomRand{goomRand},
    m_smallBitmaps{smallBitmaps},
    m_getMixedColorsFunc{getMixedColorsFunc}
{
}

auto StarDrawer::DrawStar(const Star& star,
                          const float speedFactor,
                          const DrawFunc& drawFunc) noexcept -> void
{
  const float tAge = star.GetAge() / star.GetMaxAge();
  static constexpr float EXTRA_T_AGE = 0.5F;
  const float tAgeMax = std::min(tAge + EXTRA_T_AGE, 1.0F);

  const float brightness = GetBrightness(tAge);
  const float partMultiplier = GetPartMultiplier();
  const auto [numParts, elementSize] = GetNumPartsAndElementSize(tAge);

  TValue tAgePart{TValue::StepType::SINGLE_CYCLE, numParts};
  const Point2dInt point0 = star.GetStartPos().ToInt();

  Point2dInt point1 = point0;
  for (uint32_t j = 1; j <= numParts; ++j)
  {
    const auto thisPartFraction = static_cast<float>(j) / static_cast<float>(numParts);
    const Vec2dFlt thisPartVelocity = partMultiplier * (thisPartFraction * star.GetVelocity());
    const Vec2dFlt twistFrequency = speedFactor * thisPartVelocity;

    const Point2dInt point2 = point0 - GetPointVelocity(twistFrequency, thisPartVelocity);

    const float t = STD20::lerp(tAge, tAgeMax, tAgePart());
    const float thisPartBrightness = thisPartFraction * brightness;
    const MultiplePixels thisPartColors = m_getMixedColorsFunc(thisPartBrightness, star, t);

    drawFunc(point1, point2, elementSize, thisPartColors);

    point1 = point2;
    tAgePart.Increment();
  }
}

inline auto StarDrawer::GetPointVelocity(const Vec2dFlt& twistFrequency,
                                         const Vec2dFlt& velocity) noexcept -> Vec2dInt
{
  static constexpr float HALF = 0.5F;
  return {static_cast<int32_t>(HALF * (1.0F + std::sin(twistFrequency.x)) * velocity.x),
          static_cast<int32_t>(HALF * (1.0F + std::cos(twistFrequency.y)) * velocity.y)};
}

inline auto StarDrawer::GetBrightness(const float tAge) noexcept -> float
{
  static constexpr float BRIGHTNESS_FACTOR = 10.0F;
  static constexpr float BRIGHTNESS_MIN = 0.2F;
  const float ageBrightness = (0.8F * std::fabs(0.10F - tAge)) / 0.25F;

  return BRIGHTNESS_FACTOR * (BRIGHTNESS_MIN + ageBrightness);
}

inline auto StarDrawer::GetPartMultiplier() const noexcept -> float
{
  if (m_currentActualDrawElement == DrawElementTypes::LINES)
  {
    return m_goomRand.GetRandInRange(1.0F, GetLineMaxPartMultiplier());
  }

  return m_goomRand.GetRandInRange(1.0F, GetMaxPartMultiplier());
}

inline auto StarDrawer::GetMaxPartMultiplier() const noexcept -> float
{
  static constexpr float MAX_MULTIPLIER = 20.0F;

  switch (m_drawMode)
  {
    case DrawModes::CLEAN:
    case DrawModes::SUPER_CLEAN:
      return 1.0F + UTILS::MATH::SMALL_FLOAT;
    case DrawModes::MESSY:
      return MAX_MULTIPLIER;
    default:
      FailFast();
      return 0.0F;
  }
}

inline auto StarDrawer::GetLineMaxPartMultiplier() const noexcept -> float
{
  static constexpr float LINE_MAX_MULTIPLIER = 4.0F;

  switch (m_drawMode)
  {
    case DrawModes::SUPER_CLEAN:
      return 1.0F + UTILS::MATH::SMALL_FLOAT;
    case DrawModes::CLEAN:
    case DrawModes::MESSY:
      return LINE_MAX_MULTIPLIER;
    default:
      FailFast();
      return 0.0F;
  }
}

inline auto StarDrawer::GetNumPartsAndElementSize(float tAge) const noexcept
    -> std::pair<uint32_t, uint32_t>
{
  if (static constexpr float T_OLD_AGE = 0.95F; tAge > T_OLD_AGE)
  {
    return {m_currentMaxNumParts, m_goomRand.GetRandInRange(MIN_DOT_SIZE, MAX_DOT_SIZE + 1)};
  }

  static constexpr uint32_t MIN_ELEMENT_SIZE = 1;
  const uint32_t numParts =
      MIN_NUM_PARTS +
      static_cast<uint32_t>(
          std::lround((1.0F - tAge) * static_cast<float>(m_currentMaxNumParts - MIN_NUM_PARTS)));

  return {numParts, MIN_ELEMENT_SIZE};
}

inline auto StarDrawer::DrawParticleCircle(const Point2dInt point1,
                                           [[maybe_unused]] const Point2dInt point2,
                                           const uint32_t elementSize,
                                           const MultiplePixels& colors) noexcept -> void
{
  m_draw.Circle(point1, static_cast<int>(elementSize), colors);
}

inline auto StarDrawer::DrawParticleLine(const Point2dInt point1,
                                         const Point2dInt point2,
                                         const uint32_t elementSize,
                                         const MultiplePixels& colors) noexcept -> void
{
  m_draw.Line(point1, point2, colors, static_cast<uint8_t>(elementSize));
}

inline auto StarDrawer::DrawParticleDot(const Point2dInt point1,
                                        [[maybe_unused]] const Point2dInt point2,
                                        const uint32_t elementSize,
                                        const MultiplePixels& colors) noexcept -> void
{
  const auto getMainColor =
      [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors[0]); };
  const auto getLowColor =
      [&colors]([[maybe_unused]] const size_t x, [[maybe_unused]] const size_t y, const Pixel& bgnd)
  { return GetColorMultiply(bgnd, colors[1]); };

  const ImageBitmap& bitmap = GetImageBitmap(elementSize);
  const std::vector<IGoomDraw::GetBitmapColorFunc> getColors{getMainColor, getLowColor};
  m_draw.Bitmap(point1, bitmap, getColors);
}

inline auto StarDrawer::GetImageBitmap(const uint32_t size) const noexcept -> const ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE,
                                       std::clamp(size, MIN_DOT_SIZE, MAX_DOT_SIZE));
}

} //namespace GOOM::VISUAL_FX::FLYING_STARS
