#pragma once

#include "goom/spimpl.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace GOOM
{

class Pixel;

namespace DRAW
{

class IGoomDraw;

class TextDraw
{
public:
  explicit TextDraw(const IGoomDraw& draw) noexcept;

  enum class TextAlignment
  {
    LEFT,
    CENTER,
    RIGHT
  };
  void SetAlignment(TextAlignment a);

  auto GetFontSize() const -> int32_t;
  void SetFontSize(int32_t val);
  auto GetLineSpacing() const -> int32_t;
  void SetOutlineWidth(float val);
  void SetCharSpacing(float val);
  [[nodiscard]] auto GetFontFile() const -> const std::string&;
  void SetFontFile(const std::string& filename);
  void SetText(const std::string& str);

  using FontColorFunc = std::function<Pixel(
      size_t textIndexOfChar, int32_t x, int32_t y, int32_t width, int32_t height)>;
  void SetFontColorFunc(const FontColorFunc& f);
  void SetOutlineFontColorFunc(const FontColorFunc& f);

  void Prepare();

  struct Rect
  {
    int32_t xmin{};
    int32_t xmax{};
    int32_t ymin{};
    int32_t ymax{};
    [[nodiscard]] auto Width() const -> int32_t { return (xmax - xmin) + 1; }
    [[nodiscard]] auto Height() const -> int32_t { return (ymax - ymin) + 1; }
  };
  [[nodiscard]] auto GetPreparedTextBoundingRect() const -> Rect;
  [[nodiscard]] auto GetBearingX() const -> int;
  [[nodiscard]] auto GetBearingY() const -> int;

  void Draw(int32_t xPen, int32_t yPen);
  void Draw(int32_t xPen, int32_t yPen, int32_t& xNext, int32_t& yNext);

private:
  class TextDrawImpl;
  spimpl::unique_impl_ptr<TextDrawImpl> m_textDrawImpl;
};

} // namespace DRAW
} // namespace GOOM
