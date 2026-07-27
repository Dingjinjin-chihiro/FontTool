// Derived from PoDoFo auxiliary/span.h
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <podofo/3rdparty/span.hpp>
namespace PoDoFo {
template <class T, size_t Extent = tcb::dynamic_extent>
using cspan = tcb::span<const T, Extent>;
template <class T, size_t Extent = tcb::dynamic_extent, typename std::enable_if<!std::is_const_v<T>, int>::type = 0>
using mspan = tcb::span<T, Extent>;
}
