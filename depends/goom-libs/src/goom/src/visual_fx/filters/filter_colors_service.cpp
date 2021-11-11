#include "filter_colors_service.h"

#include "utils/name_value_pairs.h"

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace FILTERS
{
#else
namespace GOOM::VISUAL_FX::FILTERS
{
#endif

using UTILS::GetPair;
using UTILS::NameValuePairs;

auto FilterColorsService::GetNameValueParams(const std::string& paramGroup) const -> NameValuePairs
{
  return {
      GetPair(paramGroup, "blockyWavy", m_blockyWavy),
      GetPair(paramGroup, "buffIntensity", m_buffSettings.buffIntensity),
  };
}

#if __cplusplus <= 201402L
} // namespace FILTERS
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::FILTERS
#endif
