/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_WICKED_RENDERER_HPP
#define BC_GRAPHICS_WICKED_RENDERER_HPP

#ifdef WITH_WICKED_ENGINE

#include "../IRenderer.hpp"
#include <memory>

// Forward-declare Wicked Engine types to avoid header pollution
namespace wi {
    class Application;
    class RenderPath3D;
    namespace scene { class Scene; }
}

namespace bc { namespace graphics { namespace wicked {

class WickedSceneManager;

// Wicked Engine renderer backend implementing IRenderer.
// Uses Metal on macOS, DX12 on Windows, Vulkan on Linux.
class WickedRenderer : public bc::graphics::IRenderer {
public:
    WickedRenderer();
    ~WickedRenderer() override;

    // IRenderer interface
    bool init(int width, int height, bool fullscreen) override;
    void shutdown() override;
    bool isRunning() const override;

    void beginFrame(unsigned clearFlags, Color clearColor) override;
    void endFrame() override;

    ISceneManager* getSceneManager() override;

    TextureHandle getTexture(const std::string& filename) override;
    TextureHandle addTexture(const std::string& name,
                              const Dimension2d& size) override;
    void removeTexture(TextureHandle tex) override;

    void draw2DImage(TextureHandle tex, const Rect& destRect,
                      const Rect& sourceRect,
                      const Color& color, bool useAlpha) override;
    void draw2DRectangle(const Color& color, const Rect& rect) override;
    void draw2DLine(const Vec2& start, const Vec2& end,
                     const Color& color) override;
    void draw3DLine(const Vec3& start, const Vec3& end,
                     const Color& color) override;

    void setRenderTarget(TextureHandle target, unsigned clearFlags,
                          Color clearColor) override;
    void setRenderTargetDefault() override;

    Dimension2d getScreenSize() const override;

    void* getNativeDriver() const override;
    void* getNativeDevice() const override;

    // Wicked-specific: set the native platform window handle
    // Must be called before init() on macOS (NSWindow*) or Windows (HWND)
    void setPlatformWindow(void* windowHandle);

    // Access the WE application (for advanced configuration)
    wi::Application* getApplication() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_RENDERER_HPP
