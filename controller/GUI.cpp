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

#include "GUI.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"

#include <iostream>
#include <limits>
#include <string>

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* device, Lang* language)
{
    this->device = device;
    guienv = device->getGUIEnvironment();

    video::IVideoDriver* driver = device->getVideoDriver();
    u32 su = driver->getScreenSize().Width;
    u32 sh = driver->getScreenSize().Height;

    this->language = language;

    //gui

    //Add a moveable window to put things in
    guiWindow = guienv->addWindow(core::rect<s32>(0.01*su,0.51*sh,0.49*su,0.99*sh),false,0,0,GUI_ID_WINDOW);
    guiWindow->getCloseButton()->setVisible(false);

    //add data display:
    dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.01*su,0.05*sh,0.35*su,0.15*sh), true, true, guiWindow, -1, true); //Actual text set later

    //Add ship selector drop down
    shipSelector = guienv->addComboBox(core::rect<s32>(0.01*su,0.20*sh,0.13*su,0.23*sh),guiWindow,GUI_ID_SHIP_COMBOBOX);
    shipSelector->addItem(language->translate("own").c_str()); //Make sure there's always at least one element
    shipSelectorTitle = guienv->addStaticText(language->translate("selectShip").c_str(),core::rect<s32>(0.01*su,0.16*sh,0.13*su,0.19*sh),false,false,guiWindow);

    //Add leg selector drop down
    legSelector  = guienv->addListBox(core::rect<s32>(0.18*su,0.20*sh,0.47*su,0.30*sh),guiWindow,GUI_ID_LEG_LISTBOX);
    legSelectorTitle = guienv->addStaticText(language->translate("selectLeg").c_str(),core::rect<s32>(0.18*su,0.16*sh,0.47*su,0.19*sh),false,false,guiWindow);

    //Add edit boxes for this leg element
    legCourseEdit   = guienv->addEditBox(L"C",core::rect<s32>(0.01*su,0.35*sh,0.13*su,0.38*sh),false,guiWindow,GUI_ID_COURSE_EDITBOX);
    legSpeedEdit    = guienv->addEditBox(L"S",core::rect<s32>(0.18*su,0.35*sh,0.30*su,0.38*sh),false,guiWindow,GUI_ID_SPEED_EDITBOX);
    legDistanceEdit = guienv->addEditBox(L"D",core::rect<s32>(0.35*su,0.35*sh,0.47*su,0.38*sh),false,guiWindow,GUI_ID_DISTANCE_EDITBOX);

    courseTitle = guienv->addStaticText(language->translate("setCourse").c_str(),core::rect<s32>(0.01*su,0.31*sh,0.13*su,0.34*sh),false,false,guiWindow);
    speedTitle = guienv->addStaticText(language->translate("setSpeed").c_str(),core::rect<s32>(0.18*su,0.31*sh,0.30*su,0.34*sh),false,false,guiWindow);
    distanceTitle = guienv->addStaticText(language->translate("setDistance").c_str(),core::rect<s32>(0.35*su,0.31*sh,0.47*su,0.34*sh),false,false,guiWindow);

    //Add buttons
    changeLeg       = guienv->addButton(core::rect<s32>     (0.03*su, 0.39*sh,0.23*su, 0.42*sh),guiWindow,GUI_ID_CHANGE_BUTTON,language->translate("changeLeg").c_str());
    changeLegCourseSpeed = guienv->addButton(core::rect<s32>(0.25*su, 0.39*sh,0.45*su, 0.42*sh),guiWindow, GUI_ID_CHANGE_COURSESPEED_BUTTON,language->translate("changeLegCourseSpeed").c_str());
    addLeg          = guienv->addButton(core::rect<s32>     (0.03*su, 0.42*sh,0.23*su, 0.45*sh),guiWindow,GUI_ID_ADDLEG_BUTTON,language->translate("addLeg").c_str());
    deleteLeg       = guienv->addButton(core::rect<s32>     (0.25*su, 0.42*sh,0.45*su, 0.45*sh),guiWindow, GUI_ID_DELETELEG_BUTTON,language->translate("deleteLeg").c_str());
    moveShip        = guienv->addButton(core::rect<s32>     (0.14*su, 0.45*sh,0.34*su, 0.48*sh),guiWindow, GUI_ID_MOVESHIP_BUTTON,language->translate("move").c_str());


    //Scroll bars for weather setting
    //core::rect<s32>(0.01*su,0.05*sh,0.35*su,0.15*sh)
    visibilityBar = guienv->addScrollBar(false,core::rect<s32>(0.38*su, 0.05*sh,0.40*su, 0.19*sh),guiWindow,GUI_ID_VISIBILITY_SCROLLBAR);
    rainBar = guienv->addScrollBar(false,core::rect<s32>(0.41*su, 0.05*sh,0.43*su, 0.19*sh),guiWindow,GUI_ID_RAIN_SCROLLBAR);
    weatherBar = guienv->addScrollBar(false,core::rect<s32>(0.44*su, 0.05*sh,0.46*su, 0.19*sh),guiWindow,GUI_ID_WEATHER_SCROLLBAR);

    visibilityBar->setToolTipText(language->translate("visibility").c_str());
    rainBar->setToolTipText(language->translate("rain").c_str());
    weatherBar->setToolTipText(language->translate("weather").c_str());

    weatherBar->setMax(120); //Divide by 10 to get weather
    weatherBar->setMin(0);
    weatherBar->setSmallStep(5);
    rainBar->setMax(100);
    rainBar->setMin(0);
    rainBar->setLargeStep(5);
    rainBar->setSmallStep(5);
    visibilityBar->setMax(101);
    visibilityBar->setMin(1);
    visibilityBar->setLargeStep(5);
    visibilityBar->setSmallStep(1);



    //This is used to track when the edit boxes need updating, when ship or legs have changed
    editBoxesNeedUpdating = false;

    //These get updated in updateGuiData
    mapCentreX = 0;
    mapCentreZ = 0;

}

