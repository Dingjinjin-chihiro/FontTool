// SPDX-License-Identifier: MPL-2.0
#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(FONTTOOL_SHARED)
#    if defined(FONTTOOL_BUILDING_LIBRARY)
#      define FONTTOOL_API __declspec(dllexport)
#    else
#      define FONTTOOL_API __declspec(dllimport)
#    endif
#  else
#    define FONTTOOL_API
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  define FONTTOOL_API __attribute__((visibility("default")))
#else
#  define FONTTOOL_API
#endif
