module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

module Goom.VisualFx.IfsDancersFx:Similitudes;

import Goom.Color.ColorMaps;
import Goom.Color.RandomColorMaps;
import Goom.Color.RandomColorMapsGroups;
import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomGraphic;
import Goom.Lib.GoomTypes;
import :IfsTypes;

using GOOM::COLOR::ColorMapPtrWrapper;
using GOOM::COLOR::GetUnweightedRandomColorMaps;
using GOOM::COLOR::WeightedRandomColorMaps;
using GOOM::UTILS::GRAPHICS::ImageBitmap;
using GOOM::UTILS::GRAPHICS::SmallImageBitmaps;
using GOOM::UTILS::MATH::DEGREES_180;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::HALF;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::PI;
using GOOM::UTILS::MATH::UNIT_RANGE;

namespace GOOM::VISUAL_FX::IFS
{

class Similitude
{
public:
  [[nodiscard]] auto GetCPoint() const -> FltPoint;
  [[nodiscard]] auto GetColor() const -> Pixel;
  [[nodiscard]] auto GetColorMap() const -> const COLOR::ColorMapPtrWrapper&;
  [[nodiscard]] auto GetCurrentPointBitmap() const -> const ImageBitmap*;

private:
  friend class Similitudes;
  Dbl m_dbl_cx                            = 0;
  Dbl m_dbl_cy                            = 0;
  Dbl m_dbl_r1                            = 0;
  Dbl m_dbl_r2                            = 0;
  Dbl m_dbl_A1                            = 0;
  Dbl m_dbl_A2                            = 0;
  Flt m_cx                                = 0;
  Flt m_cy                                = 0;
  Flt m_r1                                = 0;
  Flt m_r2                                = 0;
  Flt m_cosA1                             = 0;
  Flt m_sinA1                             = 0;
  Flt m_cosA2                             = 0;
  Flt m_sinA2                             = 0;
  Pixel m_color                           = BLACK_PIXEL;
  const ImageBitmap* m_currentPointBitmap = nullptr;
  ColorMapPtrWrapper m_colorMap{nullptr};
};

class Similitudes
{
public:
  Similitudes(const GoomRand& goomRand, const SmallImageBitmaps& smallBitmaps);

  auto Init() -> void;

  auto SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void;

  auto UpdateMainSimis(Dbl uValue) -> void;
  auto IterateSimis() -> void;

  [[nodiscard]] auto GetNumSimis() const -> size_t;
  [[nodiscard]] auto GetMainSimiGroup() const -> const std::vector<Similitude>&;
  [[nodiscard]] auto GetSimiDepth() const -> uint32_t;

  auto ResetCurrentIfsFunc() -> void;
  [[nodiscard]] auto Transform(const Similitude& simi, const FltPoint& point0) const -> FltPoint;

private:
  using SimiGroup = std::vector<Similitude>;
  SimiGroup m_mainSimiGroup;
  static constexpr size_t NUM_EXTRA_SIMI_GROUPS = 4;
  using ExtraSimiGroups                         = std::array<SimiGroup, NUM_EXTRA_SIMI_GROUPS>;
  ExtraSimiGroups m_extraSimiGroups{};
  uint32_t m_numSimis = 0;
  auto ResetSimiGroups() -> void;
  auto ResetSimiGroup(SimiGroup& simiGroup) -> void;
  auto RandomizeSimiGroup(SimiGroup& simiGroup) const -> void;
  auto UpdateMainSimisDblPart(Dbl uValue) -> void;
  using UValuesArray = std::array<Dbl, NUM_EXTRA_SIMI_GROUPS>;
  auto UpdateMainSimiDblPart(const UValuesArray& uValues,
                             size_t extraSimiIndex,
                             Similitude& mainSimi) -> void;
  auto UpdateMainSimisFltPart() -> void;
  [[nodiscard]] auto GetSimiBitmap(bool useBitmaps) const -> const ImageBitmap*;