void GUIMain::updateEditBoxes()
{
    //Trigger update the edit boxes for course, speed & distance when the selection is changed.
    editBoxesNeedUpdating = true;
}

void GUIMain::updateGuiData(irr::f32 time, irr::s32 mapOffsetX, irr::s32 mapOffsetZ, irr::f32 metresPerPx, irr::f32 ownShipPosX, irr::f32 ownShipPosZ, irr::f32 ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, irr::video::ITexture* displayMapTexture, irr::s32 selectedShip, irr::s32 selectedLeg, irr::f32 terrainLong, irr::f32 terrainLongExtent, irr::f32 terrainXWidth, irr::f32 terrainLat, irr::f32 terrainLatExtent, irr::f32 terrainZWidth, irr::f32 weather, irr::f32 visibility, irr::f32 rain)
{
    //Show map texture
    device->getVideoDriver()->draw2DImage(displayMapTexture, irr::core::position2d<irr::s32>(0,0));
    //TODO: Check that conversion to texture does not distort image

    //Calculate map centre as displayed
    mapCentreX = ownShipPosX - mapOffsetX*metresPerPx;
    mapCentreZ = ownShipPosZ + mapOffsetZ*metresPerPx;

    irr::f32 mapCentreLong = terrainLong + mapCentreX*terrainLongExtent/terrainXWidth;
    irr::f32 mapCentreLat = terrainLat + mapCentreZ*terrainLatExtent/terrainZWidth;

    //Convert lat/long into a readable format
    wchar_t eastWest;
    wchar_t northSouth;
    if (mapCentreLat >= 0) {
        northSouth='N';
    } else {
        northSouth='S';
    }
    if (mapCentreLong >= 0) {
        eastWest='E';
    } else {
        eastWest='W';
    }
    irr::f32 displayLat = fabs(mapCentreLat);
    irr::f32 displayLong = fabs(mapCentreLong);

    f32 latMinutes = (displayLat - (int)displayLat)*60;
    f32 lonMinutes = (displayLong - (int)displayLong)*60;
    u8 latDegrees = (int) displayLat;
    u8 lonDegrees = (int) displayLong;

    //update heading display element
    core::stringw displayText = language->translate("pos");
    displayText.append(irr::core::stringw(latDegrees));
    displayText.append(language->translate("deg"));
    displayText.append(f32To3dp(latMinutes).c_str());
    displayText.append(language->translate("minSymbol"));
    displayText.append(northSouth);
    displayText.append(L" ");

    displayText.append(irr::core::stringw(lonDegrees));
    displayText.append(language->translate("deg"));
    displayText.append(f32To3dp(lonMinutes).c_str());
    displayText.append(language->translate("minSymbol"));
    displayText.append(eastWest);
    displayText.append(L"\n");

    displayText.append(language->translate("visibility"));
    displayText.append(L": ");
    displayText.append(f32To1dp(visibility).c_str());
    displayText.append(L" ");
    displayText.append(language->translate("rain"));
    displayText.append(L": ");
    displayText.append(f32To1dp(rain).c_str());
    displayText.append(L" ");
    displayText.append(language->translate("weather"));
    displayText.append(L": ");
    displayText.append(f32To1dp(weather).c_str());
    displayText.append(L"\n");
    /*
    //Show selected ship and legs
    displayText.append(core::stringw(selectedShip));
    displayText.append(L" ");
    displayText.append(core::stringw(selectedLeg));
    displayText.append(L"\n");
    //Show time now
    displayText.append(core::stringw(time));
    displayText.append(L"\n");
    */
    //Display
    dataDisplay->setText(displayText.c_str());

    //Draw cross hairs, buoys, other ships
    drawInformationOnMap(time, mapOffsetX, mapOffsetZ, metresPerPx, ownShipPosX, ownShipPosZ, ownShipHeading, buoys, otherShips);

    //Update edit boxes if required, and then mark as updated
    //This must be done before we update the drop down boxes, as otherwise we'll miss the results of the manually triggered GUI change events
    if (editBoxesNeedUpdating) {
        if (selectedShip >= 0 && selectedShip < otherShips.size() && selectedLeg >= 0 && selectedLeg < otherShips.at(selectedShip).legs.size()) {
            legCourseEdit  ->setText(core::stringw(otherShips.at(selectedShip).legs.at(selectedLeg).bearing).c_str());
            legSpeedEdit   ->setText(core::stringw(otherShips.at(selectedShip).legs.at(selectedLeg).speed).c_str());
            //Distance
            if ( (selectedLeg+1) < otherShips.at(selectedShip).legs.size() ) {
                //There is a next leg, so can check distance
                irr::f32 legDurationS = otherShips.at(selectedShip).legs.at(selectedLeg+1).startTime - otherShips.at(selectedShip).legs.at(selectedLeg).startTime;
                irr::f32 legDurationH = legDurationS / SECONDS_IN_HOUR;
                irr::f32 legDistanceNm  = legDurationH * otherShips.at(selectedShip).legs.at(selectedLeg).speed;
                legDistanceEdit->setText(core::stringw(legDistanceNm).c_str());
            } else {
                legDistanceEdit->setText(L""); //No next leg, so can't find distance
            }
        } else {
            //Set blank (invalid other ship or leg)
            legCourseEdit  ->setText(L"");
            legSpeedEdit   ->setText(L"");
            legDistanceEdit->setText(L"");
        }
        editBoxesNeedUpdating = false;
    }

    //Update comboboxes for other ships and legs
    updateDropDowns(otherShips,selectedShip,time);

    //Update gui info for weather bars
    weatherBar->setPos(Utilities::round(weather*10.0));
    visibilityBar->setPos(Utilities::round(visibility*10.0));
    rainBar->setPos(Utilities::round(rain*10.0));

    guienv->drawAll();

}

