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

#include "LandObjects.hpp"

#include "LandObject.hpp"
#include "IniFile.hpp"
#include "Terrain.hpp"
//#include "Constants.hpp"
#include "SimulationModel.hpp"

#include <iostream>

//using namespace irr;

LandObjects::LandObjects()
{

}

LandObjects::~LandObjects()
{
    //dtor
}

void LandObjects::load(const std::string& worldName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain, irr::IrrlichtDevice* dev)
{
    this->model = model;
    
    //get landObject.ini filename
    std::string scenarioLandObjectFilename = worldName;
    scenarioLandObjectFilename.append("/landobject.ini");

    //Find number of objects
    irr::u32 numberOfObjects;
    numberOfObjects = IniFile::iniFileTou32(scenarioLandObjectFilename,"Number");
    for(irr::u32 currentObject=1;currentObject<=numberOfObjects;currentObject++) {

        //Get Object type and construct filename
        std::string objectName = IniFile::iniFileToString(scenarioLandObjectFilename,IniFile::enumerate1("Type",currentObject));
        //Get object position
        irr::f32 objectX = model->longToX(IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Long",currentObject)));
        irr::f32 objectZ = model->latToZ(IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Lat",currentObject)));
        irr::f32 objectY = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("HeightCorrection",currentObject));
        
        //Check if we should 'morph' the model to fit the land (mostly for OSM2World models)
        bool morph = false;
        if (IniFile::iniFileTou32(scenarioLandObjectFilename, IniFile::enumerate1("Morph", currentObject)) == 1) {
            morph = true;
        } else {
            // Don't allow both 'absolute' and 'morph'.
            //Check if land object is given in absolute height, or relative to terrain.
            if (IniFile::iniFileTou32(scenarioLandObjectFilename, IniFile::enumerate1("Absolute", currentObject)) == 0) {
                objectY += terrain->getHeight(objectX, objectZ);
            } else if (IniFile::iniFileTou32(scenarioLandObjectFilename, IniFile::enumerate1("Absolute", currentObject)) == 2) {
                objectY += std::max(0.0f, terrain->getHeight(objectX, objectZ));
            }
        }

        //Get rotation
        irr::f32 rotation = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Rotation",currentObject));

        //Check if we should be able to interact with this by collision
        bool collisionObject = IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Collision",currentObject))==1;

        //Check if we should be able to see on the radar
        bool radarObject = IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Radar",currentObject))==1;

        //Create land object and load into vector
        std::string internalName = "LandObject_";
        internalName.append(std::to_string(landObjects.size()));
        landObjects.push_back(LandObject (objectName.c_str(),internalName,worldName,irr::core::vector3df(objectX,objectY,objectZ),rotation,collisionObject,radarObject,morph,terrain,smgr,dev));

    }

    // Add 'pontoons' here. Initially a simple test implementation with a dummy object for each node
    std::string scenarioPontoonFilename = worldName;
    scenarioPontoonFilename.append("/pontoon.ini");

    //Find number of objects
    irr::u32 numberOfPontoons;
    numberOfPontoons = IniFile::iniFileTou32(scenarioPontoonFilename, "Number");
    for (irr::u32 currentPontoon = 1; currentPontoon <= numberOfPontoons; currentPontoon++) {
        irr::u32 numberOfNodes;
        numberOfNodes = IniFile::iniFileTou32(scenarioPontoonFilename, IniFile::enumerate1("Nodes", currentPontoon));
        for (irr::u32 currentNode = 1; currentNode < numberOfNodes; currentNode++) {
            //Get position of this node and next one
            irr::f32 node1X = model->longToX(IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("Long", currentPontoon, currentNode)));
            irr::f32 node1Z = model->latToZ(IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("Lat", currentPontoon, currentNode)));
            irr::f32 node1Y = IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("HeightCorrection", currentPontoon, currentNode));

            irr::f32 node2X = model->longToX(IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("Long", currentPontoon, currentNode + 1)));
            irr::f32 node2Z = model->latToZ(IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("Lat", currentPontoon, currentNode + 1)));
            irr::f32 node2Y = IniFile::iniFileTof32(scenarioPontoonFilename, IniFile::enumerate2("HeightCorrection", currentPontoon, currentNode + 1));

            irr::f32 midPointX = (node1X + node2X) / 2.0;
            irr::f32 midPointY = (node1Y + node2Y) / 2.0;
            irr::f32 midPointZ = (node1Z + node2Z) / 2.0;

            // Find length and angle of this section
            irr::f32 rotation = atan2(node2X - node1X, node2Z - node1Z) * irr::core::RADTODEG;
            irr::f32 sectionLength = pow(pow(node2X - node1X, 2.0) + pow(node2Z - node1Z, 2.0), 0.5);

            //Create land object and load into vector
            std::string internalName = "LandObject_";
            internalName.append(std::to_string(landObjects.size()));
            
            // Set properties
            std::string objectName = "PONTOON_INTERNAL"; // This name is used to trigger floating behaviour in 'land object'
            bool collisionObject = true;
            bool radarObject = false; // TODO: Try turning this on?
            bool morph = false;

            landObjects.push_back(LandObject(objectName.c_str(), internalName, worldName, irr::core::vector3df(midPointX, midPointY, midPointZ), rotation, collisionObject, radarObject, morph, terrain, smgr, dev, true, sectionLength));

        }
    }
}

void LandObjects::update(irr::f32 tideHeight, irr::core::vector3df ownShipPosition, irr::f32 ownShipLength)
{
    for (std::vector<LandObject>::iterator it = landObjects.begin(); it != landObjects.end(); ++it) {

        if (it->getFloating()) {
            irr::f32 xPos, yPos, zPos;
            irr::core::vector3df pos = it->getPosition();
            xPos = pos.X;
            //yPos = tideHeight + model->getWaveHeight(pos.X, pos.Z) + it->getHeightCorrection();
            yPos = tideHeight + it->getHeightCorrection(); // Don't include wave height for floating pontoons etc.
            zPos = pos.Z;
            it->setPosition(irr::core::vector3df(xPos, yPos, zPos));
        }
        //Set or clear triangle selector depending on distance from own ship;
        irr::f32 landObjectMaxLength = it->getSceneNode()->getTransformedBoundingBox().getExtent().getLength(); // Max length between corners of the bounding box
        if (it->getSceneNode()->getAbsolutePosition().getDistanceFrom(ownShipPosition) < (ownShipLength + landObjectMaxLength)) {
            it->enableTriangleSelector(true);
        }
        else {
            it->enableTriangleSelector(false);
        }

    }
}

irr::u32 LandObjects::getNumber() const
{
    return landObjects.size();
}

void LandObjects::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    for(std::vector<LandObject>::iterator it = landObjects.begin(); it != landObjects.end(); ++it) {
        it->moveNode(deltaX,deltaY,deltaZ);
    }
}

irr::scene::ISceneNode* LandObjects::getSceneNode(int number)
{
    if (number < (int)landObjects.size() && number >= 0) {
        return landObjects.at(number).getSceneNode();
    } else {
        return 0;
    }
}

