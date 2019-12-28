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

#include "ControllerModel.hpp"
#include "../IniFile.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"
#include <iostream>

//Constructor
ControllerModel::ControllerModel(irr::IrrlichtDevice* device, GUIMain* gui)
{

    this->gui = gui;
    this->device = device;
    driver = device->getVideoDriver();
}

//Destructor
ControllerModel::~ControllerModel()
{

}

void ControllerModel::update(const irr::f32& time, const ShipData& ownShipData)
{
    gui->updateGuiData(time,ownShipData.heading, ownShipData.rudder, ownShipData.wheel, ownShipData.portEngine, ownShipData.stbdEngine);
}
