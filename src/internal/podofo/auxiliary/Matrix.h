// Reused from PoDoFo auxiliary/Matrix.h; PDF array conversions intentionally removed.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include "MathBase.h"
#include "Vector2.h"
#include "span.h"
#include <array>
namespace PoDoFo {
class PODOFO_API Matrix final {
public:
    static Matrix Identity;
    Matrix() noexcept;
    Matrix(double a,double b,double c,double d,double e,double f) noexcept;
    static Matrix FromArray(const double arr[6]) noexcept;
    static bool TryFromArray(const cspan<double>& values,Matrix& output) noexcept;
    static Matrix CreateTranslation(const Vector2& tx) noexcept;
    static Matrix CreateScale(const Vector2& scale) noexcept;
    static Matrix CreateRotation(double theta) noexcept;
    static Matrix CreateRotation(const Vector2& center,double theta) noexcept;
    Matrix& Translate(const Vector2& tx) noexcept;
    Matrix Translated(const Vector2& tx) const noexcept;
    template <AlgebraicTrait trait> double Get() const noexcept{return MatrixTraits<trait>::Get(m_mat);}
    template <AlgebraicTrait trait> void Set(double value) noexcept{MatrixTraits<trait>::Set(m_mat,value);}
    template <AlgebraicTrait trait> Matrix& Apply(double value) noexcept{MatrixTraits<trait>::Apply(m_mat,value);return *this;}
    Matrix operator*(const Matrix& rhs) const noexcept;
    Matrix GetScalingRotation() const noexcept;
    bool TryGetRotation(Matrix& output) const noexcept;
    Vector2 GetScaleVector() const noexcept;
    Vector2 GetTranslationVector() const noexcept;
    void ToArray(double arr[6]) const noexcept;
    std::array<double,6> ToArray() const noexcept;
    bool operator==(const Matrix& rhs) const noexcept;
    bool operator!=(const Matrix& rhs) const noexcept;
    const double& operator[](unsigned index) const noexcept;
private:
    explicit Matrix(const double arr[6]) noexcept;
    double m_mat[6];
};
}
