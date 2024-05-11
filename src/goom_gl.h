#pragma once

#ifdef IS_KODI_BUILD
#include <kodi/gui/gl/GL.h> // NOLINT: Too tricky with Kodi GL.
#else
#include "glad/glad.h"
#endif
