/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedWater.hpp"
#include "WickedEngine.h"
#include <cmath>
#include <iostream>

using namespace DirectX;

namespace bc { namespace graphics { namespace wicked {

WickedWater::WickedWater() = default;
WickedWater::~WickedWater() = default;

void WickedWater::load(wi::scene::Scene* scene, float weather, int /*segments*/) {
    weScene = scene;
    currentWeather_ = weather;

    if (!weScene) return;

    // Enable ocean in the scene's weather component
    weScene->weather.SetOceanEnabled(true);

    // Configure initial ocean parameters
    auto& op = weScene->weather.oceanParameters;

    // WE defaults: patch_length=50, wave_amplitude=1000, wind_speed=600
    // BC uses 100m tile width - match it
    op.patch_length = 100.0f;

    // dmap_dim must be power of 2 (WE default 512)
    op.dmap_dim = 512;

    // Time scale controls simulation speed
    op.time_scale = 0.3f;

    // Choppy wave scale (longitudinal displacement)
    op.choppy_scale = 1.3f;

    // Surface detail controls mesh vertex count
    // 4 = default (~230K verts), reduce for lower-spec hardware
    op.surfaceDetail = 4;

    // Water color matching BC's existing aesthetic
    // BC pixel shader: vec3(0.18, 0.29, 0.31)
    op.waterColor = XMFLOAT4(0.18f, 0.29f, 0.31f, 0.6f);
    op.extinctionColor = XMFLOAT4(0.0f, 0.6f, 0.8f, 1.0f);

    // Water height will be set by tide in update()
    op.waterHeight = 0.0f;

    // Set initial wind from weather parameter
    // Beaufort scale approximate: weather 0=calm, 4=moderate, 8=gale
    float approxWindKts = weather * 5.0f; // rough mapping
    float windMps = approxWindKts * 0.5144f;
    float Hs = 0.0246f * windMps * windMps;
    if (Hs < 0.01f) Hs = 0.01f;
    if (Hs > 15.0f) Hs = 15.0f;

    // WE wave_amplitude is in internal units (scaled by 1e-7 inside WE)
    // Empirically: wave_amplitude ~1000 gives moderate seas at wind_speed ~600
    // Scale proportionally to Hs²
    op.wave_amplitude = 1000.0f * (Hs * Hs) / (0.5f * 0.5f);
    if (op.wave_amplitude < 10.0f) op.wave_amplitude = 10.0f;
    if (op.wave_amplitude > 50000.0f) op.wave_amplitude = 50000.0f;

    // Default wind direction (north)
    op.wind_dir = XMFLOAT2(0.0f, 1.0f);
    op.wind_speed = std::max(100.0f, windMps * 100.0f); // WE uses cm/s scale
    op.wind_dependency = 0.07f;

    // Create the ocean with these parameters
    weScene->ocean.Create(op);

    std::cout << "WickedWater: Initialized (patch=" << op.patch_length
              << "m, dmap=" << op.dmap_dim
              << ", detail=" << op.surfaceDetail << ")" << std::endl;
}

void WickedWater::update(float tideHeight, const Vec3& /*viewPosition*/,
                          int /*lightLevel*/, float weather,
                          float windSpeedKts, float windDirectionDeg) {
    if (!weScene) return;

    tideHeight_ = tideHeight;
    currentWeather_ = weather;

    auto& op = weScene->weather.oceanParameters;

    // Update water height for tide
    op.waterHeight = tideHeight;

    // Convert wind speed from knots to m/s
    float windMps = windSpeedKts * 0.5144f;

    // Significant wave height from fully-developed sea (Pierson-Moskowitz)
    float Hs = 0.0246f * windMps * windMps;
    if (Hs < 0.01f) Hs = 0.01f;
    if (Hs > 15.0f) Hs = 15.0f;

    // Map Hs to WE wave_amplitude
    // Calibration: Hs=0.5m → amplitude=1000 (moderate seas)
    op.wave_amplitude = 1000.0f * (Hs * Hs) / (0.5f * 0.5f);
    if (op.wave_amplitude < 10.0f) op.wave_amplitude = 10.0f;
    if (op.wave_amplitude > 50000.0f) op.wave_amplitude = 50000.0f;

    // Wind direction: BC uses meteorological convention (where wind blows FROM)
    // Waves propagate WITH the wind, so add 180 degrees
    float windRad = (windDirectionDeg + 180.0f) * 3.14159265f / 180.0f;
    float dirX = std::sin(windRad);
    float dirZ = std::cos(windRad);
    // Minimum wind to avoid zero-vector
    if (windMps < 0.5f) {
        dirX = 0.0f;
        dirZ = 1.0f;
    }
    op.wind_dir = XMFLOAT2(dirX, dirZ);

    // WE wind_speed is in cm/s scale (empirically)
    op.wind_speed = std::max(100.0f, windMps * 100.0f);

    // Choppy scale: increase with sea state
    op.choppy_scale = 1.0f + (Hs / 5.0f) * 0.5f;
    if (op.choppy_scale > 2.0f) op.choppy_scale = 2.0f;

    // Recreate ocean if parameters changed significantly
    // WE's Ocean::Create() re-initializes the spectrum
    // Only recreate when wind changes significantly to avoid visual pops
    static float lastWindSpeed = -1.0f;
    float windDelta = std::abs(windMps - lastWindSpeed);
    if (lastWindSpeed < 0 || windDelta > 1.0f) {
        weScene->ocean.Create(op);
        lastWindSpeed = windMps;
    }
}

float WickedWater::getWaveHeight(float worldX, float worldZ) const {
    if (!weScene || !weScene->weather.IsOceanEnabled()) return tideHeight_;

    // Use WE's built-in CPU readback of displacement map
    XMFLOAT3 queryPos(worldX, 0.0f, worldZ);
    XMFLOAT3 displaced = weScene->GetOceanPosAt(queryPos);
    return displaced.y;
}

Vec2 WickedWater::getLocalNormals(float worldX, float worldZ) const {
    if (!weScene || !weScene->weather.IsOceanEnabled()) return Vec2(0.0f, 0.0f);

    // Approximate surface normal via finite differences of displaced height
    float hCenter = getWaveHeight(worldX, worldZ);
    float hRight = getWaveHeight(worldX + NORMAL_SAMPLE_OFFSET, worldZ);
    float hForward = getWaveHeight(worldX, worldZ + NORMAL_SAMPLE_OFFSET);

    // Normal = normalize(cross(tangentX, tangentZ))
    // tangentX = (NORMAL_SAMPLE_OFFSET, hRight - hCenter, 0)
    // tangentZ = (0, hForward - hCenter, NORMAL_SAMPLE_OFFSET)
    // cross = (-(hRight-hCenter)*offset, offset*offset, -(hForward-hCenter)*offset) -- not needed fully
    // Simplified: slope_x = (hRight - hCenter) / offset, slope_z = (hForward - hCenter) / offset
    float slopeX = (hRight - hCenter) / NORMAL_SAMPLE_OFFSET;
    float slopeZ = (hForward - hCenter) / NORMAL_SAMPLE_OFFSET;

    // Return XZ slopes (matching BC's getLocalNormals convention)
    return Vec2(slopeX, slopeZ);
}

Vec3 WickedWater::getPosition() const {
    return Vec3(0.0f, tideHeight_, 0.0f);
}

void WickedWater::setVisible(bool visible) {
    visible_ = visible;
    if (weScene) {
        weScene->weather.SetOceanEnabled(visible);
    }
}

bool WickedWater::isValid() const {
    return weScene && weScene->ocean.IsValid();
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
