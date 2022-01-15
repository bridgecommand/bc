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

//using namespace irr;

GUIMain::GUIMain(irr::IrrlichtDevice* device, Lang* language)
{
    this->device = device;
    guienv = device->getGUIEnvironment();

    irr::video::IVideoDriver* driver = device->getVideoDriver();
    irr::u32 su = driver->getScreenSize().Width;
    irr::u32 sh = driver->getScreenSize().Height;

    this->language = language;

    //gui

    //Size dependent on font size, so stuff fits in. Choose an example letter to measure
    irr::s32 fh = guienv->getSkin()->getFont()->getDimension(L"H").Height;
    irr::s32 fw = guienv->getSkin()->getFont()->getDimension(L"H").Width;

    //Add zoom buttons
    zoomIn = guienv->addButton(irr::core::rect<irr::s32>(0.96*su,0.01*sh,0.99*su,0.05*sh),0,GUI_ID_ZOOMIN_BUTTON,L"+");
    zoomOut = guienv->addButton(irr::core::rect<irr::s32>(0.96*su,0.06*sh,0.99*su,0.10*sh),0,GUI_ID_ZOOMOUT_BUTTON,L"-");


    //Add a moveable window to put things in
    guiWindow = guienv->addWindow(irr::core::rect<irr::s32>(0.01*su,0.01*sh,0.01*su+36.5*fw,0.01*sh+20*fh),false,0,0,GUI_ID_WINDOW);
    guiWindow->getCloseButton()->setVisible(false);

    guiTabs = guienv->addTabControl(irr::core::rect<irr::s32>(0.5*fw,0.5*fh,36*fw,19.5*fh),guiWindow);
    mainTab = guiTabs->addTab(language->translate("main").c_str());
    failureTab = guiTabs->addTab(language->translate("failures").c_str());

    //add data display:
    dataDisplay = guienv->addStaticText(L"", irr::core::rect<irr::s32>(1*fw,1*fh,24*fw,4*fh), true, true, mainTab, -1, true); //Actual text set later

    //Add ship selector drop down
    shipSelector = guienv->addComboBox(irr::core::rect<irr::s32>(1*fw,6*fh,12*fw,7*fh),mainTab,GUI_ID_SHIP_COMBOBOX);
    shipSelector->addItem(language->translate("own").c_str()); //Make sure there's always at least one element
    shipSelectorTitle = guienv->addStaticText(language->translate("selectShip").c_str(),irr::core::rect<irr::s32>(1*fw,4.5*fh,12*fw,5.5*fh),false,false,mainTab);

    //Add MMSI editing
    mmsiEdit = guienv->addEditBox(L"",irr::core::rect<irr::s32>(1*fw,7.5*fh,12*fw,8.5*fh),false,mainTab,GUI_ID_MMSI_EDITBOX);
    setMMSI = guienv->addButton(irr::core::rect<irr::s32>(1*fw,8.5*fh,12*fw,9.5*fh),mainTab,GUI_ID_SETMMSI_BUTTON,language->translate("setMMSI").c_str());

    //Add leg selector drop down
    legSelector  = guienv->addListBox(irr::core::rect<irr::s32>(12.5*fw,6*fh,24*fw,9.5*fh),mainTab,GUI_ID_LEG_LISTBOX);
    legSelectorTitle = guienv->addStaticText(language->translate("selectLeg").c_str(),irr::core::rect<irr::s32>(12.5*fw,4.5*fh,24*fw,5.5*fh),false,false,mainTab);

    //Add edit boxes for this leg element
    legCourseEdit   = guienv->addEditBox(L"C",irr::core::rect<irr::s32>(1*fw,11.5*fh,12*fw,12.5*fh),false,mainTab,GUI_ID_COURSE_EDITBOX);
    legSpeedEdit    = guienv->addEditBox(L"S",irr::core::rect<irr::s32>(12.5*fw,11.5*fh,23.5*fw,12.5*fh),false,mainTab,GUI_ID_SPEED_EDITBOX);
    legDistanceEdit = guienv->addEditBox(L"D",irr::core::rect<irr::s32>(24*fw,11.5*fh,35*fw,12.5*fh),false,mainTab,GUI_ID_DISTANCE_EDITBOX);

    courseTitle = guienv->addStaticText(language->translate("setCourse").c_str(),irr::core::rect<irr::s32>(1*fw,10*fh,12*fw,11*fh),false,false,mainTab);
    speedTitle = guienv->addStaticText(language->translate("setSpeed").c_str(),irr::core::rect<irr::s32>(12.5*fw,10*fh,23.5*fw,11*fh),false,false,mainTab);
    distanceTitle = guienv->addStaticText(language->translate("setDistance").c_str(),irr::core::rect<irr::s32>(24*fw,10*fh,35*fw,11*fh),false,false,mainTab);

    //Add buttons
    changeLeg       = guienv->addButton(irr::core::rect<irr::s32>     (1.000*fw, 13*fh,17.75*fw, 14*fh),mainTab,GUI_ID_CHANGE_BUTTON,language->translate("changeLeg").c_str());
    changeLegCourseSpeed = guienv->addButton(irr::core::rect<irr::s32>(18.25*fw, 13*fh,35.00*fw, 14*fh),mainTab, GUI_ID_CHANGE_COURSESPEED_BUTTON,language->translate("changeLegCourseSpeed").c_str());
    
    addLeg          = guienv->addButton(irr::core::rect<irr::s32>     (1.000*fw, 14.25*fh,17.75*fw, 15.25*fh),mainTab,GUI_ID_ADDLEG_BUTTON,language->translate("addLeg").c_str());
    deleteLeg       = guienv->addButton(irr::core::rect<irr::s32>     (18.25*fw, 14.25*fh,35.00*fw, 15.25*fh),mainTab, GUI_ID_DELETELEG_BUTTON,language->translate("deleteLeg").c_str());
    
    moveShip        = guienv->addButton(irr::core::rect<irr::s32>     (1.000*fw, 15.5*fh,35.00*fw, 16.5*fh),mainTab, GUI_ID_MOVESHIP_BUTTON,language->translate("move").c_str());

    //Add buttons to release and retrieve man overboard dummy
    releaseMOB = guienv->addButton(irr::core::rect<irr::s32>(29.0*fw,1*fh,35*fw,3.25*fh),mainTab,GUI_ID_RELEASEMOB_BUTTON,language->translate("releaseMOB").c_str());
    retrieveMOB = guienv->addButton(irr::core::rect<irr::s32>(29.0*fw,3.25*fh,35*fw,5.5*fh),mainTab,GUI_ID_RETRIEVEMOB_BUTTON,language->translate("retrieveMOB").c_str());

    //Scroll bars for weather setting
    visibilityBar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(24.5*fw,1*fh,25.5*fw,5.5*fh),mainTab,GUI_ID_VISIBILITY_SCROLLBAR);
    rainBar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(26.0*fw,1*fh,27.0*fw,5.5*fh),mainTab,GUI_ID_RAIN_SCROLLBAR);
    weatherBar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(27.5*fw,1*fh,28.5*fw,5.5*fh),mainTab,GUI_ID_WEATHER_SCROLLBAR);

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

    //Failure parts of GUI
    guienv->addButton(irr::core::rect<irr::s32>(1*fw,1*fh,35*fw,2*fh),failureTab,GUI_ID_RUDDERPUMP_1_WORKING_BUTTON,language->translate("pump1Working").c_str());
    guienv->addButton(irr::core::rect<irr::s32>(1*fw,2*fh,35*fw,3*fh),failureTab,GUI_ID_RUDDERPUMP_1_FAILED_BUTTON,language->translate("pump1Failed").c_str());
    guienv->addButton(irr::core::rect<irr::s32>(1*fw,3*fh,35*fw,4*fh),failureTab,GUI_ID_RUDDERPUMP_2_WORKING_BUTTON,language->translate("pump2Working").c_str());
    guienv->addButton(irr::core::rect<irr::s32>(1*fw,4*fh,35*fw,5*fh),failureTab,GUI_ID_RUDDERPUMP_2_FAILED_BUTTON,language->translate("pump2Failed").c_str());

    guienv->addButton(irr::core::rect<irr::s32>(1*fw,5.5*fh,35*fw,6.5*fh),failureTab,GUI_ID_FOLLOWUP_WORKING_BUTTON,language->translate("followUpWorking").c_str());
    guienv->addButton(irr::core::rect<irr::s32>(1*fw,6.5*fh,35*fw,7.5*fh),failureTab,GUI_ID_FOLLOWUP_FAILED_BUTTON,language->translate("followUpFailed").c_str());
    

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

