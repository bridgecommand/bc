/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef BC_GRAPHICS_IRENDERER_HPP
#define BC_GRAPHICS_IRENDERER_HPP

#include "Types.hpp"
#include <string>

namespace bc { namespace graphics {

class ISceneManager;

// Abstract renderer — wraps the video driver and device lifecycle.
// Combines Irrlicht's IVideoDriver and IrrlichtDevice responsibilities.
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // Lifecycle
    virtual bool init(int width, int height, bool fullscreen) = 0;
    virtual void shutdown() = 0;
    virtual bool isRunning() const = 0;

    // Frame rendering
    virtual void beginFrame(unsigned clearFlags = ClearColor | ClearDepth,
                             Color clearColor = {0, 0, 0, 0}) = 0;
    virtual void endFrame() = 0;

    // Scene management
    virtual ISceneManager* getSceneManager() = 0;

    // Texture loading
    virtual TextureHandle getTexture(const std::string& filename) = 0;
    virtual TextureHandle addTexture(const std::string& name,
                                      const Dimension2d& size) = 0;
    virtual void removeTexture(TextureHandle tex) = 0;

    // 2D drawing (for radar, GUI overlays, etc.)
    virtual void draw2DImage(TextureHandle tex, const Rect& destRect,
                              const Rect& sourceRect,
                              const Color& color = {255,255,255,255},
                              bool useAlpha = false) = 0;
    virtual void draw2DRectangle(const Color& color, const Rect& rect) = 0;
    virtual void draw2DLine(const Vec2& start, const Vec2& end,
                             const Color& color) = 0;

    // 3D drawing (debug, mooring lines)
    virtual void draw3DLine(const Vec3& start, const Vec3& end,
                             const Color& color) = 0;

    // Render target (offscreen rendering for radar)
    virtual void setRenderTarget(TextureHandle target,
                                  unsigned clearFlags = ClearColor | ClearDepth,
                                  Color clearColor = {0, 0, 0, 0}) = 0;
    virtual void setRenderTargetDefault() = 0;

    // Screen info
    virtual Dimension2d getScreenSize() const = 0;

    // Access the underlying engine-specific driver/device (for migration)
    virtual void* getNativeDriver() const = 0;
    virtual void* getNativeDevice() const = 0;
};

}} // namespace bc::graphics

#endif // BC_GRAPHICS_IRENDERER_HPP
