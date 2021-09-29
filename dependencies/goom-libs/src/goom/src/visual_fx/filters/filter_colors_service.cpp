#include "filter_colors_service.h"

#include "utils/name_value_pairs.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace FILTERS
{
#else
namespace GOOM::FILTERS
{
#endif

using UTILS::GetPair;
using UTILS::NameValuePairs;

auto FilterColorsService::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  return {
      GetPair(paramGroup, "blockyWavy", m_blockyWavy),
      GetPair(paramGroup, "clippedColor", m_clippedColor),
      GetPair(paramGroup, "buffIntensity", m_buffSettings.buffIntensity),
      GetPair(paramGroup, "allowOverexposed", m_buffSettings.allowOverexposed),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace GOOM
#else
} // namespace GOOM::FILTERS
#endif
