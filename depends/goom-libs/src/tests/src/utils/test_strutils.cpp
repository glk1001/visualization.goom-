#include "goom/goom_types.h"
#include "utils/enum_utils.h"
#include "utils/strutils.h"

#include <string>
#include <vector>

#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
#endif
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#if __clang_major__ >= 16 // NOLINT: Can't include header for this.
#pragma GCC diagnostic pop
#endif

namespace GOOM::UNIT_TESTS
{

using GOOM::UnderlyingEnumType;
using UTILS::EnumToString;
using UTILS::FindAndReplaceAll;
using UTILS::StringJoin;
using UTILS::StringSplit;

TEST_CASE("FindAndReplaceAll")
{
  auto str               = std::string{"hello Everyone out there. hello again and Hello again."};
  const auto expectedStr = std::string{"Hello Everyone out there. Hello again and Hello again."};

  FindAndReplaceAll(str, "hello", "Hello");
  REQUIRE(str == expectedStr);

  FindAndReplaceAll(str, "zzz", "ZZZ");
  REQUIRE(str == expectedStr);
}

TEST_CASE("StringJoin")
{
  REQUIRE(StringJoin({""}, ", ").empty());
  REQUIRE("word1" == StringJoin({"word1"}, ", "));
  REQUIRE("word1, word2, word3" == StringJoin({"word1", "word2", "word3"}, ", "));
  REQUIRE("word1, word2, word3," == StringJoin({"word1", "word2", "word3,"}, ", "));
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_CASE("StringSplit")
{
  const auto testString1 = std::string{"line1: word1, word2\nline2: word3, word4\n"};
  const auto test1 = StringSplit(testString1, ",");
  UNSCOPED_INFO("testString1 = \"" << testString1 + "\"");
  REQUIRE(test1.size() == 3);
  REQUIRE(test1[0] == "line1: word1");
  REQUIRE(test1[1] == " word2\nline2: word3");
  REQUIRE(test1[2] == " word4\n");

  const auto& testString2 = testString1;
  const auto test2        = StringSplit(testString2, "\n");
  UNSCOPED_INFO("testString2 = \"" << testString2 + "\"");
  REQUIRE(test2.size() == 2);
  REQUIRE(test2[0] == "line1: word1, word2");
  REQUIRE(test2[1] == "line2: word3, word4");

  const auto testString3 = std::string{"word1; word2; word3; word4"};
  const auto test3       = StringSplit(testString3, "; ");
  UNSCOPED_INFO("testString3 = \"" << testString3 + "\"");
  REQUIRE(test3.size() == 4);
  REQUIRE(test3[0] == "word1");
  REQUIRE(test3[1] == "word2");
  REQUIRE(test3[2] == "word3");
  REQUIRE(test3[3] == "word4");

  const auto testString4 = std::string{"word1 \nword2\nword3 \nword4 "};
  const auto test4       = StringSplit(testString4, "\n");
  UNSCOPED_INFO("testString4 = \"" << testString4 + "\"");
  REQUIRE(test4.size() == 4);
  REQUIRE(test4[0] == "word1 ");
  REQUIRE(test4[1] == "word2");
  REQUIRE(test4[2] == "word3 ");
  REQUIRE(test4[3] == "word4 ");

  const auto testString5 = std::string{"word1\n\nword2\n"};
  const auto test5       = StringSplit(testString5, "\n");
  UNSCOPED_INFO("testString5 = \"" << testString5 + "\"");
  REQUIRE(test5.size() == 3);
  REQUIRE(test5[0] == "word1");
  REQUIRE(test5[1].empty());
  REQUIRE(test5[2] == "word2");
}
// NOLINTEND(readability-function-cognitive-complexity)

TEST_CASE("EnumToString")
{
#ifdef NO_MAGIC_ENUM_AVAILABLE
  return;
#else
  enum class EnumTester : UnderlyingEnumType
  {
    _NULL = -1, // NOLINT: Need special name here
    TEST1,
    TEST2,
    TEST3,
    _num // unused, and marks the enum end
  };

  auto test = EnumTester::_NULL;
  REQUIRE(EnumToString(test) == "_NULL");
  test = EnumTester::TEST1;
  REQUIRE(EnumToString(test) == "TEST1");
  test = EnumTester::TEST2;
  REQUIRE(EnumToString(test) == "TEST2");
  test = EnumTester::_num;
  REQUIRE(EnumToString(test) == "_num");

  REQUIRE(EnumToString(EnumTester::TEST3) == "TEST3");
#endif
}

} // namespace GOOM::UNIT_TESTS
