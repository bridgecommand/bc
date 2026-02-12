/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation */

#ifdef WITH_WICKED_ENGINE

#include "ImGuiOverlay.hpp"
#include "../graphics/wicked/imgui/imgui.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace bc { namespace gui {

static constexpr float PI = 3.14159265358979f;
static constexpr float DEG_TO_RAD = PI / 180.0f;
static constexpr float METRES_TO_FEET = 3.28084f;

ImGuiOverlay::ImGuiOverlay() {
    depthHistory_.fill(0.0f);
}
ImGuiOverlay::~ImGuiOverlay() = default;

void ImGuiOverlay::init(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    applyPalette();
}

void ImGuiOverlay::setSimulationData(const SimulationHUDData& data) {
    data_ = data;

    // Track depth history for trend indication
    depthHistory_[depthHistoryIndex_] = data.depth;
    depthHistoryIndex_ = (depthHistoryIndex_ + 1) % DEPTH_HISTORY_SIZE;
    if (depthHistoryIndex_ == 0) depthHistoryFull_ = true;
}

void ImGuiOverlay::render() {
    processKeyboardShortcuts();

    if (showCompass_) renderCompass();
    if (showSpeed_) renderSpeedDisplay();
    if (showRudder_) renderRudderDisplay();
    if (showDepth_) renderDepthDisplay();
    if (showEngine_) renderEngineDisplay();
    if (showWind_) renderWindDisplay();
}

// -- Compass ----------------------------------------------------------

void ImGuiOverlay::renderCompass() {
    ImGui::SetNextWindowSize(ImVec2(200, 220), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(screenWidth_ * 0.5f - 100, 10), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_)
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Compass", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetCursorScreenPos();
    float avail = ImGui::GetContentRegionAvail().x;
    float radius = avail * 0.4f;
    ImVec2 center(winPos.x + avail * 0.5f, winPos.y + radius + 5);

    // Background circle
    draw->AddCircleFilled(center, radius + 2, IM_COL32(30, 30, 30, 200));
    draw->AddCircle(center, radius + 2, IM_COL32(180, 180, 180, 255), 0, 2.0f);

    // Cardinal and intercardinal marks
    const char* labels[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    for (int i = 0; i < 8; i++) {
        float angle = i * 45.0f - data_.heading;
        float rad = angle * DEG_TO_RAD;
        float x = center.x + std::sin(rad) * (radius - 12);
        float y = center.y - std::cos(rad) * (radius - 12);

        ImU32 col = (i == 0) ? IM_COL32(255, 80, 80, 255) : IM_COL32(200, 200, 200, 255);
        ImVec2 textSize = ImGui::CalcTextSize(labels[i]);
        draw->AddText(ImVec2(x - textSize.x * 0.5f, y - textSize.y * 0.5f), col, labels[i]);
    }

    // Degree tick marks
    for (int deg = 0; deg < 360; deg += 10) {
        float angle = deg - data_.heading;
        float rad = angle * DEG_TO_RAD;
        float innerR = (deg % 30 == 0) ? radius - 22 : radius - 18;
        float x1 = center.x + std::sin(rad) * innerR;
        float y1 = center.y - std::cos(rad) * innerR;
        float x2 = center.x + std::sin(rad) * radius;
        float y2 = center.y - std::cos(rad) * radius;
        draw->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(160, 160, 160, 255), 1.0f);
    }

    // Ship heading indicator (top triangle)
    draw->AddTriangleFilled(
        ImVec2(center.x, center.y - radius - 6),
        ImVec2(center.x - 6, center.y - radius + 4),
        ImVec2(center.x + 6, center.y - radius + 4),
        IM_COL32(255, 200, 0, 255)
    );

    // Center dot
    draw->AddCircleFilled(center, 3, IM_COL32(255, 200, 0, 255));

    // Numeric heading readout below compass
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + radius * 2 + 15);
    char headingStr[32];
    snprintf(headingStr, sizeof(headingStr), "%03.1f", data_.heading);
    float textW = ImGui::CalcTextSize(headingStr).x;
    ImGui::SetCursorPosX((avail - textW) * 0.5f);
    ImGui::TextColored(ImVec4(1, 0.9f, 0.3f, 1), "%s", headingStr);

    ImGui::End();
}

