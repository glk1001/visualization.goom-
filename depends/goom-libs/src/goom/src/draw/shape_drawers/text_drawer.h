#pragma once

#include "draw/goom_draw.h"
#include "goom_types.h"
#include "point2d.h"
#include "spimpl.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace GOOM
{

class Pixel;

namespace DRAW::SHAPE_DRAWERS
{

class TextDrawer
{
public:
  explicit TextDrawer(IGoomDraw& draw) noexcept;

  enum class TextAlignment
  {
    LEFT,
    CENTER,
    RIGHT
  };
  [[nodiscard]] auto GetAlignment() const noexcept -> TextAlignment;
  auto SetAlignment(TextAlignment alignment) noexcept -> void;

  [[nodiscard]] auto GetFontSize() const noexcept -> int32_t;
  auto SetFontSize(int32_t val) -> void;
  [[nodiscard]] auto GetLineSpacing() const noexcept -> int32_t;
  auto SetOutlineWidth(float val) noexcept -> void;
  [[nodiscard]] auto GetCharSpacing() const noexcept -> float;
  auto SetCharSpacing(float val) noexcept -> void;
  [[nodiscard]] auto GetFontFile() const noexcept -> const std::string&;
  auto SetFontFile(const std::string& filename) -> void;

  using FontColorFunc = std::function<Pixel(
      size_t textIndexOfChar, Point2dInt pen, const Dimensions& charDimensions)>;
  auto SetFontColorFunc(const FontColorFunc& func) noexcept -> void;
  auto SetOutlineFontColorFunc(const FontColorFunc& func) noexcept -> void;

  auto SetParallelRender(bool val) noexcept -> void;

  auto SetText(const std::string& str) noexcept -> void;
  auto Prepare() -> void;

  struct Rect
  {
    int32_t xMin{};
    int32_t xMax{};
    int32_t yMin{};
    int32_t yMax{};
  };
  [[nodiscard]] static constexpr auto Width(const Rect& rect) noexcept -> uint32_t;
  [[nodiscard]] static constexpr auto Height(const Rect& rect) noexcept -> uint32_t;
  [[nodiscard]] static constexpr auto IntHeight(const Rect& rect) noexcept -> int32_t;
  [[nodiscard]] auto GetPreparedTextBoundingRect() const noexcept -> Rect;
  [[nodiscard]] auto GetBearingX() const noexcept -> int;
  [[nodiscard]] auto GetBearingY() const noexcept -> int;

  auto Draw(const Point2dInt& pen) -> void;
  auto Draw(const Point2dInt& pen, Point2dInt& nextPen) -> void;

private:
  class TextDrawerImpl;
  spimpl::unique_impl_ptr<TextDrawerImpl> m_pimpl;
};

} // namespace DRAW::SHAPE_DRAWERS
} // namespace GOOM
