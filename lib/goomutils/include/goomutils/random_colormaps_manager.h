#ifndef VISUALIZATION_GOOM_RANDOM_COLORMAPS_MANAGER_H
#define VISUALIZATION_GOOM_RANDOM_COLORMAPS_MANAGER_H

#include "colormaps.h"
#include "random_colormaps.h"

#include <memory>
#include <set>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

class RandomColorMapsManager
{
public:
  RandomColorMapsManager() noexcept = default;
  RandomColorMapsManager(const RandomColorMapsManager&) noexcept = delete;
  RandomColorMapsManager(RandomColorMapsManager&&) noexcept = delete;
  ~RandomColorMapsManager() noexcept = default;
  auto operator=(const RandomColorMapsManager&) -> RandomColorMapsManager& = delete;
  auto operator=(RandomColorMapsManager&&) -> RandomColorMapsManager& = delete;

  void IncCounter();
  void ResetCounter();

  struct ColorMapInfo
  {
    std::shared_ptr<const RandomColorMaps> colorMaps{};
    COLOR_DATA::ColorMapName colorMapName{COLOR_DATA::ColorMapName::_NULL};
    std::set<RandomColorMaps::ColorMapTypes> types{};
  };

  [[nodiscard]] auto GetColorMapInfo(size_t id) const -> const ColorMapInfo&;
  [[nodiscard]] auto AddColorMapInfo(const ColorMapInfo& info) -> uint32_t;

  void UpdateAllColorMapInfo(const ColorMapInfo& info);
  void UpdateColorMapInfo(size_t id, const ColorMapInfo& info);

  void UpdateAllColorMapNames(COLOR_DATA::ColorMapName colorMapName);
  void UpdateColorMapName(size_t id, COLOR_DATA::ColorMapName colorMapName);

  void UpdateAllColorMapTypes(const std::set<RandomColorMaps::ColorMapTypes>& types);
  void UpdateColorMapTypes(size_t id, const std::set<RandomColorMaps::ColorMapTypes>& types);

  void ChangeAllColorMapsNow();
  void ChangeColorMapNow(size_t id);

  [[nodiscard]] auto GetColorMap(size_t id) const -> const IColorMap&;
  [[nodiscard]] auto GetColorMapPtr(size_t id) const -> std::shared_ptr<const IColorMap>;

private:
  std::vector<ColorMapInfo> m_infoList{};
  std::vector<std::shared_ptr<const IColorMap>> m_colorMaps{};

  void DoUpdateColorMap(size_t id, const ColorMapInfo& info);
  void DoUpdateColorMapName(size_t id, COLOR_DATA::ColorMapName colorMapName);
  void DoUpdateColorMapTypes(size_t id, const std::set<RandomColorMaps::ColorMapTypes>& types);
  void DoChangeColorMap(size_t id);
};

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
#endif
