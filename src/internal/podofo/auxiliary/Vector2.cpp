// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused from PoDoFo auxiliary/Vector2.cpp; removed PdfDeclarationsPrivate include.
#include "Vector2.h"
#include "Matrix.h"
#include <cmath>
namespace PoDoFo {
Vector2::Vector2() noexcept:X(0),Y(0){}
Vector2::Vector2(double x,double y) noexcept:X(x),Y(y){}
double Vector2::GetLength() const noexcept{return std::sqrt(X*X+Y*Y);}
double Vector2::GetSquaredLength() const noexcept{return X*X+Y*Y;}
Vector2 Vector2::operator+(const Vector2& v) const noexcept{return {X+v.X,Y+v.Y};}
Vector2 Vector2::operator-(const Vector2& v) const noexcept{return {X-v.X,Y-v.Y};}
Vector2 Vector2::operator*(const Matrix& m) const noexcept{return {m[0]*X+m[2]*Y+m[4],m[1]*X+m[3]*Y+m[5]};}
Vector2& Vector2::operator+=(const Vector2& v) noexcept{X+=v.X;Y+=v.Y;return *this;}
Vector2& Vector2::operator-=(const Vector2& v) noexcept{X-=v.X;Y-=v.Y;return *this;}
double Vector2::Dot(const Vector2& v) const noexcept{return X*v.X+Y*v.Y;}
bool Vector2::operator==(const Vector2& v) const noexcept{return X==v.X&&Y==v.Y;}
bool Vector2::operator!=(const Vector2& v) const noexcept{return !(*this==v);}
}
