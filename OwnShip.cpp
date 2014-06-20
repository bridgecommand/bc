#include "irrlicht.h"

#include "OwnShip.hpp"

#include "Constants.hpp"
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

void OwnShip::load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model)
{

    //construct scenario ownship.ini filename
    std::string scenarioOwnShipFilename = scenarioName;
    scenarioOwnShipFilename.append("/ownship.ini");

    //Load from ownShip.ini file
    std::string ownShipName = IniFile::iniFileToString(scenarioOwnShipFilename,"ShipName");
    //Get initial position and heading, and set these
    speed = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialSpeed")*KTS_TO_MPS;
    xPos = model->longToX(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLong"));
    yPos = 0;
    zPos = model->latToZ(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLat"));
    heading = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialBearing");

    //Load from boat.ini file if it exists
    std::string shipIniFilename = "Models/OwnShip/";
    shipIniFilename.append(ownShipName);
    shipIniFilename.append("/boat.ini");

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
    if (shipMesh==0) {
        //Failed to load mesh - load with dummy and continue - ToDo: should also flag this up to user
        ownShip = smgr->addCubeSceneNode(0.1);
    } else {
        ownShip = smgr->addMeshSceneNode(
                        shipMesh,
                        0,
                        -1,
                        core::vector3df(0,0,0));
    }

    ownShip->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(ownShip->getMaterialCount()>0) {
        for(int mat=0;mat<ownShip->getMaterialCount();mat++) {
            ownShip->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }
}

void OwnShip::update(irr::f32 deltaTime)
{
    //move, according to heading and speed
    xPos = xPos + sin(heading*core::DEGTORAD)*speed*deltaTime;
    zPos = zPos + cos(heading*core::DEGTORAD)*speed*deltaTime;

    //Set position & speed by calling own ship methods
    setPosition(core::vector3df(xPos,yPos,zPos));
    setRotation(core::vector3df(0, heading, 0)); //Global vectors
}

irr::scene::IMeshSceneNode* OwnShip::getSceneNode() const
{
    return ownShip;
}

irr::core::vector3df OwnShip::getRotation() const
{
    return ownShip->getRotation();
}

irr::core::vector3df OwnShip::getPosition() const
{
    return ownShip->getPosition();
}

irr::core::vector3df OwnShip::getCameraOffset() const
{
    return cameraOffset;
}

void OwnShip::setHeading(irr::f32 hdg)
{
    heading = hdg;
}

void OwnShip::setSpeed(irr::f32 spd)
{
    speed = spd;
}

irr::f32 OwnShip::getHeading() const
{
    return heading;
}

irr::f32 OwnShip::getSpeed() const
{
    return speed;
}

//////////////////////////
//Private member functions
//////////////////////////

void OwnShip::setPosition(irr::core::vector3df position)
{
     ownShip->setPosition(position);
}

void OwnShip::setRotation(irr::core::vector3df rotation)
{
    ownShip->setRotation(rotation);
}