// -- Speed Display ----------------------------------------------------

void ImGuiOverlay::renderSpeedDisplay() {
    ImGui::SetNextWindowSize(ImVec2(140, 110), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(screenWidth_ - 150, 10), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_) flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Speed", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "SOG");
    ImGui::SameLine(70);
    ImGui::TextColored(ImVec4(1, 1, 0.3f, 1), "%4.1f kn", data_.speedOverGround);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "STW");
    ImGui::SameLine(70);
    ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "%4.1f kn", data_.speedThroughWater);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "COG");
    ImGui::SameLine(70);
    ImGui::Text("%03.1f", data_.courseOverGround);

    ImGui::End();
}

// -- Rudder Angle Display --------------------------------------------

void ImGuiOverlay::renderRudderDisplay() {
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(screenWidth_ * 0.5f - 100, screenHeight_ - 110), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_) flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Rudder", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetCursorScreenPos();
    float avail = ImGui::GetContentRegionAvail().x;
    float height = 30;
    ImVec2 arcCenter(winPos.x + avail * 0.5f, winPos.y + height + 10);
    float arcRadius = avail * 0.4f;

    // Arc background
    float maxAngle = 40.0f; // max rudder angle display
    draw->PathArcTo(arcCenter, arcRadius,
        PI + (-maxAngle) * DEG_TO_RAD, PI + maxAngle * DEG_TO_RAD, 40);
    draw->PathStroke(IM_COL32(60, 60, 60, 255), 0, 4.0f);

    // Port side (red) and starboard side (green) zones
    float rudderRad = data_.rudderAngle * DEG_TO_RAD;
    if (data_.rudderAngle < -0.5f) {
        draw->PathArcTo(arcCenter, arcRadius, PI, PI + rudderRad, 20);
        draw->PathStroke(IM_COL32(200, 50, 50, 255), 0, 4.0f);
    } else if (data_.rudderAngle > 0.5f) {
        draw->PathArcTo(arcCenter, arcRadius, PI, PI + rudderRad, 20);
        draw->PathStroke(IM_COL32(50, 200, 50, 255), 0, 4.0f);
    }

    // Rudder needle
    float needleAngle = PI + rudderRad;
    float nx = arcCenter.x + std::cos(needleAngle) * arcRadius;
    float ny = arcCenter.y + std::sin(needleAngle) * arcRadius;
    draw->AddLine(arcCenter, ImVec2(nx, ny), IM_COL32(255, 255, 0, 255), 2.0f);

    // Center mark
    draw->AddLine(
        ImVec2(arcCenter.x, arcCenter.y - arcRadius - 2),
        ImVec2(arcCenter.x, arcCenter.y - arcRadius + 6),
        IM_COL32(255, 255, 255, 255), 2.0f);

    // Numeric readout
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + height + 25);
    char rudderStr[32];
    snprintf(rudderStr, sizeof(rudderStr), "%+.1f", data_.rudderAngle);
    float textW = ImGui::CalcTextSize(rudderStr).x;
    ImGui::SetCursorPosX((avail - textW) * 0.5f);

    ImVec4 rudderColor = (data_.rudderAngle < -0.5f)
        ? ImVec4(1, 0.3f, 0.3f, 1)   // Port = red
        : (data_.rudderAngle > 0.5f)
            ? ImVec4(0.3f, 1, 0.3f, 1)   // Starboard = green
            : ImVec4(1, 1, 1, 1);         // Center = white
    ImGui::TextColored(rudderColor, "%s", rudderStr);

    ImGui::End();
}

// -- Depth Display (7-05) ---------------------------------------------

