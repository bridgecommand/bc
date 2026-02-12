/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     Multi-viewport rendering for bridge simulator.
     Manages multiple cameras and render targets for the 3-monitor bridge setup. */

#ifndef BC_GRAPHICS_WICKED_MULTIVIEW_HPP
#define BC_GRAPHICS_WICKED_MULTIVIEW_HPP

#ifdef WITH_WICKED_ENGINE

#include "WickedEngine.h"
#include "../Types.hpp"
#include <vector>
#include <string>

namespace bc { namespace graphics { namespace wicked {

// Represents a single bridge view (one monitor/window).
struct BridgeView {
    std::string name;                    // e.g. "Port", "Center", "Starboard"
    wi::ecs::Entity cameraEntity = wi::ecs::INVALID_ENTITY;
    wi::graphics::SwapChain swapChain;
    wi::Canvas canvas;
    void* windowHandle = nullptr;        // Platform window handle

    // Camera parameters
    float yawOffset = 0;                 // Horizontal angle offset from bow (degrees)
    float fovDegrees = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 50000.0f;           // 50km for maritime distances

    bool active = false;                 // Whether this view is currently rendering
};

// Manages multiple bridge views for the 3-monitor setup.
// One shared scene is rendered with different cameras to different windows.
class WickedMultiView {
public:
    WickedMultiView() = default;
    ~WickedMultiView();

    // Initialize the multi-view system.
    // scene: the shared WE scene
    // views: number of views (typically 3 for port/center/starboard, or 1 for single monitor)
    void init(wi::scene::Scene* scene, int viewCount);

    // Set up a view with its window handle and camera parameters.
    // viewIndex: 0=port (or single), 1=center, 2=starboard
    // windowHandle: platform window handle (HWND on Windows, NSWindow* on macOS)
    // yawOffset: horizontal camera angle offset from bow in degrees
    // fov: field of view in degrees
    void setupView(int viewIndex, const std::string& name, void* windowHandle,
                    float yawOffset, float fovDegrees, int width, int height);

    // Update all cameras based on a master position and heading.
    // This is called each frame to sync cameras with the ship's bridge position.
    // position: bridge window position in world coordinates
    // heading: ship heading in degrees (0=north, clockwise)
    // pitch: ship pitch in degrees
    // roll: ship roll in degrees
    void updateCameras(const Vec3& position, float heading, float pitch, float roll);

    // Get the number of active views.
    int getActiveViewCount() const;

    // Get a specific view for custom rendering.
    BridgeView* getView(int index);
    const BridgeView* getView(int index) const;

    // Get the camera component for a specific view.
    wi::scene::CameraComponent* getCamera(int viewIndex);

    void shutdown();

private:
    wi::scene::Scene* weScene = nullptr;
    std::vector<BridgeView> views;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_MULTIVIEW_HPP
