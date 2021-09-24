#ifndef VISUALIZATION_GOOM_LIB_VISUAL_FX_VISUAL_FX_H
#define VISUALIZATION_GOOM_LIB_VISUAL_FX_VISUAL_FX_H

#include <string>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
#else
namespace GOOM::VISUAL_FX
{
#endif

class IVisualFx
{
public:
  IVisualFx() = default;
  IVisualFx(const IVisualFx&) noexcept = delete;
  IVisualFx(IVisualFx&&) noexcept = delete;
  virtual ~IVisualFx() = default;
  auto operator=(const IVisualFx&) -> IVisualFx& = delete;
  auto operator=(IVisualFx&&) -> IVisualFx& = delete;

  [[nodiscard]] virtual auto GetFxName() const -> std::string = 0;

  virtual void Start() = 0;

  virtual void Resume(){};
  virtual void Suspend(){};

  virtual void Finish() = 0;
};

#if __cplusplus <= 201402L
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX
#endif

#endif //VISUALIZATION_GOOM_LIB_VISUAL_FX_VISUAL_FX_H
