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

#ifndef BC_GRAPHICS_ISCENENODE_HPP
#define BC_GRAPHICS_ISCENENODE_HPP

#include "Types.hpp"

namespace bc { namespace graphics {

// Abstract scene node — base interface for all objects in the scene graph.
// Wraps Irrlicht's ISceneNode and its derivatives.
class ISceneNode {
public:
    virtual ~ISceneNode() = default;

    // Transform
    virtual void setPosition(const Vec3& pos) = 0;
    virtual Vec3 getPosition() const = 0;
    virtual Vec3 getAbsolutePosition() const = 0;
    virtual void setRotation(const Vec3& rotDeg) = 0;
    virtual Vec3 getRotation() const = 0;
    virtual void setScale(const Vec3& scale) = 0;
    virtual Vec3 getScale() const = 0;

    // Visibility
    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible() const = 0;

    // Scene graph hierarchy
    virtual void setParent(ISceneNode* parent) = 0;
    virtual ISceneNode* getParent() const = 0;
    virtual void remove() = 0;

    // Material (per material index)
    virtual void setMaterialFlag(MaterialFlag flag, bool value) = 0;
    virtual void setMaterialType(MaterialType type) = 0;
    virtual void setMaterialTexture(int layer, TextureHandle tex) = 0;

    // Node identity
    virtual void setID(int id) = 0;
    virtual int getID() const = 0;

    // Access the underlying engine-specific node (for migration)
    virtual void* getNativeNode() const = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_ISCENENODE_HPP
