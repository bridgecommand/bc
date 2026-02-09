/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GRAPHICS_IRRLICHT_RENDERER_HPP
#define BC_GRAPHICS_IRRLICHT_RENDERER_HPP

#include "../IRenderer.hpp"
#include "IrrlichtSceneManager.hpp"
#include <memory>

namespace bc { namespace graphics { namespace irrlicht {

// Wraps irr::IrrlichtDevice + irr::video::IVideoDriver
class IrrlichtRenderer : public bc::graphics::IRenderer {
public:
    // Create from an existing Irrlicht device (for incremental migration)
    explicit IrrlichtRenderer(irr::IrrlichtDevice* device)
        : irrDevice(device)
        , irrDriver(device->getVideoDriver())
        , ownsDevice(false) {}

    ~IrrlichtRenderer() override {
        if (ownsDevice && irrDevice) {
            irrDevice->drop();
        }
    }

    bool init(int width, int height, bool fullscreen) override {
        // When wrapping an existing device, init is a no-op
        // For future standalone creation, this would call createDevice()
        return irrDevice != nullptr;
    }

    void shutdown() override {
        sceneManager.reset();
        if (ownsDevice && irrDevice) {
            irrDevice->closeDevice();
        }
    }

    bool isRunning() const override {
        return irrDevice && irrDevice->run();
    }

    void beginFrame(unsigned clearFlags, Color clearColor) override {
        irr::u16 irrFlags = 0;
        if (clearFlags & ClearColor)   irrFlags |= irr::video::ECBF_COLOR;
        if (clearFlags & ClearDepth)   irrFlags |= irr::video::ECBF_DEPTH;
        if (clearFlags & ClearStencil) irrFlags |= irr::video::ECBF_STENCIL;
        irrDriver->beginScene(irrFlags, toIrrColor(clearColor));
    }

    void endFrame() override {
        irrDriver->endScene();
    }

    bc::graphics::ISceneManager* getSceneManager() override {
        if (!sceneManager) {
            sceneManager = std::make_unique<IrrlichtSceneManager>(irrDevice->getSceneManager());
        }
        return sceneManager.get();
    }

    TextureHandle getTexture(const std::string& filename) override {
        return irrDriver->getTexture(filename.c_str());
    }

    TextureHandle addTexture(const std::string& name, const Dimension2d& size) override {
        return irrDriver->addTexture(
            irr::core::dimension2d<irr::u32>(size.width, size.height),
            name.c_str());
    }

    void removeTexture(TextureHandle tex) override {
        irrDriver->removeTexture(static_cast<irr::video::ITexture*>(tex));
    }

    void draw2DImage(TextureHandle tex, const Rect& destRect, const Rect& sourceRect,
                      const Color& color, bool useAlpha) override {
        irr::core::rect<irr::s32> dest(destRect.x, destRect.y,
                                        destRect.x + destRect.width,
                                        destRect.y + destRect.height);
        irr::core::rect<irr::s32> src(sourceRect.x, sourceRect.y,
                                       sourceRect.x + sourceRect.width,
                                       sourceRect.y + sourceRect.height);
        irr::video::SColor colors[4] = {
            toIrrColor(color), toIrrColor(color),
            toIrrColor(color), toIrrColor(color)
        };
        irrDriver->draw2DImage(static_cast<irr::video::ITexture*>(tex),
                                dest, src, nullptr, colors, useAlpha);
    }

    void draw2DRectangle(const Color& color, const Rect& rect) override {
        irrDriver->draw2DRectangle(toIrrColor(color),
            irr::core::rect<irr::s32>(rect.x, rect.y,
                                       rect.x + rect.width,
                                       rect.y + rect.height));
    }

    void draw2DLine(const Vec2& start, const Vec2& end, const Color& color) override {
        irrDriver->draw2DLine(
            irr::core::position2d<irr::s32>(static_cast<int>(start.x), static_cast<int>(start.y)),
            irr::core::position2d<irr::s32>(static_cast<int>(end.x), static_cast<int>(end.y)),
            toIrrColor(color));
    }

    void draw3DLine(const Vec3& start, const Vec3& end, const Color& color) override {
        irrDriver->draw3DLine(toIrr(start), toIrr(end), toIrrColor(color));
    }

    void setRenderTarget(TextureHandle target, unsigned clearFlags, Color clearColor) override {
        irr::u16 irrFlags = 0;
        if (clearFlags & ClearColor) irrFlags |= irr::video::ECBF_COLOR;
        if (clearFlags & ClearDepth) irrFlags |= irr::video::ECBF_DEPTH;
        irrDriver->setRenderTarget(static_cast<irr::video::ITexture*>(target),
                                    irrFlags, toIrrColor(clearColor));
    }

    void setRenderTargetDefault() override {
        irrDriver->setRenderTarget((irr::video::ITexture*)nullptr,
                                    irr::video::ECBF_COLOR | irr::video::ECBF_DEPTH);
    }

    Dimension2d getScreenSize() const override {
        auto sz = irrDriver->getScreenSize();
        return {static_cast<int>(sz.Width), static_cast<int>(sz.Height)};
    }

    void* getNativeDriver() const override { return irrDriver; }
    void* getNativeDevice() const override { return irrDevice; }

private:
    irr::IrrlichtDevice* irrDevice;
    irr::video::IVideoDriver* irrDriver;
    bool ownsDevice;
    std::unique_ptr<IrrlichtSceneManager> sceneManager;
};

}}} // namespace bc::graphics::irrlicht

#endif // BC_GRAPHICS_IRRLICHT_RENDERER_HPP
