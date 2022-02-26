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

#include "Buoy.hpp"
#include "RadarData.hpp"
#include "Angles.hpp"
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "Constants.hpp"

#include <iostream>
#include <algorithm>

//using namespace irr;

Buoy::Buoy(const std::string& name, const std::string& worldName, const irr::core::vector3df& location, irr::f32 radarCrossSection, bool floating, irr::f32 heightCorrection, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev)
{

    std::string basePath = "Models/Buoy/" + name + "/";
    std::string userFolder = Utilities::getUserDir();
    //Read model from user dir if it exists there.
    if (Utilities::pathExists(userFolder + basePath)) {
        basePath = userFolder + basePath;
    } else if (Utilities::pathExists(worldName + "/" + basePath)) {
        //Otherwise, if it exists in the world model folder, use this
        basePath = worldName + "/" + basePath;
    }

    this->heightCorrection = heightCorrection;
    irr::core::vector3df buoyLocation = location;
    buoyLocation.Y += heightCorrection;

    //Load from individual buoy.ini file if it exists
    std::string buoyIniFilename = basePath + "buoy.ini";

    //get filename from ini file (or empty string if file doesn't exist)
    std::string buoyFileName = IniFile::iniFileToString(buoyIniFilename,"FileName", "buoy.x");

    //get scale factor from ini file (or zero if not set - assume 1)
    irr::f32 buoyScale = IniFile::iniFileTof32(buoyIniFilename,"Scalefactor", 1.f);

    //The path to the actual model file
    std::string buoyFullPath = basePath + buoyFileName;

    //Load the mesh
    irr::scene::IMesh* buoyMesh = smgr->getMesh(buoyFullPath.c_str());

	//add to scene node
	if (buoyMesh==0) {
        //Failed to load mesh - load with dummy and continue
        dev->getLogger()->log("Failed to load buoy model:");
        dev->getLogger()->log(buoyFullPath.c_str());
        buoy = smgr->addCubeSceneNode(0.1);
        selector = 0;
    } else {
        buoy = smgr->addMeshSceneNode( buoyMesh, 0, -1, location );
        //Add triangle selector and make pickable
        buoy->setID(IDFlag_IsPickable);
        selector=smgr->createTriangleSelector(buoyMesh,buoy);
        
        //This selector is now set or not depending on the distance from the own ship
        //if(selector) {
        //    buoy->setTriangleSelector(selector);
        //}
    }

    triangleSelectorEnabled = false;
    buoy->setName("Buoy");

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(buoy->getMaterialCount()>0) {
        for(irr::u32 mat=0;mat<buoy->getMaterialCount();mat++) {
            buoy->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    buoy->setScale(irr::core::vector3df(buoyScale,buoyScale,buoyScale));
    buoy->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
    buoy->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    //store length and RCS information for radar etc
    length = buoy->getBoundingBox().getExtent().Z;
    height = buoy->getBoundingBox().getExtent().Y * 0.75 + heightCorrection; //Assume 3/4 of the mesh is above water

    rcs = radarCrossSection; //Value given to constructor by Buoys.
    if (rcs == 0.0) {
        rcs = 0.005*std::pow(length,3); //Default RCS if not set, base radar cross section on length^3 (following RCS table Ship_RCS_table.pdf)
    }

    this->floating = floating; //Does the buoy respond to the water

}

Buoy::~Buoy()
{
    //dtor
}

irr::core::vector3df Buoy::getPosition() const
{
    buoy->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return buoy->getAbsolutePosition();
}

void Buoy::setPosition(irr::core::vector3df position)
{
    buoy->setPosition(position);
}

void Buoy::setRotation(irr::core::vector3df rotation)
{
    buoy->setRotation(rotation);
}

irr::f32 Buoy::getLength() const
{
    return length;
}

irr::f32 Buoy::getHeight() const
{
    return height;
}

irr::f32 Buoy::getRCS() const
{
    return rcs;
}

irr::f32 Buoy::getHeightCorrection() const
{
    return heightCorrection;
}

bool Buoy::getFloating() const
{
    return floating;
}

irr::scene::ISceneNode* Buoy::getSceneNode() const
{
    return (irr::scene::ISceneNode*)buoy;
}

RadarData Buoy::getRadarData(irr::core::vector3df scannerPosition) const
//Get data relative to scannerPosition
//Similar code in OtherShip.cpp
{
    RadarData radarData;

    //Get information about this buoy, and return a RadarData struct containing info
    irr::core::vector3df contactPosition = getPosition();
    irr::core::vector3df relativePosition = contactPosition-scannerPosition;

    radarData.relX = relativePosition.X;
    radarData.relZ = relativePosition.Z;

    radarData.angle = relativePosition.getHorizontalAngle().Y;
    radarData.range = relativePosition.getLength();

    radarData.heading = 0.0;

    radarData.height=getHeight();
    radarData.solidHeight=0; //Assume buoy never blocks radar
    //radarData.radarHorizon=99999; //ToDo: Implement when ARPA is implemented
    radarData.length=getLength();
    radarData.rcs=getRCS();

    //Calculate angles and ranges to each end of the contact
    irr::f32 relAngle1 = Angles::normaliseAngle(irr::core::RADTODEG*std::atan2( radarData.relX + 0.5*radarData.length*std::sin(irr::core::DEGTORAD*radarData.heading), radarData.relZ + 0.5*radarData.length*std::cos(irr::core::DEGTORAD*radarData.heading) ));
    irr::f32 relAngle2 = Angles::normaliseAngle(irr::core::RADTODEG*std::atan2( radarData.relX - 0.5*radarData.length*std::sin(irr::core::DEGTORAD*radarData.heading), radarData.relZ - 0.5*radarData.length*std::cos(irr::core::DEGTORAD*radarData.heading) ));
    irr::f32 range1 = std::sqrt(std::pow(radarData.relX + 0.5*radarData.length*std::sin(irr::core::DEGTORAD*radarData.heading),2) + std::pow(radarData.relZ + 0.5*radarData.length*std::cos(irr::core::DEGTORAD*radarData.heading),2));
    irr::f32 range2 = std::sqrt(std::pow(radarData.relX - 0.5*radarData.length*std::sin(irr::core::DEGTORAD*radarData.heading),2) + std::pow(radarData.relZ - 0.5*radarData.length*std::cos(irr::core::DEGTORAD*radarData.heading),2));
    radarData.minRange=std::min(range1,range2);
    radarData.maxRange=std::max(range1,range2);
    radarData.minAngle=std::min(relAngle1,relAngle2);
    radarData.maxAngle=std::max(relAngle1,relAngle2);

    //Initial defaults: Will need changing with full implementation
    radarData.hidden=false;
    radarData.racon=""; //Racon code if set
    radarData.raconOffsetTime=0.0;
    radarData.SART=false;

    radarData.contact = (void*)this;

    return radarData;
}

void Buoy::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    irr::core::vector3df currentPos = buoy->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    buoy->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}

void Buoy::enableTriangleSelector(bool selectorEnabled)
{
    
    //Only re-set if we need to change the state
    
    if (selectorEnabled && !triangleSelectorEnabled) {
        buoy->setTriangleSelector(selector);
        triangleSelectorEnabled = true;
    } 
    
    if (!selectorEnabled && triangleSelectorEnabled) {
        buoy->setTriangleSelector(0);
        triangleSelectorEnabled = false;
    }

}
