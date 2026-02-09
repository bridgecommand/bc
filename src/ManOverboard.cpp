/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2017 James Packer

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

#include "ManOverboard.hpp"
#include "irrlicht.h"
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "SimulationModel.hpp"
#include "Constants.hpp"
#include "Terrain.hpp"

#include <iostream>

namespace {
    inline irr::core::vector3df toIrrVec(const bc::graphics::Vec3& v) {
        return irr::core::vector3df(v.x, v.y, v.z);
    }
    inline bc::graphics::Vec3 fromIrrVec(const irr::core::vector3df& v) {
        return bc::graphics::Vec3(v.X, v.Y, v.Z);
    }
}

ManOverboard::ManOverboard(const bc::graphics::Vec3& location, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev, SimulationModel* model)
{

    this->model=model;

    std::string basePath = "Models/ManOverboard/";
    std::string userFolder = Utilities::getUserDir();
    //Read model from user dir if it exists there.
    if (Utilities::pathExists(userFolder + basePath)) {
        basePath = userFolder + basePath;
    }

    //Load from individual ManOverboard.ini file if it exists
    std::string mobIniFilename = basePath + "ManOverboard.ini";

    //get filename from ini file (or empty string if file doesn't exist)
    std::string mobFileName = IniFile::iniFileToString(mobIniFilename,"FileName", "ManOverboard.x");

    //get scale factor from ini file (or zero if not set - assume 1)
    float mobScale = IniFile::iniFileTof32(mobIniFilename,"Scalefactor", 1.f);

    //The path to the actual model file
    std::string mobFullPath = basePath + mobFileName;

    //Load the mesh
    irr::scene::IMesh* mobMesh = smgr->getMesh(mobFullPath.c_str());

	//add to scene node
	if (mobMesh==0) {
        //Failed to load mesh - load with dummy and continue
        dev->getLogger()->log("Failed to load man overboard model:");
        dev->getLogger()->log(mobFullPath.c_str());
        man = smgr->addCubeSceneNode(0.1);
    } else {
        man = smgr->addMeshSceneNode( mobMesh, 0, -1, toIrrVec(location) );
    }

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(man->getMaterialCount()>0) {
        for(uint32_t mat=0;mat<man->getMaterialCount();mat++) {
            man->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    man->setScale(irr::core::vector3df(mobScale,mobScale,mobScale));
    man->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
    man->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

}

void ManOverboard::setVisible(bool isVisible)
{
    man->setVisible(isVisible);
}

bool ManOverboard::getVisible() const
{
    return man->isVisible();
}

bc::graphics::Vec3 ManOverboard::getPosition() const
{
    man->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return fromIrrVec(man->getAbsolutePosition());
}

void ManOverboard::setPosition(bc::graphics::Vec3 position)
{
    man->setPosition(toIrrVec(position));
}

void ManOverboard::setRotation(bc::graphics::Vec3 rotation)
{
    man->setRotation(toIrrVec(rotation));
}

irr::scene::ISceneNode* ManOverboard::getSceneNode() const
{
    return (irr::scene::ISceneNode*)man;
}

void ManOverboard::moveNode(float deltaX, float deltaY, float deltaZ)
{
    irr::core::vector3df currentPos = man->getPosition();
    float newPosX = currentPos.X + deltaX;
    float newPosY = currentPos.Y + deltaY;
    float newPosZ = currentPos.Z + deltaZ;

    man->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}

void ManOverboard::update(float deltaTime, float tideHeight)
{
    //Move with tide and waves
    bc::graphics::Vec3 pos=getPosition();
    pos.y = tideHeight + model->getWaveHeight(pos.x,pos.z);

    //Move with tidal stream (if not aground)
    float depth = -1*model->getTerrain()->getHeight(pos.x,pos.z)+pos.y;
    bc::graphics::Vec2 mobVector = model->getTidalStream(model->getTerrain()->xToLong(pos.x), model->getTerrain()->zToLat(pos.z),model->getTimestamp());

    // Add component from wind
    float windSpeed = model->getWindSpeed() * KTS_TO_MPS;
    float windDirection = model->getWindDirection();
    // Convert this into wind axial speed and wind lateral speed
    float windFlowDirection = windDirection + 180; // Wind direction is where the wind is from. We want where it is flowing towards
    float windX = windSpeed * sin(windFlowDirection * RAD_IN_DEG);
    float windZ = windSpeed * cos(windFlowDirection * RAD_IN_DEG);
    // Assume that the MoB moves at 1/10 of the wind speed
    mobVector.x += windX * 0.1;
    mobVector.y += windZ * 0.1;

    // Apply movement vector
    if (depth > 0) {
        float streamScaling = fmin(1,depth); //Reduce effect as water gets shallower
        pos.x += mobVector.x*deltaTime*streamScaling;
        pos.z += mobVector.y*deltaTime*streamScaling;
    }

    setPosition(pos);
}
