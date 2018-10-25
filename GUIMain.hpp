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
#include "HeadingIndicator.h"
//#include "RateOfTurnIndicator.h" // DEE addition
#include "OutlineScrollBar.h"
#include "GUIRectangle.hpp"
#include <vector>
#include <string>

struct GUIData {
    irr::f32 lat;
    irr::f32 longitude;
    irr::f32 hdg;
    irr::f32 viewAngle;
    irr::f32 viewElevationAngle;
    irr::f32 spd;
    irr::f32 portEng;
    irr::f32 stbdEng;
    irr::f32 rudder;
// DEE vvvv
    irr::f32 wheel;
    irr::f32 RateOfTurn;
// DEE ^^^
    irr::f32 depth;
    irr::f32 weather;
    irr::f32 rain;
    irr::f32 visibility;
    irr::f32 radarRangeNm;
    irr::f32 radarGain;
    irr::f32 radarClutter;
    irr::f32 radarRain;
    irr::f32 guiRadarEBLBrg;
    irr::f32 guiRadarEBLRangeNm;
    std::vector<irr::f32> CPAs;
    std::vector<irr::f32> TCPAs;
	std::vector<irr::f32> headings;
	std::vector<irr::f32> speeds;
	std::string currentTime;
    bool paused;
    bool collided;
    bool headUp;
};

class GUIMain //Create, build and update GUI
{
public:
    GUIMain();
    ~GUIMain();
    void load(irr::IrrlichtDevice* device, Lang* language, std::vector<std::string>* logMessages, bool singleEngine, bool controlsHidden, bool hasDepthSounder, irr::f32 maxSounderDepth, bool hasGPS, bool hasBowThruster, bool hasSternThruster, bool hasRateOfTurnIndicator);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR,
        GUI_ID_PORT_SCROLL_BAR,
        GUI_ID_STBD_SCROLL_BAR,
        GUI_ID_RUDDER_SCROLL_BAR,
// DEE vvvv
	GUI_ID_WHEEL_SCROLL_BAR,
        GUI_ID_RATE_OF_TURN_SCROLL_BAR,
	GUI_ID_RATE_OF_TURN_INDICATOR,
// DEE ^^^^
        GUI_ID_BOWTHRUSTER_SCROLL_BAR,
        GUI_ID_STERNTHRUSTER_SCROLL_BAR,
        GUI_ID_START_BUTTON,
        GUI_ID_BIG_RADAR_BUTTON,
        GUI_ID_SMALL_RADAR_BUTTON,
        GUI_ID_RADAR_INCREASE_BUTTON,
        GUI_ID_RADAR_DECREASE_BUTTON,
        GUI_ID_RADAR_GAIN_SCROLL_BAR,
        GUI_ID_RADAR_CLUTTER_SCROLL_BAR,
        GUI_ID_RADAR_RAIN_SCROLL_BAR,
        GUI_ID_RADAR_EBL_LEFT_BUTTON,
        GUI_ID_RADAR_EBL_RIGHT_BUTTON,
        GUI_ID_RADAR_EBL_UP_BUTTON,
        GUI_ID_RADAR_EBL_DOWN_BUTTON,
        GUI_ID_RADAR_NORTH_BUTTON,
        GUI_ID_RADAR_COURSE_BUTTON,
        GUI_ID_RADAR_HEAD_BUTTON,
        GUI_ID_PI_SELECT_BOX,
        GUI_ID_PI_RANGE_BOX,
        GUI_ID_PI_BEARING_BOX,
        GUI_ID_BIG_PI_SELECT_BOX,
        GUI_ID_BIG_PI_RANGE_BOX,
        GUI_ID_BIG_PI_BEARING_BOX,
        GUI_ID_ARPA_ON_BOX,
        GUI_ID_BIG_ARPA_ON_BOX,
        GUI_ID_ARPA_TRUE_REL_BOX,
        GUI_ID_ARPA_VECTOR_TIME_BOX,
        GUI_ID_BIG_ARPA_TRUE_REL_BOX,
        GUI_ID_BIG_ARPA_VECTOR_TIME_BOX,
        GUI_ID_WEATHER_SCROLL_BAR,
        GUI_ID_RAIN_SCROLL_BAR,
        GUI_ID_VISIBILITY_SCROLL_BAR,
        GUI_ID_SHOW_INTERFACE_BUTTON,
        GUI_ID_HIDE_INTERFACE_BUTTON,
        GUI_ID_BINOS_INTERFACE_BUTTON,
        GUI_ID_BEARING_INTERFACE_BUTTON,
        GUI_ID_SHOW_LOG_BUTTON,
        GUI_ID_CLOSE_BOX
    };

    bool getShowInterface() const;
    void toggleShow2dInterface();
    void show2dInterface();
    void hide2dInterface();
    void setLargeRadar(bool radarState);
    bool getLargeRadar() const;
    irr::u32 getRadarPixelRadius() const;
    irr::core::vector2di getCursorPositionRadar() const;
    irr::core::rect<irr::s32> getLargeRadarRect() const;
    void setSingleEngine(); //Used for single engine operation
    void hideEngineAndRudder(); //Used for secondary mode
