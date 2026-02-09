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
#include "AzimuthDial.h"
#include "GUIRectangle.hpp"
#include "RadarCalculation.hpp"
#include <vector>
#include <string>

// Forward declarations
class SimulationModel;

struct GUIData {
    float lat;
    float longitude;
    float hdg;
    float viewAngle;
    float viewElevationAngle;
    float spd; // Show speed through water
    float portEng;
    float stbdEng;
    float rudder;
    float bowThruster;
    float sternThruster;
    float wheel;
    float portAzimuthAngle;
    float stbdAzimuthAngle;
    bool azimuth1Master;
    bool azimuth2Master;
    float RateOfTurn;
    float depth;
    float weather;
    float rain;
    float visibility;
    float windDirection;
    float windSpeed;
    float streamDirection;
    float streamSpeed;
    bool streamOverride;
    bool radarOn;
    float radarRangeNm;
    float radarGain;
    float radarClutter;
    float radarRain;
    float guiRadarEBLBrg;
    float guiRadarEBLRangeNm;
    float guiRadarCursorBrg;
    float guiRadarCursorRangeNm;
    int32_t arpaListSelection;
    std::vector<ARPAEstimatedState> arpaContactStates;
	std::string currentTime;
    bool paused;
    bool collided;
    bool headUp;
    bool pump1On;
    bool pump2On;
// DEE_NOV22 Azimuth Drive related gui items
    float schottelPort; // angle of the schottels +ve clockwise 0 dead ahead
    float schottelStbd;
    float thrustLeverPort; // thrust levers (0..1) leave this in here for future graphical lever
    float thrustLeverStbd;
    float azimuthEnginePort;
    float azimuthEngineStbd;
    float emergencySteering;
    bool azimuthClutchPort;  // Clutches true for engaged false for disengaged
    bool azimuthClutchStbd;

    // the angle of each azimuth drive and each engine level is defined elsewhere
    // DEE_NOV22 some indication and or switch from normal steering to non follow up emergency steering

    float tideHeight; // DEE FEB 23
};

