/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     Terrain rendering adapter: loads BC heightmaps into WE terrain system. */

#ifndef BC_GRAPHICS_WICKED_TERRAINNODE_HPP
#define BC_GRAPHICS_WICKED_TERRAINNODE_HPP

#ifdef WITH_WICKED_ENGINE

#include "../ISceneNode.hpp"
#include "WickedSceneNode.hpp"
#include "WickedEngine.h"
#include <string>
#include <vector>

namespace bc { namespace graphics { namespace wicked {

// Configuration for a single terrain tile (maps to BC terrain.ini)
struct TerrainTileConfig {
    std::string heightmapPath;       // Path to heightmap file (PNG, F32, HDR)
    std::string texturePath;         // Path to terrain texture
    float longitude = 0;             // Westmost longitude (degrees)
    float latitude = 0;              // Southmost latitude (degrees)
    float lonExtent = 0;             // Longitude span (degrees)
    float latExtent = 0;             // Latitude span (degrees)
    float maxHeight = 100;           // Maximum terrain elevation (meters)
    float seaMaxDepth = 50;          // Maximum water depth (meters)
    bool usesRGB = false;            // RGB-encoded heightmap (chart-generated)
    int heightmapRows = 0;           // For F32/HDR: grid rows (0 = auto)
    int heightmapCols = 0;           // For F32/HDR: grid columns (0 = auto)
};

// Wraps Wicked Engine's terrain system for Bridge Command.
// Loads BC-format heightmaps and textures into WE's chunk-based terrain.
class WickedTerrainNode : public ISceneNode {
public:
    explicit WickedTerrainNode(wi::scene::Scene* scene);
    ~WickedTerrainNode() override;

    // Load terrain from BC terrain.ini configuration
    bool loadFromConfig(const TerrainTileConfig& config,
                         float refLongitude, float refLatitude);

    // Load a raw heightmap data array (already parsed by BC's Terrain class)
    // Heights are in meters, normalized. Grid is heightmapWidth x heightmapHeight.
    bool loadFromHeightData(const std::vector<std::vector<float>>& heights,
                             float maxHeight, float seaMaxDepth,
                             float worldWidth, float worldDepth,
                             const std::string& texturePath);

    // Get terrain height at a world position (for physics/collision)
    float getHeightAt(float worldX, float worldZ) const;

    // ISceneNode interface
    void setPosition(const Vec3& pos) override;
    Vec3 getPosition() const override;
    void setRotation(const Vec3& rot) override;
    Vec3 getRotation() const override { return {}; }
    void setScale(const Vec3& scale) override;
    Vec3 getScale() const override { return scale_; }
    void setVisible(bool visible) override;
    bool isVisible() const override { return visible_; }
    void setParent(ISceneNode* parent) override {}
    Vec3 getAbsolutePosition() const override { return position_; }
    ISceneNode* getParent() const override { return nullptr; }
    void remove() override;
    void setMaterialFlag(MaterialFlag, bool) override {}
    void setMaterialType(MaterialType) override {}
    void setMaterialTexture(int, TextureHandle) override {}
    void setID(int id) override { id_ = id; }
    int getID() const override { return id_; }
    void* getNativeNode() const override { return (void*)(uintptr_t)rootEntity_; }

private:
    wi::scene::Scene* weScene = nullptr;
    wi::ecs::Entity rootEntity_ = wi::ecs::INVALID_ENTITY;

    // Heightmap data for CPU-side height queries
    std::vector<std::vector<float>> heightData_;
    float worldWidth_ = 0, worldDepth_ = 0;
    float maxHeight_ = 0, seaMaxDepth_ = 0;

    Vec3 position_;
    Vec3 scale_{1, 1, 1};
    bool visible_ = true;
    int id_ = -1;

    // Load heightmap from PNG file
    bool loadHeightmapPNG(const std::string& path, const TerrainTileConfig& config);
    // Load heightmap from F32 binary file
    bool loadHeightmapF32(const std::string& path, int rows, int cols);

    // Create WE mesh from heightmap data
    bool createTerrainMesh(const std::string& texturePath);
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_TERRAINNODE_HPP