void GUIMain::drawInformationOnMap(const irr::f32& time, const irr::s32& mapOffsetX, const irr::s32& mapOffsetZ, const irr::f32& metresPerPx, const irr::f32& ownShipPosX, const irr::f32& ownShipPosZ, const irr::f32& ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips)
{

    //draw cross hairs
    irr::s32 width = device->getVideoDriver()->getScreenSize().Width;
    irr::s32 height = device->getVideoDriver()->getScreenSize().Height;
    irr::s32 screenCentreX = width/2;
    irr::s32 screenCentreY = height/2;
    device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(screenCentreX,0),irr::core::position2d<s32>(screenCentreX,height),video::SColor(255, 255, 255, 255));
    device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(0,screenCentreY),irr::core::position2d<s32>(width,screenCentreY),video::SColor(255, 255, 255, 255));

    //Dimensions for dots
    irr::u32 dotHalfWidth = width/400;
    if(dotHalfWidth<1) {dotHalfWidth=1;}


    //Draw location of own ship
    irr::s32 ownRelPosX = 0 + mapOffsetX;
    irr::s32 ownRelPosY = 0 - mapOffsetZ;
    device->getVideoDriver()->draw2DRectangle(video::SColor(255, 255, 0, 0),irr::core::rect<s32>(screenCentreX-dotHalfWidth+ownRelPosX,screenCentreY-dotHalfWidth-ownRelPosY,screenCentreX+dotHalfWidth+ownRelPosX,screenCentreY+dotHalfWidth-ownRelPosY));
    //Heading line
    irr::s32 hdgLineX = ownRelPosX + width/10*sin(ownShipHeading * RAD_IN_DEG);
    irr::s32 hdgLineY = ownRelPosY + width/10*cos(ownShipHeading * RAD_IN_DEG);
    irr::core::position2d<s32> hdgStart (screenCentreX + ownRelPosX, screenCentreY - ownRelPosY);
    irr::core::position2d<s32> hdgEnd   (screenCentreX + hdgLineX  , screenCentreY - hdgLineY  );
    device->getVideoDriver()->draw2DLine(hdgStart,hdgEnd,video::SColor(255, 255, 0, 0));

    //Draw location of buoys
    for(std::vector<PositionData>::const_iterator it = buoys.begin(); it != buoys.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx + mapOffsetX;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx - mapOffsetZ;

        device->getVideoDriver()->draw2DRectangle(video::SColor(255, 255, 255, 255),irr::core::rect<s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));
    }

    //Draw location of ships
    for(std::vector<OtherShipDisplayData>::const_iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx + mapOffsetX;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx - mapOffsetZ;

        device->getVideoDriver()->draw2DRectangle(video::SColor(255, 0, 0, 255),irr::core::rect<s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));

        //number
        int thisShipNumber = 1 + it - otherShips.begin();
        guienv->getSkin()->getFont()->draw(irr::core::stringw(thisShipNumber),irr::core::rect<s32>(screenCentreX+relPosX-0.02*width,screenCentreY-relPosY-0.02*width,screenCentreX+relPosX,screenCentreY-relPosY), video::SColor(255,0,0,255),true,true);

        //Draw leg information for each ship
        if (it->legs.size() > 0) {

            //Find current leg: This is the last leg, or the leg where the start time is in the past, and then next start time is in the future. Leg times are from the start of the day of the scenario start.
            irr::u32 currentLeg = 0;
            bool currentLegFound = false;
            for (u32 i=0; i < (it->legs.size()-1); i++) {
                if (time >= it->legs.at(i).startTime &&  time < it->legs.at(i+1).startTime) {
                    currentLeg = i;
                    currentLegFound = true;
                }
            }
            if (!currentLegFound) {
                currentLeg = it->legs.size()-1;
            }

            //find time remaining on current leg
            irr::f32 currentLegTimeRemaining = 0;
            if (currentLeg < (it->legs.size()-1) ) { //If not on the last leg, find time until we change onto next course

                irr::f32 legStartX = screenCentreX + relPosX;
                irr::f32 legStartY = screenCentreY - relPosY;

                irr::f32 legEndX = legStartX; //Default values in case current leg is zero length
                irr::f32 legEndY = legStartY;

                currentLegTimeRemaining = it->legs.at(currentLeg+1).startTime - time;
                if (currentLegTimeRemaining>0) {
                    //Line starts at screenCentreX + relPosX, screenCentreY-relPosY
                    irr::f32 legLengthPx = currentLegTimeRemaining * it->legs.at(currentLeg).speed * KTS_TO_MPS / metresPerPx;
                    if (fabs(currentLegTimeRemaining) != std::numeric_limits<irr::f32>::infinity()) {
                        legEndX = legStartX + legLengthPx*sin(it->legs.at(currentLeg).bearing * RAD_IN_DEG);
                        legEndY = legStartY - legLengthPx*cos(it->legs.at(currentLeg).bearing * RAD_IN_DEG);

                        irr::core::position2d<s32> startLine (legStartX, legStartY);
                        irr::core::position2d<s32> endLine (legEndX, legEndY);

                        device->getVideoDriver()->draw2DLine(startLine,endLine);
                    } //Not infinite

                } //If currentLegTimeRemaining > 0

                //Draw remaining legs, excluding 'stop'one
                for (irr::u32 i = currentLeg+1; i < (it->legs.size()-1); i++) {
                    irr::f32 legTimeRemaining = it->legs.at(i+1).startTime -  it->legs.at(i).startTime;
                    irr::f32 legLengthPx = legTimeRemaining * it->legs.at(i).speed * KTS_TO_MPS / metresPerPx;

                    if (fabs(legTimeRemaining) != std::numeric_limits<irr::f32>::infinity()) { //FIXME: Also check for NaN?
                        //current start is previous end
                        legStartX = legEndX;
                        legStartY = legEndY;

                        //find new end
                        legEndX = legStartX + legLengthPx*sin(it->legs.at(i).bearing * RAD_IN_DEG);
                        legEndY = legStartY - legLengthPx*cos(it->legs.at(i).bearing * RAD_IN_DEG);

                        //Draw
                        irr::core::position2d<s32> startLine (legStartX, legStartY);
                        irr::core::position2d<s32> endLine (legEndX, legEndY);

                        device->getVideoDriver()->draw2DLine(startLine,endLine);
                    } //Not infinite
                } //Each leg, except last
            } //If not currently on the last leg
        }//If Legs.size() >0
    } //Loop for each ship
}

