#pragma once

#include "utils/name_value_pairs.h"
#include "v2d.h"

#include <string>

namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{

class NormalizedCoords;
struct ZoomFilterEffectsSettings;

class IZoomVector
{
public:
  IZoomVector() noexcept = default;
  IZoomVector(const IZoomVector&) noexcept = delete;
  IZoomVector(IZoomVector&&) noexcept = delete;
  virtual ~IZoomVector() noexcept = default;
  auto operator=(const IZoomVector&) -> IZoomVector& = delete;
  auto operator=(IZoomVector&&) -> IZoomVector& = delete;

  virtual void SetFilterSettings(const ZoomFilterEffectsSettings& filterEffectsSettings) = 0;

  virtual auto GetZoomPoint(const NormalizedCoords& coords) const -> NormalizedCoords = 0;

  [[nodiscard]] virtual auto GetNameValueParams(const std::string& paramGroup) const
      -> UTILS::NameValuePairs = 0;
};

} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