void GUIMain::updateGuiData(irr::f32 time, irr::s32 mapOffsetX, irr::s32 mapOffsetZ, irr::f32 metresPerPx, irr::f32 ownShipPosX, irr::f32 ownShipPosZ, irr::f32 ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, const std::vector<AISData>& aisData, bool mobVisible, irr::f32 mobPosX, irr::f32 mobPosZ, irr::video::ITexture* displayMapTexture, irr::s32 selectedShip, irr::s32 selectedLeg, irr::f32 terrainLong, irr::f32 terrainLongExtent, irr::f32 terrainXWidth, irr::f32 terrainLat, irr::f32 terrainLatExtent, irr::f32 terrainZWidth, irr::f32 weather, irr::f32 visibility, irr::f32 rain)
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

    irr::f32 latMinutes = (displayLat - (int)displayLat)*60;
    irr::f32 lonMinutes = (displayLong - (int)displayLong)*60;
    irr::u8 latDegrees = (int) displayLat;
    irr::u8 lonDegrees = (int) displayLong;

    //update heading display element
    irr::core::stringw displayText = language->translate("pos");
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
    displayText.append(irr::core::stringw(selectedShip));
    displayText.append(L" ");
    displayText.append(irr::core::stringw(selectedLeg));
    displayText.append(L"\n");
    //Show time now
    displayText.append(irr::core::stringw(time));
    displayText.append(L"\n");
    */
    //Display
    dataDisplay->setText(displayText.c_str());

    //Draw cross hairs, buoys, other ships
    drawInformationOnMap(time, mapOffsetX, mapOffsetZ, metresPerPx, ownShipPosX, ownShipPosZ, ownShipHeading, buoys, otherShips, aisData, selectedShip, selectedLeg, mobVisible, mobPosX, mobPosZ);

    //Update edit boxes if required, and then mark as updated
    //This must be done before we update the drop down boxes, as otherwise we'll miss the results of the manually triggered GUI change events
    if (editBoxesNeedUpdating) {
        if (selectedShip >= 0 && selectedShip < otherShips.size()) {
            mmsiEdit->setText(irr::core::stringw(otherShips.at(selectedShip).mmsi).c_str());
        }
        if (selectedShip >= 0 && selectedShip < otherShips.size() && selectedLeg >= 0 && selectedLeg < otherShips.at(selectedShip).legs.size()) {
            legCourseEdit  ->setText(irr::core::stringw(otherShips.at(selectedShip).legs.at(selectedLeg).bearing).c_str());
            legSpeedEdit   ->setText(irr::core::stringw(otherShips.at(selectedShip).legs.at(selectedLeg).speed).c_str());
            //Distance
            if ( (selectedLeg+1) < otherShips.at(selectedShip).legs.size() ) {
                //There is a next leg, so can check distance
                irr::f32 legDurationS = otherShips.at(selectedShip).legs.at(selectedLeg+1).startTime - otherShips.at(selectedShip).legs.at(selectedLeg).startTime;
                irr::f32 legDurationH = legDurationS / SECONDS_IN_HOUR;
                irr::f32 legDistanceNm  = legDurationH * otherShips.at(selectedShip).legs.at(selectedLeg).speed;
                legDistanceEdit->setText(irr::core::stringw(legDistanceNm).c_str());
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

void GUIMain::drawInformationOnMap(const irr::f32& time, const irr::s32& mapOffsetX, const irr::s32& mapOffsetZ, const irr::f32& metresPerPx, const irr::f32& ownShipPosX, const irr::f32& ownShipPosZ, const irr::f32& ownShipHeading, const std::vector<PositionData>& buoys, const std::vector<OtherShipDisplayData>& otherShips, const std::vector<AISData>& aisData, const irr::s32& selectedShip, const irr::s32& selectedLeg, const bool& mobVisible, const irr::f32& mobPosX, const irr::f32& mobPosZ)
{

    //draw cross hairs
    irr::s32 width = device->getVideoDriver()->getScreenSize().Width;
    irr::s32 height = device->getVideoDriver()->getScreenSize().Height;
    irr::s32 screenCentreX = width/2;
    irr::s32 screenCentreY = height/2;
    device->getVideoDriver()->draw2DLine(irr::core::position2d<irr::s32>(screenCentreX,0),irr::core::position2d<irr::s32>(screenCentreX,height),irr::video::SColor(255, 255, 255, 255));
    device->getVideoDriver()->draw2DLine(irr::core::position2d<irr::s32>(0,screenCentreY),irr::core::position2d<irr::s32>(width,screenCentreY),irr::video::SColor(255, 255, 255, 255));

    //Dimensions for dots
    irr::u32 dotHalfWidth = width/400;
    if(dotHalfWidth<1) {dotHalfWidth=1;}


    //Draw location of own ship
    irr::s32 ownRelPosX = 0 + mapOffsetX;
    irr::s32 ownRelPosY = 0 - mapOffsetZ;
    device->getVideoDriver()->draw2DRectangle(irr::video::SColor(255, 255, 0, 0),irr::core::rect<irr::s32>(screenCentreX-dotHalfWidth+ownRelPosX,screenCentreY-dotHalfWidth-ownRelPosY,screenCentreX+dotHalfWidth+ownRelPosX,screenCentreY+dotHalfWidth-ownRelPosY));
    if (selectedShip == -1) {
        //Own ship selected
        device->getVideoDriver()->draw2DPolygon(irr::core::position2d<irr::s32>(screenCentreX+ownRelPosX,screenCentreY-ownRelPosY),dotHalfWidth*4,irr::video::SColor(255, 255, 0, 0),10);
    }

    //Heading line
    irr::s32 hdgLineX = ownRelPosX + width/10*sin(ownShipHeading * RAD_IN_DEG);
    irr::s32 hdgLineY = ownRelPosY + width/10*cos(ownShipHeading * RAD_IN_DEG);
    irr::core::position2d<irr::s32> hdgStart (screenCentreX + ownRelPosX, screenCentreY - ownRelPosY);
    irr::core::position2d<irr::s32> hdgEnd   (screenCentreX + hdgLineX  , screenCentreY - hdgLineY  );
    device->getVideoDriver()->draw2DLine(hdgStart,hdgEnd,irr::video::SColor(255, 255, 0, 0));

    //Draw location of MOB
    if (mobVisible) {
        irr::s32 relPosX = (mobPosX - ownShipPosX)/metresPerPx + mapOffsetX;
        irr::s32 relPosY = (mobPosZ - ownShipPosZ)/metresPerPx - mapOffsetZ;

        device->getVideoDriver()->draw2DPolygon(irr::core::position2d<irr::s32>(screenCentreX+relPosX,screenCentreY-relPosY),dotHalfWidth*2,irr::video::SColor(255, 255, 255, 0),10);
    }

    //Draw location of buoys
    for(std::vector<PositionData>::const_iterator it = buoys.begin(); it != buoys.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx + mapOffsetX;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx - mapOffsetZ;

        device->getVideoDriver()->draw2DRectangle(irr::video::SColor(255, 255, 255, 255),irr::core::rect<irr::s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));
    }

    //Draw location of ships
    for(std::vector<OtherShipDisplayData>::const_iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx + mapOffsetX;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx - mapOffsetZ;

        device->getVideoDriver()->draw2DRectangle(irr::video::SColor(255, 0, 0, 255),irr::core::rect<irr::s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));
        if (selectedShip == (it - otherShips.begin()) ) {
            //This ship selected
            device->getVideoDriver()->draw2DPolygon(irr::core::position2d<irr::s32>(screenCentreX+relPosX,screenCentreY-relPosY),dotHalfWidth*4,irr::video::SColor(255, 0, 0, 255),10);
        }

        //number
        int thisShipNumber = 1 + it - otherShips.begin();
        irr::core::stringw displayNumber = irr::core::stringw(thisShipNumber);
        if (it->mmsi != 0) {
            displayNumber.append(L" (MMSI:");
            displayNumber.append(irr::core::stringw(it->mmsi));
            displayNumber.append(L")");
        }
        guienv->getSkin()->getFont()->draw(displayNumber,irr::core::rect<irr::s32>(screenCentreX+relPosX-0.02*width,screenCentreY-relPosY-0.02*width,screenCentreX+relPosX,screenCentreY-relPosY), irr::video::SColor(255,0,0,255),true,true);

        //Draw leg information for each ship
        if (it->legs.size() > 0) {

            //Find current leg: This is the last leg, or the leg where the start time is in the past, and then next start time is in the future. Leg times are from the start of the day of the scenario start.
            irr::u32 currentLeg = 0;
            bool currentLegFound = false;
            for (irr::u32 i=0; i < (it->legs.size()-1); i++) {
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

                        irr::core::position2d<irr::s32> startLine (legStartX, legStartY);
                        irr::core::position2d<irr::s32> endLine (legEndX, legEndY);

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
                        irr::core::position2d<irr::s32> startLine (legStartX, legStartY);
                        irr::core::position2d<irr::s32> endLine (legEndX, legEndY);

                        device->getVideoDriver()->draw2DLine(startLine,endLine);
                    } //Not infinite
                } //Each leg, except last
            } //If not currently on the last leg
        }//If Legs.size() >0
    } //Loop for each ship

    //Draw AIS data
    for(std::vector<AISData>::const_iterator it = aisData.begin(); it != aisData.end(); ++it) {
        //Check if MMSI matches an own ship, if so, don't show
        bool showThisAISContact = true;
        for(std::vector<OtherShipDisplayData>::const_iterator it2 = otherShips.begin(); it2 != otherShips.end(); ++it2) {
            if (it2->mmsi == it->mmsi) {
                showThisAISContact = false;
            }
        }

        if (showThisAISContact) {
            
            irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx + mapOffsetX;
            irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx - mapOffsetZ;
            device->getVideoDriver()->draw2DRectangle(irr::video::SColor(255, 0, 255, 0),irr::core::rect<irr::s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));
        
            //std::cout << "Displaying MMSI " << it->mmsi << " at " << relPosX << " " << relPosY << std::endl;

            //number
            irr::core::stringw displayAIS = L"AIS: MMSI ";
            displayAIS.append(irr::core::stringw(it->mmsi));
            displayAIS.append(L" ");
            displayAIS.append(irr::core::stringw(it->name.c_str()));
            guienv->getSkin()->getFont()->draw(displayAIS,irr::core::rect<irr::s32>(screenCentreX+relPosX-0.02*width,screenCentreY-relPosY-0.02*width,screenCentreX+relPosX,screenCentreY-relPosY), irr::video::SColor(255,0,255,0),true,true);

            //Show cog if known. AIS COG is: 3600 for unknown, otherwise COG is AIS COG/10. SOG: 1023 is unknown, otherwise knots is AIS SOG/10
            if (it->cog != 3600) {
                irr::f32 cog = (irr::f32)it->cog/10.0;
                irr::f32 sog = 0;
                if (it->sog != 1023) {
                    sog = (irr::f32)it->sog/10.0;
                }
                irr::f32 cogLineLength = sog; //In pixels
                irr::f32 cogLineX = relPosX + cogLineLength*sin(cog * RAD_IN_DEG);
                irr::f32 cogLineY = relPosY + cogLineLength*cos(cog * RAD_IN_DEG);
                //Draw
                irr::core::position2d<irr::s32> startLine (screenCentreX + relPosX, screenCentreY - relPosY);
                irr::core::position2d<irr::s32> endLine (screenCentreX + cogLineX, screenCentreY - cogLineY);
                device->getVideoDriver()->draw2DLine(startLine,endLine,irr::video::SColor(255,0,255,0));
            }
        }

    }

}

