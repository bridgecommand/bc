/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

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

#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include "irrlicht.h"
#include "Lang.hpp"
#include "OperatingModeEnum.hpp"
#include <string>

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language);
    ~GUIMain();

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR,
        GUI_ID_PORT_SCROLL_BAR,
        GUI_ID_STBD_SCROLL_BAR,
        GUI_ID_RUDDER_SCROLL_BAR,
        GUI_ID_START_BUTTON,
        GUI_ID_RADAR_INCREASE_BUTTON,
        GUI_ID_RADAR_DECREASE_BUTTON,
        GUI_ID_RADAR_GAIN_SCROLL_BAR,
        GUI_ID_RADAR_CLUTTER_SCROLL_BAR,
        GUI_ID_RADAR_RAIN_SCROLL_BAR,
        GUI_ID_RADAR_EBL_LEFT_BUTTON,
        GUI_ID_RADAR_EBL_RIGHT_BUTTON,
        GUI_ID_RADAR_EBL_UP_BUTTON,
        GUI_ID_RADAR_EBL_DOWN_BUTTON,
        GUI_ID_WEATHER_SCROLL_BAR,
        GUI_ID_RAIN_SCROLL_BAR,
        GUI_ID_VISIBILITY_SCROLL_BAR,
        GUI_ID_SHOW_INTERFACE_BUTTON,
        GUI_ID_HIDE_INTERFACE_BUTTON,
        GUI_ID_BINOS_INTERFACE_BUTTON,
        GUI_ID_BEARING_INTERFACE_BUTTON
    };

    bool getShowInterface() const;
    void toggleShow2dInterface();
    void show2dInterface();
    void hide2dInterface();
    void hideStbdEngineBar(); //Used for single engine operation
    void hideEngineAndRudder(); //Used for secondary mode
    void setInstruments(bool hasDepthSounder, irr::f32 maxSounderDepth, bool hasGPS);
    void updateGuiData(irr::f32 lat, irr::f32 longitude, irr::f32 hdg, irr::f32 viewAngle, irr::f32 viewElevationAngle, irr::f32 spd, irr::f32 portEng, irr::f32 stbdEng, irr::f32 rudder, irr::f32 depth, irr::f32 weather, irr::f32 rain, irr::f32 visibility, irr::f32 radarRangeNm, irr::f32 radarGain, irr::f32 radarClutter, irr::f32 radarRain, irr::f32 guiRadarEBLBrg, irr::f32 guiRadarEBLRangeNm, std::string currentTime, bool paused,  bool collided);
    void drawGUI();


private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIScrollBar* spdScrollbar;
    irr::gui::IGUIScrollBar* portScrollbar;
    irr::gui::IGUIScrollBar* stbdScrollbar;
    irr::gui::IGUIScrollBar* rudderScrollbar;
    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIStaticText* radarText;
    irr::gui::IGUIButton* pausedButton;
    irr::gui::IGUIButton* eblLeftButton;
    irr::gui::IGUIButton* eblRightButton;
    irr::gui::IGUIButton* eblUpButton;
    irr::gui::IGUIButton* eblDownButton;
    irr::gui::IGUITabControl* radarTabControl;
    irr::gui::IGUIButton* increaseRangeButton;
    irr::gui::IGUIButton* decreaseRangeButton;
    irr::gui::IGUIScrollBar* radarGainScrollbar;
    irr::gui::IGUIScrollBar* radarClutterScrollbar;
    irr::gui::IGUIScrollBar* radarRainScrollbar;
    irr::gui::IGUIScrollBar* visibilityScrollbar;
    irr::gui::IGUIScrollBar* weatherScrollbar;
    irr::gui::IGUIScrollBar* rainScrollbar;
    irr::gui::IGUIButton* showInterfaceButton;
    irr::gui::IGUIButton* hideInterfaceButton;
    irr::gui::IGUIButton* binosButton;
    irr::gui::IGUIButton* bearingButton;

    irr::f32 guiLat;
    irr::f32 guiLong;
    irr::f32 guiHeading;
    irr::f32 viewHdg;
    irr::f32 viewElev;
    irr::f32 guiSpeed;
    irr::f32 guiDepth;
    irr::f32 guiRadarRangeNm;
    irr::f32 guiRadarGain;
    irr::f32 guiRadarClutter;
    irr::f32 guiRadarRain;
    irr::f32 guiRadarEBLBrg;
    irr::f32 guiRadarEBLRangeNm;
    std::string guiTime;
    bool guiPaused;
    bool guiCollided;
    bool showInterface;

    bool hasDepthSounder;
    irr::f32 maxSounderDepth;
    bool hasGPS;

    Lang* language;

    void updateVisibility();
    void draw2dRadar();
    void draw2dBearing();
    void drawCollisionWarning();
    std::wstring f32To1dp(irr::f32 value);
    std::wstring f32To2dp(irr::f32 value);
    std::wstring f32To3dp(irr::f32 value);
    bool manuallyTriggerClick(irr::gui::IGUIButton* button);

};

#endif

