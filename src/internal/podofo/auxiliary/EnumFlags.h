// Derived from PoDoFo auxiliary/EnumFlags.h
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <type_traits>
#define ENABLE_BITMASK_OPERATORS(EnumType) \
inline constexpr EnumType operator|(EnumType lhs, EnumType rhs) noexcept { using U=std::underlying_type_t<EnumType>; return static_cast<EnumType>(static_cast<U>(lhs)|static_cast<U>(rhs)); } \
inline constexpr EnumType operator&(EnumType lhs, EnumType rhs) noexcept { using U=std::underlying_type_t<EnumType>; return static_cast<EnumType>(static_cast<U>(lhs)&static_cast<U>(rhs)); } \
inline constexpr EnumType& operator|=(EnumType& lhs, EnumType rhs) noexcept { lhs=lhs|rhs; return lhs; } \
inline constexpr EnumType& operator&=(EnumType& lhs, EnumType rhs) noexcept { lhs=lhs&rhs; return lhs; }