void GUIMain::updateDropDowns(const std::vector<OtherShipDisplayData>& otherShips, irr::s32 selectedShip, irr::f32 time) {

//Update drop down menus for ships and legs
    if(shipSelector->getItemCount() != otherShips.size() + 1) {
        shipSelector->clear();

        //add own ship (at index 0)
        shipSelector->addItem(language->translate("own").c_str());

        //Add other ships (at index 1,2,...)
        for(irr::u32 i = 0; i<otherShips.size(); i++) {
            irr::core::stringw otherShipLabel = language->translate("other");
            otherShipLabel.append(L" ");
            otherShipLabel.append(irr::core::stringw(i+1));
            shipSelector->addItem(otherShipLabel.c_str());
        }
        manuallyTriggerGUIEvent((irr::gui::IGUIElement*)shipSelector, irr::gui::EGET_COMBO_BOX_CHANGED); //Trigger event here so any changes caused by the update are found

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
            legSelector->addItem(irr::core::stringw(i+1).c_str());
        }
        manuallyTriggerGUIEvent((irr::gui::IGUIElement*)legSelector, irr::gui::EGET_LISTBOX_CHANGED ); //Trigger event here so any changes caused by the update are found

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
                for (irr::u32 i=0; i < (selectedShipLegs.size()-1); i++) {
                    if (time >= selectedShipLegs.at(i).startTime &&  time < selectedShipLegs.at(i+1).startTime) {
                        currentLeg = i;
                        currentLegFound = true;
                    }
                }
                if (!currentLegFound) {
                    currentLeg = selectedShipLegs.size()-1;
                }

                //Update text for past, current and future legs.
                for (irr::u32 i=0; i<legSelector->getItemCount(); i++) {
                    if (i < currentLeg) {
                        std::wstring label(irr::core::stringw(i+1).c_str());
                        label.append(language->translate("past").c_str());
                        legSelector->setItem(i,label.c_str(),-1);
                    }
                    if (i == currentLeg) {
                        std::wstring label(irr::core::stringw(i+1).c_str());
                        label.append(language->translate("current").c_str());
                        legSelector->setItem(i,label.c_str(),-1);
                    }
                    if (i > currentLeg) {
                        std::wstring label(irr::core::stringw(i+1).c_str());
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
    triggerUpdateEvent.EventType = irr::EET_GUI_EVENT;
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

irr::u32 GUIMain::getEditBoxMMSI() const {
    wchar_t* endPtr;
    irr::u32 mmsi = wcstol(mmsiEdit->getText(),&endPtr,10); //TODO: Check portability
    return mmsi;
}

int GUIMain::getSelectedShip() const {
    return shipSelector->getSelected();
}

int GUIMain::getSelectedLeg() const {
    return (legSelector->getSelected() + 1);
}

irr::core::vector2df GUIMain::getScreenCentrePosition() const {
    return irr::core::vector2df(mapCentreX, mapCentreZ);
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
