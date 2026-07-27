// Reused from PoDoFo auxiliary/Rect.h; PDF array conversions intentionally removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "Corners.h"
#include <array>
#include <string>
namespace PoDoFo {
class Matrix;
class PODOFO_API Rect final {
public:
    double X; double Y; double Width; double Height;
    Rect() noexcept;
    Rect(double x,double y,double width,double height) noexcept;
    static Rect FromCorners(double x1,double y1,double x2,double y2) noexcept;
    static Rect FromCorners(const Vector2& first,const Vector2& second) noexcept;
    static Rect FromCorners(const Corners& corners) noexcept;
    static bool TryFromArray(const std::array<double,4>& values,Rect& output) noexcept;
    std::array<double,4> ToArray() const noexcept;
    std::string ToString() const;
    bool Contains(double x,double y) const noexcept;
    void Intersect(const Rect& rect) noexcept;
    bool IsValid() const noexcept;
    Corners ToCorners() const noexcept;
    Vector2 GetLeftBottom() const noexcept;
    Vector2 GetRightTop() const noexcept;
    double GetLeft() const noexcept{return X;}
    double GetBottom() const noexcept{return Y;}
    double GetRight() const noexcept;
    double GetTop() const noexcept;
    bool operator==(const Rect& rhs) const noexcept;
    bool operator!=(const Rect& rhs) const noexcept;
    Rect operator*(const Matrix& matrix) const noexcept;
    operator Corners() const noexcept;
};
}
