/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedMultiCascadeOcean.hpp"
#include "WickedEngine.h"
#include <cmath>
#include <iostream>

using namespace DirectX;

namespace bc { namespace graphics { namespace wicked {

// Default cascade parameters:
// Cascade 0: Large swells (far distance, low resolution)
// Cascade 1: Wind waves (medium distance, high resolution) - primary cascade
// Cascade 2: Ripples/capillary (near distance, low resolution)
const WickedMultiCascadeOcean::CascadeConfig
WickedMultiCascadeOcean::DEFAULT_CONFIGS[NUM_CASCADES] = {
    // patchLength, fftRes, waveAmp, choppy, minDist, maxDist
    { 2000.0f, 256, 2000.0f, 0.8f,  200.0f, 5000.0f },  // Far swells
    {  100.0f, 512, 1000.0f, 1.3f,    0.0f, 1000.0f },  // Primary wind waves
    {   20.0f, 256,  200.0f, 1.5f,    0.0f,  200.0f },  // Near ripples
};

WickedMultiCascadeOcean::WickedMultiCascadeOcean() {
    for (int i = 0; i < NUM_CASCADES; i++) {
        configs[i] = DEFAULT_CONFIGS[i];
    }
}

WickedMultiCascadeOcean::~WickedMultiCascadeOcean() {
    shutdown();
}

void WickedMultiCascadeOcean::setCascadeConfig(int index, const CascadeConfig& config) {
    if (index < 0 || index >= NUM_CASCADES) return;
    configs[index] = config;
}

void WickedMultiCascadeOcean::init(wi::scene::Scene* scene,
                                     float windSpeedMps, float windDirRad) {
    weScene = scene;
    if (!weScene) return;

    // Enable ocean in the weather component
    weScene->weather.SetOceanEnabled(true);

    // The primary cascade (index 1) drives WE's built-in scene ocean.
    // This ensures the standard rendering pipeline works out of the box.
    // Cascades 0 and 2 are auxiliary and will provide additional textures
    // for the multi-cascade surface shader (Phase 3 shader work).

    // Configure the scene's primary ocean (cascade 1 = medium/wind waves)
    auto& op = weScene->weather.oceanParameters;
    op.patch_length = configs[1].patchLength;
    op.dmap_dim = configs[1].fftResolution;
    op.wave_amplitude = configs[1].waveAmplitude;
    op.choppy_scale = configs[1].choppyScale;
    op.time_scale = 0.3f;
    op.waterHeight = waterHeight_;
    op.waterColor = XMFLOAT4(0.18f, 0.29f, 0.31f, 0.6f);
    op.extinctionColor = XMFLOAT4(0.0f, 0.6f, 0.8f, 1.0f);
    op.surfaceDetail = 4;
    op.surfaceDisplacementTolerance = 2.0f;

    // Set wind direction
    float dirX = std::sin(windDirRad);
    float dirZ = std::cos(windDirRad);
    op.wind_dir = XMFLOAT2(dirX, dirZ);
    op.wind_speed = std::max(100.0f, windSpeedMps * 100.0f);
    op.wind_dependency = 0.07f;

    // Create the primary ocean
    weScene->ocean.Create(op);
    cascadeData[1].initialized = true;

    lastWindSpeed_ = windSpeedMps;
    lastWindDir_ = windDirRad;

    // Note: Auxiliary cascades (0 and 2) are created as separate wi::Ocean
    // instances. They run their own FFT but share the scene's rendering.
    // For now, the framework creates the primary cascade and provides
    // the infrastructure for adding cascades 0 and 2 when the custom
    // multi-cascade shaders are integrated.

    // Create auxiliary cascades
    createCascade(0, windSpeedMps, windDirRad);
    createCascade(2, windSpeedMps, windDirRad);

    std::cout << "WickedMultiCascadeOcean: Initialized " << NUM_CASCADES << " cascades:"
              << std::endl;
    for (int i = 0; i < NUM_CASCADES; i++) {
        std::cout << "  Cascade " << i << ": patch=" << configs[i].patchLength
                  << "m, fft=" << configs[i].fftResolution
                  << ", amp=" << configs[i].waveAmplitude
                  << ", range=[" << configs[i].minDistance
                  << "-" << configs[i].maxDistance << "m]"
                  << std::endl;
    }
}

void WickedMultiCascadeOcean::createCascade(int index, float windSpeedMps, float windDirRad) {
    // Auxiliary cascades are managed as separate wi::Ocean objects.
    // Their displacement/gradient textures will be bound as extra textures
    // in the custom multi-cascade surface shader.
    //
    // For now we mark them as initialized to track state.
    // The actual wi::Ocean instances for cascades 0 and 2 will be created
    // when we add the custom shader pipeline that can consume their outputs.
    //
    // The primary cascade (1) uses the scene's built-in ocean,
    // so the standard rendering pipeline works immediately.
    cascadeData[index].initialized = true;
}

void WickedMultiCascadeOcean::updateWind(float windSpeedMps, float windDirRad) {
    if (!weScene) return;

    // Only recreate spectra if wind changed significantly
    float speedDelta = std::abs(windSpeedMps - lastWindSpeed_);
    float dirDelta = std::abs(windDirRad - lastWindDir_);

    if (speedDelta < 0.5f && dirDelta < 0.05f) return;

    lastWindSpeed_ = windSpeedMps;
    lastWindDir_ = windDirRad;

    // Update primary cascade via scene weather
    auto& op = weScene->weather.oceanParameters;
    float dirX = std::sin(windDirRad);
    float dirZ = std::cos(windDirRad);
    op.wind_dir = XMFLOAT2(dirX, dirZ);
    op.wind_speed = std::max(100.0f, windSpeedMps * 100.0f);

    // Scale wave amplitude with wind speed (Pierson-Moskowitz relationship)
    float Hs = 0.0246f * windSpeedMps * windSpeedMps;
    if (Hs < 0.01f) Hs = 0.01f;
    if (Hs > 15.0f) Hs = 15.0f;

    // Scale each cascade's amplitude proportionally
    float baseAmp = configs[1].waveAmplitude;
    float ampScale = (Hs * Hs) / (0.5f * 0.5f); // Normalized to Hs=0.5m baseline
    op.wave_amplitude = baseAmp * ampScale;
    if (op.wave_amplitude < 10.0f) op.wave_amplitude = 10.0f;
    if (op.wave_amplitude > 50000.0f) op.wave_amplitude = 50000.0f;

    op.choppy_scale = configs[1].choppyScale * (1.0f + Hs / 10.0f);
    if (op.choppy_scale > 2.5f) op.choppy_scale = 2.5f;

    // Recreate the primary ocean with new spectrum
    weScene->ocean.Create(op);
}

void WickedMultiCascadeOcean::setWaterHeight(float height) {
    waterHeight_ = height;
    if (weScene) {
        weScene->weather.oceanParameters.waterHeight = height;
    }
}

float WickedMultiCascadeOcean::getWaveHeight(float worldX, float worldZ) const {
    if (!weScene || !weScene->weather.IsOceanEnabled()) return waterHeight_;

    // Use WE's built-in readback for the primary cascade
    // When auxiliary cascades are fully implemented, their displacements
    // would be added here for a blended result
    XMFLOAT3 queryPos(worldX, 0.0f, worldZ);
    XMFLOAT3 displaced = weScene->GetOceanPosAt(queryPos);
    return displaced.y;
}

Vec2 WickedMultiCascadeOcean::getLocalNormals(float worldX, float worldZ) const {
    if (!weScene || !weScene->weather.IsOceanEnabled()) return Vec2(0.0f, 0.0f);

    float hCenter = getWaveHeight(worldX, worldZ);
    float hRight = getWaveHeight(worldX + NORMAL_SAMPLE_OFFSET, worldZ);
    float hForward = getWaveHeight(worldX, worldZ + NORMAL_SAMPLE_OFFSET);

    float slopeX = (hRight - hCenter) / NORMAL_SAMPLE_OFFSET;
    float slopeZ = (hForward - hCenter) / NORMAL_SAMPLE_OFFSET;

    return Vec2(slopeX, slopeZ);
}

const void* WickedMultiCascadeOcean::getDisplacementMap(int cascadeIndex) const {
    if (cascadeIndex < 0 || cascadeIndex >= NUM_CASCADES) return nullptr;

    // Primary cascade uses the scene ocean's displacement map
    if (cascadeIndex == 1 && weScene) {
        return weScene->ocean.getDisplacementMap();
    }

    // Auxiliary cascades would return their own displacement maps
    // when fully implemented with separate wi::Ocean instances
    return nullptr;
}

const void* WickedMultiCascadeOcean::getGradientMap(int cascadeIndex) const {
    if (cascadeIndex < 0 || cascadeIndex >= NUM_CASCADES) return nullptr;

    if (cascadeIndex == 1 && weScene) {
        return weScene->ocean.getGradientMap();
    }

    return nullptr;
}

const WickedMultiCascadeOcean::CascadeConfig&
WickedMultiCascadeOcean::getCascadeConfig(int index) const {
    static const CascadeConfig empty = {};
    if (index < 0 || index >= NUM_CASCADES) return empty;
    return configs[index];
}

bool WickedMultiCascadeOcean::isValid() const {
    return weScene && weScene->ocean.IsValid() && cascadeData[1].initialized;
}

void WickedMultiCascadeOcean::shutdown() {
    for (int i = 0; i < NUM_CASCADES; i++) {
        cascadeData[i].initialized = false;
    }
    weScene = nullptr;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
