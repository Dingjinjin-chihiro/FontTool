// Reused from PoDoFo auxiliary/MathBase.h.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
namespace PoDoFo {
enum class AlgebraicTrait { Tx, Ty };
constexpr AlgebraicTrait Tx = AlgebraicTrait::Tx;
constexpr AlgebraicTrait Ty = AlgebraicTrait::Ty;
template <AlgebraicTrait> struct MatrixTraits;
template <> struct MatrixTraits<Tx> {
    static double Get(const double m[6]) noexcept { return m[4]; }
    static void Set(double m[6],double value) noexcept { m[4]=value; }
    static void Apply(double m[6],double value) noexcept { m[4]=value*m[0]+m[4]; }
};
template <> struct MatrixTraits<Ty> {
    static double Get(const double m[6]) noexcept { return m[5]; }
    static void Set(double m[6],double value) noexcept { m[5]=value; }
    static void Apply(double m[6],double value) noexcept { m[5]=value*m[3]+m[5]; }
};
}
