#ifndef VISUALIZATION_GOOM_FILTER_DATA_H
#define VISUALIZATION_GOOM_FILTER_DATA_H

#include "goomutils/mathutils.h"
#include "v2d.h"

#include <cstdint>

namespace GOOM
{

namespace FILTERS
{
class ImageDisplacement;
}

enum class ZoomFilterMode
{
  _NULL = -1,
  AMULET_MODE = 0,
  CRYSTAL_BALL_MODE0,
  CRYSTAL_BALL_MODE1,
  HYPERCOS_MODE0,
  HYPERCOS_MODE1,
  HYPERCOS_MODE2,
  HYPERCOS_MODE3,
  IMAGE_DISPLACEMENT_MODE,
  NORMAL_MODE,
  SCRUNCH_MODE,
  SPEEDWAY_MODE,
  WATER_MODE,
  WAVE_MODE0,
  WAVE_MODE1,
  Y_ONLY_MODE,
  _NUM // unused and must be last
};

enum class HypercosOverlay
{
  NONE,
  MODE0,
  MODE1,
  MODE2,
  MODE3,
};

// 128 = vitesse nule...
// 256 = en arriere
//   hyper vite.. * * 0 = en avant hype vite.
// 128 = zero speed
// 256 = reverse
//   super fast ... 0 = forward quickly.
class Vitesse
{
  static constexpr int32_t MAX_VITESSE = 128;

public:
  static constexpr int32_t STOP_SPEED = MAX_VITESSE;
  static constexpr int32_t FASTEST_SPEED = 0;
  static constexpr int32_t DEFAULT_VITESSE = 127;

  auto GetVitesse() const -> int32_t { return m_vitesse; };
  void SetVitesse(int32_t val);
  void SetDefault();
  void GoSlowerBy(int32_t val);
  void GoFasterBy(int32_t val);

  auto GetReverseVitesse() const -> bool { return m_reverseVitesse; }
  void SetReverseVitesse(const bool val) { m_reverseVitesse = val; }
  void ToggleReverseVitesse() { m_reverseVitesse = !m_reverseVitesse; }

  auto GetRelativeSpeed() const -> float;

private:
  int32_t m_vitesse = DEFAULT_VITESSE;
  bool m_reverseVitesse = true;
};

inline void Vitesse::SetVitesse(const int32_t val)
{
  m_vitesse = stdnew::clamp(val, FASTEST_SPEED, STOP_SPEED);
}

inline void Vitesse::SetDefault()
{
  m_vitesse = DEFAULT_VITESSE;
  m_reverseVitesse = true;
}

inline void Vitesse::GoSlowerBy(const int32_t val)
{
  SetVitesse(m_vitesse + val);
}

inline void Vitesse::GoFasterBy(const int32_t val)
{
  SetVitesse(m_vitesse - 30 * val);
}

inline auto Vitesse::GetRelativeSpeed() const -> float
{
  const float speed = static_cast<float>(m_vitesse - MAX_VITESSE) / static_cast<float>(MAX_VITESSE);
  return m_reverseVitesse ? -speed : +speed;
}

struct ZoomFilterData
{
  //TODO - Have class with SetMode which handles hypercosOverlay
  ZoomFilterMode mode = ZoomFilterMode::NORMAL_MODE;
  HypercosOverlay hypercosOverlay = HypercosOverlay::NONE;

  void SetMode(const ZoomFilterMode zoomFilterMode)
  {
    mode = zoomFilterMode;
    switch (mode)
    {
      case ZoomFilterMode::HYPERCOS_MODE0:
        hypercosOverlay = HypercosOverlay::MODE0;
        break;
      case ZoomFilterMode::HYPERCOS_MODE1:
        hypercosOverlay = HypercosOverlay::MODE1;
        break;
      case ZoomFilterMode::HYPERCOS_MODE2:
        hypercosOverlay = HypercosOverlay::MODE2;
        break;
      case ZoomFilterMode::HYPERCOS_MODE3:
        hypercosOverlay = HypercosOverlay::MODE3;
        break;
      default:
        break;
    }
  }

