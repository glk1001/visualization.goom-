#include "bitmap_getter.h"

#include "utils/graphics/image_bitmaps.h"
#include "utils/graphics/small_image_bitmaps.h"
#include "utils/math/goom_rand_base.h"

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::GRAPHICS::ImageBitmap;
using UTILS::GRAPHICS::SmallImageBitmaps;
using UTILS::MATH::IGoomRand;

// clang-format off
static constexpr float IMAGE_NAMES_CIRCLE_WEIGHT        =  5.0F;
static constexpr float IMAGE_NAMES_SPHERE_WEIGHT        =  5.0F;
static constexpr float IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 20.0F;
static constexpr float IMAGE_NAMES_PINK_FLOWER_WEIGHT   = 20.0F;
static constexpr float IMAGE_NAMES_RED_FLOWER_WEIGHT    = 20.0F;
static constexpr float IMAGE_NAMES_WHITE_FLOWER_WEIGHT  = 10.0F;
// clang-format on

BitmapGetter::BitmapGetter(const UTILS::MATH::IGoomRand& goomRand,
                           const SmallImageBitmaps& smallBitmaps)
  : m_smallBitmaps{smallBitmaps},
    // clang-format off
    m_bitmapTypes{
      goomRand,
      {
        {SmallImageBitmaps::ImageNames::CIRCLE,        IMAGE_NAMES_CIRCLE_WEIGHT},
        {SmallImageBitmaps::ImageNames::SPHERE,        IMAGE_NAMES_SPHERE_WEIGHT},
        {SmallImageBitmaps::ImageNames::ORANGE_FLOWER, IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
        {SmallImageBitmaps::ImageNames::PINK_FLOWER,   IMAGE_NAMES_PINK_FLOWER_WEIGHT},
        {SmallImageBitmaps::ImageNames::RED_FLOWER,    IMAGE_NAMES_RED_FLOWER_WEIGHT},
        {SmallImageBitmaps::ImageNames::WHITE_FLOWER,  IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
      }
    },
    // clang-format on
    m_currentBitmapName{m_bitmapTypes.GetRandomWeighted()}
{
}

auto BitmapGetter::GetBitmap(const size_t size) const -> const UTILS::GRAPHICS::ImageBitmap&
{
  return m_smallBitmaps.GetImageBitmap(m_currentBitmapName,
                                       std::clamp(size, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER));
}

void BitmapGetter::ChangeCurrentBitmap()
{
  m_currentBitmapName = m_bitmapTypes.GetRandomWeighted();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
