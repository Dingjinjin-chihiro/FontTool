// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused from PoDoFo auxiliary/Rect.cpp; PDF object conversions removed.
#include "Rect.h"
#include "Matrix.h"
#include <algorithm>
#include <cstdio>
namespace PoDoFo {
namespace { Rect CreateRect(double x1,double y1,double x2,double y2) noexcept{if(x1>x2)std::swap(x1,x2);if(y1>y2)std::swap(y1,y2);return {x1,y1,x2-x1,y2-y1};} }
Rect::Rect() noexcept:X(0),Y(0),Width(0),Height(0){}
Rect::Rect(double x,double y,double w,double h) noexcept:X(x),Y(y),Width(w),Height(h){}
Rect Rect::FromCorners(double x1,double y1,double x2,double y2) noexcept{return CreateRect(x1,y1,x2,y2);}
Rect Rect::FromCorners(const Vector2& a,const Vector2& b) noexcept{return CreateRect(a.X,a.Y,b.X,b.Y);}
Rect Rect::FromCorners(const Corners& c) noexcept{return CreateRect(c.X1,c.Y1,c.X2,c.Y2);}
bool Rect::TryFromArray(const std::array<double,4>& v,Rect& out) noexcept{out=CreateRect(v[0],v[1],v[2],v[3]);return true;}
std::array<double,4> Rect::ToArray() const noexcept{return {X,Y,X+Width,Y+Height};}
std::string Rect::ToString() const{char b[160];const int n=std::snprintf(b,sizeof(b),"[ %.17g %.17g %.17g %.17g ]",X,Y,X+Width,Y+Height);return n>0?std::string(b,static_cast<size_t>(n)):std::string{};}
bool Rect::Contains(double x,double y) const noexcept{return x>=X&&x<=X+Width&&y>=Y&&y<=Y+Height;}
void Rect::Intersect(const Rect& r) noexcept{const double left=std::max(X,r.X),bottom=std::max(Y,r.Y),right=std::min(GetRight(),r.GetRight()),top=std::min(GetTop(),r.GetTop());X=left;Y=bottom;Width=std::max(0.0,right-left);Height=std::max(0.0,top-bottom);}
bool Rect::IsValid() const noexcept{return Width!=0&&Height!=0;}
Corners Rect::ToCorners() const noexcept{return {X,Y,X+Width,Y+Height};}
Vector2 Rect::GetLeftBottom() const noexcept{return {X,Y};}
Vector2 Rect::GetRightTop() const noexcept{return {X+Width,Y+Height};}
double Rect::GetRight() const noexcept{return X+Width;}
double Rect::GetTop() const noexcept{return Y+Height;}
bool Rect::operator==(const Rect& r) const noexcept{return X==r.X&&Y==r.Y&&Width==r.Width&&Height==r.Height;}
bool Rect::operator!=(const Rect& r) const noexcept{return !(*this==r);}
Rect Rect::operator*(const Matrix& m) const noexcept{return Rect::FromCorners(GetLeftBottom()*m,GetRightTop()*m);}
Rect::operator Corners() const noexcept{return ToCorners();}
}
