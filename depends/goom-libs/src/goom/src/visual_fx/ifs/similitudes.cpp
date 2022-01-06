#include "similitudes.h"

#include "color/colormaps.h"
#include "color/random_colormaps.h"
#include "utils/goom_rand_base.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/mathutils.h"

#undef NDEBUG
#include <cassert>

namespace GOOM::VISUAL_FX::IFS
{

using COLOR::ColorMapGroup;
using COLOR::RandomColorMaps;
using UTILS::DEGREES_180;
using UTILS::IGoomRand;
using UTILS::ImageBitmap;
using UTILS::m_pi;
using UTILS::SmallImageBitmaps;

// clang-format off
constexpr float NUM0_WEIGHT = 10.0F;
constexpr float NUM1_WEIGHT =  5.0F;
constexpr float NUM2_WEIGHT =  3.0F;
constexpr float NUM3_WEIGHT =  1.0F;
// clang-format on

Similitudes::Similitudes(const IGoomRand& goomRand,
                         const RandomColorMaps& randomColorMaps,
                         const SmallImageBitmaps& smallBitmaps)
  : m_goomRand{goomRand},
    m_smallBitmaps{smallBitmaps},
    m_colorMaps{randomColorMaps},
    m_centreWeights{
        m_goomRand,
        {
            {CentreNums::NUM0, NUM0_WEIGHT},
            {CentreNums::NUM1, NUM1_WEIGHT},
            {CentreNums::NUM2, NUM2_WEIGHT},
            {CentreNums::NUM3, NUM3_WEIGHT},
        }
    }
// clang-format on
{
  assert(m_centreWeights.GetNumElements() == CENTRE_LIST.size());

  Init();
  ResetCurrentIfsFunc();
}

void Similitudes::Init()
{
  InitCentre();
  ResetSimiGroups();
}

inline void Similitudes::InitCentre()
{
  const uint32_t numCentres = 2 + static_cast<uint32_t>(m_centreWeights.GetRandomWeighted());

  m_numSimis = numCentres;

  m_centreAttributes = CENTRE_LIST.at(numCentres - 2);
}

inline void Similitudes::ResetSimiGroups()
{
  ResetSimiGroup(m_mainSimiGroup);
  for (size_t i = 0; i < NUM_EXTRA_SIMI_GROUPS; ++i)
  {
    ResetSimiGroup(m_extraSimiGroups[i]);
  }
}

inline void Similitudes::ResetSimiGroup(SimiGroup& simiGroup)
{
  simiGroup.resize(m_numSimis);
  RandomizeSimiGroup(simiGroup);
}

void Similitudes::UpdateMainSimis(const Dbl uValue)
{
  UpdateMainSimisDblPart(uValue);

  UpdateMainSimisFltPart();
}

inline void Similitudes::UpdateMainSimisDblPart(const Dbl uValue)
{
  const Dbl u = uValue;
  const Dbl uSq = u * u;
  const Dbl v = 1.0F - u;
  const Dbl vSq = v * v;

  const UValuesArray uValues = {
      vSq * v,
      3.0F * vSq * u,
      3.0F * v * uSq,
      u * uSq,
  };

  for (size_t i = 0; i < m_numSimis; ++i)
  {
    UpdateMainSimiDblPart(uValues, i, m_mainSimiGroup[i]);
  }
}

inline void Similitudes::UpdateMainSimiDblPart(const UValuesArray& uValues,
                                               const size_t extraSimiIndex,
                                               Similitude& mainSimi)
{
  mainSimi.m_dbl_cx = 0.0F;
  mainSimi.m_dbl_cy = 0.0F;

  mainSimi.m_dbl_r1 = 0.0F;
  mainSimi.m_dbl_r2 = 0.0F;

  mainSimi.m_dbl_A1 = 0.0F;
  mainSimi.m_dbl_A2 = 0.0F;

  for (size_t j = 0; j < NUM_EXTRA_SIMI_GROUPS; ++j)
  {
    mainSimi.m_dbl_cx += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_cx;
    mainSimi.m_dbl_cy += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_cy;

    mainSimi.m_dbl_r1 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_r1;
    mainSimi.m_dbl_r2 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_r2;

    mainSimi.m_dbl_A1 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_A1;
    mainSimi.m_dbl_A2 += uValues[j] * m_extraSimiGroups[j][extraSimiIndex].m_dbl_A2;
  }
}

inline void Similitudes::UpdateMainSimisFltPart()
{
  for (size_t i = 0; i < m_numSimis; ++i)
  {
    Similitude& mainSimi = m_mainSimiGroup[i];

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

void Similitudes::IterateSimis()
{
  SimiGroup& extraSimiGroup0 = m_extraSimiGroups[0];
  SimiGroup& extraSimiGroup1 = m_extraSimiGroups[1];
  const SimiGroup& extraSimiGroup2 = m_extraSimiGroups[2];
  const SimiGroup& extraSimiGroup3 = m_extraSimiGroups[3];

  for (size_t i = 0; i < m_numSimis; ++i)
  {
    constexpr float GROUP3_FACTOR = 2.0F;
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

void Similitudes::RandomizeSimiGroup(SimiGroup& simiGroup)
{
#if __cplusplus <= 201703L
  static const Dbl c_factor = 0.8F * Get_1_minus_exp_neg_S(4.0);
  static const Dbl r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const Dbl r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const Dbl A1_factor = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const Dbl A2_factor = A1_factor;
#else
  static const constinit Dbl c_factor = 0.8f * Get_1_minus_exp_neg_S(4.0);
  static const constinit Dbl r1_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(3.0);
  static const constinit Dbl r2_1_minus_exp_neg_S = Get_1_minus_exp_neg_S(2.0);
  static const constinit Dbl A1_factor = 360.0F * Get_1_minus_exp_neg_S(4.0);
  static const constinit Dbl A2_factor = A1_factor;
#endif

  const Dbl r1Factor = m_centreAttributes.dr1Mean * r1_1_minus_exp_neg_S;
  const Dbl r2Factor = m_centreAttributes.dr2Mean * r2_1_minus_exp_neg_S;

  const ColorMapGroup colorMapGroup = m_colorMaps.GetRandomGroup();
  constexpr float PROB_USE_BITMAPS = 0.7F;
  const bool useBitmaps = m_goomRand.ProbabilityOf(PROB_USE_BITMAPS);

  for (size_t i = 0; i < m_numSimis; ++i)
  {
    Similitude& simi = simiGroup[i];

    simi.m_dbl_cx = GaussRand(0.0, 4.0, c_factor);
    simi.m_dbl_cy = GaussRand(0.0, 4.0, c_factor);
    simi.m_dbl_r1 = GaussRand(m_centreAttributes.r1Mean, 3.0, r1Factor);
    simi.m_dbl_r2 = HalfGaussRand(m_centreAttributes.r2Mean, 2.0, r2Factor);
    simi.m_dbl_A1 = GaussRand(0.0F, 4.0F, A1_factor) * (m_pi / DEGREES_180);
    simi.m_dbl_A2 = GaussRand(0.0F, 4.0F, A2_factor) * (m_pi / DEGREES_180);
    simi.m_cx = 0;
    simi.m_cy = 0;
    simi.m_r1 = 0;
    simi.m_r2 = 0;
    simi.m_cosA1 = 0;
    simi.m_sinA1 = 0;
    simi.m_cosA2 = 0;
    simi.m_sinA2 = 0;

    simi.m_colorMap = &m_colorMaps.GetRandomColorMap(colorMapGroup);
    simi.m_color = RandomColorMaps{m_goomRand}.GetRandomColor(
        m_colorMaps.GetRandomColorMap(colorMapGroup), 0.0F, 1.0F);

    simi.m_currentPointBitmap = GetSimiBitmap(useBitmaps);
  }
}

auto Similitudes::GetSimiBitmap(const bool useBitmaps) -> const ImageBitmap*
{
  if (!useBitmaps)
  {
    return nullptr;
  }

  constexpr uint32_t MIN_RES = 3;
  constexpr uint32_t MAX_RES = 7;
  const uint32_t res = m_goomRand.GetRandInRange(MIN_RES, MAX_RES);
  constexpr float PROB_SPHERE_BITMAP = 0.6F;

  if (m_goomRand.ProbabilityOf(PROB_SPHERE_BITMAP))
  {
    return &m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::SPHERE, res);
  }

  return &m_smallBitmaps.GetImageBitmap(SmallImageBitmaps::ImageNames::CIRCLE, res);
}

inline auto Similitudes::Get_1_minus_exp_neg_S(const Dbl S) -> Dbl
{
  return 1.0F - std::exp(-S);
}

inline auto Similitudes::GaussRand(const Dbl c, const Dbl S, const Dbl A_mult_1_minus_exp_neg_S)
    -> Dbl
{
  const Dbl x = m_goomRand.GetRandInRange(0.0F, 1.0F);
  const Dbl y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));
  return m_goomRand.ProbabilityOfMInN(1, 2) ? (c + y) : (c - y);
}

inline auto Similitudes::HalfGaussRand(const Dbl c, const Dbl S, const Dbl A_mult_1_minus_exp_neg_S)
    -> Dbl
{
  const Dbl x = m_goomRand.GetRandInRange(0.0F, 1.0F);
  const Dbl y = A_mult_1_minus_exp_neg_S * (1.0F - std::exp(-x * x * S));
  return c + y;
}

void Similitudes::ResetCurrentIfsFunc()
{
  constexpr float PROB_REVERSED_IFS_FUNC = 0.3F;

  if (m_goomRand.ProbabilityOf(PROB_REVERSED_IFS_FUNC))
  {
    m_currentIfsFunc =
        [&](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2, const Flt y2)
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
        [&](const Similitude& simi, const Flt x1, const Flt y1, const Flt x2, const Flt y2)
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
