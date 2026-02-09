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

#ifndef BC_GRAPHICS_ILIGHTNODE_HPP
#define BC_GRAPHICS_ILIGHTNODE_HPP

#include "ISceneNode.hpp"

namespace bc { namespace graphics {

// Light data for configuring a light node
struct LightData {
    LightType type = LightType::Directional;
    Colorf diffuse  = {1.0f, 1.0f, 1.0f, 1.0f};
    Colorf specular = {1.0f, 1.0f, 1.0f, 1.0f};
    Colorf ambient  = {0.0f, 0.0f, 0.0f, 1.0f};
    Vec3 direction  = {0.0f, -1.0f, 0.0f};
    float radius = 100.0f;
    float innerCone = 0.0f;   // spot light only (degrees)
    float outerCone = 45.0f;  // spot light only (degrees)
    float falloff = 2.0f;     // spot light only
};

// Light scene node — wraps Irrlicht's ILightSceneNode.
class ILightNode : public ISceneNode {
public:
    virtual ~ILightNode() = default;

    virtual void setLightData(const LightData& data) = 0;
    virtual LightData getLightData() const = 0;

    virtual void setLightType(LightType type) = 0;
    virtual LightType getLightType() const = 0;

    virtual void setRadius(float radius) = 0;
    virtual float getRadius() const = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_ILIGHTNODE_HPP
