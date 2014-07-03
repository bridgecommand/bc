#include "irrlicht.h"

#include "LandObject.hpp"
#include "IniFile.hpp"

using namespace irr;

LandObject::LandObject(const std::string& name, const irr::core::vector3df& location, irr::f32 rotation, irr::scene::ISceneManager* smgr)
{

    //Load from individual object.ini file if it exists
    std::string objectIniFilename = "Models/LandObject/";
    objectIniFilename.append(name);
    objectIniFilename.append("/object.ini");

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

    std::string objectFullPath = "Models/LandObject/"; //FIXME: Use proper path handling
    objectFullPath.append(name);
    objectFullPath.append("/");
    objectFullPath.append(objectFileName);

    //Load the mesh
    scene::IMesh* objectMesh = smgr->getMesh(objectFullPath.c_str());
	//add to scene node
	if (objectMesh==0) {
        //Failed to load mesh - load with dummy and continue - ToDo: should also flag this up to user
        landObject = smgr->addCubeSceneNode(0.1);
    } else {
        landObject = smgr->addMeshSceneNode( objectMesh, 0, -1, location );
    }

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(landObject->getMaterialCount()>0) {
        for(int mat=0;mat<landObject->getMaterialCount();mat++) {
            landObject->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    landObject->setScale(core::vector3df(objectScale,objectScale,objectScale));
    landObject->setRotation(irr::core::vector3df(0,rotation,0));
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
