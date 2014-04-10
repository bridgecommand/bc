#include "irrlicht.h"

#include "OwnShip.hpp"

using namespace irr;

OwnShip::OwnShip()
{

}

OwnShip::~OwnShip()
{
    //dtor
}

void OwnShip::loadModel(const irr::io::path& filename, const irr::core::vector3df& location, irr::scene::ISceneManager* smgr)
{
    scene::IMesh* shipMesh = smgr->getMesh(filename);
    ownShip = smgr->addMeshSceneNode(shipMesh,0,-1,location);
    ownShip->setMaterialFlag(video::EMF_LIGHTING, false);
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
