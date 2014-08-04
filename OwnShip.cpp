//Extends from the general 'Ship' class
#include "OwnShip.hpp"

#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "IniFile.hpp"

using namespace irr;

void OwnShip::load(const std::string& scenarioName, irr::scene::ISceneManager* smgr, SimulationModel* model, Terrain* terrain)
{
    //Store reference to terrain
    this->terrain = terrain;

    //construct scenario ownship.ini filename
    std::string scenarioOwnShipFilename = scenarioName;
    scenarioOwnShipFilename.append("/ownship.ini");

    //Load from ownShip.ini file
    std::string ownShipName = IniFile::iniFileToString(scenarioOwnShipFilename,"ShipName");
    //Get initial position and heading, and set these
    spd = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialSpeed")*KTS_TO_MPS;
    xPos = model->longToX(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLong"));
    yPos = 0;
    zPos = model->latToZ(IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialLat"));
    hdg = IniFile::iniFileTof32(scenarioOwnShipFilename,"InitialBearing");

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
    irr::u32 numberOfViews = IniFile::iniFileTof32(shipIniFilename,"Views");
    if (numberOfViews==0) {
        //ToDo: Tell user that view positions couldn't be loaded
        exit(EXIT_FAILURE);
    }
    for(int i=1;i<=numberOfViews;i++) {
        f32 camOffsetX = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewX",i));
        f32 camOffsetY = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewY",i));
        f32 camOffsetZ = IniFile::iniFileTof32(shipIniFilename,IniFile::enumerate1("ViewZ",i));
        views.push_back(core::vector3df(scaleFactor*camOffsetX,scaleFactor*(camOffsetY+yCorrection),scaleFactor*camOffsetZ));
    }

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
        ship = smgr->addCubeSceneNode(0.1);
    } else {
        ship = smgr->addMeshSceneNode(
                        shipMesh,
                        0,
                        -1,
                        core::vector3df(0,0,0));
    }

    ship->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true); //Normalise normals on scaled meshes, for correct lighting

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(ship->getMaterialCount()>0) {
        for(int mat=0;mat<ship->getMaterialCount();mat++) {
            ship->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }
}

void OwnShip::update(irr::f32 deltaTime, irr::f32 tideHeight)
{
    //move, according to heading and speed
    xPos = xPos + sin(hdg*core::DEGTORAD)*spd*deltaTime;
    zPos = zPos + cos(hdg*core::DEGTORAD)*spd*deltaTime;
    yPos = tideHeight;

    //Set position & speed by calling own ship methods
    setPosition(core::vector3df(xPos,yPos,zPos));
    setRotation(core::vector3df(0, hdg, 0)); //Global vectors

}

irr::f32 OwnShip::getDepth()
{
    return -1*terrain->getHeight(xPos,zPos)+getPosition().Y;
}

std::vector<irr::core::vector3df> OwnShip::getCameraViews() const
{
    return views;
}