void GUIMain::updateDropDowns(const std::vector<OtherShipDisplayData>& otherShips, irr::s32 selectedShip, irr::f32 time) {

//Update drop down menus for ships and legs
    if(shipSelector->getItemCount() != otherShips.size() + 1) {
        shipSelector->clear();

        //add own ship (at index 0)
        shipSelector->addItem(language->translate("own").c_str());

        //Add other ships (at index 1,2,...)
        for(u32 i = 0; i<otherShips.size(); i++) {
            core::stringw otherShipLabel = language->translate("other");
            otherShipLabel.append(L" ");
            otherShipLabel.append(core::stringw(i+1));
            shipSelector->addItem(otherShipLabel.c_str());
        }
        manuallyTriggerGUIEvent((gui::IGUIElement*)shipSelector, irr::gui::EGET_COMBO_BOX_CHANGED); //Trigger event here so any changes caused by the update are found

    } //If different number of ships to show

    //Find number of legs for selected ship if known
    irr::u32 selectedShipNoLegs = 0;
    if (selectedShip>=0) {
        if (otherShips.size() > selectedShip) { //SelectedShip is valid
            selectedShipNoLegs = otherShips.at(selectedShip).legs.size();
        }
    }
    //Update number of legs displayed, if required
    if(selectedShipNoLegs>0) {selectedShipNoLegs--;} //Note that we display legs-1, as the final 'stop' leg shouldn't be changed by the user
    if(legSelector->getItemCount() != selectedShipNoLegs) {
        legSelector->clear();
        for(irr::u32 i = 0; i<selectedShipNoLegs; i++) {
            legSelector->addItem(core::stringw(i+1).c_str());
        }
        manuallyTriggerGUIEvent((gui::IGUIElement*)legSelector, irr::gui::EGET_LISTBOX_CHANGED ); //Trigger event here so any changes caused by the update are found

    } else {
        //don't clear and update, but show which legs are past, current and future
        if (legSelector->getItemCount() > 0) {

            //Get legs for selected ship
            if (selectedShip>=0 && otherShips.size() > selectedShip) { //SelectedShip is valid
                std::vector<Leg> selectedShipLegs = otherShips.at(selectedShip).legs;

                //Find current leg (FIXME: Duplicated code)
                //Find current leg: This is the last leg, or the leg where the start time is in the past, and then next start time is in the future. Leg times are from the start of the day of the scenario start.
                irr::u32 currentLeg = 0;
                bool currentLegFound = false;
                for (u32 i=0; i < (selectedShipLegs.size()-1); i++) {
                    if (time >= selectedShipLegs.at(i).startTime &&  time < selectedShipLegs.at(i+1).startTime) {
                        currentLeg = i;
                        currentLegFound = true;
                    }
                }
                if (!currentLegFound) {
                    currentLeg = selectedShipLegs.size()-1;
                }

                //Update text for past, current and future legs.
                for (u32 i=0; i<legSelector->getItemCount(); i++) {
                    if (i < currentLeg) {
                        std::wstring label(core::stringw(i+1).c_str());
                        label.append(language->translate("past").c_str());
                        legSelector->setItem(i,label.c_str(),-1);
                    }
                    if (i == currentLeg) {
                        std::wstring label(core::stringw(i+1).c_str());
                        label.append(language->translate("current").c_str());
                        legSelector->setItem(i,label.c_str(),-1);
                    }
                    if (i > currentLeg) {
                        std::wstring label(core::stringw(i+1).c_str());
                        label.append(language->translate("future").c_str());
                        legSelector->setItem(i,label.c_str(),-1);
                    }
                }

            } //Selected ships valid
        } //At least one leg in selector
    } //Update descriptive text on legs, if they don't need updating entirely

}

