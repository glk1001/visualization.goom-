#pragma once

#include <cstddef>
#include <memory>
#include <span>

inline constexpr auto CPLUSPLUS_17 = 201703L;
static_assert(__cplusplus >= CPLUSPLUS_17, "c++ std too low");

namespace GOOM::UTILS
{

template<class T>
class BufferView
{
public:
  BufferView(size_t buffLen, const T* buff) noexcept;
  BufferView(const BufferView&)                   = delete;
  BufferView(BufferView&&) noexcept               = default;
  auto operator=(const BufferView&) -> BufferView = delete;
  auto operator=(BufferView&&) -> BufferView      = delete;
  virtual ~BufferView() noexcept                  = default;

  [[nodiscard]] auto Data() const noexcept -> const T*;
  auto operator[](size_t i) const noexcept -> const T&;

  [[nodiscard]] auto GetBufferLen() const noexcept -> size_t;
  [[nodiscard]] auto GetBufferSizeBytes() const noexcept -> size_t;

protected:
  BufferView() noexcept;
  auto SetBuffer(size_t buffLen, const T* buff) noexcept -> void;

private:
  std_spn::span<const T> m_buffer{};
};

template<class T>
inline BufferView<T>::BufferView() noexcept = default;

template<class T>
inline BufferView<T>::BufferView(const size_t buffLen, const T* buff) noexcept
  : m_buffer{buff, buffLen}
{
}

template<class T>
void inline BufferView<T>::SetBuffer(const size_t buffLen, const T* buff) noexcept
{
  m_buffer = {buff, buffLen};
}

template<class T>
inline auto BufferView<T>::Data() const noexcept -> const T*
{
  return m_buffer.data();
}

template<class T>
inline auto BufferView<T>::operator[](const size_t i) const noexcept -> const T&
{
  return m_buffer[i];
}

template<class T>
inline auto BufferView<T>::GetBufferLen() const noexcept -> size_t
{
  return m_buffer.size();
}

template<class T>
inline auto BufferView<T>::GetBufferSizeBytes() const noexcept -> size_t
{
  return m_buffer.size_bytes();
}

} // namespace GOOM::UTILS
