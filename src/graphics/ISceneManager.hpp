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

#ifndef BC_GRAPHICS_ISCENEMANAGER_HPP
#define BC_GRAPHICS_ISCENEMANAGER_HPP

#include "Types.hpp"
#include <string>

namespace bc { namespace graphics {

// Forward declarations
class ISceneNode;
class IMeshNode;
class IAnimatedMeshNode;
class ICameraNode;
class ILightNode;

// Abstract scene manager — factory for scene nodes and resource loading.
// Wraps Irrlicht's ISceneManager.
class ISceneManager {
public:
    virtual ~ISceneManager() = default;

    // Scene graph root
    virtual ISceneNode* getRootSceneNode() = 0;

    // Node creation
    virtual IMeshNode* addMeshSceneNode(MeshHandle mesh,
                                         ISceneNode* parent = nullptr,
                                         int id = -1,
                                         const Vec3& position = {0,0,0},
                                         const Vec3& rotation = {0,0,0},
                                         const Vec3& scale = {1,1,1}) = 0;

    virtual IAnimatedMeshNode* addAnimatedMeshSceneNode(MeshHandle mesh,
                                                         ISceneNode* parent = nullptr,
                                                         int id = -1,
                                                         const Vec3& position = {0,0,0},
                                                         const Vec3& rotation = {0,0,0},
                                                         const Vec3& scale = {1,1,1}) = 0;

    virtual ICameraNode* addCameraSceneNode(ISceneNode* parent = nullptr,
                                             const Vec3& position = {0,0,5},
                                             const Vec3& lookAt = {0,0,0},
                                             int id = -1) = 0;

    virtual ILightNode* addLightSceneNode(ISceneNode* parent = nullptr,
                                           const Vec3& position = {0,0,0},
                                           Colorf color = {1,1,1,1},
                                           float radius = 100.0f,
                                           int id = -1) = 0;

    virtual ISceneNode* addBillboardSceneNode(ISceneNode* parent = nullptr,
                                               const Vec2& size = {10,10},
                                               const Vec3& position = {0,0,0},
                                               int id = -1) = 0;

    virtual ISceneNode* addSkyBoxSceneNode(TextureHandle top, TextureHandle bottom,
                                            TextureHandle left, TextureHandle right,
                                            TextureHandle front, TextureHandle back,
                                            ISceneNode* parent = nullptr,
                                            int id = -1) = 0;

    // Terrain
    virtual ISceneNode* addTerrainSceneNode(const std::string& heightmapFile,
                                             ISceneNode* parent = nullptr,
                                             int id = -1,
                                             const Vec3& position = {0,0,0},
                                             const Vec3& rotation = {0,0,0},
                                             const Vec3& scale = {1,1,1}) = 0;

    // Resource loading
    virtual MeshHandle getMesh(const std::string& filename) = 0;

    // Node lookup
    virtual ISceneNode* getSceneNodeFromId(int id, ISceneNode* start = nullptr) = 0;

    // Render the scene
    virtual void drawAll() = 0;

    // Access the underlying engine-specific scene manager (for migration)
    virtual void* getNativeSceneManager() const = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_ISCENEMANAGER_HPP
