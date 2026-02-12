/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifndef BC_GUI_IMGUI_OVERLAY_HPP
#define BC_GUI_IMGUI_OVERLAY_HPP

#ifdef WITH_WICKED_ENGINE

#include <string>
#include <functional>
#include <vector>
#include <array>

namespace bc { namespace gui {

/// Simulation data snapshot for HUD rendering.
/// Updated each frame from the simulation model.
struct SimulationHUDData {
    // Navigation
    float heading = 0.0f;           // Ship heading (degrees, 0-360)
    float courseOverGround = 0.0f;   // COG (degrees)
    float rateOfTurn = 0.0f;        // ROT (degrees/minute)

    // Speed
    float speedOverGround = 0.0f;   // SOG (knots)
    float speedThroughWater = 0.0f; // STW (knots)
    float engineRPM = 0.0f;         // Engine RPM
    float rudderAngle = 0.0f;       // Rudder angle (degrees, + = starboard)
    float wheelAngle = 0.0f;        // Wheel position (degrees)

    // Depth
    float depth = 0.0f;             // Depth below keel (meters)
    float depthAlarm = 5.0f;        // Depth alarm threshold (meters)

    // Environment
    float windSpeed = 0.0f;         // Wind speed (knots)
    float windDirection = 0.0f;     // Wind direction (degrees, FROM)
    float weather = 0.0f;           // Sea state (0-12)

    // Engine
    float thrustLever = 0.0f;       // Thrust lever position (-1 to +1)
    int bowThruster = 0;            // Bow thruster (%, -100 to +100)
    int sternThruster = 0;          // Stern thruster (%, -100 to +100)

    // Time
    float simulationTime = 0.0f;    // Total simulation time (seconds)
    float timeAcceleration = 1.0f;  // Time acceleration factor
};

/// Palette names for the 4 OpenBridge brightness modes.
enum class BridgePalette : int {
    BrightDay = 0,
    Day       = 1,
    Dusk      = 2,
    Night     = 3
};

/// ImGui overlay manager for bridge instrument HUD.
///
/// Creates and manages ImGui windows for bridge instruments:
/// compass, speed, rudder angle, depth, engine, etc.
///
/// The overlay is rendered after the 3D scene on each frame.
/// Instruments can be shown/hidden and repositioned.
///
/// Phase 7-01 of the upgrade plan.
class ImGuiOverlay {
public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    /// Initialize the overlay (call after ImGui backend is initialized).
    void init(int screenWidth, int screenHeight);

    /// Update simulation data for the next frame.
    void setSimulationData(const SimulationHUDData& data);

    /// Render all active instrument windows.
    /// Call between ImGui::NewFrame() and ImGui::Render().
    void render();

    /// Show/hide individual instruments.
    void showCompass(bool show);
    void showSpeedDisplay(bool show);
    void showRudderDisplay(bool show);
    void showDepthDisplay(bool show);
    void showEngineDisplay(bool show);
    void showWindDisplay(bool show);

    /// Show/hide all instruments.
    void showAll(bool show);

    /// Toggle layout edit mode (allows dragging windows).
    void setLayoutLocked(bool locked);
    bool isLayoutLocked() const { return layoutLocked_; }

    /// Set brightness palette (0=bright day, 1=day, 2=dusk, 3=night).
    void setPalette(int paletteIndex);
    void setPalette(BridgePalette palette) { setPalette(static_cast<int>(palette)); }
    int getPalette() const { return currentPalette_; }

    /// Toggle depth display units between metres and feet.
    void setDepthUnitMetric(bool metric);
    bool isDepthUnitMetric() const { return depthUnitsMetric_; }

    /// Save/load instrument layout to/from file.
    void saveLayout(const std::string& path);
    void loadLayout(const std::string& path);

    /// Set the ImGui .ini file path for persistent window positions.
    void setIniFilePath(const std::string& path);

    /// Check if ImGui wants to capture keyboard/mouse.
    bool wantsKeyboard() const;
    bool wantsMouse() const;

private:
    SimulationHUDData data_;
    int screenWidth_ = 1920;
    int screenHeight_ = 1080;
    bool layoutLocked_ = true;
    int currentPalette_ = 1; // Default: Day palette
    bool depthUnitsMetric_ = true;
    std::string iniFilePath_;

    // Depth trend tracking
    static constexpr int DEPTH_HISTORY_SIZE = 30; // ~0.5s at 60fps
    std::array<float, 30> depthHistory_ = {};
    int depthHistoryIndex_ = 0;
    bool depthHistoryFull_ = false;

    // Instrument visibility
    bool showCompass_ = true;
    bool showSpeed_ = true;
    bool showRudder_ = true;
    bool showDepth_ = true;
    bool showEngine_ = true;
    bool showWind_ = false;

    // Rendering methods for each instrument
    void renderCompass();
    void renderSpeedDisplay();
    void renderRudderDisplay();
    void renderDepthDisplay();
    void renderEngineDisplay();
    void renderWindDisplay();

    // Process keyboard shortcuts (F5-F8 for palettes, F9 for layout lock)
    void processKeyboardShortcuts();

    // Apply color palette to ImGui style
    void applyPalette();
};

}} // namespace bc::gui

#endif // WITH_WICKED_ENGINE
#endif // BC_GUI_IMGUI_OVERLAY_HPP
