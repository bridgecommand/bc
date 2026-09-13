/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2026 James Packer

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

#include "DayShape.hpp"
#include "Utilities.hpp"

DayShape::DayShape(irr::scene::ISceneNode* parent, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev, irr::core::vector3df position, std::string shapeName, std::string shipModelPath, irr::f32 scaleFactor) {

    //Store the scene manager, so we can find the active camera
    this->smgr = smgr;

    std::string fullPath = shipModelPath + shapeName;

    // If model doesn't exist at same level as the own ship model, check in Models/Dayshapes/ folder
    if (!Utilities::pathExists(fullPath)) {
        std::string logMessage = "Cannot find dayshape file " + fullPath;
        fullPath = "Models/Dayshapes/" + shapeName;
        logMessage.append(". Trying " + fullPath);
        dev->getLogger()->log(logMessage.c_str());
     
        // As we're loading from a general model, assume it's scaled relative to metres (not parent model scaling)
        if (parent && parent->getScale().X > 0) {
            scaleFactor /= parent->getScale().X; //Assume scale in all directions is the same
        }
    }

    // If it still doesn't exist, check in user folder
    if (!Utilities::pathExists(fullPath)) {
        std::string logMessage = "Cannot find dayshape file " + fullPath;
        fullPath = Utilities::getUserDir() + fullPath;
        logMessage.append(". Trying " + fullPath);
        dev->getLogger()->log(logMessage.c_str());
    }

    //load mesh
    irr::scene::IAnimatedMesh* shapeMesh = smgr->getMesh(fullPath.c_str());

    //add to scene node
	if (shapeMesh==0) {
        //Failed to load mesh - load with dummy and continue
        dev->getLogger()->log("Failed to load other dayshape model:");
        dev->getLogger()->log(shapeName.c_str());
        shapeMesh = smgr->addSphereMesh("Sphere", 1);
    }
    shapeNode = smgr->addMeshSceneNode( shapeMesh, parent, -1);
    shapeNode->setScale(irr::core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    shapeNode->setPosition(position);

	shapeNode->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
	shapeNode->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting


    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(shapeNode->getMaterialCount()>0) {
        for(irr::u32 mat=0;mat<shapeNode->getMaterialCount();mat++) {
            shapeNode->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }


}

DayShape::~DayShape() {
    //TODO: Check if day shapes are being repeatedly created and destroyed at startup
}

irr::core::vector3df DayShape::getPosition() const
{
    shapeNode->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return shapeNode->getAbsolutePosition();
}

void DayShape::setPosition(irr::core::vector3df position)
{
    shapeNode->setPosition(position);
}


void DayShape::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    irr::core::vector3df currentPos = shapeNode->getPosition();
    irr::f32 newPosX = currentPos.X + deltaX;
    irr::f32 newPosY = currentPos.Y + deltaY;
    irr::f32 newPosZ = currentPos.Z + deltaZ;

    shapeNode->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}
