//Parent class for own and other ships - not used un-extended

#include "irrlicht.h"

#include "Ship.hpp"

#include "Constants.hpp"
#include "SimulationModel.hpp"
#include "IniFile.hpp"

using namespace irr;

Ship::Ship()
{

}

Ship::~Ship()
{
    //dtor
}

irr::scene::IMeshSceneNode* Ship::getSceneNode() const
{
    return ship;
}

irr::core::vector3df Ship::getRotation() const
{
    return ship->getRotation();
}

irr::core::vector3df Ship::getPosition() const
{
    ship->updateAbsolutePosition();//ToDo: This may be needed, but seems odd that it's required
    return ship->getAbsolutePosition();
}

void Ship::setHeading(irr::f32 hdg)
{
    heading = hdg;
}

void Ship::setSpeed(irr::f32 spd)
{
    speed = spd;
}

irr::f32 Ship::getHeading() const
{
    return heading;
}

irr::f32 Ship::getSpeed() const
{
    return speed;
}

//////////////////////////
//Private member functions (Protected in superclass)
//////////////////////////

void Ship::setPosition(irr::core::vector3df position)
{
     ship->setPosition(position);
}

void Ship::setRotation(irr::core::vector3df rotation)
{
    ship->setRotation(rotation);
}

