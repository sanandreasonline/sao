// Surface in three-dimensional space
// Author(s):       iFarbod <ifarbod@outlook.com>
//
// Copyright (c) 2015-2017 Project CtNorth
//
// Distributed under the MIT license (See accompanying file LICENSE or copy at
// https://opensource.org/licenses/MIT)

#include <PCH.hpp>

#include <Math/Plane.hpp>

#include <DebugNew.hpp>

namespace Util
{

// Static initialization order can not be relied on, so do not use Vector3 constants
const Plane Plane::UP(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

void Plane::Transform(const Matrix3& transform)
{
    Define(Matrix4(transform).Inverse().Transpose() * ToVector4());
}

void Plane::Transform(const Matrix3x4& transform)
{
    Define(transform.ToMatrix4().Inverse().Transpose() * ToVector4());
}

void Plane::Transform(const Matrix4& transform)
{
    Define(transform.Inverse().Transpose() * ToVector4());
}

Matrix3x4 Plane::ReflectionMatrix() const
{
    return Matrix3x4(
        -2.0f * normal_.x_ * normal_.x_ + 1.0f,
        -2.0f * normal_.x_ * normal_.y_,
        -2.0f * normal_.x_ * normal_.z_,
        -2.0f * normal_.x_ * d_,
        -2.0f * normal_.y_ * normal_.x_,
        -2.0f * normal_.y_ * normal_.y_ + 1.0f,
        -2.0f * normal_.y_ * normal_.z_,
        -2.0f * normal_.y_ * d_,
        -2.0f * normal_.z_ * normal_.x_,
        -2.0f * normal_.z_ * normal_.y_,
        -2.0f * normal_.z_ * normal_.z_ + 1.0f,
        -2.0f * normal_.z_ * d_
    );
}

Plane Plane::Transformed(const Matrix3& transform) const
{
    return Plane(Matrix4(transform).Inverse().Transpose() * ToVector4());
}

Plane Plane::Transformed(const Matrix3x4& transform) const
{
    return Plane(transform.ToMatrix4().Inverse().Transpose() * ToVector4());
}

Plane Plane::Transformed(const Matrix4& transform) const
{
    return Plane(transform.Inverse().Transpose() * ToVector4());
}

}
