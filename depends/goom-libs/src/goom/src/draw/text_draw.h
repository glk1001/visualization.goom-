#pragma once

#include "goom/spimpl.h"
#include "point2d.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace GOOM
{

class Pixel;

namespace DRAW
{

class IGoomDraw;

class TextDraw
{
public:
  explicit TextDraw(IGoomDraw& draw) noexcept;

  enum class TextAlignment
  {
    LEFT,
    CENTER,
    RIGHT
  };
  [[nodiscard]] auto GetAlignment() const -> TextAlignment;
  void SetAlignment(TextAlignment alignment);

  [[nodiscard]] auto GetFontSize() const -> int32_t;
  void SetFontSize(int32_t val);
  [[nodiscard]] auto GetLineSpacing() const -> int32_t;
  void SetOutlineWidth(float val);
  [[nodiscard]] auto GetCharSpacing() const -> float;
  void SetCharSpacing(float val);
  [[nodiscard]] auto GetFontFile() const -> const std::string&;
  void SetFontFile(const std::string& filename);

  using FontColorFunc =
      std::function<Pixel(size_t textIndexOfChar, Point2dInt pen, int32_t width, int32_t height)>;
  void SetFontColorFunc(const FontColorFunc& func);
  void SetOutlineFontColorFunc(const FontColorFunc& func);

  void SetParallelRender(bool val);

  void SetText(const std::string& str);
  void Prepare();

  struct Rect
  {
    int32_t xMin{};
    int32_t xMax{};
    int32_t yMin{};
    int32_t yMax{};
    [[nodiscard]] auto Width() const -> int32_t { return (xMax - xMin) + 1; }
    [[nodiscard]] auto Height() const -> int32_t { return (yMax - yMin) + 1; }
  };
  [[nodiscard]] auto GetPreparedTextBoundingRect() const -> Rect;
  [[nodiscard]] auto GetBearingX() const -> int;
  [[nodiscard]] auto GetBearingY() const -> int;

  void Draw(Point2dInt pen);
  void Draw(Point2dInt pen, Point2dInt& nextPen);

private:
  class TextDrawImpl;
  spimpl::unique_impl_ptr<TextDrawImpl> m_pimpl;
};

} // namespace DRAW
} // namespace GOOM
