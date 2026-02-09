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

#ifndef BC_GRAPHICS_IMESHNODE_HPP
#define BC_GRAPHICS_IMESHNODE_HPP

#include "ISceneNode.hpp"

namespace bc { namespace graphics {

// Mesh scene node — a scene node that renders a static mesh.
// Wraps Irrlicht's IMeshSceneNode.
class IMeshNode : public ISceneNode {
public:
    virtual ~IMeshNode() = default;

    virtual MeshHandle getMesh() const = 0;

    // Per-material control (mesh may have multiple material slots)
    virtual int getMaterialCount() const = 0;
    virtual void setMaterialFlagForAll(MaterialFlag flag, bool value) = 0;
    virtual void setMaterialTypeForAll(MaterialType type) = 0;
};

// Animated mesh scene node — a mesh with skeletal animation.
// Wraps Irrlicht's IAnimatedMeshSceneNode.
class IAnimatedMeshNode : public ISceneNode {
public:
    virtual ~IAnimatedMeshNode() = default;

    virtual MeshHandle getMesh() const = 0;

    // Animation control
    virtual void setCurrentFrame(float frame) = 0;
    virtual float getFrameCount() const = 0;
    virtual void setAnimationSpeed(float fps) = 0;
    virtual void setLoopMode(bool loop) = 0;

    // Per-material control
    virtual int getMaterialCount() const = 0;
    virtual void setMaterialFlagForAll(MaterialFlag flag, bool value) = 0;
    virtual void setMaterialTypeForAll(MaterialType type) = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_IMESHNODE_HPP