  const GoomRand* m_goomRand;
  const SmallImageBitmaps* m_smallBitmaps;
  WeightedRandomColorMaps m_colorMaps;

  using IfsFunc = std::function<FltPoint(const Similitude& simi, Flt x1, Flt y1, Flt x2, Flt y2)>;
  IfsFunc m_currentIfsFunc;

  struct CentreType
  {
    uint32_t depth;
    Dbl r1Mean;
    Dbl r2Mean;
    Dbl dr1Mean;
    Dbl dr2Mean;
  };
  // clang-format off
  static constexpr std::array CENTRE_LIST{
      CentreType{
          .depth = 10,
          .r1Mean = 0.7F, .r2Mean = 0.0F, .dr1Mean = 0.3F, .dr2Mean = 0.4F},
      CentreType{
          .depth = 6,
          .r1Mean = 0.6F, .r2Mean = 0.0F, .dr1Mean = 0.4F, .dr2Mean = 0.3F},
      CentreType{
          .depth =  4,
          .r1Mean = 0.5F, .r2Mean = 0.0F, .dr1Mean = 0.4F, .dr2Mean = 0.3F},
      CentreType{
          .depth =  2,
          .r1Mean = 0.4F, .r2Mean =  0.0F, .dr1Mean =  0.5F, .dr2Mean = 0.3F},
  };
  // clang-format on
  enum class CentreNums : UnderlyingEnumType
  {
    NUM0 = 0,
    NUM1,
    NUM2,
    NUM3,
  };
  UTILS::MATH::Weights<CentreNums> m_centreWeights;
  CentreType m_centreAttributes{};
  auto InitCentre() -> void;

  // NOLINTBEGIN(readability-identifier-length,readability-identifier-naming)
  [[nodiscard]] auto GaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) const -> Dbl;
  [[nodiscard]] auto HalfGaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) const -> Dbl;
  [[nodiscard]] static auto Get_1_minus_exp_neg_S(Dbl S) -> Dbl;
  // NOLINTEND(readability-identifier-length,readability-identifier-naming)
};

} // namespace GOOM::VISUAL_FX::IFS

