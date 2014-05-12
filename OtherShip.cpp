//FIXME: Lots of duplication between OwnShip and OtherShip -> should extend from a single base class

#include "irrlicht.h"

#include "Constants.hpp"

#include "OtherShip.hpp"

using namespace irr;

OtherShip::OtherShip(const irr::io::path& filename, const irr::core::vector3df& location, const irr::f32 scaleFactor, const irr::f32 yCorrection, std::vector<Leg> legsLoaded, irr::scene::ISceneManager* smgr)
{
    //FIXME: Use similar code for ownship loading (could extend from this?)
    scene::IMesh* shipMesh = smgr->getMesh(filename);

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

    //store leg information
    legs=legsLoaded;
}

OtherShip::~OtherShip()
{
    //dtor
}

void OtherShip::update(irr::f32 deltaTime, irr::f32 scenarioTime)
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
