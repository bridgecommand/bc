/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "WickedRenderer.hpp"
#include "WickedSceneManager.hpp"
#include "WickedSceneNode.hpp"
#include "WickedEngine.h"

#include <iostream>

namespace bc { namespace graphics { namespace wicked {

// Private implementation details
struct WickedRenderer::Impl {
    std::unique_ptr<wi::Application> application;
    std::unique_ptr<wi::RenderPath3D> renderPath;
    std::unique_ptr<WickedSceneManager> sceneManager;
    void* platformWindow = nullptr;
    int width = 0;
    int height = 0;
    bool running = false;
};

WickedRenderer::WickedRenderer()
    : impl(std::make_unique<Impl>()) {}

WickedRenderer::~WickedRenderer() {
    shutdown();
}

void WickedRenderer::setPlatformWindow(void* windowHandle) {
    impl->platformWindow = windowHandle;
}

bool WickedRenderer::init(int width, int height, bool fullscreen) {
    impl->width = width;
    impl->height = height;

    // Create the WE application
    impl->application = std::make_unique<wi::Application>();

    if (impl->platformWindow) {
        impl->application->SetWindow(
            static_cast<wi::platform::window_type>(impl->platformWindow));
    }

    // Create 3D render path
    impl->renderPath = std::make_unique<wi::RenderPath3D>();

    // Configure render path for maritime simulation
    impl->renderPath->setSSREnabled(true);     // Screen-space reflections (water)
    impl->renderPath->setFXAAEnabled(true);     // Anti-aliasing
    impl->renderPath->setBloomEnabled(true);    // Bloom (for lights/sun)

    // Activate the render path
    impl->application->ActivatePath(impl->renderPath.get());

    // Initialize the application
    impl->application->Initialize();

    // Create scene manager wrapping WE's global scene
    wi::scene::Scene& scene = wi::scene::GetScene();
    impl->sceneManager = std::make_unique<WickedSceneManager>(&scene);

    impl->running = true;

    std::cout << "WickedRenderer: Initialized (" << width << "x" << height << ")" << std::endl;
    return true;
}

void WickedRenderer::shutdown() {
    if (impl->running) {
        impl->sceneManager.reset();
        impl->renderPath.reset();
        impl->application.reset();
        impl->running = false;
        wi::jobsystem::ShutDown();
    }
}

bool WickedRenderer::isRunning() const {
    return impl->running;
}

void WickedRenderer::beginFrame(unsigned clearFlags, Color clearColor) {
    // WE handles frame begin internally in Application::Run()
    // Clear color is managed by the render path's background color settings
}

void WickedRenderer::endFrame() {
    // Drive the WE application for one frame
    if (impl->application) {
        impl->application->Run();
    }
}

ISceneManager* WickedRenderer::getSceneManager() {
    return impl->sceneManager.get();
}

TextureHandle WickedRenderer::getTexture(const std::string& filename) {
    // Load texture via WE resource manager
    wi::Resource resource = wi::resourcemanager::Load(filename);
    // Store resource to keep it alive; return opaque handle
    // (WE manages texture lifetime via resource reference counting)
    return resource.IsValid() ? (TextureHandle)&resource.GetTexture() : nullptr;
}

TextureHandle WickedRenderer::addTexture(const std::string& name,
                                           const Dimension2d& size) {
    // Create an empty texture for render targets (e.g., radar)
    // TODO: Use wi::graphics::CreateTexture for dynamic textures
    return nullptr;
}

void WickedRenderer::removeTexture(TextureHandle tex) {
    // WE manages texture lifetime via resource manager
}

void WickedRenderer::draw2DImage(TextureHandle tex, const Rect& destRect,
                                   const Rect& sourceRect,
                                   const Color& color, bool useAlpha) {
    // Use WE's sprite rendering for 2D overlays
    // This will be needed for radar display, HUD elements
    // TODO: Implement via wi::image::Draw or ImGui
}

void WickedRenderer::draw2DRectangle(const Color& color, const Rect& rect) {
    // TODO: Implement via wi::image or ImGui
}

void WickedRenderer::draw2DLine(const Vec2& start, const Vec2& end,
                                  const Color& color) {
    // TODO: Implement via wi::renderer::DrawLine2D or ImGui
}

void WickedRenderer::draw3DLine(const Vec3& start, const Vec3& end,
                                  const Color& color) {
    wi::renderer::RenderablePoint p1, p2;
    p1.position = toXM(start);
    p1.color = wi::Color(color.r, color.g, color.b, color.a).toFloat4();
    p1.size = 0.01f;
    p2.position = toXM(end);
    p2.color = p1.color;
    p2.size = 0.01f;
    wi::renderer::DrawPoint(p1);
    wi::renderer::DrawPoint(p2);
    // TODO: Use wi::renderer::DrawLine for proper line rendering
}

void WickedRenderer::setRenderTarget(TextureHandle target, unsigned clearFlags,
                                       Color clearColor) {
    // TODO: Implement render-to-texture for radar
}

void WickedRenderer::setRenderTargetDefault() {
    // Return to default framebuffer - handled by WE
}

Dimension2d WickedRenderer::getScreenSize() const {
    return {impl->width, impl->height};
}

void* WickedRenderer::getNativeDriver() const {
    // Return WE graphics device
    return wi::graphics::GetDevice();
}

void* WickedRenderer::getNativeDevice() const {
    return impl->application.get();
}

wi::Application* WickedRenderer::getApplication() const {
    return impl->application.get();
}

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
