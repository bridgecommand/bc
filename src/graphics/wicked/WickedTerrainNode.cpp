/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedTerrainNode.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace wi::graphics;
using namespace wi::scene;
using namespace wi::ecs;

namespace bc { namespace graphics { namespace wicked {

static constexpr float DEG_TO_RAD = 3.14159265358979f / 180.0f;
static constexpr float EARTH_RADIUS = 6371000.0f; // meters

WickedTerrainNode::WickedTerrainNode(Scene* scene)
    : weScene(scene) {}

WickedTerrainNode::~WickedTerrainNode() {
    remove();
}

bool WickedTerrainNode::loadFromConfig(const TerrainTileConfig& config,
                                         float refLongitude, float refLatitude) {
    // Calculate world-space dimensions from geo coordinates
    float cosLat = std::cos(config.latitude * DEG_TO_RAD);
    worldWidth_ = config.lonExtent * DEG_TO_RAD * EARTH_RADIUS * cosLat;
    worldDepth_ = config.latExtent * DEG_TO_RAD * EARTH_RADIUS;
    maxHeight_ = config.maxHeight;
    seaMaxDepth_ = config.seaMaxDepth;

    // Calculate position offset relative to reference point
    float offsetX = (config.longitude - refLongitude) * DEG_TO_RAD * EARTH_RADIUS * cosLat;
    float offsetZ = (config.latitude - refLatitude) * DEG_TO_RAD * EARTH_RADIUS;
    position_ = {offsetX, -seaMaxDepth_, offsetZ};

    // Determine heightmap format and load
    std::string ext;
    {
        size_t dotPos = config.heightmapPath.find_last_of('.');
        if (dotPos != std::string::npos) {
            ext = config.heightmapPath.substr(dotPos);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }
    }

    if (ext == ".f32") {
        if (!loadHeightmapF32(config.heightmapPath, config.heightmapRows, config.heightmapCols))
            return false;
    } else {
        // PNG or other image format
        if (!loadHeightmapPNG(config.heightmapPath, config))
            return false;
    }

    return createTerrainMesh(config.texturePath);
}

bool WickedTerrainNode::loadFromHeightData(const std::vector<std::vector<float>>& heights,
                                             float maxHeight, float seaMaxDepth,
                                             float worldWidth, float worldDepth,
                                             const std::string& texturePath) {
    heightData_ = heights;
    maxHeight_ = maxHeight;
    seaMaxDepth_ = seaMaxDepth;
    worldWidth_ = worldWidth;
    worldDepth_ = worldDepth;
    return createTerrainMesh(texturePath);
}

bool WickedTerrainNode::loadHeightmapPNG(const std::string& path,
                                           const TerrainTileConfig& config) {
    // Load via WE's resource manager
    wi::vector<uint8_t> filedata;
    if (!wi::helper::FileRead(path, filedata)) {
        std::cerr << "WickedTerrainNode: Failed to read heightmap: " << path << std::endl;
        return false;
    }

    // Decode PNG using WE's image loading
    int width = 0, height = 0;
    wi::graphics::TextureDesc texDesc;
    wi::vector<uint8_t> decoded;

    // Use WE's texture loading to decode the image
    wi::Resource resource = wi::resourcemanager::Load(path);
    if (!resource.IsValid()) {
        std::cerr << "WickedTerrainNode: Failed to decode heightmap: " << path << std::endl;
        return false;
    }

    // For now, create a placeholder heightmap from file metadata
    // Full implementation would decode PNG pixels and build height grid
    // This is a framework that will be completed when rendering is tested

    // Create empty heightmap with correct dimensions
    int gridSize = 256; // Default resolution
    if (config.heightmapRows > 0) gridSize = config.heightmapRows;

    heightData_.resize(gridSize, std::vector<float>(gridSize, 0.0f));

    std::cout << "WickedTerrainNode: Loaded heightmap placeholder ("
              << gridSize << "x" << gridSize << ") from " << path << std::endl;
    return true;
}

bool WickedTerrainNode::loadHeightmapF32(const std::string& path, int rows, int cols) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "WickedTerrainNode: Failed to open F32 heightmap: " << path << std::endl;
        return false;
    }

