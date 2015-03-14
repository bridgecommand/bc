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
#include <iostream>

//Constructor
ControllerModel::ControllerModel(GUIMain* gui)
{

    this->gui = gui;
    ownShipPosX = 0;
    ownShipPosZ = 0;

}

//Destructor
ControllerModel::~ControllerModel()
{

}

irr::f32 ControllerModel::getPosX() const
{
    return ownShipPosX;
}

irr::f32 ControllerModel::getPosZ() const
{
    return ownShipPosZ;
}

void ControllerModel::setPosX(irr::f32 x)
{
    ownShipPosX = x;
    //std::cout << "Current X: " << ownShipPosX << std::endl;
}

void ControllerModel::setPosZ(irr::f32 z)
{
    ownShipPosZ = z;
    //std::cout << "Current Z: " << ownShipPosZ << std::endl;
}

void ControllerModel::update()
{
    //Send the current data to the gui
    gui->updateGuiData(ownShipPosX,ownShipPosZ);
}
