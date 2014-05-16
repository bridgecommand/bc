//FIXME: Lots of duplication between OwnShip and OtherShip -> should extend from a single base class

#include "irrlicht.h"

#include "IniFile.hpp"
#include "Constants.hpp"
#include "OtherShip.hpp"

using namespace irr;

OtherShip::OtherShip (const std::string& name,const irr::core::vector3df& location, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr)
{

    //Load from individual boat.ini file
    std::string iniFilename = "Models/Othership/";
    iniFilename.append(name);
    iniFilename.append("/boat.ini");

    //load information about this model from its ini file
    std::string shipFileName = IniFile::iniFileToString(iniFilename,"FileName");

    //get scale factor from ini file (or zero if not set - assume 1)
    f32 scaleFactor = IniFile::iniFileTof32(iniFilename,"Scalefactor");
    if (scaleFactor==0.0) {
        scaleFactor = 1.0; //Default if not set
    }

    f32 yCorrection = IniFile::iniFileTof32(iniFilename,"YCorrection");

    std::string shipFullPath = "Models/Othership/"; //FIXME: Use proper path handling
    shipFullPath.append(name);
    shipFullPath.append("/");
    shipFullPath.append(shipFileName);

    //get light locations:

    //load mesh
    scene::IMesh* shipMesh = smgr->getMesh(shipFullPath.c_str());

    //scale and translate
    core::matrix4 transformMatrix;
    transformMatrix.setScale(core::vector3df(scaleFactor,scaleFactor,scaleFactor));
    transformMatrix.setTranslation(core::vector3df(0,yCorrection*scaleFactor,0));
    smgr->getMeshManipulator()->transform(shipMesh,transformMatrix);

    //add to scene node
	otherShip = smgr->addMeshSceneNode( shipMesh, 0, -1);

    //store initial x,y,z positions
    xPos = location.X;
    yPos = location.Y;
    zPos = location.Z;
    //speed and heading will come from leg data

    //Set lighting to use diffuse and ambient, so lighting of untextured models works
	if(otherShip->getMaterialCount()>0) {
        for(int mat=0;mat<otherShip->getMaterialCount();mat++) {
            otherShip->getMaterial(mat).ColorMaterial = video::ECM_DIFFUSE_AND_AMBIENT;
        }
    }

    u32 numberOfLights = IniFile::iniFileTou32(iniFilename,"NumberOfLights");
    if (numberOfLights>0) {
        for (u32 currentLight=1; currentLight<=numberOfLights; currentLight++) {
            f32 lightX = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightX",currentLight));
            f32 lightY = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightY",currentLight));
            f32 lightZ = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightZ",currentLight));

            u32 lightR = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightRed",currentLight));
            u32 lightG = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightGreen",currentLight));
            u32 lightB = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightBlue",currentLight));

            f32 lightStartAngle = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightStartAngle",currentLight)); //Degrees 0-360
            f32 lightEndAngle = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightEndAngle",currentLight)); //Degrees 0-720, should be greater than LightStartAngle
            f32 lightRange = IniFile::iniFileTof32(iniFilename,IniFile::enumerate1("LightRange",currentLight)); //Range (Nm)
            lightRange = lightRange * M_IN_NM; //Convert to metres

            //correct to local scaled coordinates
            lightX *= scaleFactor;
            lightY = (lightY+yCorrection)*scaleFactor;
            lightZ *= scaleFactor;

            //add this Nav light into array
            navLights.push_back(NavLight (otherShip,smgr,core::dimension2d<f32>(5, 5), core::vector3df(lightX,lightY,lightZ),video::SColor(255,lightR,lightG,lightB),lightStartAngle,lightEndAngle,lightRange));
        }
    }

    //store leg information
    legs=legsLoaded;
}

OtherShip::~OtherShip()
{
    //dtor
}

void OtherShip::update(irr::f32 deltaTime, irr::f32 scenarioTime, irr::core::vector3df viewPosition)
{

    //move according to leg information
    if (legs.empty()) {
        speed = 0;
        heading = 0;
    } else {
        //Work out which leg we're on
        int currentLeg; //Fixme,should probably be a size_type
        for(currentLeg = 0; currentLeg<legs.size()-1; currentLeg++) {
            if (legs[currentLeg].startTime <=scenarioTime && legs[currentLeg+1].startTime > scenarioTime ) {
                break;
            }
        }
        speed = legs[currentLeg].speed*KTS_TO_MPS;
        heading = legs[currentLeg].bearing;
    }

    xPos = xPos + sin(heading*core::DEGTORAD)*speed*deltaTime;
    zPos = zPos + cos(heading*core::DEGTORAD)*speed*deltaTime;

    //Set position & speed by calling ship methods
    setPosition(core::vector3df(xPos,yPos,zPos));
    setRotation(core::vector3df(0, heading, 0)); //Global vectors

    //for each light, find range and angle
    for(int currentLight = 0; currentLight<navLights.size(); currentLight++) {
        navLights[currentLight].update(scenarioTime,viewPosition);
    }

}

void OtherShip::setHeading(irr::f32 hdg)
{
    heading = hdg;
}

void OtherShip::setSpeed(irr::f32 spd)
{
    speed = spd;
}

irr::f32 OtherShip::getHeading() const
{
    return heading;
}

irr::f32 OtherShip::getSpeed() const
{
    return speed;
}

void OtherShip::setPosition(irr::core::vector3df position) //FIXME: Are these needed at all
{
     otherShip->setPosition(position);
}

void OtherShip::setRotation(irr::core::vector3df rotation) //FIXME: Are these needed at all
{
    otherShip->setRotation(rotation);
}
