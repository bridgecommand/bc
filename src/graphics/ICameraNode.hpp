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

#ifndef BC_GRAPHICS_ICAMERANODE_HPP
#define BC_GRAPHICS_ICAMERANODE_HPP

#include "ISceneNode.hpp"

namespace bc { namespace graphics {

// Camera scene node — wraps Irrlicht's ICameraSceneNode.
class ICameraNode : public ISceneNode {
public:
    virtual ~ICameraNode() = default;

    virtual void setTarget(const Vec3& target) = 0;
    virtual Vec3 getTarget() const = 0;

    virtual void setUpVector(const Vec3& up) = 0;
    virtual Vec3 getUpVector() const = 0;

    virtual void setFOV(float fovRadians) = 0;
    virtual float getFOV() const = 0;

    virtual void setAspectRatio(float aspect) = 0;
    virtual float getAspectRatio() const = 0;

    virtual void setNearValue(float near) = 0;
    virtual float getNearValue() const = 0;

    virtual void setFarValue(float far) = 0;
    virtual float getFarValue() const = 0;

    virtual Matrix4 getViewMatrix() const = 0;
    virtual Matrix4 getProjectionMatrix() const = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_ICAMERANODE_HPP
