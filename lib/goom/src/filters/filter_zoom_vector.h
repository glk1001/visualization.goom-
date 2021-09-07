#ifndef VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
#define VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H

#include "filter_normalized_coords.h"
#include "filter_zoom_vector_effects.h"
#include "goom_zoom_vector.h"
#include "v2d.h"

#include <cstdint>
#include <memory>

namespace GOOM
{

struct ZoomFilterEffectsSettings;

namespace FILTERS
{

class FilterZoomVector : public IZoomVector
{
public:
  explicit FilterZoomVector(uint32_t screenWidth) noexcept;

  void SetMaxSpeedCoeff(float val) override;

  void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) override;

  [[nodiscard]] auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords override;

  [[nodiscard]] auto GetHypercosNameValueParams() const -> std::vector<std::pair<std::string, std::string>>;

private:
  ZoomVectorEffects m_zoomVectorEffects;

  void GetZoomEffectsAdjustedVelocity(float sqDistFromZero,
                                      const NormalizedCoords& coords,
                                      NormalizedCoords& velocity) const;
};

inline auto FilterZoomVector::GetHypercosNameValueParams() const -> std::vector<std::pair<std::string, std::string>>
{
  return m_zoomVectorEffects.GetHypercosNameValueParams();
}

} // namespace FILTERS
} // namespace GOOM

#endif //VISUALIZATION_GOOM_FILTER_ZOOM_VECTOR_H
