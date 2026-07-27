// Font-only adaptation derived from PoDoFo.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
// Reused from PoDoFo auxiliary/Matrix.cpp; PDF object conversions removed.
#include "Matrix.h"
#include <cmath>
namespace PoDoFo {
Matrix Matrix::Identity;
Matrix::Matrix() noexcept:m_mat{1,0,0,1,0,0}{}
Matrix::Matrix(double a,double b,double c,double d,double e,double f) noexcept:m_mat{a,b,c,d,e,f}{}
Matrix::Matrix(const double arr[6]) noexcept:m_mat{arr[0],arr[1],arr[2],arr[3],arr[4],arr[5]}{}
Matrix Matrix::FromArray(const double arr[6]) noexcept{return Matrix(arr);}
bool Matrix::TryFromArray(const cspan<double>& values,Matrix& output) noexcept{if(values.size()!=6)return false;double arr[6];for(size_t i=0;i<6;++i)arr[i]=values[i];output=Matrix(arr);return true;}
Matrix Matrix::CreateTranslation(const Vector2& tx) noexcept{return {1,0,0,1,tx.X,tx.Y};}
Matrix Matrix::CreateScale(const Vector2& scale) noexcept{return {scale.X,0,0,scale.Y,0,0};}
Matrix Matrix::CreateRotation(double theta) noexcept{return CreateRotation(Vector2(),theta);}
Matrix Matrix::CreateRotation(const Vector2& c,double theta) noexcept{const double a=std::cos(theta),b=std::sin(theta);return {a,b,-b,a,-c.X*a+c.Y*b+c.X,-c.X*b-c.Y*a+c.Y};}
Matrix& Matrix::Translate(const Vector2& tx) noexcept{m_mat[4]=tx.X*m_mat[0]+tx.Y*m_mat[2]+m_mat[4];m_mat[5]=tx.X*m_mat[1]+tx.Y*m_mat[3]+m_mat[5];return *this;}
Matrix Matrix::Translated(const Vector2& tx) const noexcept{auto out=*this;out.Translate(tx);return out;}
Matrix Matrix::operator*(const Matrix& r) const noexcept{return {m_mat[0]*r[0]+m_mat[1]*r[2],m_mat[0]*r[1]+m_mat[1]*r[3],m_mat[2]*r[0]+m_mat[3]*r[2],m_mat[2]*r[1]+m_mat[3]*r[3],m_mat[4]*r[0]+m_mat[5]*r[2]+r[4],m_mat[4]*r[1]+m_mat[5]*r[3]+r[5]};}
Matrix Matrix::GetScalingRotation() const noexcept{return {m_mat[0],m_mat[1],m_mat[2],m_mat[3],0,0};}
bool Matrix::TryGetRotation(Matrix& output) const noexcept{const auto s=GetScaleVector();if(s.X==0||s.Y==0)return false;output=Matrix(m_mat[0]/s.X,m_mat[1]/s.Y,m_mat[2]/s.X,m_mat[3]/s.Y,0,0);return true;}
Vector2 Matrix::GetScaleVector() const noexcept{return {std::sqrt(m_mat[0]*m_mat[0]+m_mat[2]*m_mat[2]),std::sqrt(m_mat[1]*m_mat[1]+m_mat[3]*m_mat[3])};}
Vector2 Matrix::GetTranslationVector() const noexcept{return {m_mat[4],m_mat[5]};}
void Matrix::ToArray(double arr[6]) const noexcept{for(size_t i=0;i<6;++i)arr[i]=m_mat[i];}
std::array<double,6> Matrix::ToArray() const noexcept{return {m_mat[0],m_mat[1],m_mat[2],m_mat[3],m_mat[4],m_mat[5]};}
bool Matrix::operator==(const Matrix& r) const noexcept{for(size_t i=0;i<6;++i)if(m_mat[i]!=r.m_mat[i])return false;return true;}
bool Matrix::operator!=(const Matrix& r) const noexcept{return !(*this==r);}
const double& Matrix::operator[](unsigned i) const noexcept{return m_mat[i<6?i:0];}
}