bool GUIMain::manuallyTriggerGUIEvent(irr::gui::IGUIElement* caller, irr::gui::EGUI_EVENT_TYPE eType) {

    irr::SEvent triggerUpdateEvent;
    triggerUpdateEvent.EventType = EET_GUI_EVENT;
    triggerUpdateEvent.GUIEvent.Caller = caller;
    triggerUpdateEvent.GUIEvent.Element = 0;
    triggerUpdateEvent.GUIEvent.EventType = eType;
    return device->postEventFromUser(triggerUpdateEvent);
}

irr::f32 GUIMain::getEditBoxCourse() const {
    //irr::f32 course = _wtof(legCourseEdit->getText()); //TODO: Check portability
    wchar_t* endPtr;
    irr::f32 course = wcstof(legCourseEdit->getText(), &endPtr);
    return course;
}

irr::f32 GUIMain::getEditBoxSpeed() const {
    //irr::f32 speed = _wtof(legSpeedEdit->getText()); //TODO: Check portability
    wchar_t* endPtr;
    irr::f32 speed = wcstof(legSpeedEdit->getText(), &endPtr); //TODO: Check portability
    return speed;
}

irr::f32 GUIMain::getEditBoxDistance() const {
    //irr::f32 distance = _wtof(legDistanceEdit->getText()); //TODO: Check portability
    wchar_t* endPtr;
    irr::f32 distance = wcstof(legDistanceEdit->getText(), &endPtr); //TODO: Check portability
    return distance;
}

int GUIMain::getSelectedShip() const {
    return shipSelector->getSelected();
}

int GUIMain::getSelectedLeg() const {
    return (legSelector->getSelected() + 1);
}

irr::core::vector2df GUIMain::getScreenCentrePosition() const {
    return core::vector2df(mapCentreX, mapCentreZ);
}

irr::f32 GUIMain::getWeather() const {
    return (irr::f32)(weatherBar->getPos())/10.0;
}

irr::f32 GUIMain::getRain() const {
    return (irr::f32)(rainBar->getPos())/10.0;
}

irr::f32 GUIMain::getVisibility() const {
    return (irr::f32)(visibilityBar->getPos())/10.0;
}

std::wstring GUIMain::f32To1dp(irr::f32 value)
{
    //Convert a floating point value to a wstring, with 1dp
    char tempStr[100];
    snprintf(tempStr,100,"%.1f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}

std::wstring GUIMain::f32To3dp(irr::f32 value)
{
    //Convert a floating point value to a wstring, with 3dp
    char tempStr[100];
    snprintf(tempStr,100,"%.3f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}
