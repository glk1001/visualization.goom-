module;

#include <span>

export module Goom.Color.ColorData.ExtraColorGroups;

import Goom.Color.ColorData.ColorMapEnums;

export namespace GOOM::COLOR::COLOR_DATA
{

auto GetWesAndersonMaps() noexcept -> std::span<const ColorMapName>;
auto GetBlueMaps() noexcept -> std::span<const ColorMapName>;
auto GetRedMaps() noexcept -> std::span<const ColorMapName>;
auto GetGreenMaps() noexcept -> std::span<const ColorMapName>;
auto GetYellowMaps() noexcept -> std::span<const ColorMapName>;
auto GetOrangeMaps() noexcept -> std::span<const ColorMapName>;
auto GetPurpleMaps() noexcept -> std::span<const ColorMapName>;
auto GetCityMaps() noexcept -> std::span<const ColorMapName>;
auto GetSeasonMaps() noexcept -> std::span<const ColorMapName>;
auto GetHeatMaps() noexcept -> std::span<const ColorMapName>;
auto GetColdMaps() noexcept -> std::span<const ColorMapName>;
auto GetPastelMaps() noexcept -> std::span<const ColorMapName>;

} // namespace GOOM::COLOR::COLOR_DATA
