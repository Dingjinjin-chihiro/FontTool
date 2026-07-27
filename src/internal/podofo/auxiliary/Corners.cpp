// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused from PoDoFo auxiliary/Corners.cpp; PDF array conversions removed.
#include "Corners.h"
#include "Rect.h"
#include <cmath>
namespace PoDoFo {
Corners::Corners() noexcept:X1(0),Y1(0),X2(0),Y2(0){}
Corners::Corners(double x1,double y1,double x2,double y2) noexcept:X1(x1),Y1(y1),X2(x2),Y2(y2){}
Corners Corners::FromCorners(const Vector2& a,const Vector2& b) noexcept{return {a.X,a.Y,b.X,b.Y};}
Vector2 Corners::GetCorner1() const noexcept{return {X1,Y1};}
Vector2 Corners::GetCorner2() const noexcept{return {X2,Y2};}
double Corners::GetWidth() const noexcept{return std::abs(X1-X2);}
double Corners::GetHeight() const noexcept{return std::abs(Y1-Y2);}
Rect Corners::GetNormalized() const noexcept{return Rect::FromCorners(*this);}
bool Corners::operator==(const Corners& r) const noexcept{return X1==r.X1&&Y1==r.Y1&&X2==r.X2&&Y2==r.Y2;}
bool Corners::operator!=(const Corners& r) const noexcept{return !(*this==r);}
Corners::operator Rect() const noexcept{return Rect::FromCorners(*this);}
}
