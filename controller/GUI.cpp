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
        ownShipPosX = 0;
        ownShipPosZ = 0;


    }

    void GUIMain::updateGuiData(irr::f32 ownShipPosX, irr::f32 ownShipPosZ)
    {
        this->ownShipPosX = ownShipPosX;
        this->ownShipPosZ = ownShipPosZ;
    }

    void GUIMain::drawGUI()
    {
        //update heading display element
        core::stringw displayText = language->translate("pos");
        displayText.append(core::stringw(ownShipPosX));
        displayText.append(L" ");
        displayText.append(core::stringw(ownShipPosZ));
        displayText.append(L"\n");


        dataDisplay->setText(displayText.c_str());

        guienv->drawAll();

    }

