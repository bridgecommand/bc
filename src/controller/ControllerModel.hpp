
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
#include <cstdint>

#include "PositionDataStruct.hpp"

// Forward declarations for Irrlicht pointer types
namespace irr {
    class IrrlichtDevice;
    namespace video {
        class IVideoDriver;
        class IImage;
    }
}
#include "ShipDataStruct.hpp"
#include "OtherShipDataStruct.hpp"
#include "AISData.hpp"
#include "Network.hpp"

#include "GUI.hpp"

class ControllerModel //Start of the 'Model' part of MVC
{

public:

    //ControllerModel(irr::IrrlichtDevice* dev, irr::scene::ISceneManager* scene, GUIMain* gui, std::string scenarioName);
    ControllerModel(irr::IrrlichtDevice* device, GUIMain* gui, Network* network, std::string worldName, uint32_t _zoomLevels);
    ~ControllerModel();
    float longToX(float longitude) const;
    float latToZ(float latitude) const;
    void update(const float& time, const ShipData& ownShipData, const std::vector<OtherShipDisplayData>& otherShipsData, const std::vector<PositionData>& buoysData, const float& weather, const float& visibility, const float& rain, bool& mobVisible, PositionData& mobData, const std::vector<AISData>& aisData, const float& windDirection, const float& windSpeed, const float& streamDirection, const float& streamSpeed, const bool& streamOverride);
    void resetOffset(); //Re-centre the map on the own-ship
    void updateSelectedShip(int32_t index); //To be called from eventReceiver, where index is from the combo box
    void updateSelectedLeg(int32_t index); //To be called from eventReceiver, where index is from the combo box
    void setMouseDown(bool isMouseDown); //To be called from event receiver, each time mouse left click state changes.
    void increaseZoom();
    void decreaseZoom();

private:

    GUIMain* gui;
    Network* network;
    irr::IrrlichtDevice* device;
    irr::video::IVideoDriver* driver;

	uint32_t zoomLevels;

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
    int32_t mouseLastPositionX;
    int32_t mouseLastPositionY;

    int32_t mapOffsetX; //Pixel offset of maps, to allow click and drag.
    int32_t mapOffsetZ;

    std::vector<AISData> aisShips;

    int32_t selectedShip; //Own ship as -1, other ships as 0 upwards
    int32_t selectedLeg; //No leg as -1, legs as 0 upwards

};

#endif // __CONTROLLERMODEL_HPP_INCLUDED__