class GUIMain //Create, build and update GUI
{
public:
    GUIMain();
    ~GUIMain();
    void load(irr::IrrlichtDevice* device, Lang* language, std::vector<std::string>* logMessages, SimulationModel* model, bool singleEngine, bool azimuthDrive, bool controlsHidden, bool hasDepthSounder, float maxSounderDepth, bool hasGPS, bool showTideHeight, bool hasBowThruster, bool hasSternThruster, bool hasRateOfTurnIndicator, bool showCollided, bool vr3dMode);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR,
        GUI_ID_PORT_SCROLL_BAR,
        GUI_ID_STBD_SCROLL_BAR,
        GUI_ID_AZIMUTH_1,
        GUI_ID_AZIMUTH_2,
        GUI_ID_AZIMUTH_1_MASTER_CHECKBOX,
        GUI_ID_AZIMUTH_2_MASTER_CHECKBOX,
// DEE_NOV22 vvvv
	GUI_ID_SCHOTTEL_PORT,
	GUI_ID_SCHOTTEL_STBD,
	GUI_ID_AZIMUTH_ENGINE_PORT,
	GUI_ID_AZIMUTH_ENGINE_STBD,
	GUI_ID_AZIMUTH_CLUTCH_PORT,
	GUI_ID_AZIMUTH_CLUTCH_STBD,
	GUI_ID_EMERGENCY_STEERING,
// DEE_NOV22 ^^^^
        GUI_ID_RUDDER_SCROLL_BAR,
// DEE vvvv
	GUI_ID_WHEEL_SCROLL_BAR,
        GUI_ID_RATE_OF_TURN_SCROLL_BAR,
	GUI_ID_RATE_OF_TURN_INDICATOR,
// DEE ^^^^
        GUI_ID_BOWTHRUSTER_SCROLL_BAR,
        GUI_ID_STERNTHRUSTER_SCROLL_BAR,
        GUI_ID_START_BUTTON,
		GUI_ID_RADAR_ONOFF_BUTTON,
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
        GUI_ID_RADAR_INCREASE_X_BUTTON,
        GUI_ID_RADAR_DECREASE_X_BUTTON,
        GUI_ID_RADAR_INCREASE_Y_BUTTON,
        GUI_ID_RADAR_DECREASE_Y_BUTTON,
        GUI_ID_RADAR_NORTH_BUTTON,
        GUI_ID_RADAR_COURSE_BUTTON,
        GUI_ID_RADAR_HEAD_BUTTON,
        GUI_ID_RADAR_COLOUR_BUTTON,
        GUI_ID_NFU_PORT_BUTTON,
        GUI_ID_NFU_STBD_BUTTON,
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
        GUI_ID_ARPA_LIST,
        GUI_ID_MANUAL_SCAN_BUTTON,
        GUI_ID_MANUAL_NEW_BUTTON,
        GUI_ID_MANUAL_CLEAR_BUTTON,
        GUI_ID_BIG_ARPA_LIST,
        GUI_ID_WEATHER_SCROLL_BAR,
        GUI_ID_RAIN_SCROLL_BAR,
        GUI_ID_VISIBILITY_SCROLL_BAR,
        GUI_ID_WINDDIRECTION_SCROLL_BAR,
        GUI_ID_WINDSPEED_SCROLL_BAR,
        GUI_ID_STREAMDIRECTION_SCROLL_BAR,
        GUI_ID_STREAMSPEED_SCROLL_BAR,
        GUI_ID_STREAMOVERRIDE_BOX,
        GUI_ID_MAGNIFICATION_SCROLL_BAR,
        GUI_ID_SHOW_INTERFACE_BUTTON,
        GUI_ID_HIDE_INTERFACE_BUTTON,
        GUI_ID_BINOS_INTERFACE_BUTTON,
        GUI_ID_BEARING_INTERFACE_BUTTON,
        GUI_ID_SHOW_LOG_BUTTON,
        GUI_ID_SHOW_EXTRA_CONTROLS_BUTTON,
        GUI_ID_HIDE_EXTRA_CONTROLS_BUTTON,
        GUI_ID_SHOW_LINES_CONTROLS_BUTTON,
        GUI_ID_HIDE_LINES_CONTROLS_BUTTON,
        GUI_ID_RUDDERPUMP_1_WORKING_BUTTON,
        GUI_ID_RUDDERPUMP_1_FAILED_BUTTON,
        GUI_ID_RUDDERPUMP_2_WORKING_BUTTON,
        GUI_ID_RUDDERPUMP_2_FAILED_BUTTON,
        GUI_ID_FOLLOWUP_WORKING_BUTTON,
        GUI_ID_FOLLOWUP_FAILED_BUTTON,
        GUI_ID_ACK_ALARMS_BUTTON,
        GUI_ID_ADD_LINE_BUTTON,
        GUI_ID_REMOVE_LINE_BUTTON,
        GUI_ID_KEEP_SLACK_LINE_CHECKBOX,
        GUI_ID_HAUL_IN_LINE_CHECKBOX,
        GUI_ID_ANCHOR_LINE_CHECKBOX,
        GUI_ID_LINES_LIST,
        GUI_ID_CHANGE_VIEW_BUTTON,
        GUI_ID_EXIT_BUTTON,
        GUI_ID_CLOSE_BOX
    };

    bool getShowInterface() const;
    void toggleShow2dInterface();
    void show2dInterface();
    void hide2dInterface();
    bool getShow3d() const;
    void zoomOn();
    void zoomOff();
    void toggleBearings();
    void showBearings();
    void hideBearings();
    void setLargeRadar(bool radarState);
    bool getLargeRadar() const;
    void setARPAComboboxes(int32_t arpaState);
    void setARPAList(int arpaSelected);
    uint32_t getRadarPixelRadius() const;
    irr::core::vector2di getCursorPositionRadar() const;
    irr::core::rect<int32_t> getSmallRadarRect() const;
    irr::core::rect<int32_t> getLargeRadarRect() const;
    bool isNFUActive() const;
    void setSingleEngine(); //Used for single engine operation
    void hideEngineAndRudder(); //Used for secondary mode