void ImGuiOverlay::renderDepthDisplay() {
    ImGui::SetNextWindowSize(ImVec2(150, 160), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(10, screenHeight_ - 170), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_) flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Depth", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    float avail = ImGui::GetContentRegionAvail().x;

    // Convert depth to display units
    float displayDepth = depthUnitsMetric_ ? data_.depth : data_.depth * METRES_TO_FEET;
    float displayAlarm = depthUnitsMetric_ ? data_.depthAlarm : data_.depthAlarm * METRES_TO_FEET;
    const char* unitStr = depthUnitsMetric_ ? "m" : "ft";

    // Alarm state
    bool alarm = data_.depth > 0 && data_.depth < data_.depthAlarm;
    bool alarmFlash = alarm && (std::fmod(data_.simulationTime * 2.0f, 1.0f) > 0.5f);

    // Flashing background when alarm active
    if (alarmFlash) {
        ImVec2 wp = ImGui::GetWindowPos();
        ImVec2 ws = ImGui::GetWindowSize();
        draw->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y),
            IM_COL32(120, 20, 20, 60), 4.0f);
    }

    // Depth value - large text
    ImVec4 depthColor;
    if (alarm) {
        depthColor = ImVec4(1, 0.2f, 0.2f, 1); // Red when below alarm
    } else if (data_.depth > 0 && data_.depth < data_.depthAlarm * 2.0f) {
        depthColor = ImVec4(1, 0.8f, 0.2f, 1); // Yellow when approaching alarm
    } else {
        depthColor = ImVec4(0.3f, 0.8f, 1, 1);  // Blue for safe depth
    }

    char depthStr[32];
    snprintf(depthStr, sizeof(depthStr), "%.1f %s", displayDepth, unitStr);
    ImVec2 textSize = ImGui::CalcTextSize(depthStr);
    ImGui::SetCursorPosX((avail - textSize.x) * 0.5f);
    ImGui::TextColored(depthColor, "%s", depthStr);

    // Depth trend arrow
    float depthTrend = 0.0f;
    if (depthHistoryFull_ || depthHistoryIndex_ > 5) {
        int oldest = depthHistoryFull_
            ? (depthHistoryIndex_ + 1) % DEPTH_HISTORY_SIZE
            : 0;
        depthTrend = data_.depth - depthHistory_[oldest];
    }

    ImGui::SameLine();
    if (depthTrend > 0.1f) {
        ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "^"); // Deepening
    } else if (depthTrend < -0.1f) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0.3f, 1), "v"); // Shoaling
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "-"); // Steady
    }

    // Vertical depth gauge bar
    ImGui::Spacing();
    ImVec2 barPos = ImGui::GetCursorScreenPos();
    float barWidth = avail - 30;
    float barHeight = 60;

    // Determine scale range: show 0 to max(depth*2, alarm*3, 20)
    float maxScale = std::fmax(displayDepth * 2.0f, std::fmax(displayAlarm * 3.0f, depthUnitsMetric_ ? 20.0f : 60.0f));
    if (maxScale < 1.0f) maxScale = 1.0f;

    // Bar background
    draw->AddRectFilled(
        ImVec2(barPos.x + 15, barPos.y),
        ImVec2(barPos.x + 15 + barWidth, barPos.y + barHeight),
        IM_COL32(20, 30, 50, 200), 2.0f);

    // Depth fill (from top down)
    float depthFrac = std::fmin(displayDepth / maxScale, 1.0f);
    if (depthFrac > 0) {
        ImU32 fillCol = alarm ? IM_COL32(180, 40, 40, 180) : IM_COL32(40, 100, 180, 180);
        draw->AddRectFilled(
            ImVec2(barPos.x + 15, barPos.y),
            ImVec2(barPos.x + 15 + barWidth, barPos.y + barHeight * depthFrac),
            fillCol, 2.0f);
    }

    // Alarm threshold marker (horizontal dashed line)
    float alarmFrac = std::fmin(displayAlarm / maxScale, 1.0f);
    float alarmY = barPos.y + barHeight * alarmFrac;
    for (float x = barPos.x + 15; x < barPos.x + 15 + barWidth; x += 8) {
        draw->AddLine(
            ImVec2(x, alarmY),
            ImVec2(std::fmin(x + 4, barPos.x + 15 + barWidth), alarmY),
            IM_COL32(255, 80, 80, 200), 1.5f);
    }

    // Depth marker (current depth line)
    float depthY = barPos.y + barHeight * depthFrac;
    draw->AddLine(
        ImVec2(barPos.x + 12, depthY),
        ImVec2(barPos.x + 18 + barWidth, depthY),
        IM_COL32(255, 255, 0, 255), 2.0f);

    // Scale labels
    char topLabel[16], botLabel[16];
    snprintf(topLabel, sizeof(topLabel), "0");
    snprintf(botLabel, sizeof(botLabel), "%.0f", maxScale);
    draw->AddText(ImVec2(barPos.x + 15, barPos.y - 14), IM_COL32(150, 150, 150, 200), topLabel);
    draw->AddText(ImVec2(barPos.x + 15, barPos.y + barHeight + 2), IM_COL32(150, 150, 150, 200), botLabel);

    // Advance cursor past the bar
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + barHeight + 18);

    // Alarm threshold label
    char alarmStr[32];
    snprintf(alarmStr, sizeof(alarmStr), "Alarm: %.1f %s", displayAlarm, unitStr);
    ImVec4 alarmLabelColor = alarm ? ImVec4(1, 0.3f, 0.3f, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1);
    ImGui::TextColored(alarmLabelColor, "%s", alarmStr);

    // Unit toggle (click to switch)
    ImGui::SameLine(avail - 20);
    if (ImGui::SmallButton(depthUnitsMetric_ ? "m" : "ft")) {
        depthUnitsMetric_ = !depthUnitsMetric_;
    }

    ImGui::End();
}