  Vitesse vitesse{};
  static constexpr uint32_t DEFAULT_MIDDLE_X = 16;
  static constexpr uint32_t DEFAULT_MIDDLE_Y = 1;
  V2dInt zoomMidPoint = {DEFAULT_MIDDLE_X, DEFAULT_MIDDLE_Y}; // milieu de l'effet
  bool tanEffect = false;
  bool blockyWavy = false;
  static constexpr float MIN_ROTATE_SPEED = -0.5F;
  static constexpr float MAX_ROTATE_SPEED = +0.5F;
  static constexpr float DEFAULT_ROTATE_SPEED = 0.0F;
  float rotateSpeed = DEFAULT_ROTATE_SPEED;

  static constexpr float SPEED_COEFF_DENOMINATOR = 50.0F;
  static constexpr float MIN_SPEED_COEFF = -4.01F;
  static constexpr float MAX_MAX_SPEED_COEFF = +4.01F;
  static constexpr float DEFAULT_MAX_SPEED_COEFF = +2.01F;

  // Noise:
  bool noisify = false; // ajoute un bruit a la transformation
  float noiseFactor = 1.0; // in range [0, 1]
  // For noise amplitude, take the reciprocal of these.
  static constexpr float NOISE_MIN = 40.0F;
  static constexpr float NOISE_MAX = 120.0F;

  // H Plane:
  // @since June 2001
  int hPlaneEffect = 0; // deviation horitontale
  static constexpr float DEFAULT_H_PLANE_EFFECT_AMPLITUDE = 0.0025F;
  static constexpr float MIN_H_PLANE_EFFECT_AMPLITUDE = 0.0015F;
  static constexpr float MAX_H_PLANE_EFFECT_AMPLITUDE = 0.0035F;
  float hPlaneEffectAmplitude = DEFAULT_H_PLANE_EFFECT_AMPLITUDE;

  // V Plane:
  int vPlaneEffect = 0; // deviation verticale
  static constexpr float DEFAULT_V_PLANE_EFFECT_AMPLITUDE = 0.0025F;
  static constexpr float MIN_V_PLANE_EFFECT_AMPLITUDE = 0.0015F;
  static constexpr float MAX_V_PLANE_EFFECT_AMPLITUDE = 0.0035F;
  float vPlaneEffectAmplitude = DEFAULT_V_PLANE_EFFECT_AMPLITUDE;

  // Image Displacement:
  static constexpr float DEFAULT_IMAGE_DISPL_AMPLITUDE = 0.0250F;
  static constexpr float MIN_IMAGE_DISPL_AMPLITUDE = 0.0025F;
  static constexpr float MAX_IMAGE_DISPL_AMPLITUDE = 0.1000F;
  float imageDisplacementAmplitude = DEFAULT_IMAGE_DISPL_AMPLITUDE;
  std::shared_ptr<FILTERS::ImageDisplacement> imageDisplacement{};
  static constexpr float MIN_IMAGE_DISPL_COLOR_CUTOFF = 0.1F;
  static constexpr float MAX_IMAGE_DISPL_COLOR_CUTOFF = 0.9F;
  static constexpr float DEFAULT_IMAGE_DISPL_COLOR_CUTOFF = 0.5F;
  float imageDisplacementXColorCutoff = DEFAULT_IMAGE_DISPL_COLOR_CUTOFF;
  float imageDisplacementYColorCutoff = DEFAULT_IMAGE_DISPL_COLOR_CUTOFF;
  static constexpr float MIN_IMAGE_DISPL_ZOOM_FACTOR = 1.00F;
  static constexpr float MAX_IMAGE_DISPL_ZOOM_FACTOR = 10.0F;
  static constexpr float DEFAULT_IMAGE_DISPL_ZOOM_FACTOR = 5.0F;
  float imageDisplacementZoomFactor = DEFAULT_IMAGE_DISPL_ZOOM_FACTOR;
};

} // namespace GOOM
#endif
