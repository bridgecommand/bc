/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedVRView.hpp"
#include <iostream>
#include <cmath>

namespace bc { namespace graphics { namespace wicked {

WickedVRView::~WickedVRView() {
    shutdown();
}

void WickedVRView::init(wi::scene::Scene* scene, int eyeWidth, int eyeHeight) {
    if (initialized) {
        shutdown();
    }

    weScene = scene;

    for (int eye = 0; eye < EYE_COUNT; ++eye) {
        auto& ev = eyes[eye];
        ev.width = eyeWidth;
        ev.height = eyeHeight;

        // Create a camera entity for this eye
        std::string name = (eye == EYE_LEFT) ? "BC_VR_LeftEye" : "BC_VR_RightEye";
        ev.cameraEntity = weScene->Entity_CreateCamera(
            name, eyeWidth, eyeHeight, nearPlane, farPlane);

        // Create render target texture for this eye.
        // The rendered image will be copied into the OpenXR swapchain.
        wi::graphics::TextureDesc desc;
        desc.width = eyeWidth;
        desc.height = eyeHeight;
        desc.format = wi::graphics::Format::R8G8B8A8_UNORM;
        desc.bind_flags = wi::graphics::BindFlag::RENDER_TARGET |
                          wi::graphics::BindFlag::SHADER_RESOURCE;
        desc.usage = wi::graphics::Usage::DEFAULT;
        desc.sample_count = 1;
        desc.mip_levels = 1;
        desc.array_size = 1;
        desc.layout = wi::graphics::ResourceState::RENDERTARGET;

        wi::graphics::GraphicsDevice* device = wi::graphics::GetDevice();
        bool ok = device->CreateTexture(&desc, nullptr, &ev.renderTarget);
        if (!ok) {
            std::cerr << "WickedVRView: Failed to create render target for eye " << eye << std::endl;
            continue;
        }

        // Create depth target
        wi::graphics::TextureDesc depthDesc;
        depthDesc.width = eyeWidth;
        depthDesc.height = eyeHeight;
        depthDesc.format = wi::graphics::Format::D32_FLOAT;
        depthDesc.bind_flags = wi::graphics::BindFlag::DEPTH_STENCIL;
        depthDesc.usage = wi::graphics::Usage::DEFAULT;
        depthDesc.sample_count = 1;
        depthDesc.mip_levels = 1;
        depthDesc.array_size = 1;
        depthDesc.layout = wi::graphics::ResourceState::DEPTHSTENCIL;

        ok = device->CreateTexture(&depthDesc, nullptr, &ev.depthTarget);
        if (!ok) {
            std::cerr << "WickedVRView: Failed to create depth target for eye " << eye << std::endl;
            continue;
        }

        ev.active = true;
    }

    initialized = true;
    std::cout << "WickedVRView: Initialized stereo rendering ("
              << eyeWidth << "x" << eyeHeight << " per eye)" << std::endl;
}

void WickedVRView::updateEyePose(int eyeIndex,
                                  const Vec3& position,
                                  const Quaternion& orientation,
                                  float fovLeft, float fovRight,
                                  float fovUp, float fovDown) {
    if (eyeIndex < 0 || eyeIndex >= EYE_COUNT || !initialized) return;

    auto& ev = eyes[eyeIndex];
    if (!ev.active) return;

    auto* cam = weScene->cameras.GetComponent(ev.cameraEntity);
    if (!cam) return;

    // Set eye position
    cam->Eye = XMFLOAT3(position.x, position.y, position.z);

    // Convert quaternion orientation to a forward/up vector pair for WE camera
    // OpenXR uses right-handed coordinates; WE uses left-handed.
    // The coordinate transform (Z negation) should be applied by the caller,
    // consistent with how VRInterface already transforms poses.

    // Convert quaternion to rotation matrix manually
    float qx = orientation.x, qy = orientation.y, qz = orientation.z, qw = orientation.w;
    float xx = qx * qx, yy = qy * qy, zz = qz * qz;
    float xy = qx * qy, xz = qx * qz, xw = qx * qw;
    float yz = qy * qz, yw = qy * qw, zw = qz * qw;

    // Forward direction (negative Z in OpenXR convention, already Z-flipped by caller)
    XMFLOAT3 forward;
    forward.x = 2.0f * (xy - zw);
    forward.y = 1.0f - 2.0f * (xx + zz);
    forward.z = 2.0f * (yz + xw);

    // Actually, forward is -Z column of rotation matrix:
    forward.x = -(2.0f * (xz + yw));
    forward.y = -(2.0f * (yz - xw));
    forward.z = -(1.0f - 2.0f * (xx + yy));

    cam->At = XMFLOAT3(
        cam->Eye.x + forward.x,
        cam->Eye.y + forward.y,
        cam->Eye.z + forward.z);

    // Up direction (Y column of rotation matrix)
    cam->Up = XMFLOAT3(
        2.0f * (xy - zw),
        1.0f - 2.0f * (xx + zz),
        2.0f * (yz + xw));

    // Set asymmetric projection from OpenXR FOV angles.
    // WE's CameraComponent supports custom projection matrices.
    float tanLeft = std::tan(fovLeft);    // negative for left
    float tanRight = std::tan(fovRight);  // positive for right
    float tanUp = std::tan(fovUp);        // positive for up
    float tanDown = std::tan(fovDown);    // negative for down

    // Build asymmetric perspective projection matrix (row-major for WE/DirectXMath)
    // This matches the OpenXR asymmetric frustum exactly.
    float nearZ = nearPlane;
    float farZ = farPlane;
    float invWidth = 1.0f / (tanRight - tanLeft);
    float invHeight = 1.0f / (tanUp - tanDown);
    float range = farZ / (farZ - nearZ);

    XMFLOAT4X4 proj;
    std::memset(&proj, 0, sizeof(proj));
    proj._11 = 2.0f * invWidth;
    proj._22 = 2.0f * invHeight;
    proj._31 = -(tanRight + tanLeft) * invWidth;
    proj._32 = -(tanUp + tanDown) * invHeight;
    proj._33 = range;
    proj._34 = 1.0f;
    proj._43 = -range * nearZ;

    cam->Projection = proj;
    cam->SetDirty();
}

void WickedVRView::renderEye(int eyeIndex) {
    if (eyeIndex < 0 || eyeIndex >= EYE_COUNT || !initialized) return;

    auto& ev = eyes[eyeIndex];
    if (!ev.active) return;

    // TODO: When OpenXR graphics binding is changed from OpenGL to Vulkan/D3D12,
    // render the scene using WE's RenderPath3D targeting ev.renderTarget.
    //
    // The render flow would be:
    //   1. Set the active camera to this eye's camera
    //   2. Configure the render path to output to ev.renderTarget
    //   3. Execute the render path for this eye
    //   4. The resulting texture can then be copied to the OpenXR swapchain
    //
    // For now, this is a no-op as the actual rendering integration requires
    // the OpenXR Vulkan/D3D12 extension and swapchain interop that can only
    // be tested with VR hardware.
    //
    // Approach options:
    //   A) Render to WE texture, then CopyTexture into OpenXR swapchain image
    //   B) Create OpenXR swapchains with WE-compatible format and render directly
    //   C) Use WE's render-to-texture, resolve MSAA, then blit to OpenXR
    //
    // Option B is preferred for performance (avoids copy).
}

const wi::graphics::Texture* WickedVRView::getEyeTexture(int eyeIndex) const {
    if (eyeIndex < 0 || eyeIndex >= EYE_COUNT || !initialized) return nullptr;
    if (!eyes[eyeIndex].active) return nullptr;
    return &eyes[eyeIndex].renderTarget;
}

wi::scene::CameraComponent* WickedVRView::getCamera(int eyeIndex) {
    if (eyeIndex < 0 || eyeIndex >= EYE_COUNT || !initialized) return nullptr;
    return weScene->cameras.GetComponent(eyes[eyeIndex].cameraEntity);
}

void WickedVRView::setClipPlanes(float nearZ, float farZ) {
    nearPlane = nearZ;
    farPlane = farZ;
}

void WickedVRView::shutdown() {
    if (!initialized) return;

    for (int eye = 0; eye < EYE_COUNT; ++eye) {
        auto& ev = eyes[eye];
        if (ev.cameraEntity != wi::ecs::INVALID_ENTITY && weScene) {
            weScene->Entity_Remove(ev.cameraEntity);
            ev.cameraEntity = wi::ecs::INVALID_ENTITY;
        }
        ev.renderTarget = {};
        ev.depthTarget = {};
        ev.active = false;
    }

    weScene = nullptr;
    initialized = false;
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
