#pragma once

#include <cassert>

import Goom.GoomVisualization.GlUtils;

// NOLINTNEXTLINE: Happy with this case for my GL macro.
#define GlCall(x) \
  GOOM::OPENGL::GlClearError(); \
  x; \
  assert(GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__))
