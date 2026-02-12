/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     VR stereo rendering via Wicked Engine.
     Manages left/right eye cameras and render targets for OpenXR submission. */

#ifndef BC_GRAPHICS_WICKED_VRVIEW_HPP
#define BC_GRAPHICS_WICKED_VRVIEW_HPP

#ifdef WITH_WICKED_ENGINE

#include "WickedEngine.h"
#include "../Types.hpp"

namespace bc { namespace graphics { namespace wicked {

// Per-eye render data for VR stereo rendering via Wicked Engine.
struct VREyeView {
    wi::ecs::Entity cameraEntity = wi::ecs::INVALID_ENTITY;
    wi::graphics::Texture renderTarget;
    wi::graphics::Texture depthTarget;
    int width = 0;
    int height = 0;
    bool active = false;
};

// Manages stereo VR rendering through Wicked Engine.
// Creates two cameras (left/right eye) and renders each to a texture
// that can then be submitted to OpenXR.
//
// Usage:
//   1. Call init() with the WE scene and per-eye resolution
//   2. Each frame, call updateEyePose() for each eye with OpenXR tracking data
//   3. Call renderEye() for each eye to render the scene
//   4. Call getEyeTexture() to get the rendered result for OpenXR submission
//
// NOTE: This is scaffolding code. The actual OpenXR graphics binding must
// change from OpenGL (XR_KHR_OPENGL_ENABLE) to either Vulkan
// (XR_KHR_VULKAN_ENABLE) or D3D12 (XR_KHR_D3D12_ENABLE) depending on
// the WE backend, and the OpenXR swapchain image types must match.
class WickedVRView {
public:
    WickedVRView() = default;
    ~WickedVRView();

    // Initialize stereo rendering with per-eye resolution.
    // scene: the shared WE scene (same scene used for bridge monitors)
    // eyeWidth/eyeHeight: resolution per eye (from OpenXR viewconfig)
    void init(wi::scene::Scene* scene, int eyeWidth, int eyeHeight);

    // Update an eye's camera from OpenXR tracking data.
    // eyeIndex: 0 = left, 1 = right
    // position: eye position in world space (already transformed by ship pose)
    // orientation: eye orientation quaternion from OpenXR
    // fovLeft/fovRight/fovUp/fovDown: asymmetric FOV angles in radians
    void updateEyePose(int eyeIndex,
                       const Vec3& position,
                       const Quaternion& orientation,
                       float fovLeft, float fovRight,
                       float fovUp, float fovDown);

    // Render the scene from the perspective of one eye.
    // Must be called after updateEyePose().
    // eyeIndex: 0 = left, 1 = right
    void renderEye(int eyeIndex);

    // Get the rendered texture for an eye (for submission to OpenXR).
    // Returns the native WE Texture. The caller is responsible for
    // copying/blitting this into the OpenXR swapchain image.
    const wi::graphics::Texture* getEyeTexture(int eyeIndex) const;

    // Get the WE camera component for an eye (for custom adjustments).
    wi::scene::CameraComponent* getCamera(int eyeIndex);

    // Check if VR view is initialized.
    bool isInitialized() const { return initialized; }

    // Clean up resources.
    void shutdown();

    // Set near/far planes (default 0.01m / 100m suitable for VR).
    void setClipPlanes(float nearZ, float farZ);

private:
    static constexpr int EYE_COUNT = 2;
    static constexpr int EYE_LEFT = 0;
    static constexpr int EYE_RIGHT = 1;

    wi::scene::Scene* weScene = nullptr;
    VREyeView eyes[EYE_COUNT];
    float nearPlane = 0.01f;
    float farPlane = 100.0f;
    bool initialized = false;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_VRVIEW_HPP