// -- Engine Display ---------------------------------------------------

void ImGuiOverlay::renderEngineDisplay() {
    ImGui::SetNextWindowSize(ImVec2(140, 90), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(screenWidth_ - 150, 130), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_) flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Engine", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "RPM");
    ImGui::SameLine(70);
    ImGui::TextColored(ImVec4(0.3f, 1, 0.3f, 1), "%4.0f", data_.engineRPM);

    // RPM bar
    float rpmFrac = std::abs(data_.engineRPM) / 200.0f; // assume 200 RPM max
    if (rpmFrac > 1.0f) rpmFrac = 1.0f;
    ImGui::ProgressBar(rpmFrac, ImVec2(-1, 12), "");

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Thrust");
    ImGui::SameLine(70);
    ImGui::Text("%+.0f%%", data_.thrustLever * 100);

    ImGui::End();
}

// -- Wind Display -----------------------------------------------------

void ImGuiOverlay::renderWindDisplay() {
    ImGui::SetNextWindowSize(ImVec2(130, 70), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(10, 10), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    if (layoutLocked_) flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    if (!ImGui::Begin("Wind", nullptr, flags)) {
        ImGui::End();
        return;
    }

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "Speed");
    ImGui::SameLine(70);
    ImGui::Text("%.0f kn", data_.windSpeed);

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "From");
    ImGui::SameLine(70);
    ImGui::Text("%03.0f", data_.windDirection);

    ImGui::End();
}

// -- Visibility toggles -----------------------------------------------

void ImGuiOverlay::showCompass(bool show) { showCompass_ = show; }
void ImGuiOverlay::showSpeedDisplay(bool show) { showSpeed_ = show; }
void ImGuiOverlay::showRudderDisplay(bool show) { showRudder_ = show; }
void ImGuiOverlay::showDepthDisplay(bool show) { showDepth_ = show; }
void ImGuiOverlay::showEngineDisplay(bool show) { showEngine_ = show; }
void ImGuiOverlay::showWindDisplay(bool show) { showWind_ = show; }

void ImGuiOverlay::showAll(bool show) {
    showCompass_ = showSpeed_ = showRudder_ = showDepth_ = showEngine_ = showWind_ = show;
}

void ImGuiOverlay::setLayoutLocked(bool locked) { layoutLocked_ = locked; }

void ImGuiOverlay::setDepthUnitMetric(bool metric) { depthUnitsMetric_ = metric; }

// -- Keyboard shortcuts (7-06) ----------------------------------------

void ImGuiOverlay::processKeyboardShortcuts() {
    // F5 = Bright Day, F6 = Day, F7 = Dusk, F8 = Night
    if (ImGui::IsKeyPressed(ImGuiKey_F5, false)) setPalette(0);
    if (ImGui::IsKeyPressed(ImGuiKey_F6, false)) setPalette(1);
    if (ImGui::IsKeyPressed(ImGuiKey_F7, false)) setPalette(2);
    if (ImGui::IsKeyPressed(ImGuiKey_F8, false)) setPalette(3);

    // F9 = Toggle layout lock
    if (ImGui::IsKeyPressed(ImGuiKey_F9, false)) {
        layoutLocked_ = !layoutLocked_;
    }

    // F10 = Toggle depth units (m/ft)
    if (ImGui::IsKeyPressed(ImGuiKey_F10, false)) {
        depthUnitsMetric_ = !depthUnitsMetric_;
    }
}

