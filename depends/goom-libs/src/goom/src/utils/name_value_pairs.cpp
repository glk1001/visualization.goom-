#include "name_value_pairs.h"

#include "strutils.h"

#include <algorithm>
#include <string>
#include <vector>

namespace GOOM::UTILS
{


[[nodiscard]] auto GetNameValuesString(const NameValuePairs& nameValuePairs) -> std::string
{
  std::vector<std::string> nameValueStrings{};
  std::transform(cbegin(nameValuePairs), cend(nameValuePairs), std::back_inserter(nameValueStrings),
                 [](const NameValuePair& pair) { return GetNameValueString(pair); });
  return StringJoin(nameValueStrings, "\n");
}

} // namespace GOOM::UTILS