namespace GOOM::VISUAL_FX::IFS
{

inline auto Similitudes::GetNumSimis() const -> size_t
{
  return m_numSimis;
}

inline auto Similitudes::GetMainSimiGroup() const -> const std::vector<Similitude>&
{
  return m_mainSimiGroup;
}

inline auto Similitudes::GetSimiDepth() const -> uint32_t
{
  return m_centreAttributes.depth;
}

inline auto Similitudes::Transform(const Similitude& simi, const FltPoint& point0) const -> FltPoint
{
  const auto x1 = DivByUnit((point0.x - simi.m_cx) * simi.m_r1);
  const auto y1 = DivByUnit((point0.y - simi.m_cy) * simi.m_r1);

  const auto x2 = DivByUnit((+x1 - simi.m_cx) * simi.m_r2);
  const auto y2 = DivByUnit((-y1 - simi.m_cy) * simi.m_r2);

  return m_currentIfsFunc(simi, x1, y1, x2, y2);
}

inline auto Similitude::GetCPoint() const -> FltPoint
{
  return {m_cx, m_cy};
}

inline auto Similitude::GetColor() const -> Pixel
{
  return m_color;
}

inline auto Similitude::GetColorMap() const -> const COLOR::ColorMapPtrWrapper&
{
  return m_colorMap;
}

inline auto Similitude::GetCurrentPointBitmap() const -> const UTILS::GRAPHICS::ImageBitmap*
{
  return m_currentPointBitmap;
}

static constexpr auto NUM0_WEIGHT = 10.0F;
static constexpr auto NUM1_WEIGHT = 05.0F;
static constexpr auto NUM2_WEIGHT = 03.0F;
static constexpr auto NUM3_WEIGHT = 01.0F;

Similitudes::Similitudes(const GoomRand& goomRand,
                         const SmallImageBitmaps& smallBitmaps)
  : m_goomRand{&goomRand},
    m_smallBitmaps{&smallBitmaps},
    m_colorMaps{GetUnweightedRandomColorMaps(*m_goomRand, MAX_ALPHA)},
    m_centreWeights{
        *m_goomRand,
        {
            {CentreNums::NUM0, NUM0_WEIGHT},
            {CentreNums::NUM1, NUM1_WEIGHT},
            {CentreNums::NUM2, NUM2_WEIGHT},
            {CentreNums::NUM3, NUM3_WEIGHT},
        }
    }
{
  Expects(m_centreWeights.GetNumElements() == CENTRE_LIST.size());

  Init();
  ResetCurrentIfsFunc();
}

auto Similitudes::SetWeightedColorMaps(const WeightedRandomColorMaps& weightedColorMaps) -> void
{
  m_colorMaps = weightedColorMaps;
}

auto Similitudes::Init() -> void
{
  InitCentre();
  ResetSimiGroups();
}

inline auto Similitudes::InitCentre() -> void
{
  const auto numCentres = 2 + static_cast<uint32_t>(m_centreWeights.GetRandomWeighted());

  m_numSimis = numCentres;

  m_centreAttributes = CENTRE_LIST.at(numCentres - 2);
}

inline auto Similitudes::ResetSimiGroups() -> void
{
  ResetSimiGroup(m_mainSimiGroup);
  for (auto i = 0U; i < NUM_EXTRA_SIMI_GROUPS; ++i)
  {
    ResetSimiGroup(m_extraSimiGroups.at(i));
  }
}

inline auto Similitudes::ResetSimiGroup(SimiGroup& simiGroup) -> void
{
  simiGroup.resize(m_numSimis);
  RandomizeSimiGroup(simiGroup);
}

auto Similitudes::UpdateMainSimis(const Dbl uValue) -> void
{
  UpdateMainSimisDblPart(uValue);

  UpdateMainSimisFltPart();
}

inline auto Similitudes::UpdateMainSimisDblPart(const Dbl uValue) -> void
{
  const auto u   = uValue;
  const auto uSq = u * u;
  const auto v   = 1.0F - u;
  const auto vSq = v * v;

  const auto uValues = UValuesArray{
      vSq * v,
      3.0F * vSq * u,
      3.0F * v * uSq,
      u * uSq,
  };

  for (auto i = 0U; i < m_numSimis; ++i)
  {
    UpdateMainSimiDblPart(uValues, i, m_mainSimiGroup[i]);
  }
}

inline auto Similitudes::UpdateMainSimiDblPart(const UValuesArray& uValues,
                                               const size_t extraSimiIndex,
                                               Similitude& mainSimi) -> void
{
  mainSimi.m_dbl_cx = 0.0F;
  mainSimi.m_dbl_cy = 0.0F;

  mainSimi.m_dbl_r1 = 0.0F;
  mainSimi.m_dbl_r2 = 0.0F;

  mainSimi.m_dbl_A1 = 0.0F;
  mainSimi.m_dbl_A2 = 0.0F;

  for (auto j = 0U; j < NUM_EXTRA_SIMI_GROUPS; ++j)
  {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    mainSimi.m_dbl_cx += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_cx;
    mainSimi.m_dbl_cy += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_cy;

    mainSimi.m_dbl_r1 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_r1;
    mainSimi.m_dbl_r2 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_r2;

    mainSimi.m_dbl_A1 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_A1;
    mainSimi.m_dbl_A2 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_A2;
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
  }
}

inline auto Similitudes::UpdateMainSimisFltPart() -> void
{
  for (auto i = 0U; i < m_numSimis; ++i)
  {
    auto& mainSimi = m_mainSimiGroup[i];

    mainSimi.m_cx = MultByUnit(mainSimi.m_dbl_cx);
    mainSimi.m_cy = MultByUnit(mainSimi.m_dbl_cy);

    mainSimi.m_r1 = MultByUnit(mainSimi.m_dbl_r1);
    mainSimi.m_r2 = MultByUnit(mainSimi.m_dbl_r2);

    mainSimi.m_cosA1 = MultByUnit(std::cos(mainSimi.m_dbl_A1));
    mainSimi.m_sinA1 = MultByUnit(std::sin(mainSimi.m_dbl_A1));
    mainSimi.m_cosA2 = MultByUnit(std::cos(mainSimi.m_dbl_A2));
    mainSimi.m_sinA2 = MultByUnit(std::sin(mainSimi.m_dbl_A2));
  }
}

auto Similitudes::IterateSimis() -> void
{
  auto& extraSimiGroup0       = m_extraSimiGroups[0];
  auto& extraSimiGroup1       = m_extraSimiGroups[1];
  const auto& extraSimiGroup2 = m_extraSimiGroups[2];
  const auto& extraSimiGroup3 = m_extraSimiGroups[3];

  for (auto i = 0U; i < m_numSimis; ++i)
  {
    static constexpr auto GROUP3_FACTOR = 2.0F;
    extraSimiGroup1[i].m_dbl_cx =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_cx) - extraSimiGroup2[i].m_dbl_cx;
    extraSimiGroup1[i].m_dbl_cy =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_cy) - extraSimiGroup2[i].m_dbl_cy;

