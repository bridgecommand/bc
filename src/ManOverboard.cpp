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
#include "IniFile.hpp"
#include "Utilities.hpp"
#include "SimulationModel.hpp"

#include <iostream>

//using namespace irr;

ManOverboard::ManOverboard(const irr::core::vector3df& location, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev, SimulationModel* model, Terrain* terrain)
{

    this->model=model;
    this->terrain=terrain;

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
    irr::f32 mobScale = IniFile::iniFileTof32(mobIniFilename,"Scalefactor", 1.f);

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
        man = smgr->addMeshSceneNode( mobMesh, 0, -1, location );
    }

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(man->getMaterialCount()>0) {
        for(irr::u32 mat=0;mat<man->getMaterialCount();mat++) {
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

irr::core::vector3df ManOverboard::getPosition() const
{
    man->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return man->getAbsolutePosition();
}

void ManOverboard::setPosition(irr::core::vector3df position)
{
    man->setPosition(position);
}

void ManOverboard::setRotation(irr::core::vector3df rotation)
{
    man->setRotation(rotation);
}

irr::scene::ISceneNode* ManOverboard::getSceneNode() const
{
    return (irr::scene::ISceneNode*)man;
}

void ManOverboard::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    irr::core::vector3df currentPos = man->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    man->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}

void ManOverboard::update(irr::f32 deltaTime, irr::f32 tideHeight)
{
    //Move with tide and waves
    irr::core::vector3df pos=getPosition();
    pos.Y = tideHeight + model->getWaveHeight(pos.X,pos.Z);

    //Move with tidal stream (if not aground)
    irr::f32 depth = -1*terrain->getHeight(pos.X,pos.Z)+pos.Y;
    irr::core::vector2df stream = model->getTidalStream(terrain->xToLong(pos.X),terrain->zToLat(pos.Z),model->getTimestamp());
    if (depth > 0) {
        irr::f32 streamScaling = fmin(1,depth); //Reduce effect as water gets shallower
        pos.X += stream.X*deltaTime*streamScaling;
        pos.Z += stream.Y*deltaTime*streamScaling;
    }

    setPosition(pos);
}