// -- Palettes (7-06) --------------------------------------------------

void ImGuiOverlay::setPalette(int index) {
    if (index < 0 || index > 3) return;
    currentPalette_ = index;
    applyPalette();
}

void ImGuiOverlay::applyPalette() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.WindowBorderSize = 1.0f;

    ImVec4* colors = style.Colors;

    switch (currentPalette_) {
    case 0: // Bright Day - white backgrounds, high contrast
        colors[ImGuiCol_WindowBg]       = ImVec4(0.95f, 0.95f, 0.95f, 0.9f);
        colors[ImGuiCol_TitleBg]        = ImVec4(0.82f, 0.82f, 0.82f, 1.0f);
        colors[ImGuiCol_TitleBgActive]  = ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        colors[ImGuiCol_Text]           = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
        colors[ImGuiCol_Border]         = ImVec4(0.5f, 0.5f, 0.5f, 0.8f);
        colors[ImGuiCol_FrameBg]        = ImVec4(0.85f, 0.85f, 0.85f, 0.6f);
        colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.9f, 0.9f, 0.9f, 0.5f);
        colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.6f, 0.6f, 0.6f, 0.8f);
        colors[ImGuiCol_Button]         = ImVec4(0.8f, 0.8f, 0.8f, 0.7f);
        colors[ImGuiCol_ButtonHovered]  = ImVec4(0.7f, 0.7f, 0.7f, 0.8f);
        colors[ImGuiCol_ButtonActive]   = ImVec4(0.6f, 0.6f, 0.6f, 0.9f);
        colors[ImGuiCol_PlotHistogram]  = ImVec4(0.2f, 0.5f, 0.2f, 0.8f);
        break;

    case 1: // Day - dark background, standard brightness
        colors[ImGuiCol_WindowBg]       = ImVec4(0.2f, 0.2f, 0.22f, 0.92f);
        colors[ImGuiCol_TitleBg]        = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        colors[ImGuiCol_TitleBgActive]  = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);
        colors[ImGuiCol_Text]           = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        colors[ImGuiCol_Border]         = ImVec4(0.4f, 0.4f, 0.4f, 0.6f);
        colors[ImGuiCol_FrameBg]        = ImVec4(0.12f, 0.12f, 0.14f, 0.5f);
        colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.15f, 0.15f, 0.17f, 0.5f);
        colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.4f, 0.4f, 0.4f, 0.6f);
        colors[ImGuiCol_Button]         = ImVec4(0.25f, 0.25f, 0.28f, 0.7f);
        colors[ImGuiCol_ButtonHovered]  = ImVec4(0.35f, 0.35f, 0.38f, 0.8f);
        colors[ImGuiCol_ButtonActive]   = ImVec4(0.4f, 0.4f, 0.45f, 0.9f);
        colors[ImGuiCol_PlotHistogram]  = ImVec4(0.3f, 0.8f, 0.3f, 0.8f);
        break;

    case 2: // Dusk - reduced brightness, warm tones
        colors[ImGuiCol_WindowBg]       = ImVec4(0.12f, 0.11f, 0.10f, 0.95f);
        colors[ImGuiCol_TitleBg]        = ImVec4(0.08f, 0.07f, 0.06f, 1.0f);
        colors[ImGuiCol_TitleBgActive]  = ImVec4(0.14f, 0.12f, 0.10f, 1.0f);
        colors[ImGuiCol_Text]           = ImVec4(0.65f, 0.6f, 0.5f, 1.0f);
        colors[ImGuiCol_Border]         = ImVec4(0.3f, 0.25f, 0.2f, 0.5f);
        colors[ImGuiCol_FrameBg]        = ImVec4(0.08f, 0.07f, 0.06f, 0.5f);
        colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.1f, 0.09f, 0.08f, 0.5f);
        colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.3f, 0.25f, 0.2f, 0.5f);
        colors[ImGuiCol_Button]         = ImVec4(0.15f, 0.13f, 0.11f, 0.7f);
        colors[ImGuiCol_ButtonHovered]  = ImVec4(0.22f, 0.18f, 0.15f, 0.8f);
        colors[ImGuiCol_ButtonActive]   = ImVec4(0.28f, 0.22f, 0.18f, 0.9f);
        colors[ImGuiCol_PlotHistogram]  = ImVec4(0.5f, 0.4f, 0.2f, 0.8f);
        break;

    case 3: // Night - black background, red/amber only (preserves night vision)
        colors[ImGuiCol_WindowBg]       = ImVec4(0.02f, 0.01f, 0.01f, 0.98f);
        colors[ImGuiCol_TitleBg]        = ImVec4(0.04f, 0.01f, 0.01f, 1.0f);
        colors[ImGuiCol_TitleBgActive]  = ImVec4(0.08f, 0.02f, 0.02f, 1.0f);
        colors[ImGuiCol_Text]           = ImVec4(0.5f, 0.12f, 0.08f, 1.0f);
        colors[ImGuiCol_Border]         = ImVec4(0.15f, 0.04f, 0.04f, 0.6f);
        colors[ImGuiCol_FrameBg]        = ImVec4(0.04f, 0.01f, 0.01f, 0.5f);
        colors[ImGuiCol_ScrollbarBg]    = ImVec4(0.03f, 0.01f, 0.01f, 0.5f);
        colors[ImGuiCol_ScrollbarGrab]  = ImVec4(0.15f, 0.04f, 0.04f, 0.5f);
        colors[ImGuiCol_Button]         = ImVec4(0.08f, 0.02f, 0.02f, 0.7f);
        colors[ImGuiCol_ButtonHovered]  = ImVec4(0.12f, 0.03f, 0.03f, 0.8f);
        colors[ImGuiCol_ButtonActive]   = ImVec4(0.18f, 0.05f, 0.05f, 0.9f);
        colors[ImGuiCol_PlotHistogram]  = ImVec4(0.4f, 0.1f, 0.05f, 0.8f);
        break;
    }
}

