/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_WATER_HPP
#define BC_GRAPHICS_WICKED_WATER_HPP

#ifdef WITH_WICKED_ENGINE

#include "../Types.hpp"

// Forward declarations
namespace wi { namespace scene { class Scene; } }

namespace bc { namespace graphics { namespace wicked {

/// Wraps Wicked Engine's built-in wi::Ocean for water rendering.
/// WE's ocean uses GPU-accelerated FFT (Phillips spectrum, 512x512)
/// with automatic mesh generation and CPU displacement readback.
///
/// This replaces the BC CPU FFT ocean (64x64) with significant
/// performance gains. JONSWAP spectrum and multi-cascade support
/// will be added in Phase 3.
class WickedWater {
public:
    WickedWater();
    ~WickedWater();

    /// Initialize ocean rendering on the given scene.
    /// @param scene WE scene (owns the WeatherComponent and Ocean)
    /// @param weather Initial sea state (0=calm, 12=hurricane)
    /// @param segments Unused (WE manages its own mesh resolution)
    void load(wi::scene::Scene* scene, float weather, int segments = 0);

    /// Update ocean parameters for current conditions.
    /// Maps BC wind/weather to WE ocean parameters.
    /// @param tideHeight Vertical offset for tide (meters)
    /// @param viewPosition Camera position for ocean centering
    /// @param lightLevel Ambient light (0-255)
    /// @param weather Sea state (0-12)
    /// @param windSpeedKts Wind speed in knots
    /// @param windDirectionDeg Meteorological wind direction (degrees, where wind blows FROM)
    void update(float tideHeight, const Vec3& viewPosition,
                int lightLevel, float weather,
                float windSpeedKts, float windDirectionDeg);

    /// Get wave height at a world position (includes tide + wave displacement).
    /// Uses WE's async CPU readback (2-3 frame latency, acceptable for physics).
    float getWaveHeight(float worldX, float worldZ) const;

    /// Get surface normal at a world position (for ship roll/pitch coupling).
    /// Approximated from displaced positions around the query point.
    Vec2 getLocalNormals(float worldX, float worldZ) const;

    /// Get current ocean surface base position.
    Vec3 getPosition() const;

    /// Show/hide ocean rendering.
    void setVisible(bool visible);

    /// Check if ocean is initialized and valid.
    bool isValid() const;

private:
    wi::scene::Scene* weScene = nullptr;
    float tideHeight_ = 0.0f;
    float currentWeather_ = 0.0f;
    bool visible_ = true;

    // Sampling offset for finite-difference normals (meters)
    static constexpr float NORMAL_SAMPLE_OFFSET = 1.0f;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_WATER_HPP
