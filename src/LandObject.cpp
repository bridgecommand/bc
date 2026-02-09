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

#include "irrlicht.h"

#include "IniFile.hpp"
#include "Utilities.hpp"
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

LandObject::LandObject(const std::string& name, const std::string& internalName, const std::string& worldName, const bc::graphics::Vec3& location, float rotation, bool collisionObject, bool radarObject, bool morph, Terrain* terrain, irr::scene::ISceneManager* smgr, irr::IrrlichtDevice* dev)
{

    const float tallHeightRatio = 5.0;  // Minimum ratio of height to max of width, length for an object to be considered 'tall'
    const float nearlyFlatHeight = 1.0; // Max height in model units for a 'flat' object
    
    device = dev;
    
    std::string basePath = "Models/LandObject/" + name + "/";
    std::string userFolder = Utilities::getUserDir();
    
    if (Utilities::pathExists(userFolder + basePath)) {
        //Read model from user dir if it exists there.
        basePath = userFolder + basePath;
    } else if (Utilities::pathExists(worldName + "/" + basePath)) {
        //Otherwise, if it exists in the world model folder, use this
        basePath = worldName + "/" + basePath;
    }

    //Load from individual object.ini file if it exists
    std::string objectIniFilename = basePath + "/object.ini";

    //get filename from ini file (or empty string if file doesn't exist)
    std::string objectFileName = IniFile::iniFileToString(objectIniFilename,"FileName", "object.x");

    //get scale factor from ini file (or zero if not set - assume 1)
    float objectScale = IniFile::iniFileTof32(objectIniFilename,"Scalefactor", 1.f);

    std::string objectFullPath = basePath + objectFileName;

    //Load the mesh
    irr::scene::IMesh* objectMesh = smgr->getMesh(objectFullPath.c_str());
	//add to scene node
	if (objectMesh==0) {
        //Failed to load mesh - load with dummy and continue
        dev->getLogger()->log("Failed to load land object model:");
        dev->getLogger()->log(objectFullPath.c_str());
        landObject = smgr->addCubeSceneNode(0.1, 0, -1, toIrrVec(location));
    } else {
        if (morph) {
            // Remove nearly flat mesh buffers
            irr::scene::SMesh* updatedMesh = new irr::scene::SMesh();
            irr::core::vector3df boundingBoxExtent = objectMesh->getBoundingBox().getExtent();
            for (int bufferId =  0; bufferId < objectMesh->getMeshBufferCount(); bufferId++) {
                irr::scene::IMeshBuffer* mb = objectMesh->getMeshBuffer(bufferId);
                if (boundingBoxExtent.Y > nearlyFlatHeight) {
                    updatedMesh->addMeshBuffer(mb);
                    updatedMesh->recalculateBoundingBox();
                }
            }
            // TODO for future: Use OctreeSceneNode, but probably need to modify mesh before creating Octree (currently modify after scene node is created)
            //landObject = smgr->addOctreeSceneNode(updatedMesh);
            //landObject->setPosition(location);
            landObject = smgr->addMeshSceneNode(updatedMesh, 0, -1, toIrrVec(location));
            updatedMesh->drop();
        } else {
            landObject = smgr->addMeshSceneNode(objectMesh, 0, -1, toIrrVec(location));
        }
    }

    //Set ID as a flag if we should model collisions with this, also used to get radar points
    if (collisionObject || radarObject) {
        landObject->setID(IDFlag_IsPickable);

        //Add a triangle selector
        irr::scene::ITriangleSelector* selector=smgr->createTriangleSelector(objectMesh,landObject);
        if(selector) {
            landObject->setTriangleSelector(selector);
        }
    }

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(landObject->getMaterialCount()>0) {
        for(uint32_t mat=0;mat<landObject->getMaterialCount();mat++) {
            landObject->getMaterial(mat).ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    landObject->setScale(irr::core::vector3df(objectScale,objectScale,objectScale));
    landObject->setRotation(irr::core::vector3df(0,rotation,0));
    landObject->setMaterialFlag(irr::video::EMF_FOG_ENABLE, true);
    landObject->setMaterialFlag(irr::video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    // Force recalculation of transformation matrix
    landObject->updateAbsolutePosition();

    // Morph
    if (morph) {
        irr::scene::IMesh* mesh = landObject->getMesh();
        for (int bufferId = 0; bufferId < mesh->getMeshBufferCount(); bufferId++) {
            irr::scene::IMeshBuffer* mb = mesh->getMeshBuffer(bufferId);
            irr::core::vector3df boundingBoxExtent = mb->getBoundingBox().getExtent();

            if (mb->getVertexType() == irr::video::EVT_STANDARD) {
                const uint32_t vtxCnt = mb->getVertexCount();
                irr::video::S3DVertex* v = (irr::video::S3DVertex*)mb->getVertices();
                for (int vertexId = 0; vertexId < vtxCnt; vertexId++) {
                    // Find global location for each vertex, using transformation matrix
                    // Local coordinate
                    irr::core::vector3df localCoord(v[vertexId].Pos.X, v[vertexId].Pos.Y, v[vertexId].Pos.Z);

                    // Bounding box coordinate
                    irr::core::vector3df localBoundingBoxCentre = mb->getBoundingBox().getCenter();

                    // Get transformation matrix and inverse
                    irr::core::matrix4 localToWorld = landObject->getAbsoluteTransformation();
                    irr::core::matrix4 worldToLocal;
                    localToWorld.getInverse(worldToLocal);

                    // Find world location for this local point
                    irr::core::vector3df worldCoord(v[vertexId].Pos.X, v[vertexId].Pos.Y, v[vertexId].Pos.Z);
                    localToWorld.transformVect(worldCoord);

                    // Find world location of this bounding box
                    irr::core::vector3df worldBoundingBoxCentre = localBoundingBoxCentre;
                    localToWorld.transformVect(worldBoundingBoxCentre);

                    // Find terrain height here
                    float terrainY;

                    // Choose if we modify vertex based on the terrain at the centre of the bounding box, or move locally
                    // Default to 'morph' locally, unless height is more than 5x the maximum of the width or length
                    bool fullMorph = true;
                    if (boundingBoxExtent.Y > tallHeightRatio * std::max(boundingBoxExtent.X, boundingBoxExtent.Z)) {
                        fullMorph = false;
                    }

                    if (fullMorph) {
                        // Local height
                        terrainY = terrain->getHeight(worldCoord.X, worldCoord.Z);
                    }
                    else {
                        // Bounding box centre height
                        terrainY = terrain->getHeight(worldBoundingBoxCentre.X, worldBoundingBoxCentre.Z);
                    }

                    // Modify world location of coordinate. Don't reduce height unless the meshbuffer is almost flat
                    // Note that we now remove nearly flat buffers, so this check may not be needed any more
                    if ((terrainY > 0) || (boundingBoxExtent.Y <= nearlyFlatHeight)) {
                        worldCoord.Y += terrainY;
                    }

                    // Transform back to model coordinate
                    irr::core::vector3df newLocalCoord = worldCoord;
                    worldToLocal.transformVect(newLocalCoord);

                    // Apply vertical correction
                    v[vertexId].Pos.Y = newLocalCoord.Y;
                }
            }
            mb->recalculateBoundingBox();
        }
        mesh->setDirty();
    }

    landObject->setName(internalName.c_str());

    //===========================================
    //Get contact points for radar detection here
    if (radarObject) {

        irr::core::aabbox3df boundingBox = landObject->getTransformedBoundingBox();
        float minX = boundingBox.MinEdge.X;
        float maxX = boundingBox.MaxEdge.X;
        float minY = boundingBox.MinEdge.Y;
        float maxY = boundingBox.MaxEdge.Y;
        float minZ = boundingBox.MinEdge.Z;
        float maxZ = boundingBox.MaxEdge.Z;

        //Grid from above looking down (hard coded 129x129 points)
        std::vector<std::vector<float>> generatedMap;
        for (int i = 0; i<129; i++) {
            std::vector<float> generatedMapLine;
            for (int j = 0; j<129; j++) {

                float xTestPos = minX + (maxX-minX)*(float)j/(float)(129-1);
                float zTestPos = minZ + (maxZ-minZ)*(float)i/(float)(129-1);

                //Check the ray and add the contact point if it exists
                float pointY = findContactYFromRay(
                    bc::graphics::Vec3(xTestPos, maxY+0.1f, zTestPos),
                    bc::graphics::Vec3(xTestPos, minY-0.1f, zTestPos));
                generatedMapLine.push_back(pointY);
            }
            generatedMap.push_back(generatedMapLine);
        }

        //use the 'generatedMap' to add an invisible dummy terrain here
        terrain->addRadarReflectingTerrain(generatedMap, minX, minZ, maxX-minX, maxZ-minZ);
    }
    //We don't want to do further triangle selection, unless it's a collision object
    if (!collisionObject) {
        landObject->setID(-1);
        landObject->setTriangleSelector(0);
    }
    //End contact points for radar detection
    //======================================

    //landObject->setDebugDataVisible(irr::scene::EDS_BBOX_ALL);

}

LandObject::~LandObject()
{
    //dtor
}

float LandObject::findContactYFromRay(const bc::graphics::Vec3& start, const bc::graphics::Vec3& end)
{
    irr::core::line3df ray(toIrrVec(start), toIrrVec(end));
    irr::core::vector3df intersection;
    irr::core::triangle3df hitTriangle;

    irr::scene::ISceneNode * selectedSceneNode =
        device->getSceneManager()->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(
        ray,
        intersection, // This will be the position of the collision
        hitTriangle, // This will be the triangle hit in the collision
        IDFlag_IsPickable, // (bitmask)
        0); // Check all nodes

    if(selectedSceneNode) {
        return intersection.Y;
    } else {
        return -1e3; //A big negative value
    }
}

bc::graphics::Vec3 LandObject::getPosition() const
{
    landObject->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return fromIrrVec(landObject->getAbsolutePosition());
}

void LandObject::moveNode(float deltaX, float deltaY, float deltaZ)
{
    irr::core::vector3df currentPos = landObject->getPosition();
    float newPosX = currentPos.X + deltaX;
    float newPosY = currentPos.Y + deltaY;
    float newPosZ = currentPos.Z + deltaZ;

    landObject->setPosition(irr::core::vector3df(newPosX,newPosY,newPosZ));
}

irr::scene::ISceneNode* LandObject::getSceneNode() const
{
    return (irr::scene::ISceneNode*)landObject;
}
