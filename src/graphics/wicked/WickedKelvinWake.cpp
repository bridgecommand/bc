/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedKelvinWake.hpp"
#include "WickedEngine.h"
#include <cmath>
#include <algorithm>
#include <iostream>

using namespace wi::ecs;
using namespace wi::scene;

namespace bc { namespace graphics { namespace wicked {

WickedKelvinWake::WickedKelvinWake() = default;

WickedKelvinWake::~WickedKelvinWake() {
    shutdown();
}

void WickedKelvinWake::init(Scene* scene, const WakeParams& params) {
    weScene = scene;
    params_ = params;
}

WickedKelvinWake::ShipWake* WickedKelvinWake::findWake(int shipId) {
    for (auto& w : wakes) {
        if (w.shipId == shipId) return &w;
    }
    return nullptr;
}

WickedKelvinWake::ShipWake& WickedKelvinWake::getOrCreateWake(int shipId) {
    auto* existing = findWake(shipId);
    if (existing) return *existing;

    wakes.push_back({});
    auto& wake = wakes.back();
    wake.shipId = shipId;
    return wake;
}

void WickedKelvinWake::update(int shipId, const Vec3& position, float heading,
                                float speed, float dt) {
    if (!weScene || !visible_) return;

    auto& wake = getOrCreateWake(shipId);

    // Age existing trail points
    for (auto& p : wake.trail) {
        p.age += dt;
    }

    // Remove old trail points beyond max wake length
    float maxAge = params_.maxLength / std::max(speed, params_.speedThreshold);
    wake.trail.erase(
        std::remove_if(wake.trail.begin(), wake.trail.end(),
            [maxAge](const TrailPoint& p) { return p.age > maxAge; }),
        wake.trail.end()
    );

    // Add new trail point if ship is moving fast enough
    if (speed > params_.speedThreshold) {
        // Only add if moved far enough from last point (prevents clustering)
        float minSpacing = params_.maxLength / params_.numSegments;
        bool addPoint = wake.trail.empty();
        if (!addPoint) {
            const auto& last = wake.trail.front();
            float dx = position.x - last.position.x;
            float dz = position.z - last.position.z;
            addPoint = (dx * dx + dz * dz) > (minSpacing * minSpacing);
        }

        if (addPoint) {
            TrailPoint p;
            p.position = position;
            p.heading = heading;
            p.speed = speed;
            p.age = 0.0f;
            wake.trail.insert(wake.trail.begin(), p); // newest first

            // Cap trail length
            if ((int)wake.trail.size() > params_.numSegments) {
                wake.trail.resize(params_.numSegments);
            }

            wake.dirty = true;
        }
    }

    // Rebuild mesh if trail changed
    if (wake.dirty && wake.trail.size() >= 2) {
        rebuildWakeMesh(wake);
        wake.dirty = false;
    }
}

void WickedKelvinWake::rebuildWakeMesh(ShipWake& wake) {
    if (!weScene) return;

    // Remove old entities
    removeWakeEntities(wake);

    if (wake.trail.size() < 2) return;

    // Create new mesh entity
    wake.objectEntity = weScene->Entity_CreateObject("BC_Wake_" + std::to_string(wake.shipId));
    wake.meshEntity = weScene->Entity_CreateMesh("BC_Wake_mesh_" + std::to_string(wake.shipId));
    weScene->Component_Attach(wake.meshEntity, wake.objectEntity);

    auto* object = weScene->objects.GetComponent(wake.objectEntity);
    auto* mesh = weScene->meshes.GetComponent(wake.meshEntity);
    if (!object || !mesh) return;

    object->meshID = wake.meshEntity;

    // Create semi-transparent wake material
    wake.materialEntity = weScene->Entity_CreateMaterial("BC_WakeMat_" + std::to_string(wake.shipId));
    weScene->Component_Attach(wake.materialEntity, wake.objectEntity);
    auto* material = weScene->materials.GetComponent(wake.materialEntity);
    if (material) {
        material->baseColor = DirectX::XMFLOAT4(0.9f, 0.95f, 1.0f, 0.6f);
        material->SetCastShadow(false);
        material->userBlendMode = wi::enums::BLENDMODE_ALPHA;
        material->CreateRenderData();
    }

    mesh->subsets.push_back(MeshComponent::MeshSubset());
    mesh->subsets.back().materialID = wake.materialEntity;
    mesh->subsets.back().indexOffset = 0;

    float kelvinAngle = KELVIN_HALF_ANGLE * DEG_TO_RAD;
    int numPoints = static_cast<int>(wake.trail.size());
    float maxAge = 0.0f;
    for (const auto& p : wake.trail) {
        if (p.age > maxAge) maxAge = p.age;
    }
    if (maxAge < 0.001f) maxAge = 1.0f;

    // Build wake geometry: for each trail point, create a cross-section
    // The wake widens with Kelvin angle from the ship
    for (int i = 0; i < numPoints; i++) {
        const auto& pt = wake.trail[i];

        // Wake width expands with distance behind ship
        float distBehind = pt.age * pt.speed;
        float halfWidth = distBehind * std::tan(kelvinAngle);
        halfWidth = std::min(halfWidth, params_.width * 0.5f);

        // Fade out with age
        float fade = 1.0f - (pt.age / maxAge);
        fade = fade * fade; // quadratic fade

        // Cross direction (perpendicular to ship heading)
        float crossX = std::cos(pt.heading);
        float crossZ = -std::sin(pt.heading);

        // Left vertex
        DirectX::XMFLOAT3 posL(
            pt.position.x - crossX * halfWidth,
            pt.position.y + 0.05f, // slightly above water to prevent z-fight
            pt.position.z - crossZ * halfWidth
        );

        // Center vertex (for foam strip)
        DirectX::XMFLOAT3 posC(
            pt.position.x,
            pt.position.y + 0.08f,
            pt.position.z
        );

        // Right vertex
        DirectX::XMFLOAT3 posR(
            pt.position.x + crossX * halfWidth,
            pt.position.y + 0.05f,
            pt.position.z + crossZ * halfWidth
        );

        DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
        float u = static_cast<float>(i) / (numPoints - 1);

        // 3 vertices per cross-section: left, center, right
        mesh->vertex_positions.push_back(posL);
        mesh->vertex_normals.push_back(up);
        mesh->vertex_uvset_0.push_back(DirectX::XMFLOAT2(0.0f, u));

        mesh->vertex_positions.push_back(posC);
        mesh->vertex_normals.push_back(up);
        mesh->vertex_uvset_0.push_back(DirectX::XMFLOAT2(0.5f, u));

        mesh->vertex_positions.push_back(posR);
        mesh->vertex_normals.push_back(up);
        mesh->vertex_uvset_0.push_back(DirectX::XMFLOAT2(1.0f, u));
    }

    // Generate triangle indices between cross-sections
    for (int i = 0; i < numPoints - 1; i++) {
        uint32_t base = i * 3;
        uint32_t next = (i + 1) * 3;

        // Left-center quad (two triangles)
        mesh->indices.push_back(base + 0);
        mesh->indices.push_back(next + 0);
        mesh->indices.push_back(base + 1);

        mesh->indices.push_back(base + 1);
        mesh->indices.push_back(next + 0);
        mesh->indices.push_back(next + 1);

        // Center-right quad (two triangles)
        mesh->indices.push_back(base + 1);
        mesh->indices.push_back(next + 1);
        mesh->indices.push_back(base + 2);

        mesh->indices.push_back(base + 2);
        mesh->indices.push_back(next + 1);
        mesh->indices.push_back(next + 2);
    }

    mesh->subsets.back().indexCount = static_cast<uint32_t>(mesh->indices.size());
    mesh->CreateRenderData();
}

void WickedKelvinWake::removeWakeEntities(ShipWake& wake) {
    if (!weScene) return;
    if (wake.objectEntity != INVALID_ENTITY) {
        weScene->Entity_Remove(wake.objectEntity);
        wake.objectEntity = INVALID_ENTITY;
        wake.meshEntity = INVALID_ENTITY;
        wake.materialEntity = INVALID_ENTITY;
    }
}

void WickedKelvinWake::removeWake(int shipId) {
    for (auto it = wakes.begin(); it != wakes.end(); ++it) {
        if (it->shipId == shipId) {
            removeWakeEntities(*it);
            wakes.erase(it);
            return;
        }
    }
}

void WickedKelvinWake::setVisible(bool visible) {
    visible_ = visible;
    if (!weScene) return;
    for (auto& wake : wakes) {
        if (wake.objectEntity != INVALID_ENTITY) {
            auto* obj = weScene->objects.GetComponent(wake.objectEntity);
            if (obj) obj->SetRenderable(visible);
        }
    }
}

void WickedKelvinWake::shutdown() {
    if (weScene) {
        for (auto& wake : wakes) {
            removeWakeEntities(wake);
        }
    }
    wakes.clear();
    weScene = nullptr;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