// -- Layout save/load (7-07) ------------------------------------------

void ImGuiOverlay::setIniFilePath(const std::string& path) {
    iniFilePath_ = path;
    // ImGui uses a static const char* for IniFilename, so we store the string
    // and point ImGui at our persistent storage
    ImGui::GetIO().IniFilename = iniFilePath_.empty() ? nullptr : iniFilePath_.c_str();
}

void ImGuiOverlay::saveLayout(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;

    file << "[ImGuiOverlay]\n";
    file << "Palette=" << currentPalette_ << "\n";
    file << "LayoutLocked=" << (layoutLocked_ ? 1 : 0) << "\n";
    file << "DepthUnitsMetric=" << (depthUnitsMetric_ ? 1 : 0) << "\n";
    file << "ShowCompass=" << (showCompass_ ? 1 : 0) << "\n";
    file << "ShowSpeed=" << (showSpeed_ ? 1 : 0) << "\n";
    file << "ShowRudder=" << (showRudder_ ? 1 : 0) << "\n";
    file << "ShowDepth=" << (showDepth_ ? 1 : 0) << "\n";
    file << "ShowEngine=" << (showEngine_ ? 1 : 0) << "\n";
    file << "ShowWind=" << (showWind_ ? 1 : 0) << "\n";

    // Also trigger ImGui's own window position save
    if (!iniFilePath_.empty()) {
        ImGui::SaveIniSettingsToDisk(iniFilePath_.c_str());
    }
}

void ImGuiOverlay::loadLayout(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        // Skip section headers and empty lines
        if (line.empty() || line[0] == '[') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if (key == "Palette") { setPalette(std::stoi(val)); }
        else if (key == "LayoutLocked") { layoutLocked_ = (val == "1"); }
        else if (key == "DepthUnitsMetric") { depthUnitsMetric_ = (val == "1"); }
        else if (key == "ShowCompass") { showCompass_ = (val == "1"); }
        else if (key == "ShowSpeed") { showSpeed_ = (val == "1"); }
        else if (key == "ShowRudder") { showRudder_ = (val == "1"); }
        else if (key == "ShowDepth") { showDepth_ = (val == "1"); }
        else if (key == "ShowEngine") { showEngine_ = (val == "1"); }
        else if (key == "ShowWind") { showWind_ = (val == "1"); }
    }
}

bool ImGuiOverlay::wantsKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool ImGuiOverlay::wantsMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

}} // namespace bc::gui

#endif // WITH_WICKED_ENGINE