//    void setInstruments(bool hasDepthSounder, float maxSounderDepth, bool hasGPS);
    void updateGuiData(GUIData* guiData);
    void showLogWindow();
    void drawGUI();
    void setExtraControlsWindowVisible(bool windowVisible);
    void setLinesControlsWindowVisible(bool windowVisible);
    void setLinesControlsText(std::string textToShow);
    bool getAnchorLine() const;

private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIScrollBar* spdScrollbar;
    irr::gui::IGUIScrollBar* portScrollbar;
    irr::gui::IGUIScrollBar* stbdScrollbar;
    irr::gui::OutlineScrollBar* wheelScrollbar;
    irr::gui::IGUIScrollBar* bowThrusterScrollbar;
    irr::gui::IGUIScrollBar* sternThrusterScrollbar;
    irr::gui::IGUIStaticText* portText;
    irr::gui::IGUIStaticText* stbdText;
    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIStaticText* radarText;
    irr::gui::IGUIScrollBar* rateofturnScrollbar;

    irr::gui::AzimuthDial* azimuth1Control;
    irr::gui::AzimuthDial* azimuth2Control;

    irr::gui::IGUICheckBox* azimuth1Master;
    irr::gui::IGUICheckBox* azimuth2Master;

    // DEE_NOV22 vvvv
    irr::gui::AzimuthDial* azimuthEnginePort;
    irr::gui::AzimuthDial* azimuthEngineStbd;
    irr::gui::AzimuthDial* schottelPort;
    irr::gui::AzimuthDial* schottelStbd;
    irr::gui::IGUICheckBox* azimuthClutchPort;
    irr::gui::IGUICheckBox* azimuthClutchStbd;
    irr::gui::IGUICheckBox* emergencySteering;


    // DEE_NOV22 ^^^^

    irr::gui::IGUIListBox* arpaList;
    irr::gui::IGUIListBox* arpaText;
    irr::gui::IGUIButton* pausedButton;
    irr::gui::IGUIButton* bigRadarButton;
    irr::gui::IGUIButton* smallRadarButton;
	irr::gui::IGUIButton* radarOnOffButton;
    irr::gui::IGUIButton* eblLeftButton;
    irr::gui::IGUIButton* eblRightButton;
    irr::gui::IGUIButton* eblUpButton;
    irr::gui::IGUIButton* eblDownButton;
    irr::gui::IGUIButton* radarCursorLeftButton;
    irr::gui::IGUIButton* radarCursorRightButton;
    irr::gui::IGUIButton* radarCursorUpButton;
    irr::gui::IGUIButton* radarCursorDownButton;
    irr::gui::IGUIButton* radarCursorLeftButton2;
    irr::gui::IGUIButton* radarCursorRightButton2;
    irr::gui::IGUIButton* radarCursorUpButton2;
    irr::gui::IGUIButton* radarCursorDownButton2;
    irr::gui::IGUIButton* radarColourButton;
    irr::gui::IGUIButton* radarColourButton2;
    irr::gui::IGUIButton* nonFollowUpPortButton;
    irr::gui::IGUIButton* nonFollowUpStbdButton;
    irr::gui::IGUIButton* changeViewButton;

    irr::gui::IGUITabControl* radarTabControl;
    irr::gui::IGUIScrollBar* radarGainScrollbar;
    irr::gui::IGUIScrollBar* radarClutterScrollbar;
    irr::gui::IGUIScrollBar* radarRainScrollbar;

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
    irr::gui::IGUIListBox* arpaList2;
    irr::gui::IGUIListBox* arpaText2;

    irr::gui::IGUIScrollBar* visibilityScrollbar;
    irr::gui::IGUIScrollBar* weatherScrollbar;
    irr::gui::IGUIScrollBar* rainScrollbar;
    irr::gui::IGUIScrollBar* windDirectionScrollbar;
    irr::gui::IGUIScrollBar* windSpeedScrollbar;
    irr::gui::IGUIScrollBar* streamDirectionScrollbar;
    irr::gui::IGUIScrollBar* streamSpeedScrollbar;
    irr::gui::IGUICheckBox* streamOverride;
    irr::gui::HeadingIndicator* headingIndicator;

    irr::gui::IGUIScrollBar* magnificationScrollbar;
    irr::gui::IGUICheckBox* show3d;

    irr::gui::IGUIButton* showInterfaceButton;
    irr::gui::IGUIButton* hideInterfaceButton;
    irr::gui::IGUIButton* binosButton;
    irr::gui::IGUIButton* bearingButton;
    irr::gui::IGUIButton* exitButton;
    irr::gui::IGUIButton* pcLogButton;
    irr::gui::IGUIButton* showExtraControlsButton;
    irr::gui::IGUIButton* showLinesControlsButton;

    irr::gui::IGUIButton* addLine;
    irr::gui::IGUIButton* removeLine;
    irr::gui::IGUICheckBox* keepLineSlack;
    irr::gui::IGUICheckBox* heaveLineIn;
    irr::gui::IGUICheckBox* anchorLine;
    irr::gui::IGUIListBox* linesList;
    irr::gui::IGUIStaticText* linesText;

    irr::gui::IGUIStaticText* pump1On;
    irr::gui::IGUIStaticText* pump2On;
    irr::gui::IGUIButton* ackAlarms;

    irr::gui::IGUIStaticText* clickForRudderText;
    irr::gui::IGUIStaticText* clickForEngineText;

    irr::gui::IGUIWindow* extraControlsWindow;
    irr::gui::IGUIWindow* linesControlsWindow;

    uint32_t su;
    uint32_t sh;

    int32_t azimuthGUIOffsetL;
    int32_t azimuthGUIOffsetR;

    float guiLat;
    float guiLong;
    float guiHeading;
    float viewHdg;
    float viewElev;
    float guiSpeed;
    float guiDepth;
    float guiTideHeight;
    bool guiRadarOn;
    float guiRadarRangeNm;
    float guiRadarGain;
    float guiRadarClutter;
    float guiRadarRain;
    float guiRadarEBLBrg;
    float guiRadarEBLRangeNm;
    float guiRadarCursorBrg;
    float guiRadarCursorRangeNm;
    bool radarHeadUp;
    bool radarLarge;
    irr::core::rect<int32_t> radarLargeRect;
    int32_t largeRadarScreenCentreX;
    int32_t largeRadarScreenCentreY;
    int32_t largeRadarScreenRadius;
    int32_t smallRadarScreenCentreX;
    int32_t smallRadarScreenCentreY;
    int32_t smallRadarScreenRadius;
    std::vector<ARPAEstimatedState> arpaContactStates;
    std::string guiTime;
    bool singleEngine;
    bool azimuthDrive;
    bool hasBowThruster;
    bool hasSternThruster;
    bool hasRateOfTurnIndicator;
    bool guiPaused;
    bool guiCollided;
    bool showInterface;
    bool controlsHidden; //If controls should always be hidden (if a secondary screen etc)

    bool hasDepthSounder;
    float maxSounderDepth;
    bool hasGPS;
    bool showTideHeight;
    bool showCollided;

    Lang* language;
    std::vector<std::string>* logMessages;
    SimulationModel* model;

    //Different locations for heading indicator depending on GUI visibility
    irr::core::rect<int32_t> stdHdgIndicatorPos;
    irr::core::rect<int32_t> radHdgIndicatorPos;
    irr::core::rect<int32_t> maxHdgIndicatorPos;

    irr::core::rect<int32_t> stdDataDisplayPos;
    irr::core::rect<int32_t> radDataDisplayPos;
    irr::core::rect<int32_t> altDataDisplayPos;
    irr::video::SColor stdDataDisplayBG;
    irr::video::SColor altDataDisplayBG;
    irr::video::SColor radDataDisplayBG;

    irr::core::rect<int32_t> stdRateOfTurnIndicatorPos;

    bool nfuPortDown;
    bool nfuStbdDown;

    void updateVisibility();
    void hideInSecondary();
    void draw2dRadar();
    void draw2dBearing();
    void drawCollisionWarning();
    std::wstring f32To1dp(float value);
    std::wstring f32To2dp(float value);
    std::wstring f32To3dp(float value);
    bool manuallyTriggerClick(irr::gui::IGUIButton* button);
    bool manuallyTriggerScroll(irr::gui::IGUIScrollBar* bar);

};

#endif

