// Reused from PoDoFo auxiliary/basecompat.h.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif
#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif
#ifdef _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif
