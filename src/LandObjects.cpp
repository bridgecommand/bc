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
        irr::f32 objectY = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("HeightCorrection",currentObject));;
        //Check if land object is given in absolute height, or relative to terrain.
        if (IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Absolute",currentObject))!=1) {
            objectY += terrain->getHeight(objectX,objectZ);
        }

        //Get rotation
        irr::f32 rotation = IniFile::iniFileTof32(scenarioLandObjectFilename,IniFile::enumerate1("Rotation",currentObject));

        //Check if we should be able to interact with this by collision
        bool collisionObject = IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Collision",currentObject))==1;

        //Check if we should be able to see on the radar
        bool radarObject = IniFile::iniFileTou32(scenarioLandObjectFilename,IniFile::enumerate1("Radar",currentObject))==1;

        //Create land object and load into vector
        landObjects.push_back(LandObject (objectName.c_str(),worldName,irr::core::vector3df(objectX,objectY,objectZ),rotation,collisionObject,radarObject,terrain,smgr,dev));

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

