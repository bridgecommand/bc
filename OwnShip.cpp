#include "irrlicht.h"
#include <string>
#include <iostream>

#include "OwnShip.hpp"

#include "SimulationModel.hpp"
#include "IniFile.hpp"

using namespace irr;

OwnShip::OwnShip()
{

}

OwnShip::~OwnShip()
{
    //dtor
}

void OwnShip::loadModel(const std::string& scenarioOwnShipFilename, const irr::core::vector3df& location, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //Load from ownShip.ini file

     std::string ownShipName = IniFile::iniFileToString(scenarioOwnShipFilename,"ShipName");

    //Load from boat.ini file if it exists
    std::string shipIniFilename = "Models/OwnShip/";
    shipIniFilename.append(ownShipName);
    shipIniFilename.append("/boat.ini");

    //Do stuff with the boat.ini information here
    std::cout <<shipIniFilename <<std::endl;

    //get the model file
    std::string ownShipFileName = IniFile::iniFileToString(shipIniFilename,"FileName");
    std::string ownShipFullPath = "Models/OwnShip/"; //FIXME: Use proper path handling
                ownShipFullPath.append(ownShipName);
                ownShipFullPath.append("/");
                ownShipFullPath.append(ownShipFileName);

    //Scale
    f32 scaleFactor = IniFile::iniFileTof32(shipIniFilename,"ScaleFactor");
    f32 yCorrection = IniFile::iniFileTof32(shipIniFilename,"YCorrection");

    //camera offset (in unscaled and uncorrected ship coords)
    f32 camOffsetX = IniFile::iniFileTof32(shipIniFilename,"ViewX(1)"); //FIXME: Hardcoding for initial view only
    f32 camOffsetY = IniFile::iniFileTof32(shipIniFilename,"ViewY(1)");
    f32 camOffsetZ = IniFile::iniFileTof32(shipIniFilename,"ViewZ(1)");
    //camera offset in scaled and corrected coords
    cameraOffset = core::vector3df(scaleFactor*camOffsetX,scaleFactor*(camOffsetY+yCorrection),scaleFactor*camOffsetZ);

    //Load the model
    scene::IMesh* shipMesh = smgr->getMesh(ownShipFullPath.c_str());

    //Translate and scale mesh here
    core::matrix4 transformMatrix;
    transformMatrix.setScale(core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    transformMatrix.setTranslation(core::vector3df(0,yCorrection*scaleFactor,0));
    smgr->getMeshManipulator()->transform(shipMesh,transformMatrix);

    //Make mesh scene node
    ownShip = smgr->addMeshSceneNode(
                        shipMesh,
                        0,
                        -1,
                        location);
    ownShip->setMaterialFlag(video::EMF_LIGHTING, false);

    //Set position (x,y initial position)

    //set view positions, and send to model (so they can be used by the camera)

}

irr::scene::IMeshSceneNode* OwnShip::getSceneNode()
{
    return ownShip;
}

void OwnShip::setPosition(irr::core::vector3df position)
{
     ownShip->setPosition(position);
}

void OwnShip::setRotation(irr::core::vector3df rotation)
{
    ownShip->setRotation(rotation);
}

irr::core::vector3df OwnShip::getRotation()
{
    return ownShip->getRotation();
}

irr::core::vector3df OwnShip::getPosition()
{
    return ownShip->getPosition();
}

irr::core::vector3df OwnShip::getCameraOffset()
{
    return cameraOffset;
}
