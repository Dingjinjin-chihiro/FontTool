// Reused from PoDoFo auxiliary/Vector2.h; PDF-independent.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "basedefs.h"
namespace PoDoFo {
class Matrix;
class PODOFO_API Vector2 final {
public:
    Vector2() noexcept;
    Vector2(double x,double y) noexcept;
    double GetLength() const noexcept;
    double GetSquaredLength() const noexcept;
    Vector2 operator+(const Vector2& v) const noexcept;
    Vector2 operator-(const Vector2& v) const noexcept;
    Vector2 operator*(const Matrix& m) const noexcept;
    Vector2& operator+=(const Vector2& v) noexcept;
    Vector2& operator-=(const Vector2& v) noexcept;
    double Dot(const Vector2& v) const noexcept;
    bool operator==(const Vector2& v) const noexcept;
    bool operator!=(const Vector2& v) const noexcept;
    double X;
    double Y;
};
}
