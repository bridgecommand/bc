/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     Dear ImGui integration with Wicked Engine.
     Adapted from WE Example_ImGui (MIT license). */

#ifndef BC_GRAPHICS_WICKED_IMGUI_HPP
#define BC_GRAPHICS_WICKED_IMGUI_HPP

#ifdef WITH_WICKED_ENGINE

#include "WickedEngine.h"

namespace bc { namespace graphics { namespace wicked {

// Initialize Dear ImGui for use with Wicked Engine.
// Call once after the WE Application is initialized.
// platformWindow: native window handle (HWND on Windows, NSWindow* on macOS)
void ImGuiInit(void* platformWindow);

// Shut down Dear ImGui. Call before WE shutdown.
void ImGuiShutdown();

// Begin a new ImGui frame. Call once per frame before any ImGui widget calls.
void ImGuiNewFrame();

// Render ImGui draw data to the screen via WE graphics device.
// Call after all ImGui widget calls, typically in Application::Compose().
void ImGuiRender(wi::graphics::CommandList cmd);

// Returns true if ImGui wants to capture keyboard input (e.g., typing in a text field)
bool ImGuiWantsKeyboard();

// Returns true if ImGui wants to capture mouse input (e.g., hovering over a window)
bool ImGuiWantsMouse();

}}} // namespace bc::graphics::wicked

#endif // WITH_WICKED_ENGINE
#endif // BC_GRAPHICS_WICKED_IMGUI_HPP
