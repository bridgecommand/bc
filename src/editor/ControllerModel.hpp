
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
#include "../ScenarioDataStructure.hpp"

#include "GUI.hpp"
#include "../Lang.hpp"

class ControllerModel //Start of the 'Model' part of MVC
{

public:

    //ControllerModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ControllerModel(irr::IrrlichtDevice* device, Lang* lang, GUIMain* gui, std::string worldName, ScenarioData* scenarioData, std::vector<PositionData>* buoysData, std::vector<PositionData>* landObjectsData, uint32_t _zoomLevels);
    ~ControllerModel();
    float longToX(float longitude) const;
    float latToZ(float latitude) const;
    float xToLong(float x) const;
    float zToLat(float z) const;
    void update(); //Called once per loop from the main function.
    void resetOffset(); //Re-centre the map on the own-ship

    //Methods used to update the state, called by the event receiver:
    void setShipPosition(int32_t ship, irr::core::vector2df position); //To be called from eventReceiver
    void updateSelectedShip(int32_t index); //To be called from eventReceiver, where index is from the combo box
    void updateSelectedLeg(int32_t index); //To be called from eventReceiver, where index is from the combo box
    void setGeneralScenarioData(ScenarioData newData); //To be called from event receiver
    void checkName(); //Check if the scenario name chosen will mean that an existing scenario gets overwritten, and update flag in GeneralData

    void changeLeg(int32_t ship, int32_t index, float legCourse, float legSpeed, float legDistance); //Change othership (or ownship) course, speed etc.
    void deleteLeg(int32_t ship, int32_t index);
    void addLeg(int32_t ship, int32_t afterLegNumber, float legCourse, float legSpeed, float legDistance);
    void setMMSI(int32_t ship, int mmsi);
    void setDrifting(int32_t ship, bool drifting);
    void addShip(std::string name, irr::core::vector2df position);
	void deleteShip(int32_t ship);
    void recalculateLegTimes();

    void changeOwnShipName(std::string name);
    void changeOtherShipName(int32_t ship, std::string name);

    void save();

    void setMouseDown(bool isMouseDown); //To be called from event receiver, each time mouse left click state changes.
    void increaseZoom();
    void decreaseZoom();

private:

    GUIMain* gui;
    Lang* lang;
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;

	uint32_t zoomLevels;

    //Data shared from main
    std::vector<PositionData>* buoysData;
    std::vector<PositionData>* landObjectsData;
    ScenarioData* scenarioData;
    std::string worldName;

    irr::video::IImage* unscaledMap;
    std::vector<irr::video::IImage*> scaledMap;
    uint32_t currentZoom;

    float terrainLong;
    float terrainLat;
    float terrainLongExtent;
    float terrainLatExtent;
    float terrainXWidth;
    float terrainZWidth;

    std::vector<float> metresPerPx;

    bool mouseDown; //This is controlled via setMouseDown(bool) from the event receiver
    bool mouseClickedLastUpdate;
    irr::core::position2d<int32_t> mouseLastPosition;

    int32_t mapOffsetX; //Pixel offset of maps, to allow click and drag.
    int32_t mapOffsetZ;

    int32_t selectedShip; //Own ship as -1, other ships as 0 upwards
    int32_t selectedLeg; //No leg as -1, legs as 0 upwards

};

#endif // __CONTROLLERMODEL_HPP_INCLUDED__
