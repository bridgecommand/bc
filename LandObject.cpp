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

#include "LandObject.hpp"
#include "IniFile.hpp"
#include "Utilities.hpp"

#include <iostream>

using namespace irr;

LandObject::LandObject(const std::string& name, const irr::core::vector3df& location, irr::f32 rotation, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev)
{

    std::string basePath = "Models/LandObject/" + name + "/";
    std::string userFolder = Utilities::getUserDir();
    //Read model from user dir if it exists there.
    if (Utilities::pathExists(userFolder + basePath)) {
        basePath = userFolder + basePath;
    }

    //Load from individual object.ini file if it exists
    std::string objectIniFilename = basePath + "/object.ini";

    //get filename from ini file (or empty string if file doesn't exist)
    std::string objectFileName = IniFile::iniFileToString(objectIniFilename,"FileName");
    if (objectFileName=="") {
        objectFileName = "object.x"; //Default if not set
    }

    //get scale factor from ini file (or zero if not set - assume 1)
    f32 objectScale = IniFile::iniFileTof32(objectIniFilename,"Scalefactor");
    if (objectScale==0.0) {
        objectScale = 1.0; //Default if not set
    }

    std::string objectFullPath = basePath + objectFileName;

    //Load the mesh
    scene::IMesh* objectMesh = smgr->getMesh(objectFullPath.c_str());
	//add to scene node
	if (objectMesh==0) {
        //Failed to load mesh - load with dummy and continue
        dev->getLogger()->log("Failed to load land object model:");
        dev->getLogger()->log(objectFullPath.c_str());
        landObject = smgr->addCubeSceneNode(0.1);
    } else {
        landObject = smgr->addMeshSceneNode( objectMesh, 0, -1, location );
    }

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(landObject->getMaterialCount()>0) {
        for(u32 mat=0;mat<landObject->getMaterialCount();mat++) {
            landObject->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    landObject->setScale(core::vector3df(objectScale,objectScale,objectScale));
    landObject->setRotation(irr::core::vector3df(0,rotation,0));
    landObject->setMaterialFlag(video::EMF_FOG_ENABLE, true);
    landObject->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

}

LandObject::~LandObject()
{
    //dtor
}

irr::core::vector3df LandObject::getPosition() const
{
    landObject->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return landObject->getAbsolutePosition();
}

void LandObject::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    core::vector3df currentPos = landObject->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    landObject->setPosition(core::vector3df(newPosX,newPosY,newPosZ));
}
