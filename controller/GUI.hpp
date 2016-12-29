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

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device, Lang* language);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_WINDOW = 101,
        //GUI_ID_WEATHER_WINDOW,
        GUI_ID_SHIP_COMBOBOX,
        GUI_ID_LEG_LISTBOX,
        GUI_ID_COURSE_EDITBOX,
        GUI_ID_SPEED_EDITBOX,
        GUI_ID_DISTANCE_EDITBOX,
        GUI_ID_CHANGE_BUTTON,
        GUI_ID_CHANGE_COURSESPEED_BUTTON,
        GUI_ID_ADDLEG_BUTTON,
        GUI_ID_DELETELEG_BUTTON,
        GUI_ID_MOVESHIP_BUTTON,
        GUI_ID_WEATHER_SCROLLBAR,
        GUI_ID_RAIN_SCROLLBAR,
        GUI_ID_VISIBILITY_SCROLLBAR
    };

    void updateGuiData(irr::f32 time, irr::s32 mapOffsetX, irr::s32 mapOffsetZ, irr::f32 metresPerPx, irr::f32 ownShipPosX, irr::f32 ownShipPosZ, irr::f32 ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, irr::video::ITexture* displayMapTexture, irr::s32 selectedShip, irr::s32 selectedLeg, irr::f32 terrainLong, irr::f32 terrainLongExtent, irr::f32 terrainXWidth, irr::f32 terrainLat, irr::f32 terrainLatExtent, irr::f32 terrainZWidth, irr::f32 weather, irr::f32 visibility, irr::f32 rain);
    void updateEditBoxes(); //Trigger an update of the edit boxes (carried out in next updateGuiData)
    irr::f32 getEditBoxCourse() const;
    irr::f32 getEditBoxSpeed() const;
    irr::f32 getEditBoxDistance() const;
    int getSelectedShip() const;
    int getSelectedLeg() const;
    irr::core::vector2df getScreenCentrePosition() const;
    irr::f32 getWeather() const;
    irr::f32 getRain() const;
    irr::f32 getVisibility() const;

private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;

    irr::gui::IGUIWindow* guiWindow;
    //irr::gui::IGUIWindow* guiWeatherWindow;

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
    irr::gui::IGUIButton* changeLeg;
    irr::gui::IGUIButton* changeLegCourseSpeed;
    irr::gui::IGUIButton* addLeg;
    irr::gui::IGUIButton* deleteLeg;
    irr::gui::IGUIButton* moveShip;
    irr::gui::IGUIScrollBar* weatherBar;
    irr::gui::IGUIScrollBar* rainBar;
    irr::gui::IGUIScrollBar* visibilityBar;
    irr::f32 mapCentreX;
    irr::f32 mapCentreZ;

    bool editBoxesNeedUpdating;

    void drawInformationOnMap(const irr::f32& time, const irr::s32& mapOffsetX, const irr::s32& mapOffsetZ, const irr::f32& metresPerPx, const irr::f32& ownShipPosX, const irr::f32& ownShipPosZ, const irr::f32& ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips );
    void updateDropDowns(const std::vector<OtherShipDisplayData>& otherShips, irr::s32 selectedShip, irr::f32 time);
    bool manuallyTriggerGUIEvent(irr::gui::IGUIElement* caller, irr::gui::EGUI_EVENT_TYPE eType);
    std::wstring f32To3dp(irr::f32 value);
    std::wstring f32To1dp(irr::f32 value);

};

#endif


