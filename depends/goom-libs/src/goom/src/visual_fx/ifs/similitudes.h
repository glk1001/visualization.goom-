#pragma once

#include "goom_graphic.h"
#include "ifs_types.h"
#include "utils/goom_rand_base.h"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace GOOM
{

namespace COLOR
{
class IColorMap;
class RandomColorMaps;
}

namespace UTILS
{
class IGoomRand;
class ImageBitmap;
class SmallImageBitmaps;
}

namespace VISUAL_FX
{
namespace IFS
{

class Similitude
{
public:
  [[nodiscard]] auto GetCPoint() const -> FltPoint;
  [[nodiscard]] auto GetColor() const -> Pixel;
  [[nodiscard]] auto GetColorMap() const -> const COLOR::IColorMap*;
  [[nodiscard]] auto GetCurrentPointBitmap() const -> const UTILS::ImageBitmap*;

private:
  friend class Similitudes;
  Dbl m_dbl_cx = 0;
  Dbl m_dbl_cy = 0;
  Dbl m_dbl_r1 = 0;
  Dbl m_dbl_r2 = 0;
  Dbl m_dbl_A1 = 0;
  Dbl m_dbl_A2 = 0;
  Flt m_cx = 0;
  Flt m_cy = 0;
  Flt m_r1 = 0;
  Flt m_r2 = 0;
  Flt m_cosA1 = 0;
  Flt m_sinA1 = 0;
  Flt m_cosA2 = 0;
  Flt m_sinA2 = 0;
  const COLOR::IColorMap* m_colorMap = nullptr;
  Pixel m_color = Pixel::BLACK;
  const UTILS::ImageBitmap* m_currentPointBitmap = nullptr;
};

class Similitudes
{
public:
  Similitudes(const UTILS::IGoomRand& goomRand,
              const COLOR::RandomColorMaps& randomColorMaps,
              const UTILS::SmallImageBitmaps& smallBitmaps);

  void Init();

  void UpdateMainSimis(Dbl uValue);
  void IterateSimis();

  [[nodiscard]] auto GetNumSimis() const -> size_t;
  [[nodiscard]] auto GetMainSimiGroup() const -> const std::vector<Similitude>&;
  [[nodiscard]] auto GetSimiDepth() const -> uint32_t;

  void ResetCurrentIfsFunc();
  [[nodiscard]] auto Transform(const Similitude& simi, const FltPoint& point0) const -> FltPoint;

private:
  using SimiGroup = std::vector<Similitude>;
  SimiGroup m_mainSimiGroup{};
  static constexpr size_t NUM_EXTRA_SIMI_GROUPS = 4;
  using ExtraSimiGroups = std::array<SimiGroup, NUM_EXTRA_SIMI_GROUPS>;
  ExtraSimiGroups m_extraSimiGroups{};
  uint32_t m_numSimis = 0;
  void ResetSimiGroups();
  void ResetSimiGroup(SimiGroup& simiGroup);
  void RandomizeSimiGroup(SimiGroup& simiGroup);
  void UpdateMainSimisDblPart(Dbl uValue);
  using UValuesArray = std::array<Dbl, NUM_EXTRA_SIMI_GROUPS>;
  void UpdateMainSimiDblPart(const UValuesArray& uValues,
                             size_t extraSimiIndex,
                             Similitude& mainSimi);
  void UpdateMainSimisFltPart();
  [[nodiscard]] auto GetSimiBitmap(bool useBitmaps) -> const UTILS::ImageBitmap*;

  const UTILS::IGoomRand& m_goomRand;
  const UTILS::SmallImageBitmaps& m_smallBitmaps;
  const COLOR::RandomColorMaps& m_colorMaps;

  using IfsFunc = std::function<FltPoint(const Similitude& simi, Flt x1, Flt y1, Flt x2, Flt y2)>;
  IfsFunc m_currentIfsFunc{};

  struct CentreType
  {
    uint32_t depth;
    Dbl r1Mean;
    Dbl r2Mean;
    Dbl dr1Mean;
    Dbl dr2Mean;
  };
  const std::vector<CentreType> m_centreList;
  enum class CentreNums
  {
    NUM0 = 0,
    NUM1,
    NUM2,
    NUM3,
    _NUM // unused and must be last
  };
  const UTILS::Weights<CentreNums> m_centreWeights;
  CentreType m_centreAttributes{};
  void InitCentre();

  [[nodiscard]] auto GaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) -> Dbl;
  [[nodiscard]] auto HalfGaussRand(Dbl c, Dbl S, Dbl A_mult_1_minus_exp_neg_S) -> Dbl;
  [[nodiscard]] static constexpr auto Get_1_minus_exp_neg_S(Dbl S) -> Dbl;
};

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
  const Flt x1 = DivByUnit((point0.x - simi.m_cx) * simi.m_r1);
  const Flt y1 = DivByUnit((point0.y - simi.m_cy) * simi.m_r1);

  const Flt x2 = DivByUnit((+x1 - simi.m_cx) * simi.m_r2);
  const Flt y2 = DivByUnit((-y1 - simi.m_cy) * simi.m_r2);

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

inline auto Similitude::GetColorMap() const -> const COLOR::IColorMap*
{
  return m_colorMap;
}

inline auto Similitude::GetCurrentPointBitmap() const -> const UTILS::ImageBitmap*
{
  return m_currentPointBitmap;
}

} // namespace IFS
} // namespace VISUAL_FX
} // namespace GOOM
