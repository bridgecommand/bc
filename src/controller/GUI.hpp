/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#include <string>
#include <vector>

#include "irrlicht.h"

#include "../Lang.hpp"
#include "PositionDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "AISData.hpp"

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language);
    ~GUIMain();

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_WINDOW = 101,
        //GUI_ID_WEATHER_WINDOW,
        GUI_ID_SHIP_COMBOBOX,
        GUI_ID_LEG_LISTBOX,
        GUI_ID_MMSI_EDITBOX,
        GUI_ID_COURSE_EDITBOX,
        GUI_ID_SPEED_EDITBOX,
        GUI_ID_DISTANCE_EDITBOX,
        GUI_ID_CHANGE_BUTTON,
        GUI_ID_CHANGE_COURSESPEED_BUTTON,
        GUI_ID_ADDLEG_BUTTON,
        GUI_ID_SETMMSI_BUTTON,
        GUI_ID_DELETELEG_BUTTON,
        GUI_ID_MOVESHIP_BUTTON,
        GUI_ID_RELEASEMOB_BUTTON,
        GUI_ID_RETRIEVEMOB_BUTTON,
        GUI_ID_ZOOMIN_BUTTON,
        GUI_ID_ZOOMOUT_BUTTON,
        GUI_ID_RUDDERPUMP_1_WORKING_BUTTON,
        GUI_ID_RUDDERPUMP_1_FAILED_BUTTON,
        GUI_ID_RUDDERPUMP_2_WORKING_BUTTON,
        GUI_ID_RUDDERPUMP_2_FAILED_BUTTON,
        GUI_ID_FOLLOWUP_WORKING_BUTTON,
        GUI_ID_FOLLOWUP_FAILED_BUTTON,
        GUI_ID_WEATHER_SCROLLBAR,
        GUI_ID_RAIN_SCROLLBAR,
        GUI_ID_VISIBILITY_SCROLLBAR,
        GUI_ID_WINDDIRECTION_SCROLL_BAR,
        GUI_ID_WINDSPEED_SCROLL_BAR,
        GUI_ID_STREAMDIRECTION_SCROLL_BAR,
        GUI_ID_STREAMSPEED_SCROLL_BAR,
        GUI_ID_STREAMOVERRIDE_BOX,
        GUI_ID_BRIGHTNESS_SCROLLBAR
    };

    void updateGuiData(float time, int32_t mapOffsetX, int32_t mapOffsetZ, float metresPerPx, float ownShipPosX, float ownShipPosZ, float ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, const std::vector<AISData>& aisData, bool mobVisible, float mobPosX, float mobPosZ, irr::video::ITexture* displayMapTexture, int32_t selectedShip, int32_t selectedLeg, float terrainLong, float terrainLongExtent, float terrainXWidth, float terrainLat, float terrainLatExtent, float terrainZWidth, float weather, float visibility, float rain, float windDirection, float windSpeed, float streamDirection, float streamSpeed, bool streamOverride);
    void updateEditBoxes(); //Trigger an update of the edit boxes (carried out in next updateGuiData)
    float getEditBoxCourse() const;
    float getEditBoxSpeed() const;
    float getEditBoxDistance() const;
    uint32_t getEditBoxMMSI() const;
    int getSelectedShip() const;
    int getSelectedLeg() const;
    irr::core::vector2df getScreenCentrePosition() const;
    float getWeather() const;
    float getRain() const;
    float getVisibility() const;
    float getWindDirection() const;
    float getWindSpeed() const;
    float getStreamDirection() const;
    float getStreamSpeed() const;
    bool getStreamOverride() const;
    float getBrightnessScaling() const;

private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIWindow* guiWindow;

    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIStaticText* shipSelectorTitle;
    irr::gui::IGUIStaticText* legSelectorTitle;
    irr::gui::IGUIStaticText* courseTitle;
    irr::gui::IGUIStaticText* speedTitle;
    irr::gui::IGUIStaticText* distanceTitle;
    irr::gui::IGUIComboBox* shipSelector;
    irr::gui::IGUIListBox* legSelector;
    irr::gui::IGUIEditBox* legCourseEdit;
    irr::gui::IGUIEditBox* legSpeedEdit;
    irr::gui::IGUIEditBox* legDistanceEdit;
    irr::gui::IGUIEditBox* mmsiEdit;
    irr::gui::IGUIButton* changeLeg;
    irr::gui::IGUIButton* changeLegCourseSpeed;
    irr::gui::IGUIButton* addLeg;
    irr::gui::IGUIButton* deleteLeg;
    irr::gui::IGUIButton* moveShip;
    irr::gui::IGUIButton* setMMSI;
    irr::gui::IGUIScrollBar* weatherBar;
    irr::gui::IGUIScrollBar* rainBar;
    irr::gui::IGUIScrollBar* visibilityBar;
    irr::gui::IGUIButton* releaseMOB;
    irr::gui::IGUIButton* retrieveMOB;
    irr::gui::IGUIButton* zoomIn;
    irr::gui::IGUIButton* zoomOut;
    irr::gui::IGUIScrollBar* windDirectionBar;
    irr::gui::IGUIScrollBar* windSpeedBar;
    irr::gui::IGUIScrollBar* streamDirectionBar;
    irr::gui::IGUIScrollBar* streamSpeedBar;
    irr::gui::IGUICheckBox* streamOverrideBox;
    irr::gui::IGUIScrollBar* brightnessBar;
    float mapCentreX;
    float mapCentreZ;

    bool editBoxesNeedUpdating;

    void drawInformationOnMap(const float& time, const int32_t& mapOffsetX, const int32_t& mapOffsetZ, const float& metresPerPx, const float& ownShipPosX, const float& ownShipPosZ, const float& ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, const std::vector<AISData>& aisData, const int32_t& selectedShip, const int32_t& selectedLeg, const bool& mobVisible, const float& mobPosX, const float& mobPosZ);
    void updateDropDowns(const std::vector<OtherShipDisplayData>& otherShips, int32_t selectedShip, float time);
    bool manuallyTriggerGUIEvent(irr::gui::IGUIElement* caller, irr::gui::EGUI_EVENT_TYPE eType);
    std::wstring f32To3dp(float value);
    std::wstring f32To1dp(float value);

};

#endif


