/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_KELVIN_WAKE_HPP
#define BC_GRAPHICS_WICKED_KELVIN_WAKE_HPP

#ifdef WITH_WICKED_ENGINE

#include "../Types.hpp"
#include <cstdint>
#include <vector>

// Forward declarations
namespace wi {
    namespace scene { class Scene; }
}

namespace bc { namespace graphics { namespace wicked {

/// Renders Kelvin wake patterns behind ships.
///
/// The Kelvin wake forms a V-shape at 19.47 degrees half-angle
/// (constant for all ship speeds in deep water). The wake is rendered
/// as a pair of textured quads extending behind the ship, with:
///   - Transverse waves (perpendicular to ship track)
///   - Divergent waves (at the V-angle)
///   - Foam trail along the ship's centerline
///
/// Implementation uses a trail of quad segments that follow the ship's
/// path, fading with distance. Each ship can have its own wake instance.
///
/// Phase 3-07 of the upgrade plan.
class WickedKelvinWake {
public:
    struct WakeParams {
        float maxLength = 200.0f;       // Maximum wake trail length (meters)
        float width = 40.0f;            // Wake width at widest point (meters)
        float fadeStartDist = 50.0f;    // Distance where fade begins
        float speedThreshold = 0.5f;    // Minimum speed to show wake (m/s)
        float foamIntensity = 0.8f;     // Center foam brightness (0-1)
        int numSegments = 32;           // Trail segments for smooth curve
    };

    WickedKelvinWake();
    ~WickedKelvinWake();

    /// Initialize wake rendering for the scene.
    void init(wi::scene::Scene* scene, const WakeParams& params);

    /// Update wake trail for a ship.
    /// @param shipId Unique ship identifier (for tracking multiple wakes)
    /// @param position Current ship position (world space)
    /// @param heading Ship heading (radians, 0=north/Z+, CW)
    /// @param speed Ship speed (m/s)
    /// @param dt Delta time (seconds)
    void update(int shipId, const Vec3& position, float heading,
                float speed, float dt);

    /// Remove a ship's wake (when ship is removed from simulation).
    void removeWake(int shipId);

    /// Set visibility of all wakes.
    void setVisible(bool visible);

    /// Clean up all wake geometry.
    void shutdown();

private:
    static constexpr float KELVIN_HALF_ANGLE = 19.47f; // degrees
    static constexpr float DEG_TO_RAD = 3.14159265358979f / 180.0f;

    wi::scene::Scene* weScene = nullptr;
    WakeParams params_;
    bool visible_ = true;

    /// Trail point for wake history
    struct TrailPoint {
        Vec3 position;
        float heading;
        float speed;
        float age;          // seconds since created
    };

    /// Per-ship wake data
    struct ShipWake {
        int shipId = 0;
        std::vector<TrailPoint> trail;
        uint64_t meshEntity = 0;
        uint64_t objectEntity = 0;
        uint64_t materialEntity = 0;
        bool dirty = true;
    };

    std::vector<ShipWake> wakes;

    ShipWake* findWake(int shipId);
    ShipWake& getOrCreateWake(int shipId);
    void rebuildWakeMesh(ShipWake& wake);
    void removeWakeEntities(ShipWake& wake);
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_KELVIN_WAKE_HPP
