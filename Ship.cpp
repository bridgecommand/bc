//Parent class for own and other ships - not used un-extended
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
    this->hdg = hdg;
}

void Ship::setSpeed(irr::f32 spd)
{
    this->spd = spd;
}

irr::f32 Ship::getHeading() const
{
    return hdg;
}

irr::f32 Ship::getSpeed() const
{
    return spd;
}

void Ship::moveNode(irr::f32 deltaX, irr::f32 deltaY, irr::f32 deltaZ)
{
    xPos += deltaX;
    yPos += deltaY;
    zPos += deltaZ;
    setPosition(core::vector3df(xPos,yPos,zPos));
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

