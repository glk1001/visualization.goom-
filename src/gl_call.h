#pragma once

import Goom.GoomVisualization.GlUtils;
import Goom.Lib.AssertUtils;

// NOLINTNEXTLINE: Happy with this case for my GL macro.
#define GlCall(x) \
  GOOM::OPENGL::GlClearError(); \
  x; \
  Assert(GOOM::OPENGL::CheckForOpenGLError(__FILE__, __LINE__))
