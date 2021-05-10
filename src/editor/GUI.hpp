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
#include "OwnShipDataStruct.hpp"
#include "GeneralDataStruct.hpp"

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language, std::vector<std::string> ownShipTypes, std::vector<std::string> otherShipTypes, bool multiplayer);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_SHIP_COMBOBOX = 101,
        GUI_ID_LEG_LISTBOX,
        GUI_ID_MMSI_EDITBOX,
        GUI_ID_COURSE_EDITBOX,
        GUI_ID_SPEED_EDITBOX,
        GUI_ID_DISTANCE_EDITBOX,
        GUI_ID_ZOOMIN_BUTTON,
        GUI_ID_ZOOMOUT_BUTTON,
        GUI_ID_CHANGE_BUTTON,
        GUI_ID_SETMMSI_BUTTON,
//        GUI_ID_CHANGE_COURSESPEED_BUTTON,
        GUI_ID_ADDSHIP_BUTTON,
		GUI_ID_DELETESHIP_BUTTON,
        GUI_ID_ADDLEG_BUTTON,
        GUI_ID_DELETELEG_BUTTON,
        GUI_ID_MOVESHIP_BUTTON,
        GUI_ID_STARTHOURS_EDITBOX,
        GUI_ID_STARTMINS_EDITBOX,
        GUI_ID_STARTDAY_EDITBOX,
        GUI_ID_STARTMONTH_EDITBOX,
        GUI_ID_STARTYEAR_EDITBOX,
        GUI_ID_SUNRISE_EDITBOX,
        GUI_ID_SUNSET_EDITBOX,
        GUI_ID_WEATHER_COMBOBOX,
        GUI_ID_VISIBILITY_COMBOBOX,
        GUI_ID_RAIN_COMBOBOX,
        GUI_ID_SCENARIONAME_EDITBOX,
        GUI_ID_DESCRIPTION_EDITBOX,
        GUI_ID_APPLY_BUTTON,
        GUI_ID_SAVE_BUTTON,
        GUI_ID_OWNSHIPSELECT_COMBOBOX,
        GUI_ID_OTHERSHIPSELECT_COMBOBOX
    };

    void updateGuiData(GeneralData scenarioInfo, irr::s32 mapOffsetX, irr::s32 mapOffsetZ, irr::f32 metresPerPx, const OwnShipEditorData& ownShipData, const std::vector<PositionData>& buoys, const std::vector<OtherShipEditorData>& otherShips, irr::video::ITexture* displayMapTexture, irr::s32 selectedShip, irr::s32 selectedLeg, irr::f32 terrainLong, irr::f32 terrainLongExtent, irr::f32 terrainXWidth, irr::f32 terrainLat, irr::f32 terrainLatExtent, irr::f32 terrainZWidth);
    void updateEditBoxes(); //Trigger an update of the edit boxes (carried out in next updateGuiData)
    irr::f32 getEditBoxCourse() const;
    irr::f32 getEditBoxSpeed() const;
    irr::f32 getEditBoxDistance() const;
    int getSelectedShip() const;
    int getSelectedLeg() const;
    std::string getOwnShipTypeSelected() const;
    std::string getOtherShipTypeSelected() const;
    irr::f32 getStartTime() const;
    irr::u32 getStartDay() const;
    irr::u32 getStartMonth() const;
    irr::u32 getStartYear() const;
    irr::f32 getSunRise() const;
    irr::f32 getSunSet() const;
    irr::f32 getWeather() const;
    irr::f32 getRain() const;
    irr::f32 getVisibility() const;
    irr::u32 getEditBoxMMSI() const;
    std::string getScenarioName() const;
    std::string getDescription() const;
    irr::core::vector2df getScreenCentrePosition() const;


private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIButton* zoomIn;
    irr::gui::IGUIButton* zoomOut;

    irr::gui::IGUIWindow* guiWindow;
    irr::gui::IGUIWindow* generalDataWindow;

    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIEditBox* descriptionEdit;
    irr::gui::IGUIComboBox* shipSelector;
    irr::gui::IGUIListBox* legSelector;
    irr::gui::IGUIEditBox* legCourseEdit;
    irr::gui::IGUIEditBox* legSpeedEdit;
    irr::gui::IGUIEditBox* legDistanceEdit;
    irr::gui::IGUIEditBox* mmsiEdit;
    irr::gui::IGUIButton* changeLeg;
    //irr::gui::IGUIButton* changeLegCourseSpeed;
    irr::gui::IGUIButton* addShip;
	irr::gui::IGUIButton* deleteShip;
    irr::gui::IGUIButton* addLeg;
    irr::gui::IGUIButton* deleteLeg;
    irr::gui::IGUIButton* moveShip;
    irr::gui::IGUIButton* setMMSI;

    irr::gui::IGUIComboBox* ownShipTypeSelector;
    irr::gui::IGUIComboBox* otherShipTypeSelector;

    irr::gui::IGUIEditBox* startHours;
    irr::gui::IGUIEditBox* startMins;
    irr::gui::IGUIEditBox* startDay;
    irr::gui::IGUIEditBox* startMonth;
    irr::gui::IGUIEditBox* startYear;
    irr::gui::IGUIEditBox* sunRise;
    irr::gui::IGUIEditBox* sunSet;
    irr::gui::IGUIComboBox* weather;
    irr::gui::IGUIComboBox* visibility;
    irr::gui::IGUIComboBox* rain;
    irr::gui::IGUIEditBox* scenarioName;
    irr::gui::IGUIStaticText* overwriteWarning;
    irr::gui::IGUIStaticText* notMultiplayerNameWarning;
    irr::gui::IGUIStaticText* multiplayerNameWarning;
    irr::gui::IGUIButton* apply;
    irr::gui::IGUIButton* save;


    irr::f32 mapCentreX;
    irr::f32 mapCentreZ;

    bool editBoxesNeedUpdating;
    bool multiplayer;

    GeneralData oldScenarioInfo; //Keep a copy of the data we have already displayed, so the dialog boxes only get updated when needed

    void drawInformationOnMap(const irr::f32& time, const irr::s32& mapOffsetX, const irr::s32& mapOffsetZ, const irr::f32& metresPerPx, const irr::f32& ownShipPosX, const irr::f32& ownShipPosZ, const irr::f32& ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipEditorData>& otherShips, const irr::s32& selectedShip, const irr::s32& selectedLeg);
    void updateDropDowns(const std::vector<OtherShipEditorData>& otherShips, irr::s32 selectedShip, irr::f32 time);
    bool manuallyTriggerGUIEvent(irr::gui::IGUIElement* caller, irr::gui::EGUI_EVENT_TYPE eType);
    std::wstring f32To3dp(irr::f32 value) const;
    std::wstring f32To4dp(irr::f32 value) const;

};

#endif