//    void setInstruments(bool hasDepthSounder, irr::f32 maxSounderDepth, bool hasGPS);
    void updateGuiData(GUIData* guiData);
    void showLogWindow();
    void drawGUI();

private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIScrollBar* spdScrollbar;
    irr::gui::IGUIScrollBar* portScrollbar;
    irr::gui::IGUIScrollBar* stbdScrollbar;
//    irr::gui::IGUIScrollBar* rudderScrollbar;

// DEE vvvvv
    irr::gui::OutlineScrollBar* wheelScrollbar;
// DEE ^^^^^

    irr::gui::IGUIScrollBar* bowThrusterScrollbar;
    irr::gui::IGUIScrollBar* sternThrusterScrollbar;
    irr::gui::IGUIStaticText* portText;
    irr::gui::IGUIStaticText* stbdText;
    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIStaticText* radarText;
// DEE vvvvv
    //irr::gui::IGUIStaticText* rudderText;
    //irr::gui::IGUIStaticText* wheelText;
    irr::gui::IGUIScrollBar* rateofturnScrollbar;

// DEE ^^^^^
    irr::gui::IGUIListBox* arpaText;
    irr::gui::IGUIButton* pausedButton;
    irr::gui::IGUIButton* bigRadarButton;
    irr::gui::IGUIButton* smallRadarButton;
    irr::gui::IGUIButton* eblLeftButton;
    irr::gui::IGUIButton* eblRightButton;
    irr::gui::IGUIButton* eblUpButton;
    irr::gui::IGUIButton* eblDownButton;
    irr::gui::IGUITabControl* radarTabControl;
    irr::gui::IGUIScrollBar* radarGainScrollbar;
    irr::gui::IGUIScrollBar* radarClutterScrollbar;
    irr::gui::IGUIScrollBar* radarRainScrollbar;

    //irr::gui::IGUIElement* largeRadarGUIParent; //Parent to make it easier to show/hide large radar gui
    irr::gui::IGUIRectangle* largeRadarControls; //Parent rectangle for large radar controls
    irr::gui::IGUIRectangle* largeRadarPIControls; //Parent for PI controls on large radar
    irr::gui::IGUIScrollBar* radarGainScrollbar2; //For large radar
    irr::gui::IGUIScrollBar* radarClutterScrollbar2; //For large radar
    irr::gui::IGUIScrollBar* radarRainScrollbar2; //For large radar
    irr::gui::IGUIButton* eblLeftButton2;
    irr::gui::IGUIButton* eblRightButton2;
    irr::gui::IGUIButton* eblUpButton2;
    irr::gui::IGUIButton* eblDownButton2;
    irr::gui::IGUIStaticText* radarText2;
    irr::gui::IGUIListBox* arpaText2;

    irr::gui::IGUIScrollBar* visibilityScrollbar;
    irr::gui::IGUIScrollBar* weatherScrollbar;
    irr::gui::IGUIScrollBar* rainScrollbar;
    irr::gui::HeadingIndicator* headingIndicator;

// DEE vvv this is the larger rot indicator
//    irr::gui::RateOfTurnIndicator* rateofturnIndicator;

// DEE ^^^

    irr::gui::IGUIButton* showInterfaceButton;
    irr::gui::IGUIButton* hideInterfaceButton;
    irr::gui::IGUIButton* binosButton;
    irr::gui::IGUIButton* bearingButton;
    irr::gui::IGUIButton* pcLogButton;

    irr::u32 su;
    irr::u32 sh;

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
    bool radarHeadUp;
    bool radarLarge;
    irr::core::rect<irr::s32> radarLargeRect;
    irr::s32 largeRadarScreenCentreX;
    irr::s32 largeRadarScreenCentreY;
    irr::s32 largeRadarScreenRadius;
    irr::s32 smallRadarScreenCentreX;
    irr::s32 smallRadarScreenCentreY;
    irr::s32 smallRadarScreenRadius;
    std::vector<irr::f32> guiCPAs;
    std::vector<irr::f32> guiTCPAs; //Time to CPA in minutes
	std::vector<irr::f32> guiARPAheadings;
	std::vector<irr::f32> guiARPAspeeds; //in knots
    std::string guiTime;
    bool singleEngine;
    bool hasBowThruster;
    bool hasSternThruster;
    bool hasRateOfTurnIndicator;
    bool guiPaused;
    bool guiCollided;
    bool showInterface;

    bool hasDepthSounder;
    irr::f32 maxSounderDepth;
    bool hasGPS;

    Lang* language;
    std::vector<std::string>* logMessages;

    //Different locations for heading indicator depending on GUI visibility
    irr::core::rect<irr::s32> stdHdgIndicatorPos;
    irr::core::rect<irr::s32> altHdgIndicatorPos;


    //DEE defines the position of the rate of turn indicator
    irr::core::rect<irr::s32> stdRateOfTurnIndicatorPos;


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

