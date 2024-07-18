module;

#include <cstdint>
#include <span>

export module Goom.Lib.FrameData;

import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomGraphic;
import Goom.Lib.Point2d;

using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM
{

inline constexpr auto MIN_NORMALIZED_COORD   = -2.0F;
inline constexpr auto MAX_NORMALIZED_COORD   = -MIN_NORMALIZED_COORD;
inline constexpr auto NORMALIZED_COORD_WIDTH = MAX_NORMALIZED_COORD - MIN_NORMALIZED_COORD;

struct FilterPosArrays
{
  std::span<Point2dFlt> filterDestPos;
  float filterPosBuffersLerpFactor                 = 0.0F;
  static constexpr auto POS1_POS2_MIX_FREQ_RANGE   = NumberRange{0.001F, 0.010F};
  static constexpr auto DEFAULT_POS1_POS2_MIX_FREQ = POS1_POS2_MIX_FREQ_RANGE.max;
  float filterPos1Pos2FreqMixFreq                  = 0.0F;
  bool filterDestPosNeedsUpdating                  = false;
};
struct ImageArrays
{
  PixelBuffer mainImagePixelBuffer;
  bool mainImagePixelBufferNeedsUpdating = false;
  PixelBuffer lowImagePixelBuffer;
  bool lowImagePixelBufferNeedsUpdating = false;
};
struct MiscData
{
  // TODO(glk) - blending params, contrast, other command/effects
  float brightness          = 1.0F;
  float hueShift            = 0.0F;
  float chromaFactor        = 1.0F;
  float baseColorMultiplier = 1.0F;
  float prevFrameTMix       = 0.0F;
  float gamma               = 1.0F;
  uint64_t goomTime         = 0U;
};
struct FrameData
{
  MiscData miscData{};
  FilterPosArrays filterPosArrays{};
  ImageArrays imageArrays{};
};

} // namespace GOOM