    extraSimiGroup1[i].m_dbl_r1 =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_r1) - extraSimiGroup2[i].m_dbl_r1;
    extraSimiGroup1[i].m_dbl_r2 =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_r2) - extraSimiGroup2[i].m_dbl_r2;

    extraSimiGroup1[i].m_dbl_A1 =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_A1) - extraSimiGroup2[i].m_dbl_A1;
    extraSimiGroup1[i].m_dbl_A2 =
        (GROUP3_FACTOR * extraSimiGroup3[i].m_dbl_A2) - extraSimiGroup2[i].m_dbl_A2;

    extraSimiGroup0[i] = extraSimiGroup3[i];
  }

  RandomizeSimiGroup(m_extraSimiGroups[2]);
  RandomizeSimiGroup(m_extraSimiGroups[3]);
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
auto Similitudes::RandomizeSimiGroup(SimiGroup& simiGroup) const -> void
{
  // NOLINTBEGIN(readability-identifier-naming)
#if __cplusplus <= 202303L // NOLINT: Can't include header for this.
  static const auto c_factor             = 0.8F * Get_1_minus_exp_neg_S(4.0);
  static const auto r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const auto r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const auto A1_factor            = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const auto A2_factor            = A1_factor;
#else
  static const constinit auto c_factor             = 0.8f * Get_1_minus_exp_neg_S(4.0);
  static const constinit auto r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const constinit auto r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const constinit auto A1_factor            = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const constinit auto A2_factor            = A1_factor;
#endif
  // NOLINTEND(readability-identifier-naming)

  const auto r1Factor = m_centreAttributes.dr1Mean * r1_1_minus_exp_neg_S;
  const auto r2Factor = m_centreAttributes.dr2Mean * r2_1_minus_exp_neg_S;

  const auto colorMapGroup               = m_colorMaps.GetRandomGroup();
  static constexpr auto PROB_USE_BITMAPS = 0.7F;
  const auto useBitmaps                  = m_goomRand->ProbabilityOf<PROB_USE_BITMAPS>();

  for (auto i = 0U; i < m_numSimis; ++i)
  {
    auto& simi = simiGroup[i];

    simi.m_dbl_cx = GaussRand(0.0, 4.0, c_factor);
    simi.m_dbl_cy = GaussRand(0.0, 4.0, c_factor);
    simi.m_dbl_r1 = GaussRand(m_centreAttributes.r1Mean, 3.0, r1Factor);
    simi.m_dbl_r2 = HalfGaussRand(m_centreAttributes.r2Mean, 2.0, r2Factor);
    simi.m_dbl_A1 = GaussRand(0.0F, 4.0F, A1_factor) * (PI / DEGREES_180);
    simi.m_dbl_A2 = GaussRand(0.0F, 4.0F, A2_factor) * (PI / DEGREES_180);
    simi.m_cx     = 0;
    simi.m_cy     = 0;
    simi.m_r1     = 0;
    simi.m_r2     = 0;
    simi.m_cosA1  = 0;
    simi.m_sinA1  = 0;
    simi.m_cosA2  = 0;
    simi.m_sinA2  = 0;

    simi.m_colorMap = m_colorMaps.GetRandomColorMap(colorMapGroup);
    simi.m_color =
        m_colorMaps.GetRandomColor(m_colorMaps.GetRandomColorMap(colorMapGroup), 0.0F, 1.0F);

    simi.m_currentPointBitmap = GetSimiBitmap(useBitmaps);
  }
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

auto Similitudes::GetSimiBitmap(const bool useBitmaps) const -> const ImageBitmap*
{
  if (not useBitmaps)
  {
    return nullptr;
  }

  static constexpr auto RES_RANGE = NumberRange{3U, 7U};
  const auto res                  = m_goomRand->GetRandInRange<RES_RANGE>();

  if (static constexpr auto PROB_SPHERE_BITMAP = 0.6F;
      m_goomRand->ProbabilityOf<PROB_SPHERE_BITMAP>())
  {
    return &m_smallBitmaps->GetImageBitmap(SmallImageBitmaps::ImageNames::SPHERE, res);
  }

  return &m_smallBitmaps->GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE, res);
}

