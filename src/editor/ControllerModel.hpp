
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
#include "OwnShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "GeneralDataStruct.hpp"

#include "GUI.hpp"
#include "../Lang.hpp"

class ControllerModel //Start of the 'Model' part of MVC
{

public:

    //ControllerModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ControllerModel(irr::IrrlichtDevice* device, Lang* lang, GUIMain* gui, std::string worldName, OwnShipEditorData* ownShipData, std::vector<OtherShipEditorData>* otherShipsData, std::vector<PositionData>* buoysData, GeneralData* generalData, irr::u32 _zoomLevels);
    ~ControllerModel();
    irr::f32 longToX(irr::f32 longitude) const;
    irr::f32 latToZ(irr::f32 latitude) const;
    irr::f32 xToLong(irr::f32 x) const;
    irr::f32 zToLat(irr::f32 z) const;
    void update(); //Called once per loop from the main function.
    void resetOffset(); //Re-centre the map on the own-ship

    //Methods used to update the state, called by the event receiver:
    void setShipPosition(irr::s32 ship, irr::core::vector2df position); //To be called from eventReceiver
    void updateSelectedShip(irr::s32 index); //To be called from eventReceiver, where index is from the combo box
    void updateSelectedLeg(irr::s32 index); //To be called from eventReceiver, where index is from the combo box
    void setScenarioData(GeneralData newData); //To be called from event receiver
    void checkName(); //Check if the scenario name chosen will mean that an existing scenario gets overwritten, and update flag in GeneralData

    void changeLeg(irr::s32 ship, irr::s32 index, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance); //Change othership (or ownship) course, speed etc.
    void deleteLeg(irr::s32 ship, irr::s32 index);
    void addLeg(irr::s32 ship, irr::s32 afterLegNumber, irr::f32 legCourse, irr::f32 legSpeed, irr::f32 legDistance);
    void setMMSI(irr::s32 ship, int mmsi);
    void addShip(std::string name, irr::core::vector2df position);
	void deleteShip(irr::s32 ship);
    void recalculateLegTimes();

    void changeOwnShipName(std::string name);
    void changeOtherShipName(irr::s32 ship, std::string name);

    void save();

    void setMouseDown(bool isMouseDown); //To be called from event receiver, each time mouse left click state changes.
    void increaseZoom();
    void decreaseZoom();

private:

    GUIMain* gui;
    Lang* lang;
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;

	irr::u32 zoomLevels;

    //Data shared from main
    OwnShipEditorData* ownShipData;
    std::vector<PositionData>* buoysData;
    std::vector<OtherShipEditorData>* otherShipsData;
    GeneralData* generalData;
    std::string worldName;

    irr::video::IImage* unscaledMap;
    std::vector<irr::video::IImage*> scaledMap;
    irr::u32 currentZoom;

    irr::f32 terrainLong;
    irr::f32 terrainLat;
    irr::f32 terrainLongExtent;
    irr::f32 terrainLatExtent;
    irr::f32 terrainXWidth;
    irr::f32 terrainZWidth;

    std::vector<irr::f32> metresPerPx;

    bool mouseDown; //This is controlled via setMouseDown(bool) from the event receiver
    bool mouseClickedLastUpdate;
    irr::core::position2d<irr::s32> mouseLastPosition;

    irr::s32 mapOffsetX; //Pixel offset of maps, to allow click and drag.
    irr::s32 mapOffsetZ;

    irr::s32 selectedShip; //Own ship as -1, other ships as 0 upwards
    irr::s32 selectedLeg; //No leg as -1, legs as 0 upwards

};

#endif // __CONTROLLERMODEL_HPP_INCLUDED__
