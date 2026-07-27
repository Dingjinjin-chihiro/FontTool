// Derived from PoDoFo auxiliary/basedefs.h
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#if defined(_WIN32) && defined(PODOFO_FONT_TOOLS_SHARED)
  #if defined(PODOFO_FONT_TOOLS_BUILD)
    #define PODOFO_API __declspec(dllexport)
  #else
    #define PODOFO_API __declspec(dllimport)
  #endif
#else
  #define PODOFO_API
#endif
