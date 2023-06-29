#pragma once

#include "goom/goom_config.h"
#include "goom/goom_logger.h"

#include <chrono>
#include <condition_variable>
#include <format>
#include <functional>
#include <mutex>
#include <queue>

namespace GOOM
{

template<typename TResource>
class SlotProducerConsumer
{
public:
  SlotProducerConsumer(GOOM::GoomLogger& goomLogger,
                       size_t maxInUseSlots,
                       size_t maxResourceItems) noexcept;

  auto ProducerThread() noexcept -> void;
  auto Start() noexcept -> void;
  auto Stop() noexcept -> void;

  [[nodiscard]] auto AddResource(const TResource& resource) noexcept -> bool;
  [[nodiscard]] auto ConsumeWithoutRelease(uint32_t waitMs) noexcept -> bool;
  auto Release(size_t slot) noexcept -> void;

  using ProduceItemFunc = std::function<void(size_t slot, const TResource& resource)>;
  auto SetProduceItem(const ProduceItemFunc& produceItemFunc) noexcept -> void;

  using ConsumeItemFunc = std::function<void(size_t slot)>;
  auto SetConsumeItem(const ConsumeItemFunc& consumeItemFunc) noexcept -> void;

private:
  GOOM::GoomLogger* m_goomLogger;
  bool m_finished = false;
  std::mutex m_mutex{};
  std::condition_variable m_producer_cv{};
  std::condition_variable m_consumer_cv{};
  std::condition_variable m_resourcer_cv{};

  size_t m_maxInUseSlots;
  size_t m_maxResourceItems;
  std::queue<size_t> m_inUseSlotsQueue{};
  std::queue<size_t> m_freeSlotsQueue;
  std::queue<TResource> m_resourceQueue{};

  ProduceItemFunc m_produceItem{};
  ConsumeItemFunc m_consumeItem{};

  auto Produce() noexcept -> void;
};

template<typename TResource>
SlotProducerConsumer<TResource>::SlotProducerConsumer(GOOM::GoomLogger& goomLogger,
                                                      const size_t maxInUseSlots,
                                                      const size_t maxResourceItems) noexcept
  : m_goomLogger{&goomLogger}, m_maxInUseSlots{maxInUseSlots}, m_maxResourceItems{maxResourceItems}
{
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Start() noexcept -> void
{
  m_finished = false;

  m_inUseSlotsQueue = std::queue<size_t>{};
  m_freeSlotsQueue  = std::queue<size_t>{};
  m_resourceQueue   = std::queue<TResource>{};

  for (auto slot = 0U; slot < m_maxInUseSlots; ++slot)
  {
    m_freeSlotsQueue.push(slot);
  }
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Stop() noexcept -> void
{
  m_finished = true;
  m_producer_cv.notify_all();
  m_consumer_cv.notify_all();
  m_resourcer_cv.notify_all();
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::SetProduceItem(
    const ProduceItemFunc& produceItemFunc) noexcept -> void
{
  m_produceItem = produceItemFunc;
}

template<typename TResource>
inline auto SlotProducerConsumer<TResource>::SetConsumeItem(
    const ConsumeItemFunc& consumeItemFunc) noexcept -> void
{
  m_consumeItem = consumeItemFunc;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ProducerThread() noexcept -> void
{
  while (not m_finished)
  {
    Produce();
  }
}

// TODO - MOVE???
template<typename TResource>
auto SlotProducerConsumer<TResource>::AddResource(const TResource& resource) noexcept -> bool
{
  const auto lock = std::lock_guard<std::mutex>{m_mutex};

  if (m_resourceQueue.size() >= m_maxResourceItems)
  {
    return false;
  }

  Expects(m_resourceQueue.size() < m_maxResourceItems);
  m_resourceQueue.push(resource);
  m_producer_cv.notify_all();
  return true;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::ConsumeWithoutRelease(uint32_t waitMs) noexcept -> bool
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};

  if (m_inUseSlotsQueue.empty())
  {
    // LogInfo(*m_goomLogger, "*** Consumer is waiting {}ms for non-empty in-use queue.", waitMs);
    if (not m_consumer_cv.wait_for(lock,
                                   std::chrono::milliseconds{waitMs},
                                   [this]
                                   { return m_finished or (not m_inUseSlotsQueue.empty()); }))
    {
      return false;
    }
  }
  if (m_finished)
  {
    return false;
  }

  Expects((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);
  const auto slot = m_inUseSlotsQueue.front();

  lock.unlock();
  m_consumeItem(slot);

  return true;
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Release(const size_t slot) noexcept -> void
{
  const auto lock = std::lock_guard<std::mutex>{m_mutex};

  Expects(m_inUseSlotsQueue.front() == slot);

  m_freeSlotsQueue.push(slot);
  m_inUseSlotsQueue.pop();
  m_producer_cv.notify_all();
}

template<typename TResource>
auto SlotProducerConsumer<TResource>::Produce() noexcept -> void
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};

  if (m_resourceQueue.empty())
  {
    // LogInfo(*m_goomLogger, "### Producer is waiting for non-empty resource queue.");
    m_producer_cv.wait(lock, [this] { return m_finished or (not m_resourceQueue.empty()); });
  }
  if (m_finished)
  {
    return;
  }
  if (m_inUseSlotsQueue.size() >= m_maxInUseSlots)
  {
    // LogInfo(*m_goomLogger, "### Producer is waiting for in-use queue to decrease.");
    m_producer_cv.wait(
        lock, [this] { return m_finished or (m_inUseSlotsQueue.size() < m_maxInUseSlots); });
  }
  if (m_finished)
  {
    return;
  }

  Expects((m_inUseSlotsQueue.size() + m_freeSlotsQueue.size()) == m_maxInUseSlots);
  Expects(not m_resourceQueue.empty());
  const auto nextSlot = m_freeSlotsQueue.front();

  lock.unlock();
  m_produceItem(nextSlot, m_resourceQueue.front());
  lock.lock();

  m_resourceQueue.pop();
  m_inUseSlotsQueue.push(nextSlot);
  m_freeSlotsQueue.pop();
  m_consumer_cv.notify_all();
  m_resourcer_cv.notify_all();
}

} // namespace GOOM
