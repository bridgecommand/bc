
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

#include <vector>

#include "irrlicht.h"

#include "PositionDataStruct.hpp"
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"

#include "GUI.hpp"

class ControllerModel //Start of the 'Model' part of MVC
{

public:

    //ControllerModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ControllerModel(irr::IrrlichtDevice* device, GUIMain* gui);
    ~ControllerModel();
    void update(const irr::f32& time, const ShipData& ownShipData, const std::vector<OtherShipData>& otherShipsData, const std::vector<PositionData>& buoysData);
    void updateSelectedShip(irr::s32 index); //To be called from eventReceiver, where index is from the combo box
    void updateSelectedLeg(irr::s32 index); //To be called from eventReceiver, where index is from the combo box

private:

    GUIMain* gui;
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;

    irr::video::IImage* unscaledMap;
    irr::video::IImage* scaledMap;

    irr::f32 terrainLong;
    irr::f32 terrainLat;
    irr::f32 terrainLongExtent;
    irr::f32 terrainLatExtent;
    irr::f32 terrainXWidth;
    irr::f32 terrainZWidth;

    irr::f32 metresPerPx;

    irr::s32 selectedShip; //Own ship as -1, other ships as 0 upwards
    irr::s32 selectedLeg; //No leg as -1, legs as 0 upwards

};

#endif // __CONTROLLERMODEL_HPP_INCLUDED__
