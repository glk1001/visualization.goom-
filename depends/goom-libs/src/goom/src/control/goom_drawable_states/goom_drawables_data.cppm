module;

#include <string>
#include <string_view>
#include <vector>

export module Goom.Control.GoomDrawablesData;

import Goom.Control.GoomDrawables;
import Goom.Utils.EnumUtils;
import Goom.Utils.Math.GoomRandBase;
import Goom.Lib.GoomTypes;

using GOOM::UTILS::EnumMap;
using GOOM::UTILS::NUM;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::CONTROL
{

inline constexpr auto DRAWABLE_NAMES = EnumMap<GoomDrawables, std::string_view>{{{
    {GoomDrawables::CIRCLES, "CIRCLS"},
    {GoomDrawables::DOTS, "DOTS"},
    {GoomDrawables::IFS, "IFS"},
    {GoomDrawables::L_SYSTEM, "LSYS"},
    {GoomDrawables::LINES, "LNS"},
    {GoomDrawables::IMAGE, "IMG"},
    {GoomDrawables::PARTICLES, "PART"},
    {GoomDrawables::RAINDROPS, "DROPS"},
    {GoomDrawables::SHAPES, "SHPS"},
    {GoomDrawables::STARS, "STARS"},
    {GoomDrawables::TENTACLES, "TENTCL"},
    {GoomDrawables::TUBES, "TUBES"},
}}};

inline constexpr auto STATE_MULTI_THREADED = EnumMap<GoomDrawables, bool>{{{
    {GoomDrawables::CIRCLES, false},
    {GoomDrawables::DOTS, false},
    {GoomDrawables::IFS, false},
    {GoomDrawables::L_SYSTEM, false},
    {GoomDrawables::LINES, false},
    {GoomDrawables::IMAGE, true},
    {GoomDrawables::PARTICLES, false},
    {GoomDrawables::RAINDROPS, false},
    {GoomDrawables::SHAPES, false},
    {GoomDrawables::STARS, false},
    {GoomDrawables::TENTACLES, false},
    {GoomDrawables::TUBES, false},
}}};

inline constexpr auto BUFF_INTENSITY_RANGES = EnumMap<GoomDrawables, NumberRange<float>>{{{
    {GoomDrawables::CIRCLES, {0.50F, 0.80F}},
    {GoomDrawables::DOTS, {0.40F, 0.70F}},
    {GoomDrawables::IFS, {0.40F, 0.70F}},
    {GoomDrawables::L_SYSTEM, {0.70F, 0.80F}},
    {GoomDrawables::LINES, {0.50F, 0.70F}},
    {GoomDrawables::IMAGE, {0.05F, 0.30F}},
    {GoomDrawables::PARTICLES, {0.50F, 0.80F}},
    {GoomDrawables::RAINDROPS, {0.60F, 0.80F}},
    {GoomDrawables::SHAPES, {0.50F, 0.80F}},
    {GoomDrawables::STARS, {0.50F, 0.60F}},
    {GoomDrawables::TENTACLES, {0.30F, 0.50F}},
    {GoomDrawables::TUBES, {0.70F, 0.80F}},
}}};

inline constexpr auto PROB_SINGLE_DRAWABLE = EnumMap<GoomDrawables, float>{{{
    {GoomDrawables::CIRCLES, 1.0F},
    {GoomDrawables::DOTS, 1.0F},
    {GoomDrawables::IFS, 1.0F},
    {GoomDrawables::L_SYSTEM, 1.0F},
    {GoomDrawables::LINES, 1.0F},
    {GoomDrawables::IMAGE, 0.0F},
    {GoomDrawables::PARTICLES, 1.0F},
    {GoomDrawables::RAINDROPS, 1.0F},
    {GoomDrawables::SHAPES, 1.0F},
    {GoomDrawables::STARS, 1.0F},
    {GoomDrawables::TENTACLES, 1.0F},
    {GoomDrawables::TUBES, 1.0F},
}}};

} // namespace GOOM::CONTROL
