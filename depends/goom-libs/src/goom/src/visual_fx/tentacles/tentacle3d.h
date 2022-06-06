#pragma once

#include "color/color_adjustment.h"
#include "color/random_colormaps.h"
#include "color/random_colormaps_manager.h"
#include "goom_graphic.h"
#include "tentacle2d.h"
#include "utils/math/goom_rand_base.h"

#include <memory>
#include <vector>

namespace GOOM::VISUAL_FX::TENTACLES
{

struct V3dFlt
{
  float x = 0.0;
  float y = 0.0;
  float z = 0.0;
  bool ignore = false;
};

class Tentacle3D
{
public:
  Tentacle3D() noexcept = delete;
  Tentacle3D(std::unique_ptr<Tentacle2D> tentacle,
             const Pixel& headMainColor,
             const Pixel& headLowColor,
             const V3dFlt& head,
             size_t numHeadNodes,
             const UTILS::MATH::IGoomRand& goomRand) noexcept;
  Tentacle3D(const Tentacle3D&) noexcept = delete;
  Tentacle3D(Tentacle3D&&) noexcept = default;
  ~Tentacle3D() noexcept;
  auto operator=(const Tentacle3D&) -> Tentacle3D& = delete;
  auto operator=(Tentacle3D&&) -> Tentacle3D& = delete;

  auto Get2DTentacle() -> Tentacle2D& { return *m_tentacle; }
  [[nodiscard]] auto Get2DTentacle() const -> const Tentacle2D& { return *m_tentacle; }

  void SetWeightedColorMaps(const std::shared_ptr<COLOR::RandomColorMaps>& weightedMaps);
  void ColorMapsChanged();

  [[nodiscard]] auto GetMixedColors(size_t nodeNum,
                                    const Pixel& mainColor,
                                    const Pixel& lowColor,
                                    float brightness) const -> std::pair<Pixel, Pixel>;

  void SetReverseColorMix(const bool val) { m_reverseColorMix = val; }

  static constexpr float HEAD_SMALL_X = 10.0F;
  [[nodiscard]] auto GetHead() const -> const V3dFlt& { return m_head; }
  void SetHead(const V3dFlt& val) { m_head = val; }

  [[nodiscard]] auto GetNumHeadNodes() const -> size_t { return m_numHeadNodes; }
  void SetNumHeadNodes(const size_t val) { m_numHeadNodes = val; }

  [[nodiscard]] auto GetVertices() const -> std::vector<V3dFlt>;

private:
  const UTILS::MATH::IGoomRand& m_goomRand;
  std::unique_ptr<Tentacle2D> m_tentacle;

  COLOR::RandomColorMapsManager m_colorMapsManager{};
  std::shared_ptr<COLOR::RandomColorMaps> m_colorMaps{};
  COLOR::RandomColorMapsManager::ColorMapId m_mainColorMapID;
  COLOR::RandomColorMapsManager::ColorMapId m_lowColorMapID;
  static constexpr float MIN_COLOR_SEGMENT_MIX_T = 0.4F;
  static constexpr float MAX_COLOR_SEGMENT_MIX_T = 1.0F;
  static constexpr float DEFAULT_COLOR_SEGMENT_MIX_T = 0.8F;
  float m_mainColorSegmentMixT = DEFAULT_COLOR_SEGMENT_MIX_T;
  float m_lowColorSegmentMixT = DEFAULT_COLOR_SEGMENT_MIX_T;
  bool m_reverseColorMix = false;
  static constexpr float GAMMA = 1.0F / 2.0F;
  const COLOR::ColorAdjustment m_colorAdjust{GAMMA,
                                             COLOR::ColorAdjustment::INCREASED_CHROMA_FACTOR};

  Pixel m_headMainColor{};
  Pixel m_headLowColor{};
  V3dFlt m_head{};
  size_t m_numHeadNodes{};

  [[nodiscard]] auto GetMixedColors(size_t nodeNum,
                                    const Pixel& mainColor,
                                    const Pixel& lowColor) const -> std::pair<Pixel, Pixel>;
  [[nodiscard]] auto GetMixedHeadColors(size_t nodeNum,
                                        const Pixel& mainColor,
                                        const Pixel& lowColor) const -> std::pair<Pixel, Pixel>;
  [[nodiscard]] auto GetFinalMixedColor(const Pixel& color,
                                        const Pixel& segmentColor,
                                        float t) const -> Pixel;
};

} // namespace GOOM::VISUAL_FX::TENTACLES