    if (rows <= 0 || cols <= 0) {
        // Try to infer dimensions from file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0);
        size_t numFloats = fileSize / sizeof(float);
        int dim = static_cast<int>(std::sqrt(static_cast<double>(numFloats)));
        if (dim * dim != (int)numFloats) {
            std::cerr << "WickedTerrainNode: Cannot infer F32 heightmap dimensions" << std::endl;
            return false;
        }
        rows = cols = dim;
    }

    heightData_.resize(rows, std::vector<float>(cols));
    for (int r = 0; r < rows; r++) {
        file.read(reinterpret_cast<char*>(heightData_[r].data()), cols * sizeof(float));
    }

    std::cout << "WickedTerrainNode: Loaded F32 heightmap (" << rows << "x" << cols
              << ") from " << path << std::endl;
    return true;
}

bool WickedTerrainNode::createTerrainMesh(const std::string& texturePath) {
    if (heightData_.empty()) return false;

    int rows = static_cast<int>(heightData_.size());
    int cols = static_cast<int>(heightData_[0].size());

    // Subsample large heightmaps for performance (max 512x512 mesh)
    int step = 1;
    while (rows / step > 512 || cols / step > 512) step *= 2;
    int meshRows = rows / step;
    int meshCols = cols / step;

    float cellWidth = worldWidth_ / meshCols;
    float cellDepth = worldDepth_ / meshRows;

    // Create mesh entity
    rootEntity_ = weScene->Entity_CreateObject("BC_Terrain");
    Entity meshEntity = weScene->Entity_CreateMesh("BC_Terrain_mesh");
    weScene->Component_Attach(meshEntity, rootEntity_);

    auto* object = weScene->objects.GetComponent(rootEntity_);
    auto* mesh = weScene->meshes.GetComponent(meshEntity);
    if (!object || !mesh) return false;

    object->meshID = meshEntity;

    // Create material
    Entity matEntity = weScene->Entity_CreateMaterial("BC_TerrainMat");
    weScene->Component_Attach(matEntity, rootEntity_);
    auto* material = weScene->materials.GetComponent(matEntity);
    if (material && !texturePath.empty()) {
        material->textures[MaterialComponent::BASECOLORMAP].name = texturePath;
        material->CreateRenderData();
    }

    // Build terrain mesh vertices and indices
    mesh->subsets.push_back(MeshComponent::MeshSubset());
    mesh->subsets.back().materialID = matEntity;
    mesh->subsets.back().indexOffset = 0;

    for (int r = 0; r < meshRows; r++) {
        for (int c = 0; c < meshCols; c++) {
            int srcR = std::min(r * step, rows - 1);
            int srcC = std::min(c * step, cols - 1);
            float height = heightData_[srcR][srcC];

            DirectX::XMFLOAT3 pos(
                c * cellWidth,
                height,
                r * cellDepth
            );

            // Compute normal from neighboring heights
            float hL = (c > 0) ? heightData_[srcR][std::max(0, srcC - step)] : height;
            float hR = (c < meshCols - 1) ? heightData_[srcR][std::min(cols - 1, srcC + step)] : height;
            float hD = (r > 0) ? heightData_[std::max(0, srcR - step)][srcC] : height;
            float hU = (r < meshRows - 1) ? heightData_[std::min(rows - 1, srcR + step)][srcC] : height;

            DirectX::XMFLOAT3 nor(
                (hL - hR) / (2.0f * cellWidth),
                1.0f,
                (hD - hU) / (2.0f * cellDepth)
            );
            // Normalize
            float len = std::sqrt(nor.x * nor.x + nor.y * nor.y + nor.z * nor.z);
            if (len > 0) { nor.x /= len; nor.y /= len; nor.z /= len; }

            DirectX::XMFLOAT2 uv(
                static_cast<float>(c) / (meshCols - 1),
                static_cast<float>(r) / (meshRows - 1)
            );

            // WE is left-handed, flip Z for coordinate consistency
            pos.z *= -1;
            nor.z *= -1;

            mesh->vertex_positions.push_back(pos);
            mesh->vertex_normals.push_back(nor);
            mesh->vertex_uvset_0.push_back(uv);
        }
    }

    // Generate triangle indices
    for (int r = 0; r < meshRows - 1; r++) {
        for (int c = 0; c < meshCols - 1; c++) {
            uint32_t tl = r * meshCols + c;
            uint32_t tr = tl + 1;
            uint32_t bl = (r + 1) * meshCols + c;
            uint32_t br = bl + 1;

            // Two triangles per quad
            mesh->indices.push_back(tl);
            mesh->indices.push_back(bl);
            mesh->indices.push_back(tr);

            mesh->indices.push_back(tr);
            mesh->indices.push_back(bl);
            mesh->indices.push_back(br);
        }
    }

    mesh->subsets.back().indexCount = static_cast<uint32_t>(mesh->indices.size());
    mesh->CreateRenderData();

    // Position the terrain
    auto* transform = weScene->transforms.GetComponent(rootEntity_);
    if (transform) {
        transform->Translate({position_.x, position_.y, -position_.z}); // flip Z
        transform->UpdateTransform();
    }

    std::cout << "WickedTerrainNode: Created terrain mesh ("
              << meshCols << "x" << meshRows << " vertices, "
              << mesh->indices.size() / 3 << " triangles)" << std::endl;
    return true;
}

float WickedTerrainNode::getHeightAt(float worldX, float worldZ) const {
    if (heightData_.empty() || worldWidth_ <= 0 || worldDepth_ <= 0) return 0;

    int rows = static_cast<int>(heightData_.size());
    int cols = static_cast<int>(heightData_[0].size());

    // Convert world position to grid coordinates
    float localX = worldX - position_.x;
    float localZ = worldZ - position_.z;
    float gridX = (localX / worldWidth_) * (cols - 1);
    float gridZ = (localZ / worldDepth_) * (rows - 1);

    // Clamp to grid bounds
    gridX = std::max(0.0f, std::min(gridX, static_cast<float>(cols - 2)));
    gridZ = std::max(0.0f, std::min(gridZ, static_cast<float>(rows - 2)));

    // Bilinear interpolation
    int ix = static_cast<int>(gridX);
    int iz = static_cast<int>(gridZ);
    float fx = gridX - ix;
    float fz = gridZ - iz;

    float h00 = heightData_[iz][ix];
    float h10 = heightData_[iz][ix + 1];
    float h01 = heightData_[iz + 1][ix];
    float h11 = heightData_[iz + 1][ix + 1];

    return h00 * (1 - fx) * (1 - fz) +
           h10 * fx * (1 - fz) +
           h01 * (1 - fx) * fz +
           h11 * fx * fz;
}

void WickedTerrainNode::setPosition(const Vec3& pos) {
    position_ = pos;
    if (rootEntity_ != INVALID_ENTITY) {
        auto* transform = weScene->transforms.GetComponent(rootEntity_);
        if (transform) {
            transform->ClearTransform();
            transform->Translate({pos.x, pos.y, -pos.z});
            transform->UpdateTransform();
        }
    }
}

Vec3 WickedTerrainNode::getPosition() const {
    return position_;
}

void WickedTerrainNode::setRotation(const Vec3& rot) {
    // Terrain rotation is rarely used but supported
    if (rootEntity_ != INVALID_ENTITY) {
        auto* transform = weScene->transforms.GetComponent(rootEntity_);
        if (transform) {
            float degToRad = 3.14159265358979f / 180.0f;
            transform->ClearTransform();
            transform->Translate({position_.x, position_.y, -position_.z});
            transform->RotateRollPitchYaw({rot.x * degToRad, rot.y * degToRad, rot.z * degToRad});
            transform->UpdateTransform();
        }
    }
}

void WickedTerrainNode::setScale(const Vec3& scale) {
    scale_ = scale;
    if (rootEntity_ != INVALID_ENTITY) {
        auto* transform = weScene->transforms.GetComponent(rootEntity_);
        if (transform) {
            transform->scale_local = {scale.x, scale.y, scale.z};
            transform->SetDirty();
        }
    }
}

void WickedTerrainNode::setVisible(bool visible) {
    visible_ = visible;
    if (rootEntity_ != INVALID_ENTITY) {
        auto* object = weScene->objects.GetComponent(rootEntity_);
        if (object) object->SetRenderable(visible);
    }
}

void WickedTerrainNode::remove() {
    if (rootEntity_ != INVALID_ENTITY && weScene) {
        weScene->Entity_Remove(rootEntity_);
        rootEntity_ = INVALID_ENTITY;
    }
    heightData_.clear();
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
