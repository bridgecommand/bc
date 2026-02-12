/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedMultiView.hpp"
#include "WickedSceneNode.hpp"
#include <cmath>
#include <iostream>

namespace bc { namespace graphics { namespace wicked {

static constexpr float DEG_TO_RAD = 3.14159265358979f / 180.0f;

WickedMultiView::~WickedMultiView() {
    shutdown();
}

void WickedMultiView::init(wi::scene::Scene* scene, int viewCount) {
    weScene = scene;
    views.resize(viewCount);
}

void WickedMultiView::setupView(int viewIndex, const std::string& name,
                                  void* windowHandle, float yawOffset,
                                  float fovDegrees, int width, int height) {
    if (viewIndex < 0 || viewIndex >= (int)views.size()) return;

    BridgeView& view = views[viewIndex];
    view.name = name;
    view.windowHandle = windowHandle;
    view.yawOffset = yawOffset;
    view.fovDegrees = fovDegrees;

    // Create camera entity in the scene
    view.cameraEntity = weScene->Entity_CreateCamera(
        "BC_Camera_" + name,
        static_cast<float>(width),
        static_cast<float>(height),
        view.nearPlane,
        view.farPlane,
        fovDegrees * DEG_TO_RAD
    );

    // Create swapchain for this view's window
    if (windowHandle) {
        wi::graphics::SwapChainDesc scDesc;
        scDesc.width = width;
        scDesc.height = height;
        scDesc.buffer_count = 2;
        scDesc.format = wi::graphics::Format::R10G10B10A2_UNORM;
        scDesc.vsync = true;

        bool success = wi::graphics::GetDevice()->CreateSwapChain(
            &scDesc,
            static_cast<wi::platform::window_type>(windowHandle),
            &view.swapChain
        );

        if (success) {
            view.canvas.init(static_cast<wi::platform::window_type>(windowHandle));
            view.active = true;
            std::cout << "WickedMultiView: View '" << name << "' created ("
                      << width << "x" << height << ", yaw=" << yawOffset << "deg)"
                      << std::endl;
        } else {
            std::cerr << "WickedMultiView: Failed to create SwapChain for view '"
                      << name << "'" << std::endl;
        }
    } else {
        // No window yet -- view configured but not renderable
        // This allows setting up camera parameters before window creation
        view.active = false;
        std::cout << "WickedMultiView: View '" << name
                  << "' configured (no window yet)" << std::endl;
    }
}

void WickedMultiView::updateCameras(const Vec3& position,
                                      float heading, float pitch, float roll) {
    for (auto& view : views) {
        if (view.cameraEntity == wi::ecs::INVALID_ENTITY) continue;

        auto* cam = weScene->cameras.GetComponent(view.cameraEntity);
        if (!cam) continue;

        // Set camera position (same for all bridge views - they're on the same bridge)
        cam->Eye = {position.x, position.y, position.z};

        // Calculate look-at direction for this view
        // heading: ship heading (degrees from north, clockwise)
        // yawOffset: this view's angle offset from bow
        float totalYawDeg = heading + view.yawOffset;
        float totalYawRad = totalYawDeg * DEG_TO_RAD;
        float pitchRad = pitch * DEG_TO_RAD;

        // BC convention: heading 0=north (Z+), 90=east (X+)
        // Look direction in world space
        float lookX = std::sin(totalYawRad) * std::cos(pitchRad);
        float lookY = -std::sin(pitchRad);
        float lookZ = std::cos(totalYawRad) * std::cos(pitchRad);

        cam->At = {
            position.x + lookX,
            position.y + lookY,
            position.z + lookZ
        };

        // Up vector (account for roll)
        float rollRad = roll * DEG_TO_RAD;
        cam->Up = {
            std::sin(rollRad),
            std::cos(rollRad),
            0.0f
        };

        cam->SetDirty();
    }
}

int WickedMultiView::getActiveViewCount() const {
    int count = 0;
    for (const auto& v : views) {
        if (v.active) count++;
    }
    return count;
}

BridgeView* WickedMultiView::getView(int index) {
    if (index < 0 || index >= (int)views.size()) return nullptr;
    return &views[index];
}

const BridgeView* WickedMultiView::getView(int index) const {
    if (index < 0 || index >= (int)views.size()) return nullptr;
    return &views[index];
}

wi::scene::CameraComponent* WickedMultiView::getCamera(int viewIndex) {
    if (viewIndex < 0 || viewIndex >= (int)views.size()) return nullptr;
    auto& view = views[viewIndex];
    if (view.cameraEntity == wi::ecs::INVALID_ENTITY) return nullptr;
    return weScene->cameras.GetComponent(view.cameraEntity);
}

void WickedMultiView::shutdown() {
    for (auto& view : views) {
        if (view.cameraEntity != wi::ecs::INVALID_ENTITY && weScene) {
            weScene->Entity_Remove(view.cameraEntity);
            view.cameraEntity = wi::ecs::INVALID_ENTITY;
        }
        view.active = false;
    }
    views.clear();
    weScene = nullptr;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
