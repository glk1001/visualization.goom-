#pragma once

#include "tentacle3d.h"

#include <cstdint>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace VISUAL_FX
{
namespace TENTACLES
{
#else
namespace GOOM::VISUAL_FX::TENTACLES
{
#endif

class Tentacles3D
{
private:
  class Iter
  {
  public:
    Iter(Tentacles3D* const tents, const size_t pos) noexcept;
    auto operator!=(const Iter& other) const -> bool;
    auto operator*() const -> Tentacle3D&;
    auto operator++() -> const Iter&;

  private:
    size_t m_pos;
    Tentacles3D* m_tentacles;
  };

public:
  Tentacles3D() noexcept = default;

  Iter begin() { return {this, 0}; }
  Iter end() { return {this, m_tentacles.size()}; }

  [[nodiscard]] auto operator[](const size_t i) const -> const Tentacle3D&;
  [[nodiscard]] auto operator[](const size_t i) -> Tentacle3D&;

  void AddTentacle(Tentacle3D&& tentacle);
  void ColorMapsChanged();

private:
  std::vector<Tentacle3D> m_tentacles{};
};

inline auto Tentacles3D::operator[](const size_t i) const -> const Tentacle3D&
{
  return m_tentacles.at(i);
}

inline auto Tentacles3D::operator[](const size_t i) -> Tentacle3D&
{
  return m_tentacles.at(i);
}

inline void Tentacles3D::AddTentacle(Tentacle3D&& tentacle)
{
  m_tentacles.emplace_back(std::move(tentacle));
}

inline void Tentacles3D::ColorMapsChanged()
{
  for (auto& tentacle : m_tentacles)
  {
    tentacle.ColorMapsChanged();
  }
}

inline Tentacles3D::Iter::Iter(Tentacles3D* const tents, const size_t pos) noexcept
  : m_pos{pos}, m_tentacles{tents}
{
}

inline auto Tentacles3D::Iter::operator!=(const Iter& other) const -> bool
{
  return m_pos != other.m_pos;
}

inline auto Tentacles3D::Iter::operator*() const -> Tentacle3D&
{
  return (*m_tentacles)[m_pos];
}

inline auto Tentacles3D::Iter::operator++() -> const Iter&
{
  ++m_pos;
  return *this;
}

#if __cplusplus <= 201402L
} // namespace TENTACLES
} // namespace VISUAL_FX
} // namespace GOOM
#else
} // namespace GOOM::VISUAL_FX::TENTACLES
#endif