// NOLINTBEGIN(readability-identifier-length,readability-identifier-naming)
// TODO(glk) - Make this constexpr with C++23.
auto Similitudes::Get_1_minus_exp_neg_S(const Dbl S) -> Dbl
{
  return 1.0F - std::exp(-S);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto Similitudes::GaussRand(const Dbl c,
                            const Dbl S,
                            const Dbl A_mult_1_minus_exp_neg_S) const -> Dbl
{
  const auto x = m_goomRand->GetRandInRange<UNIT_RANGE>();
  const auto y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));

  return m_goomRand->ProbabilityOf<HALF>() ? (c + y) : (c - y);
}

auto Similitudes::HalfGaussRand(const Dbl c,
                                const Dbl S,
                                const Dbl A_mult_1_minus_exp_neg_S) const -> Dbl
{
  const auto x = m_goomRand->GetRandInRange<UNIT_RANGE>();
  const auto y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));
  return c + y;
}
// NOLINTEND(bugprone-easily-swappable-parameters)
// NOLINTEND(readability-identifier-length,readability-identifier-naming)

auto Similitudes::ResetCurrentIfsFunc() -> void
{
  if (static constexpr auto PROB_REVERSED_IFS_FUNC = 0.3F;
      m_goomRand->ProbabilityOf<PROB_REVERSED_IFS_FUNC>())
  {
    m_currentIfsFunc =
        [](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2, const Flt y2)
    {
      return FltPoint{
          DivByUnit((x1 * simi.m_sinA1) - (y1 * simi.m_cosA1) + (x2 * simi.m_sinA2) -
                    (y2 * simi.m_cosA2)) +
              simi.m_cx,
          DivByUnit((x1 * simi.m_cosA1) + (y1 * simi.m_sinA1) + (x2 * simi.m_cosA2) +
                    (y2 * simi.m_sinA2)) +
              simi.m_cy,
      };
    };
  }
  else
  {
    m_currentIfsFunc =
        [](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2, const Flt y2)
    {
      return FltPoint{
          DivByUnit((x1 * simi.m_cosA1) - (y1 * simi.m_sinA1) + (x2 * simi.m_cosA2) -
                    (y2 * simi.m_sinA2)) +
              simi.m_cx,
          DivByUnit((x1 * simi.m_sinA1) + (y1 * simi.m_cosA1) + (x2 * simi.m_sinA2) +
                    (y2 * simi.m_cosA2)) +
              simi.m_cy,
      };
    };
  }
}

} // namespace GOOM::VISUAL_FX::IFS
