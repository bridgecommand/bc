/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_MULTI_CASCADE_OCEAN_HPP
#define BC_GRAPHICS_WICKED_MULTI_CASCADE_OCEAN_HPP

#ifdef WITH_WICKED_ENGINE

#include "../Types.hpp"

// Forward declarations
namespace wi {
    class Ocean;
    namespace scene { class Scene; }
    namespace graphics { class CommandList; }
}

namespace bc { namespace graphics { namespace wicked {

/// Multi-cascade ocean rendering system using Wicked Engine's wi::Ocean.
///
/// Extends the single-cascade GPU FFT ocean to 3 cascades covering
/// different spatial scales:
///   - Cascade 0 (far):    500-2000m patch, 256x256 FFT - large swells
///   - Cascade 1 (medium): 50-200m patch, 512x512 FFT - wind waves
///   - Cascade 2 (near):   5-50m patch, 256x256 FFT - ripples/capillary
///
/// Each cascade runs an independent FFT. The displacement and gradient
/// textures from all 3 cascades are blended in the surface shader.
///
/// Phase 3-02 of the upgrade plan.
class WickedMultiCascadeOcean {
public:
    static constexpr int NUM_CASCADES = 3;

    enum class SpectrumType {
        Phillips,   // WE default - fully developed sea
        JONSWAP,    // Fetch-limited seas (more realistic)
        TMA         // Shallow water modification of JONSWAP
    };

    struct CascadeConfig {
        float patchLength;      // World-space size of one FFT tile (meters)
        int   fftResolution;    // Must be power of 2 (256 or 512)
        float waveAmplitude;    // Amplitude scaling for this cascade
        float choppyScale;      // Choppy wave multiplier
        float minDistance;      // Start blending at this camera distance
        float maxDistance;      // Fully blended at this camera distance
        SpectrumType spectrum = SpectrumType::JONSWAP;
        float fetchKm = 100.0f; // Fetch distance for JONSWAP/TMA (km)
        float depth = 50.0f;    // Water depth for TMA (meters)
    };

    WickedMultiCascadeOcean();
    ~WickedMultiCascadeOcean();

    /// Initialize with a scene and wind parameters.
    void init(wi::scene::Scene* scene, float windSpeedMps, float windDirRad);

    /// Set cascade configuration (call before or after init).
    void setCascadeConfig(int cascadeIndex, const CascadeConfig& config);

    /// Update wind parameters (recreates spectra if changed significantly).
    void updateWind(float windSpeedMps, float windDirRad);

    /// Update tide/water height.
    void setWaterHeight(float height);

    /// Get blended wave height at world position (sum of all cascades).
    float getWaveHeight(float worldX, float worldZ) const;

    /// Get blended surface normal at world position.
    Vec2 getLocalNormals(float worldX, float worldZ) const;

    /// Get displacement texture for a specific cascade (for shader binding).
    const void* getDisplacementMap(int cascadeIndex) const;

    /// Get gradient texture for a specific cascade (for shader binding).
    const void* getGradientMap(int cascadeIndex) const;

    /// Get cascade configuration.
    const CascadeConfig& getCascadeConfig(int index) const;

    /// Check if all cascades are initialized.
    bool isValid() const;

    /// Clean up all resources.
    void shutdown();

private:
    wi::scene::Scene* weScene = nullptr;

    // Default cascade configurations
    static const CascadeConfig DEFAULT_CONFIGS[NUM_CASCADES];

    CascadeConfig configs[NUM_CASCADES];
    float waterHeight_ = 0.0f;
    float lastWindSpeed_ = -1.0f;
    float lastWindDir_ = 0.0f;

    // We store cascade ocean instances as indices into the scene's weather
    // and use the scene's built-in ocean. For additional cascades we create
    // separate wi::Ocean instances.
    struct CascadeData {
        bool initialized = false;
    };
    CascadeData cascadeData[NUM_CASCADES];

    void createCascade(int index, float windSpeedMps, float windDirRad);

    static constexpr float NORMAL_SAMPLE_OFFSET = 1.0f;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_MULTI_CASCADE_OCEAN_HPP
