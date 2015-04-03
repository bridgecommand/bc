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

    void GUIMain::updateGuiData(irr::f32 ownShipPosX, irr::f32 ownShipPosZ, irr::u32 numberOfBuoys, irr::video::ITexture* displayMapTexture)
    {

        //(Todo - use references where appropriate)

        //Show map texture
        device->getVideoDriver()->draw2DImage(displayMapTexture, irr::core::position2d<irr::s32>(0,0));
        //TODO: Check that conversion to texture does not distort image

        //update heading display element
        core::stringw displayText = language->translate("pos");
        displayText.append(core::stringw(ownShipPosX));
        displayText.append(L" ");
        displayText.append(core::stringw(ownShipPosZ));
        displayText.append(L"\n");

        //Show number of buoys
        displayText.append(core::stringw(numberOfBuoys));
        displayText.append(L"\n");

        dataDisplay->setText(displayText.c_str());

        //draw cross hairs
        irr::s32 width = device->getVideoDriver()->getScreenSize().Width;
        irr::s32 height = device->getVideoDriver()->getScreenSize().Height;
        device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(width/2,0),irr::core::position2d<s32>(width/2,height),video::SColor(255, 255, 255, 255));
        device->getVideoDriver()->draw2DLine(irr::core::position2d<s32>(0,height/2),irr::core::position2d<s32>(width,height/2),video::SColor(255, 255, 255, 255));

        guienv->drawAll();

    }
