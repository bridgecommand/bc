/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef BC_GRAPHICS_TYPES_HPP
#define BC_GRAPHICS_TYPES_HPP

#include <cstdint>
#include <cmath>

namespace bc { namespace graphics {

struct Vec2 {
    float x = 0, y = 0;
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
};

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
    Vec3 normalized() const {
        float len = length();
        return len > 0 ? Vec3{x/len, y/len, z/len} : Vec3{0,0,0};
    }
};

struct Vec3i {
    int32_t x = 0, y = 0, z = 0;
    Vec3i() = default;
    Vec3i(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}
};

struct Line3d {
    Vec3 start, end;
    Line3d() = default;
    Line3d(const Vec3& start, const Vec3& end) : start(start), end(end) {}
    Line3d(float sx, float sy, float sz, float ex, float ey, float ez)
        : start(sx, sy, sz), end(ex, ey, ez) {}
};

struct Color {
    uint8_t a = 255, r = 0, g = 0, b = 0;
    Color() = default;
    Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) : a(a), r(r), g(g), b(b) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : a(255), r(r), g(g), b(b) {}
};

struct Colorf {
    float r = 0, g = 0, b = 0, a = 1.0f;
    Colorf() = default;
    Colorf(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

struct Rect {
    int x = 0, y = 0, width = 0, height = 0;
    Rect() = default;
    Rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
};

struct Dimension2d {
    int width = 0, height = 0;
    Dimension2d() = default;
    Dimension2d(int w, int h) : width(w), height(h) {}
};

struct Quaternion {
    float x = 0, y = 0, z = 0, w = 1;
    Quaternion() = default;
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

// 4x4 transformation matrix, row-major (matching Irrlicht convention)
struct Matrix4 {
    float m[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    static Matrix4 identity() { return Matrix4{}; }

    Vec3 transformVec3(const Vec3& v) const {
        float w = m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15];
        if (w == 0) w = 1;
        return {
            (m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12]) / w,
            (m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13]) / w,
            (m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]) / w
        };
    }
};

// Material flags
enum class MaterialFlag {
    Lighting,
    Fog,
    BackFaceCulling,
    FrontFaceCulling,
    Wireframe,
    ZBuffer,
    ZWrite,
    AntiAliasing,
    BilinearFilter,
    TrilinearFilter,
    AnisotropicFilter,
    NormalizeNormals,
    ColorMask
};

// Material rendering types
enum class MaterialType {
    Solid,
    TransparentAddColor,
    TransparentAlphaChannel,
    TransparentAlphaChannelRef,
    TransparentVertexAlpha,
    Lightmap,
    LightmapAdd,
    DetailMap,
    SphereMap,
    OneTextureBlend
};

// Light types
enum class LightType {
    Point,
    Directional,
    Spot
};

// Opaque handle for engine-specific resources (meshes, textures, etc.)
using MeshHandle = void*;
using TextureHandle = void*;

// Clearing flags
enum ClearFlag : unsigned {
    ClearColor = 1,
    ClearDepth = 2,
    ClearStencil = 4
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_TYPES_HPP
