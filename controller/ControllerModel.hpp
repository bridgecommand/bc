
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

#ifndef __CONTROLLERMODEL_HPP_INCLUDED__
#define __CONTROLLERMODEL_HPP_INCLUDED__

#include "irrlicht.h"
#include "GUI.hpp"

class ControllerModel //Start of the 'Model' part of MVC
{

public:

    //ControllerModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ControllerModel(GUIMain* gui);
    ~ControllerModel();
    irr::f32 getPosX() const;
    irr::f32 getPosZ() const;
    void setPosX(irr::f32 x);
    void setPosZ(irr::f32 z);
    void update();

private:

    GUIMain* gui;
    irr::f32 ownShipPosX;
    irr::f32 ownShipPosZ;

};

#endif // __CONTROLLERMODEL_HPP_INCLUDED__
