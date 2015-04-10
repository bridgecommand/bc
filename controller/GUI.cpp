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
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.09*su,0.61*sh,0.45*su,0.95*sh), true, false, 0, -1, true); //Actual text set later
        //dataTree = guienv->addTreeView(core::rect<s32>(0.09*su,0.61*sh,0.45*su,0.95*sh));
        //dataTree->setToolTipText ( L"Show the current Scenegraph" );
        //dataTree->getRoot()->clearChildren();

}

    void GUIMain::updateGuiData(irr::f32 metresPerPx, irr::f32 ownShipPosX, irr::f32 ownShipPosZ, const std::vector<PositionData>& buoys, const std::vector<OtherShipData>& otherShips, irr::video::ITexture* displayMapTexture)
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

        dataDisplay->setText(displayText.c_str());

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
        }

        guienv->drawAll();

    }
