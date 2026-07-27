// Reused from PoDoFo auxiliary/Corners.h; PDF array conversions intentionally removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "Vector2.h"
namespace PoDoFo {
class Rect;
class PODOFO_API Corners final {
public:
    double X1; double Y1; double X2; double Y2;
    Corners() noexcept;
    Corners(double x1,double y1,double x2,double y2) noexcept;
    static Corners FromCorners(const Vector2& first,const Vector2& second) noexcept;
    Vector2 GetCorner1() const noexcept;
    Vector2 GetCorner2() const noexcept;
    double GetWidth() const noexcept;
    double GetHeight() const noexcept;
    Rect GetNormalized() const noexcept;
    bool operator==(const Corners& rhs) const noexcept;
    bool operator!=(const Corners& rhs) const noexcept;
    explicit operator Rect() const noexcept;
};
}
