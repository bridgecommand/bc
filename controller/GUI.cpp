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

#include <iostream>

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

        //add data display:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.09*su,0.6*sh,0.45*su,0.7*sh), true, false, 0, -1, true); //Actual text set later

        shipSelector = guienv->addComboBox(core::rect<s32>(0.09*su,0.75*sh,0.45*su,0.80*sh));
        shipSelector->addItem(language->translate("own").c_str()); //Make sure there's always at least one element

        legSelector  = guienv->addComboBox(core::rect<s32>(0.09*su,0.85*sh,0.45*su,0.90*sh));

}

    void GUIMain::updateGuiData(irr::f32 time, irr::f32 metresPerPx, irr::f32 ownShipPosX, irr::f32 ownShipPosZ, const std::vector<PositionData>& buoys, const std::vector<OtherShipData>& otherShips, irr::video::ITexture* displayMapTexture)
    {

        //Show map texture
        device->getVideoDriver()->draw2DImage(displayMapTexture, irr::core::position2d<irr::s32>(0,0));
        //TODO: Check that conversion to texture does not distort image

        //update heading display element
        core::stringw displayText = language->translate("pos");
        displayText.append(core::stringw(ownShipPosX));
        displayText.append(L" ");
        displayText.append(core::stringw(ownShipPosZ));
        displayText.append(L"\n");
        //Show number of buoys and ships
        displayText.append(core::stringw(buoys.size()));
        displayText.append(L" ");
        displayText.append(core::stringw(otherShips.size()));
        displayText.append(L"\n");
        //Show time now
        displayText.append(core::stringw(time));
        displayText.append(L"\n");
        //Display
        dataDisplay->setText(displayText.c_str());

        //Draw cross hairs, buoys, other ships
        drawInformationOnMap(time, metresPerPx, ownShipPosX, ownShipPosZ, buoys, otherShips);

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

        }

        guienv->drawAll();

    }

void GUIMain::drawInformationOnMap(const irr::f32& time, const irr::f32& metresPerPx, const irr::f32& ownShipPosX, const irr::f32& ownShipPosZ, const std::vector<PositionData>& buoys, const std::vector<OtherShipData>& otherShips)
{
    //draw cross hairs
    irr::s32 width = device->getVideoDriver()->getScreenSize().Width;
    irr::s32 height = device->getVideoDriver()->getScreenSize().Height;
    irr::s32 screenCentreX = width/2;
    irr::s32 screenCentreY = height/2;
    device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(screenCentreX,0),irr::core::position2d<s32>(screenCentreX,height),video::SColor(255, 255, 255, 255));
    device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(0,screenCentreY),irr::core::position2d<s32>(width,screenCentreY),video::SColor(255, 255, 255, 255));

    //Draw location of buoys
    for(std::vector<PositionData>::const_iterator it = buoys.begin(); it != buoys.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx;

        irr::u32 dotHalfWidth = width/400;
        if(dotHalfWidth<1) {dotHalfWidth=1;}
        device->getVideoDriver()->draw2DRectangle(video::SColor(255, 255, 255, 255),irr::core::rect<s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));
    }

    //Draw location of ships
    for(std::vector<OtherShipData>::const_iterator it = otherShips.begin(); it != otherShips.end(); ++it) {
        irr::s32 relPosX = (it->X - ownShipPosX)/metresPerPx;
        irr::s32 relPosY = (it->Z - ownShipPosZ)/metresPerPx;

        irr::u32 dotHalfWidth = width/400;
        if(dotHalfWidth<1) {dotHalfWidth=1;}
        device->getVideoDriver()->draw2DRectangle(video::SColor(255, 0, 0, 255),irr::core::rect<s32>(screenCentreX-dotHalfWidth+relPosX,screenCentreY-dotHalfWidth-relPosY,screenCentreX+dotHalfWidth+relPosX,screenCentreY+dotHalfWidth-relPosY));

        //Draw leg information for each ship
        if (it->legs.size() > 0) {

            //Find first leg: This is the last leg, or the leg where the start time is in the past, and then next start time is in the future. Leg times are from the start of the day of the scenario start.
            irr::u32 currentLeg = 0;
            bool currentLegFound = false;
            for (u32 i=0; i < it->legs.size()-1; i++) {
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
            if (currentLeg < it->legs.size() - 1) { //If not on the last leg, find time until we change onto next course

                irr::f32 legStartX = screenCentreX + relPosX;
                irr::f32 legStartY = screenCentreY - relPosY;

                irr::f32 legEndX = legStartX; //Default values in case current leg is zero length
                irr::f32 legEndY = legStartY;

                currentLegTimeRemaining = it->legs.at(currentLeg+1).startTime - time;
                if (currentLegTimeRemaining>0) {
                    //Line starts at screenCentreX + relPosX, screenCentreY-relPosY
                    irr::f32 legLengthPx = currentLegTimeRemaining * it->legs.at(currentLeg).speed * KTS_TO_MPS / metresPerPx;
                    legEndX = legStartX + legLengthPx*sin(it->legs.at(currentLeg).bearing * RAD_IN_DEG);
                    legEndY = legStartY - legLengthPx*cos(it->legs.at(currentLeg).bearing * RAD_IN_DEG);

                    irr::core::position2d<s32> startLine (legStartX, legStartY);
                    irr::core::position2d<s32> endLine (legEndX, legEndY);

                    device->getVideoDriver()->draw2DLine(startLine,endLine);

                } //If currentLegTimeRemaining > 0

                //Draw remaining legs, excluding 'stop'one
                for (irr::u32 i = currentLeg+1; i < it->legs.size() - 1; i++) {
                    irr::f32 legTimeRemaining = it->legs.at(i+1).startTime -  it->legs.at(i).startTime;
                    irr::f32 legLengthPx = legTimeRemaining * it->legs.at(i).speed * KTS_TO_MPS / metresPerPx;

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
                }

            }

            //std::cout << "Leg " << currentLeg << " Time until course change: " << currentLegTimeRemaining << std::endl;

        }//If Legs.size() >0

    }

}
